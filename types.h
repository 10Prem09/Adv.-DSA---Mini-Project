#ifndef TYPES_H
#define TYPES_H

#define MAX_TREE_HT 256
#define ALPHABET_SIZE 256

typedef struct Node {
    int symbol;
    unsigned freq;
    struct Node *left, *right;
} Node;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    Node **array;
} MinHeap;

typedef struct {
    int symbol;
    int length;
} SymLen;

typedef struct {
    unsigned int code;
    int length;
} CanonicalCode;

#endif // TYPES_H