#ifndef MIN_HEAP_H
#define MIN_HEAP_H
#include "types.h"

Node* allocate_node(int symbol, unsigned freq);
MinHeap* create_min_heap(unsigned capacity);
void swap_nodes(Node** a, Node** b);
void min_heapify(MinHeap* minHeap, int idx);
int is_size_one(MinHeap* minHeap);
Node* extract_min(MinHeap* minHeap);
void insert_min_heap(MinHeap* minHeap, Node* minHeapNode);
void build_min_heap(MinHeap* minHeap);
MinHeap* create_and_build_min_heap(int symbols[], int freq[], int size);

#endif // MIN_HEAP_H