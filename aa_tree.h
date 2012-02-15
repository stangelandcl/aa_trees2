#ifndef AA_TREE_H__
#define AA_TREE_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct aa_elem
{
  struct aa_elem *left;
  struct aa_elem *right;
  unsigned        level;
};

struct aa_tree;

typedef signed (*aa_cmp) (struct aa_elem *a,
                          struct aa_elem *b,
                          struct aa_tree *tree);

struct aa_tree
{
  struct aa_elem *root;
  aa_cmp          cmp_fun;
};

void aa_init (struct aa_tree *tree, aa_cmp cmp_fun);

// time: O(log n), stack space: O(log n)

struct aa_elem *aa_insert (struct aa_tree *tree, struct aa_elem *e);
struct aa_elem *aa_remove (struct aa_tree *tree, struct aa_elem *e);

// time: O(log n), stack space: O(1)

struct aa_elem *aa_find (struct aa_tree *tree, struct aa_elem *e);

// time: O(n), stack space: O(log n)

size_t aa_depth (const struct aa_tree *tree);
size_t aa_size (const struct aa_tree *tree);

typedef bool (*aa_action) (struct aa_elem *e, void *aux);
void aa_foreach (struct aa_tree *tree, aa_action action, void *aux);

static inline bool
aa_is_empty (const struct aa_tree *tree)
{
  return tree->root->level == 0;
}

#define aa_entry(PTR, TYPE, ELEM) \
    ((TYPE *) ((uintptr_t) (PTR) - offsetof (TYPE, ELEM)))

#endif
