/*
 * AUTHOR:  2012 René Kijewski  (rene.<surname>@fu-berlin.de)
            2017 Clayton Stangeland
 * LICENSE: MIT  (http://www.opensource.org/licenses/mit-license.php)
 */

#ifndef AA_TREE_H__
#define AA_TREE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct aa_elem aa_elem;
typedef struct aa_iter aa_iter;
typedef struct aa_tree aa_tree;

typedef enum aa_op
{
    aa_op_equal,
    aa_op_less,
    aa_op_less_or_equal,
    aa_op_greater,
    aa_op_greater_or_equal
} aa_op;


struct aa_elem
{
  aa_elem *left;
  aa_elem *right;
  unsigned        level;
};

typedef signed (*aa_cmp) (aa_elem *a, aa_elem *b,aa_tree *tree);

struct aa_iter
{
    aa_elem* nodes[64];
    size_t count;
    aa_elem* current;
    aa_tree* tree;
    aa_cmp cmp;
};

struct aa_tree
{
    aa_elem *root;
    aa_cmp cmp_fun;
    void* user_data;
};

void aa_init (aa_tree *tree, aa_cmp cmp_fun, void* user_data);
/* set tree to empty. Doesn't cleanup node, do that before calling this */
void aa_clear(aa_tree* tree);


/* time: O(log n), stack space: O(log n) */

enum aa_insert_mode
{
  AA_INSERT_NEW,
  AA_REPLACE_ONLY,
  AA_INSERT_OR_REPLACE
};

aa_elem *aa_insert2 (aa_tree      *tree,
                            aa_elem      *e,
                            enum aa_insert_mode  mode);

static aa_elem *
aa_insert (aa_tree *tree, aa_elem *e)
{
  return aa_insert2 (tree, e, AA_INSERT_NEW);
}

static aa_elem *
aa_replace (aa_tree *tree, aa_elem *e)
{
  return aa_insert2 (tree, e, AA_REPLACE_ONLY);
}

static aa_elem *
aa_insert_or_replace (aa_tree *tree, aa_elem *e)
{
  return aa_insert2 (tree, e, AA_INSERT_OR_REPLACE);
}

aa_elem *aa_remove (aa_tree *tree, aa_elem *e);

/* time: O(log n), stack space: O(1) */

aa_elem *aa_find (aa_tree *tree, aa_elem *e);
bool aa_find_iter (aa_tree *tree, aa_elem *datum,
                   aa_cmp cmp, aa_op op, aa_iter* iter);


/* time: O(n), stack space: O(log n) */

size_t aa_depth (const aa_tree *tree);
size_t aa_size (const aa_tree *tree);


typedef bool (*aa_action) (aa_elem *e, void *aux);
void aa_foreach (aa_tree *tree, aa_action action, void *aux);
bool aa_first(aa_tree* tree, aa_iter* iter);
bool aa_last(aa_tree* tree, aa_iter* iter);
bool aa_next(aa_iter* iter);
bool aa_previous(aa_iter* iter);
bool aa_iter_has_data(aa_iter* iter);

static bool
aa_is_empty (const aa_tree *tree)
{
  return tree->root->level == 0;
}

#define aa_entry(PTR, TYPE, ELEM) \
    ((TYPE *) ((uintptr_t) (PTR) - offsetof (TYPE, ELEM)))

#endif
