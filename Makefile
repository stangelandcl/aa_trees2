CC?=gcc
WARN=\
	-Wall \
	-Werror \
	-Wpedantic \
	-Wno-unused-function \
	-Wno-maybe-uninitialized

ifdef DEBUG
OPT=-O0 -g3
else
OPT=-O3 -fomit-frame-pointer -mtune=native
endif

.PHONY: all
all:
	$(CC) $(OPT) -std=c89 $(WARN) -I. aa_tree.c test_aa_tree.c -o test
	./test
