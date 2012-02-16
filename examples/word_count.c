/*
 * AUTHOR:  René Kijewski  (rene.<surname>@fu-berlin.de)
 * LICENSE: MIT
 */

// Compile:
// gcc -O3 word_count.c ../aa_tree.c

#include "../aa_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct entry
{
  struct aa_elem elem1, elem2;
  char line[128];
  int counter, length;
};

static bool
read_entry (FILE *input, struct entry *entry)
{
  if (fscanf (input, "%128s%n", entry->line, &entry->length) == EOF)
    return false;
  entry->counter = 1;
  return true;
}

static signed
cmp1 (struct aa_elem *a, struct aa_elem *b, struct aa_tree *tree)
{
  (void) tree;
  struct entry *left = aa_entry (a, struct entry, elem1);
  struct entry *right = aa_entry (b, struct entry, elem1);
  if (left->length != right->length)
    return left->length - right->length;
  return memcmp (left->line, right->line, left->length);
}

static signed
cmp2 (struct aa_elem *a, struct aa_elem *b, struct aa_tree *tree)
{
  (void) tree;
  struct entry *left = aa_entry (a, struct entry, elem2);
  struct entry *right = aa_entry (b, struct entry, elem2);
  if (left->counter != right->counter)
    return left->counter - right->counter;
  return cmp1 (&left->elem1, &right->elem1, NULL);
}

static bool
insert_into_2 (struct aa_elem *elem, void *tree2_)
{
  struct entry *ee = aa_entry (elem, struct entry, elem1);
  aa_insert ((struct aa_tree *) tree2_, &ee->elem2);
  return true;
}

static bool
print_entry (struct aa_elem *elem, void *null)
{
  (void) null;
  struct entry *ee = aa_entry (elem, struct entry, elem2);
  printf ("%.5d %s\n", ee->counter, ee->line);
  return true;
}

int
main (int argc, char **argv)
{
  FILE *input;
  if (argc > 1)
    input = fopen (argv[1], "r");
  else
    input = stdin;

  struct aa_tree tree1, tree2;
  aa_init (&tree1, cmp1);
  aa_init (&tree2, cmp2);

  struct entry *entry = malloc (sizeof (*entry));
  while (read_entry (input, entry))
    {
      struct aa_elem *dupl = aa_insert (&tree1, &entry->elem1);
      if (!dupl)
        entry = malloc (sizeof (*entry));
      else
        aa_entry (dupl, struct entry, elem1)->counter++;
    }

  aa_foreach (&tree1, insert_into_2, &tree2);
  aa_foreach (&tree2, print_entry, NULL);
  return 0;
}
