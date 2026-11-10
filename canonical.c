#include <stdlib.h>
#include <stdio.h>
#include "canonical.h"
#include "min_heap.h"

Node* build_huffman_tree(int symbols[], int freq[], int size) {
    Node *left, *right, *top;
    MinHeap* minHeap = create_and_build_min_heap(symbols, freq, size);

    while (!is_size_one(minHeap)) {
        left = extract_min(minHeap);
        right = extract_min(minHeap);
        
        top = allocate_node(-1, left->freq + right->freq);
        top->left = left;
        top->right = right;
        insert_min_heap(minHeap, top);
    }
    return extract_min(minHeap);
}

void extract_prefix_lengths(Node* root, int depth, int lengths_array[]) {
    if (root->left) extract_prefix_lengths(root->left, depth + 1, lengths_array);
    if (root->right) extract_prefix_lengths(root->right, depth + 1, lengths_array);
    
    // If it's a leaf node
    if (!(root->left) && !(root->right)) {
        lengths_array[root->symbol] = depth;
    }
}

int compare_sym_len(const void* a, const void* b) {
    SymLen* s1 = (SymLen*)a;
    SymLen* s2 = (SymLen*)b;
    if (s1->length != s2->length)
        return s1->length - s2->length;
    return s1->symbol - s2->symbol;
}

void generate_canonical_codes(int lengths_array[], CanonicalCode final_codes[], int* unique_symbol_count) {
    SymLen sorted_syms[ALPHABET_SIZE];
    int count = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (lengths_array[i] > 0) {
            sorted_syms[count].symbol = i;
            sorted_syms[count].length = lengths_array[i];
            count++;
        }
    }
    *unique_symbol_count = count;

    qsort(sorted_syms, count, sizeof(SymLen), compare_sym_len);

    unsigned int current_code = 0;
    int current_len = sorted_syms[0].length;
    
    final_codes[sorted_syms[0].symbol].code = current_code;
    final_codes[sorted_syms[0].symbol].length = current_len;

    for (int i = 1; i < count; i++) {
        current_code = (current_code + 1) << (sorted_syms[i].length - current_len);
        current_len = sorted_syms[i].length;
        
        final_codes[sorted_syms[i].symbol].code = current_code;
        final_codes[sorted_syms[i].symbol].length = current_len;
    }
}