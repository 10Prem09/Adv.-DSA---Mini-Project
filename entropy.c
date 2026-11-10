#include <math.h>
#include "entropy.h"

double calculate_shannon_entropy(int frequencies[], int total_chars) {
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            double probability = (double)frequencies[i] / total_chars;
            entropy -= probability * log2(probability);
        }
    }
    return entropy;
}