#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "codec.h"
#include "canonical.h"
#include "min_heap.h"
#include "bitstream.h"

// ============================================================================
// Standard Huffman Tree Helpers
// ============================================================================

static void serialize_tree(Node* root, BitWriter* bw) {
    if (!root) return;
    if (!root->left && !root->right) {
        write_bit(bw, 1);
        write_bits(bw, (unsigned int)(unsigned char)root->symbol, 8);
    } else {
        write_bit(bw, 0);
        serialize_tree(root->left, bw);
        serialize_tree(root->right, bw);
    }
}

static Node* deserialize_tree(BitReader* br) {
    int flag = read_bit(br);
    if (flag == -1) return NULL;

    if (flag == 1) {
        unsigned int sym = 0;
        for (int i = 7; i >= 0; i--) {
            int b = read_bit(br);
            if (b == -1) return NULL;
            sym = (sym << 1) | b;
        }
        return allocate_node((int)sym, 0);
    } else {
        Node* internal = allocate_node(-1, 0);
        internal->left = deserialize_tree(br);
        internal->right = deserialize_tree(br);
        return internal;
    }
}

static void extract_standard_codes(Node* root, unsigned int code, int depth, CanonicalCode codes[]) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[(unsigned char)root->symbol].code = code;
        codes[(unsigned char)root->symbol].length = depth;
        return;
    }
    extract_standard_codes(root->left, (code << 1) | 0, depth + 1, codes);
    extract_standard_codes(root->right, (code << 1) | 1, depth + 1, codes);
}

static void free_tree(Node* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int verify_files(const char* file1, const char* file2) {
    FILE* f1 = fopen(file1, "rb");
    FILE* f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    int match = 1;
    while (1) {
        int b1 = fgetc(f1);
        int b2 = fgetc(f2);
        if (b1 != b2) {
            match = 0;
            break;
        }
        if (b1 == EOF) break;
    }
    fclose(f1);
    fclose(f2);
    return match;
}

// ============================================================================
// Standard Huffman Pipeline
// ============================================================================

int compress_standard_huffman(const char* src_path, const char* dest_path) {
    FILE* src = fopen(src_path, "rb");
    if (!src) return 0;

    fseek(src, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(src);
    fseek(src, 0, SEEK_SET);
    if (file_size == 0) { fclose(src); return 0; }

    unsigned char* buffer = (unsigned char*)malloc(file_size);
    fread(buffer, 1, file_size, src);
    fclose(src);

    int frequencies[ALPHABET_SIZE] = {0};
    for (uint32_t i = 0; i < file_size; i++) frequencies[buffer[i]]++;

    int symbols[ALPHABET_SIZE], freq_compact[ALPHABET_SIZE], unique_count = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (frequencies[i] > 0) {
            symbols[unique_count] = i;
            freq_compact[unique_count++] = frequencies[i];
        }
    }

    Node* root = build_huffman_tree(symbols, freq_compact, unique_count);
    CanonicalCode codes[ALPHABET_SIZE] = {0};
    extract_standard_codes(root, 0, 0, codes);

    FILE* dest = fopen(dest_path, "wb");
    if (!dest) { free(buffer); free_tree(root); return 0; }

    fwrite(&file_size, sizeof(uint32_t), 1, dest);

    BitWriter* bw = open_bit_writer(dest);
    serialize_tree(root, bw);

    for (uint32_t i = 0; i < file_size; i++) {
        unsigned char c = buffer[i];
        write_bits(bw, codes[c].code, codes[c].length);
    }
    flush_bit_writer(bw);

    fclose(dest);
    free(buffer);
    free_tree(root);
    return 1;
}

int decompress_standard_huffman(const char* src_path, const char* dest_path) {
    FILE* src = fopen(src_path, "rb");
    if (!src) return 0;

    uint32_t original_size = 0;
    if (fread(&original_size, sizeof(uint32_t), 1, src) != 1) {
        fclose(src);
        return 0;
    }

    BitReader* br = open_bit_reader(src);
    Node* root = deserialize_tree(br);
    if (!root) { free(br); fclose(src); return 0; }

    FILE* dest = fopen(dest_path, "wb");
    if (!dest) { free_tree(root); free(br); fclose(src); return 0; }

    uint32_t decoded_bytes = 0;
    while (decoded_bytes < original_size) {
        Node* curr = root;
        while (curr->left || curr->right) {
            int bit = read_bit(br);
            if (bit == -1) break;
            curr = (bit == 0) ? curr->left : curr->right;
        }
        fputc((unsigned char)curr->symbol, dest);
        decoded_bytes++;
    }

    free_tree(root);
    free(br);
    fclose(src);
    fclose(dest);
    return 1;
}

// ============================================================================
// Adaptive Canonical Huffman Pipeline
// ============================================================================

int compress_canonical_huffman(const char* src_path, const char* dest_path) {
    FILE* src = fopen(src_path, "rb");
    if (!src) return 0;

    fseek(src, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(src);
    fseek(src, 0, SEEK_SET);
    if (file_size == 0) { fclose(src); return 0; }

    unsigned char* buffer = (unsigned char*)malloc(file_size);
    fread(buffer, 1, file_size, src);
    fclose(src);

    int frequencies[ALPHABET_SIZE] = {0};
    for (uint32_t i = 0; i < file_size; i++) frequencies[buffer[i]]++;

    int symbols[ALPHABET_SIZE], freq_compact[ALPHABET_SIZE], unique_count = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (frequencies[i] > 0) {
            symbols[unique_count] = i;
            freq_compact[unique_count++] = frequencies[i];
        }
    }

    Node* root = build_huffman_tree(symbols, freq_compact, unique_count);
    int lengths_array[ALPHABET_SIZE] = {0};
    extract_prefix_lengths(root, 0, lengths_array);

    CanonicalCode codes[ALPHABET_SIZE] = {0};
    int total_symbols = 0;
    generate_canonical_codes(lengths_array, codes, &total_symbols);

    FILE* dest = fopen(dest_path, "wb");
    if (!dest) { free(buffer); free_tree(root); return 0; }

    fwrite(&file_size, sizeof(uint32_t), 1, dest);

    uint8_t mode = (unique_count > 84) ? 1 : 0;
    fputc(mode, dest);

    if (mode == 0) {
        uint8_t count = (uint8_t)unique_count;
        fputc(count, dest);
        BitWriter* hw = open_bit_writer(dest);
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (lengths_array[i] > 0) {
                write_bits(hw, (unsigned int)i, 8);
                write_bits(hw, (unsigned int)lengths_array[i], 4);
            }
        }
        flush_bit_writer(hw);
    } else {
        for (int i = 0; i < ALPHABET_SIZE; i += 2) {
            uint8_t packed = ((lengths_array[i] & 0x0F) << 4) | (lengths_array[i + 1] & 0x0F);
            fputc(packed, dest);
        }
    }

    BitWriter* bw = open_bit_writer(dest);
    for (uint32_t i = 0; i < file_size; i++) {
        unsigned char c = buffer[i];
        write_bits(bw, codes[c].code, codes[c].length);
    }
    flush_bit_writer(bw);

    fclose(dest);
    free(buffer);
    free_tree(root);
    return 1;
}

