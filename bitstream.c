#include <stdlib.h>
#include "bitstream.h"

BitWriter* open_bit_writer(FILE* fp) {
    BitWriter* bw = (BitWriter*)malloc(sizeof(BitWriter));
    bw->fp = fp;
    bw->buffer = 0;
    bw->bit_count = 0;
    return bw;
}

void write_bit(BitWriter* bw, int bit) {
    bw->buffer = (bw->buffer << 1) | (bit & 1);
    bw->bit_count++;
    if (bw->bit_count == 8) {
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
}

void write_bits(BitWriter* bw, unsigned int code, int length) {
    for (int i = length - 1; i >= 0; i--) {
        write_bit(bw, (code >> i) & 1);
    }
}

void flush_bit_writer(BitWriter* bw) {
    if (bw->bit_count > 0) {
        bw->buffer <<= (8 - bw->bit_count);
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
    free(bw);
}

BitReader* open_bit_reader(FILE* fp) {
    BitReader* br = (BitReader*)malloc(sizeof(BitReader));
    br->fp = fp;
    br->buffer = 0;
    br->bit_count = 0;
    return br;
}

int read_bit(BitReader* br) {
    if (br->bit_count == 0) {
        int byte = fgetc(br->fp);
        if (byte == EOF) return -1;
        br->buffer = (unsigned char)byte;
        br->bit_count = 8;
    }
    br->bit_count--;
    return (br->buffer >> br->bit_count) & 1;
}