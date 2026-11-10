# Canonical Compression Engine

## Canonical Huffman Compression.pptx
Provides a high level overview for the project and highlights the findings. 

## Output.png 
Shows the output which compares the performance of using a traditional Huffman compression engine versus a canonical compression engine. 


## To run the code, please enter this in terminal

```
gcc main.c min_heap.c entropy.c canonical.c bitstream.c codec.c -o huffman_engine -lm
```
```
./huffman_engine book.pdf
```
```
./huffman_engine types.h
```
```
./huffman_engine dna.txt
```