int decompress_canonical_huffman(const char* src_path, const char* dest_path) {
    FILE* src = fopen(src_path, "rb");
    if (!src) return 0;

    uint32_t original_size = 0;
    if (fread(&original_size, sizeof(uint32_t), 1, src) != 1) {
        fclose(src);
        return 0;
    }

    int mode = fgetc(src);
    if (mode == EOF) { fclose(src); return 0; }

    int lengths_array[ALPHABET_SIZE] = {0};

    if (mode == 0) {
        int count = fgetc(src);
        if (count == EOF) { fclose(src); return 0; }

        BitReader* hr = open_bit_reader(src);
        for (int i = 0; i < count; i++) {
            unsigned int sym = 0;
            for (int b = 7; b >= 0; b--) {
                int bit = read_bit(hr);
                sym = (sym << 1) | bit;
            }
            unsigned int len = 0;
            for (int b = 3; b >= 0; b--) {
                int bit = read_bit(hr);
                len = (len << 1) | bit;
            }
            lengths_array[sym] = (int)len;
        }
        free(hr);
    } else {
        for (int i = 0; i < ALPHABET_SIZE; i += 2) {
            int byte = fgetc(src);
            if (byte == EOF) { fclose(src); return 0; }
            lengths_array[i]     = (byte >> 4) & 0x0F;
            lengths_array[i + 1] = byte & 0x0F;
        }
    }

    CanonicalCode codes[ALPHABET_SIZE] = {0};
    int extracted_count = 0;
    generate_canonical_codes(lengths_array, codes, &extracted_count);

    FILE* dest = fopen(dest_path, "wb");
    if (!dest) { fclose(src); return 0; }

    BitReader* br = open_bit_reader(src);
    uint32_t decoded_bytes = 0;
    unsigned int current_code = 0;
    int current_len = 0;

    while (decoded_bytes < original_size) {
        int bit = read_bit(br);
        if (bit == -1) break;

        current_code = (current_code << 1) | bit;
        current_len++;

        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (lengths_array[i] == current_len && codes[i].code == current_code) {
                fputc((unsigned char)i, dest);
                decoded_bytes++;
                current_code = 0;
                current_len = 0;
                break;
            }
        }
    }

    free(br);
    fclose(src);
    fclose(dest);
    return 1;
}