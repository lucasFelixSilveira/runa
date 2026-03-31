#ifndef RUNA_LZMA_H
#define RUNA_LZMA_H

#include <lzma.h>
#include <stdbool.h>
#include <stdlib.h>

bool runa_compress(char *input, int input_size, char **output, int *output_size);

char *runa_decompress(char *input, int input_size, int *output_size);

#endif
