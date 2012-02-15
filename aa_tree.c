/*
 * AUTHOR:  2012  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT  (http://www.opensource.org/licenses/mit-license.php)
 */

#include "aa_tree.h"
#include <stdbool.h>

#define NIL ((struct aa_elem *) &nil)

static const struct aa_elem nil = { NIL, NIL, 0 };

static struct aa_elem *
skew_ (struct aa_elem *t)
{
  if (t->left->level != t->level)
    return t;
  struct aa_elem *l = t->left;
  t->left = l->right;
  l->right = t;
  return l;
}

#define skew(X) \
  if ((X) != NIL) \
    (X) = skew_ ((X));

static struct aa_elem *
split_ (struct aa_elem *t)
{
  if (t->level != t->right->right->level)
    return t;
  struct aa_elem *r = t->right;
  t->right = r->left;
  r->left = t;
  ++r->level;
  return r;
}

#define split(X) \
  if ((X) != NIL) \
    (X) = split_ ((X));

void
aa_init (struct aa_tree *tree, aa_cmp cmp_fun)
{
  tree->root = NIL;
  tree->cmp_fun = cmp_fun;
}

static struct aa_elem *
aa_insert_ (struct aa_elem  *node,
            struct aa_elem  *datum,
            struct aa_tree  *tree,
            struct aa_elem **result)
{
  if (node == NIL)
    {
      *result = NULL;
      datum->level = 1;
      datum->left = datum->right = NIL;
      return datum;
    }

  int cmp_result = tree->cmp_fun (datum, node, tree);
  if (cmp_result < 0)
    {
      struct aa_elem *t = aa_insert_ (node->left, datum, tree, result);
      if (!t)
        return NULL;
      node->left = t;
    }
  else if (cmp_result > 0)
    {
      struct aa_elem *t = aa_insert_ (node->right, datum, tree, result);
      if (!t)
        return NULL;
      node->right = t;
    }
  else
    {
      *result = node;
      return NULL;
    }

  skew (node);
  split (node);
  return node;
}

struct aa_elem *
aa_insert (struct aa_tree *tree, struct aa_elem *datum)
{
  struct aa_elem *result;
  struct aa_elem *t = aa_insert_ (tree->root, datum, tree, &result);
  if (t)
    tree->root = t;
  return result;
}

struct aa_elem *
aa_find (struct aa_tree *tree, struct aa_elem *datum)
{
  struct aa_elem *node = tree->root;
  while (node != NIL)
    {
      int cmp_result = tree->cmp_fun (datum, node, tree);
      if (cmp_result < 0)
        node = node->left;
      else if (cmp_result > 0)
        node = node->right;
      else
        return node;
    }
  return NULL;
}

static struct aa_elem *
aa_remove_ (struct aa_elem  *t,
            struct aa_elem  *datum,
            struct aa_elem **deletee,
            struct aa_tree  *tree,
            struct aa_elem **result)
{
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

  bool is_bottom;
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
          struct aa_elem *v = aa_remove_ (t->right, datum, deletee, tree,
                                          result);
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

      struct aa_elem *v = t->right;
      t->left = (**deletee).left;
      t->right = (**deletee).right;
      t->level = (**deletee).level;
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

struct aa_elem *
aa_remove (struct aa_tree *tree, struct aa_elem *datum)
{
  if (tree->root == NIL)
    return NULL;

  struct aa_elem *deletee = NULL, *result = NULL;
  tree->root = aa_remove_ (tree->root, datum, &deletee, tree, &result);
  return result;
}

static bool
aa_foreach_ (struct aa_elem *node, aa_action action, void *aux)
{
  return node == NIL ||
         (aa_foreach_ (node->left, action, aux) &&
          action (node, aux) &&
          aa_foreach_ (node->right, action, aux));
}

void
aa_foreach (struct aa_tree *tree, aa_action action, void *aux)
{
  aa_foreach_ (tree->root, action, aux);
}

static size_t
aa_depth_ (const struct aa_elem *t, size_t max, size_t depth)
{
  if (t == NIL)
    return max >= depth ? max : depth;
  max = aa_depth_ (t->left, max, depth+1);
  max = aa_depth_ (t->right, max, depth+1);
  return max;
}

size_t
aa_depth (const struct aa_tree *tree)
{
  return aa_depth_ (tree->root, 0, 0);
}

static size_t
aa_size_ (const struct aa_elem *t, size_t size)
{
  if (t == NIL)
    return size;
  size = aa_size_ (t->left, size+1);
  return aa_size_ (t->right, size);
}

size_t
aa_size (const struct aa_tree *tree)
{
  return aa_size_ (tree->root, 0);
}
