#ifndef RUNA_COMPRESSION_H
#define RUNA_COMPRESSION_H
#include <stdbool.h>

bool runa_compress(char *input, int input_size, char **output, int *output_size);
char *runa_decompress(char *input, int input_size, int *output_size);
#endif
