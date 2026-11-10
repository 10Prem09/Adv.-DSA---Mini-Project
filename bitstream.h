#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdio.h>

typedef struct {
    FILE* fp;
    unsigned char buffer;
    int bit_count;
} BitWriter;

typedef struct {
    FILE* fp;
    unsigned char buffer;
    int bit_count;
} BitReader;

BitWriter* open_bit_writer(FILE* fp);
void write_bit(BitWriter* bw, int bit);
void write_bits(BitWriter* bw, unsigned int code, int length);
void flush_bit_writer(BitWriter* bw);

BitReader* open_bit_reader(FILE* fp);
int read_bit(BitReader* br);

#endif // BITSTREAM_H