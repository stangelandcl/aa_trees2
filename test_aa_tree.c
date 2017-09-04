#include "aa_tree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Elem
{
    aa_elem base;
    int i;
} Elem;

static int cmp(aa_elem* a, aa_elem* b, aa_tree* tree)
{
    Elem* x = (Elem*)a;
    Elem* y = (Elem*)b;

    return (x->i > y->i) - (x->i < y->i);
}

Elem* New(int i)
{
    Elem* x = malloc(sizeof(Elem));
    x->i = i;
    return x;
}

#define C ((Elem*)iter.current)

static bool PrintElement(aa_elem* e, void* aux)
{
    Elem* x = (Elem*)e;
    printf("element=%d\n", x->i);
    return true;
}

static void PrintTree(aa_tree* tree)
{
    aa_foreach(tree, PrintElement, NULL);
}

int main(int argc, char** argv)
{
    aa_tree tree;
    int i;
    Elem* x;
    Elem y;
    aa_iter iter;
    aa_init(&tree, cmp, NULL);

    /* 0,2,4,6,8 */
    for(i=0;i<10;i+=2)
    {
        x = New(i);
        aa_insert(&tree, &x->base);
    }

#if 0
    PrintTree(&tree);
#endif
    assert(aa_first(&tree, &iter));
    assert(iter.current);
    assert(C->i == 0);

    for(i=2;i<10;i+=2)
    {
        assert(aa_next(&iter));
        assert(iter.current);
#if 0
        printf("i=%d C->i=%d\n", i, C->i);
#endif
        assert(C->i == i);
    }

    assert(!aa_next(&iter));

    assert(aa_last(&tree, &iter));
    assert(iter.current);
    assert(C->i == 8);

    for(i=6;i>=0;i-=2)
    {
        assert(aa_previous(&iter));
        assert(iter.current);
#if 0
        printf("i=%d C->i=%d\n", i, C->i);
#endif
        assert(C->i == i);
    }

    assert(!aa_previous(&iter));


    for(i=0;i<10;i+=2)
    {
        y.i = i;
        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_equal, &iter));
        assert(C->i == i);

        assert(aa_find_iter(&tree, &y.base, cmp,
                            aa_op_greater_or_equal, &iter));
        assert(C->i == i);

        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_less_or_equal, &iter));
        assert(C->i == i);

        if(i == 0)
            assert(!aa_find_iter(&tree, &y.base, cmp, aa_op_less, &iter));
        else
        {
#if 0
            printf("less i=%d\n", i);
#endif
            assert(aa_find_iter(&tree, &y.base, cmp, aa_op_less, &iter));
            assert(C->i == i - 2);
        }

        if(i == 8)
            assert(!aa_find_iter(&tree, &y.base, cmp, aa_op_greater, &iter));
        else
        {
#if 0
            printf("i=%d\n", i);
#endif
            assert(aa_find_iter(&tree, &y.base, cmp, aa_op_greater, &iter));
#if 0
            printf("i=%d C->i=%d\n", i, C->i);
#endif
            assert(C->i == i + 2);
        }

    }

    for(i=1;i<8;i+=2)
    {
        y.i = i;
        assert(!aa_find_iter(&tree, &y.base, cmp, aa_op_equal, &iter));
#if 0
        printf("less=%d\n", i);
#endif
        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_less, &iter));
        assert(C->i == i - 1);
        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_less_or_equal, &iter));
        assert(C->i == i - 1);
        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_greater, &iter));
        assert(C->i == i + 1);
        assert(aa_find_iter(&tree, &y.base, cmp, aa_op_greater_or_equal, &iter));
        assert(C->i == i + 1);
    }

    return 0;
}
