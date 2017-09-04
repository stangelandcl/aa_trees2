/*
 * AUTHOR:  2012 René Kijewski  (rene.<surname>@fu-berlin.de)
 *          2017 Clayton Stangeland
 * LICENSE: MIT  (http://www.opensource.org/licenses/mit-license.php)
 */

#include "aa_tree.h"
#include <stdbool.h>
#include <stdio.h>

#define NIL ((aa_elem *) &nil)

static const aa_elem nil = { NIL, NIL, 0 };

#if 0
typedef struct Elem
{
    aa_elem base;
    int i;
} Elem;

#define E (((Elem*)iter->current)->i)

static void Print(aa_elem* e)
{
    printf("elem=%d\n", ((Elem*)e)->i);
}
#endif

static aa_elem *
skew_ (aa_elem *t)
{
    aa_elem *l;
    if (t->left->level != t->level)
        return t;
    l = t->left;
    t->left = l->right;
    l->right = t;
    return l;
}

#define skew(X)                                 \
    if ((X) != NIL)                             \
        (X) = skew_ ((X));

static aa_elem *
split_ (aa_elem *t)
{
    aa_elem *r;
    if (t->level != t->right->right->level)
        return t;
    r = t->right;
    t->right = r->left;
    r->left = t;
    ++r->level;
    return r;
}

#define split(X)                                \
    if ((X) != NIL)                             \
        (X) = split_ ((X));

void
aa_init (aa_tree *tree, aa_cmp cmp_fun, void* user_data)
{
    tree->root = NIL;
    tree->cmp_fun = cmp_fun;
    tree->user_data = user_data;
}

void
aa_clear(aa_tree* tree)
{
    tree->root = NIL;
}

static aa_elem *
aa_insert_ (aa_elem       *node,
            aa_elem       *datum,
            enum aa_insert_mode   mode,
            aa_tree       *tree,
            aa_elem      **result)
{
    int cmp_result;
    aa_elem *t;
    if (node == NIL)
    {
        *result = NULL;
        if (mode == AA_REPLACE_ONLY)
            return NULL;
        datum->level = 1;
        datum->left = datum->right = NIL;
        return datum;
    }

    cmp_result = tree->cmp_fun (datum, node, tree);
    if (cmp_result < 0)
    {

        t = aa_insert_ (node->left, datum, mode, tree, result);
        if (!t)
            return NULL;
        node->left = t;
    }
    else if (cmp_result > 0)
    {
        t = aa_insert_ (node->right, datum, mode, tree, result);
        if (!t)
            return NULL;
        node->right = t;
    }
    else
    {
        *result = node;
        if (mode == AA_INSERT_NEW)
            return NULL;
        *datum = *node;
        return datum;
    }

    if (*result)
        return NULL;

    skew (node);
    split (node);
    return node;
}

aa_elem *
aa_insert2 (aa_tree      *tree,
            aa_elem      *datum,
            enum aa_insert_mode  mode)
{
    aa_elem *result;
    aa_elem *t = aa_insert_ (tree->root, datum, mode, tree, &result);
    if (t)
        tree->root = t;
    return result;
}

aa_elem *
aa_find (aa_tree *tree, aa_elem *datum)
{
  aa_elem *node = tree->root;
  int c;
  while (node != NIL)
  {
      c = tree->cmp_fun (datum, node, tree);
      if (c < 0)
          node = node->left;
      else if (c > 0)
          node = node->right;
      else
          return node;
  }
  return NULL;
}

bool aa_iter_has_data(aa_iter* iter)
{
    return iter->current != NIL;
}

bool
aa_find_iter (aa_tree *tree, aa_elem *datum,
              aa_cmp cmp, aa_op op, aa_iter* iter)
{
    int c;

    iter->tree = tree;
    iter->cmp = cmp;
    iter->count = 0;
    iter->current = tree->root;
    while (iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
#if 0
        printf("find_iter count=%d e=%d\n", (int)iter->count, E);
#endif
        c = cmp (datum, iter->current, tree);
        if (c < 0)
            iter->current = iter->current->left;
        else if (c > 0)
            iter->current = iter->current->right;
        else
        {
            --iter->count;
            switch(op)
            {
            case aa_op_equal:
            case aa_op_less_or_equal:
            case aa_op_greater_or_equal:
                return true;
            case aa_op_less:
                return aa_previous(iter);
            case aa_op_greater:
                return aa_next(iter);
            }
        }
    }

    if(iter->count)
    {
        iter->current = iter->nodes[--iter->count];

        switch(op)
        {
        case aa_op_equal: return false;
        case aa_op_less:
        case aa_op_less_or_equal:
            if(c > 0)
                return true;
            return aa_previous(iter);
        case aa_op_greater:
        case aa_op_greater_or_equal:
            if(c < 0)
                return true;
            return aa_next(iter);
        }
    }

    return false;
}

