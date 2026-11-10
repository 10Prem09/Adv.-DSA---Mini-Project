#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "entropy.h"
#include "canonical.h"
#include "codec.h"

static long get_file_size(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fclose(fp);
    return sz;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* std_comp_path = "output_standard.huff";
    const char* std_decomp_path = "decompressed_standard.bin";
    const char* can_comp_path = "output_canonical.chuff";
    const char* can_decomp_path = "decompressed_canonical.bin";

    FILE* f = fopen(input_path, "rb");
    if (!f) {
        perror("Error opening input file");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long original_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (original_size == 0) {
        printf("Error: File is empty.\n");
        fclose(f);
        return 1;
    }

    int frequencies[ALPHABET_SIZE] = {0};
    int ch;
    int unique_count = 0;
    while ((ch = fgetc(f)) != EOF) {
        if (frequencies[(unsigned char)ch] == 0) unique_count++;
        frequencies[(unsigned char)ch]++;
    }
    fclose(f);

    double shannon_entropy = calculate_shannon_entropy(frequencies, original_size);

    // Executing Standard Huffman Compression & Decompression
    if (!compress_standard_huffman(input_path, std_comp_path)) {
        printf("Error: Standard Huffman compression failed.\n");
        return 1;
    }
    if (!decompress_standard_huffman(std_comp_path, std_decomp_path)) {
        printf("Error: Standard Huffman decompression failed.\n");
        return 1;
    }

    // Execute Canonical Huffman Compression & Decompression
    if (!compress_canonical_huffman(input_path, can_comp_path)) {
        printf("Error: Canonical Huffman compression failed.\n");
        return 1;
    }
    if (!decompress_canonical_huffman(can_comp_path, can_decomp_path)) {
        printf("Error: Canonical Huffman decompression failed.\n");
        return 1;
    }

    // File Size & Verification Metrics
    long std_comp_size = get_file_size(std_comp_path);
    long can_comp_size = get_file_size(can_comp_path);

    int std_valid = verify_files(input_path, std_decomp_path);
    int can_valid = verify_files(input_path, can_decomp_path);

    int std_tree_bits = (2 * unique_count - 1) + (unique_count * 8);

        int can_table_bits;
        if (unique_count <= 84) {
            can_table_bits = 8 + 8 + (unique_count * 12);
        } else {
            can_table_bits = 8 + (ALPHABET_SIZE * 4);     
        }
    printf("======================================================================\n");
    printf("         COMPRESSION ENGINE BENCHMARK & COMPARATIVE ANALYSIS          \n");
    printf("======================================================================\n");
    printf("Target File                : %s\n", input_path);
    printf("Original Size              : %ld bytes (%ld bits)\n", original_size, original_size * 8);
    printf("Alphabet Symbols Active    : %d / 256\n", unique_count);
    printf("Shannon Entropy Bound      : %.4f bits/byte\n", shannon_entropy);
    printf("----------------------------------------------------------------------\n");
    printf("%-28s | %-17s | %-17s\n", "Metric", "Standard Huffman", "Canonical Huffman");
    printf("----------------------------------------------------------------------\n");
    printf("%-28s | %14ld B | %14ld B\n", "On-Disk File Size", std_comp_size, can_comp_size);
    printf("%-28s | %13.2f %% | %13.2f %%\n", "Compression Ratio",
           ((double)std_comp_size / original_size) * 100.0,
           ((double)can_comp_size / original_size) * 100.0);
    printf("%-28s | %13d b | %13d b\n", "Header Metadata Footprint", std_tree_bits, can_table_bits);
    printf("%-28s | %17s | %17s\n", "Tree Stored in Header?", "Yes (Serialized)", "No (Only Lengths)");
    printf("%-28s | %17s | %17s\n", "Integrity Check",
           std_valid ? "PASSED" : "FAILED",
           can_valid ? "PASSED" : "FAILED");
    printf("----------------------------------------------------------------------\n");

    long byte_savings = std_comp_size - can_comp_size;
    printf("Canonical Optimization Result:\n");
    if (byte_savings >= 0) {
        printf("  Canonical format saved %ld bytes (%.2f%% smaller than Standard Huffman).\n",
               byte_savings, ((double)byte_savings / std_comp_size) * 100.0);
    } else {
        printf("  Standard format was %ld bytes smaller on this payload profile.\n", -byte_savings);
    }
    printf("======================================================================\n");

    return 0;
}