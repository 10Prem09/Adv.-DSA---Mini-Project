#ifndef CANONICAL_H
#define CANONICAL_H
#include "types.h"

Node* build_huffman_tree(int symbols[], int freq[], int size);
void extract_prefix_lengths(Node* root, int depth, int lengths_array[]);
int compare_sym_len(const void* a, const void* b);
void generate_canonical_codes(int lengths_array[], CanonicalCode final_codes[], int* unique_symbol_count);

#endif // CANONICAL_H