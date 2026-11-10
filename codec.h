#ifndef CODEC_H
#define CODEC_H

#include <stdint.h>
#include "types.h"

// Standard Huffman
int compress_standard_huffman(const char* src_path, const char* dest_path);
int decompress_standard_huffman(const char* src_path, const char* dest_path);

// Canonical Huffman
int compress_canonical_huffman(const char* src_path, const char* dest_path);
int decompress_canonical_huffman(const char* src_path, const char* dest_path);

// Helper for byte-for-byte validation
int verify_files(const char* file1, const char* file2);

#endif // CODEC_H