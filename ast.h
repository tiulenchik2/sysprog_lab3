#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *kind;
    char *name;
    int val;
    struct Node *a, *b, *c, *next;
} Node;

Node *mk(char *kind, Node *a, Node *b, Node *c);
Node *num(int v);
Node *var(char *kind, char *name, Node *a);