static aa_elem *
aa_remove_ (aa_elem  *t,
            aa_elem  *datum,
            aa_elem **deletee,
            aa_tree  *tree,
            aa_elem **result)
{
    bool is_bottom;
    aa_elem *v;
    int cmp_result = !*deletee ? tree->cmp_fun (datum, t, tree) : -1;
    if (cmp_result == 0)
    {
        *result = t;
        if (t->right == NIL)
        {
          *deletee = NIL;
          return t->left;
        }
        *deletee = t;
    }


    if (cmp_result < 0)
    {
        is_bottom = t->left == NIL;
        if (!is_bottom)
            t->left = aa_remove_ (t->left, datum, deletee, tree, result);
    }
    else
    {
        is_bottom = t->right == NIL;
        if (!is_bottom)
        {
            v = aa_remove_ (t->right, datum, deletee, tree, result);
            if (cmp_result == 0)
            {
                t = *deletee;
                *deletee = NIL;
            }
            t->right = v;
        }
    }

    if (is_bottom)
    {
        if (!*deletee)
            return t;

        v = t->right;
        *t = **deletee;
        *deletee = t;
        return v;
    }

    if (t->left ->level >= t->level -1 &&
        t->right->level >= t->level -1)
        return t;

    --t->level;
    if (t->right->level > t->level)
        t->right->level = t->level;
    skew (t);
    skew (t->right);
    skew (t->right->right);
    split (t);
    split (t->right);
    return t;
}

aa_elem *
aa_remove (aa_tree *tree, aa_elem *datum)
{
    aa_elem *deletee, *result;
    if (tree->root == NIL)
        return NULL;

    deletee = NULL;
    result = NULL;
    tree->root = aa_remove_ (tree->root, datum, &deletee, tree, &result);
    return result;
}

static bool
aa_foreach_ (aa_elem *node, aa_action action, void *aux)
{
    aa_elem *right;

    if(node == NIL) return true;

    right = node->right;

    return (aa_foreach_ (node->left, action, aux) &&
            action (node, aux) &&
            aa_foreach_ (right, action, aux));
}

void
aa_foreach (aa_tree *tree, aa_action action, void *aux)
{
    aa_foreach_ (tree->root, action, aux);
}


bool
aa_next(aa_iter* iter)
{
    int c;
    aa_elem *old;
    if(iter->current == NIL)
    {
        if(!iter->count)
            return false;

        iter->current = iter->nodes[--iter->count];
        return true;
    }

    old = iter->current;
    iter->current = iter->current->right;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->left;
    }

    while(iter->count)
    {
        iter->current = iter->nodes[--iter->count];
        c = iter->cmp(old, iter->current, iter->tree);
        if(c < 0)
            return true;
    }

    iter->current = iter->current->left;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->right;
    }
    return iter->current != NIL;
}

bool
aa_previous(aa_iter* iter)
{
    int c;
    aa_elem *old;
    if(iter->current == NIL)
    {
        if(!iter->count)
            return false;

        iter->current = iter->nodes[--iter->count];
        return true;
    }

#if 0
    printf("previous count=%d e=%d\n", (int)iter->count, E);
#endif
    old = iter->current;
    iter->current = iter->current->left;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->right;
    }

    while(iter->count)
    {
        iter->current = iter->nodes[--iter->count];
        c = iter->cmp(old, iter->current, iter->tree);
        if(c > 0)
            return true;
    }

    iter->current = iter->current->left;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->right;
    }

    return iter->current != NIL;
}

bool
aa_first(aa_tree* tree, aa_iter* iter)
{
    iter->tree = tree;
    iter->cmp = tree->cmp_fun;
    iter->count = 0;
    iter->current = tree->root;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->left;
    }

    return aa_next(iter);
}

bool
aa_last(aa_tree* tree, aa_iter* iter)
{
    iter->tree = tree;
    iter->cmp = tree->cmp_fun;
    iter->count = 0;
    iter->current = tree->root;
    while(iter->current != NIL)
    {
        iter->nodes[iter->count++] = iter->current;
        iter->current = iter->current->right;
    }

    return aa_previous(iter);
}

static size_t
aa_depth_ (const aa_elem *t, size_t max, size_t depth)
{
    if (t == NIL)
        return max >= depth ? max : depth;
    max = aa_depth_ (t->left, max, depth+1);
    max = aa_depth_ (t->right, max, depth+1);
    return max;
}

size_t
aa_depth (const aa_tree *tree)
{
    return aa_depth_ (tree->root, 0, 0);
}

static size_t
aa_size_ (const aa_elem *t, size_t size)
{
    if (t == NIL)
        return size;
    size = aa_size_ (t->left, size+1);
    return aa_size_ (t->right, size);
}

size_t
aa_size (const aa_tree *tree)
{
    return aa_size_ (tree->root, 0);
}
