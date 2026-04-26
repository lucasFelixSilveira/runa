#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LZMA
#include <lzma.h>
#endif

bool runa_compress(char *input, int input_size, char **output, int *output_size) {

#ifndef HAVE_LZMA
    *output = malloc(input_size);
    if(!* output ) return false;

    memcpy(*output, input, input_size);
    *output_size = input_size;
    return true;
#else

    size_t out_capacity = input_size + input_size / 3 + 128;
    *output = malloc(out_capacity);
    if(! *output ) return false;

    lzma_stream strm = LZMA_STREAM_INIT;

    if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64) != LZMA_OK) {
        free(*output);
        *output = NULL;
        return false;
    }

    strm.next_in = (uint8_t*)input;
    strm.avail_in = input_size;
    strm.next_out = (uint8_t*)*output;
    strm.avail_out = out_capacity;

    while(true) {
        lzma_ret ret = lzma_code(&strm, LZMA_FINISH);

        if( ret == LZMA_STREAM_END ) break;

        if( ret != LZMA_OK ) {
            lzma_end(&strm);
            free(*output);
            *output = NULL;
            return false;
        }
    }

    *output_size = out_capacity - strm.avail_out;
    lzma_end(&strm);
    return true;
#endif
}

char *runa_decompress(char *input, int input_size, int *output_size) {

#ifndef HAVE_LZMA
    char *output = malloc(input_size);
    if(! output ) return NULL;

    memcpy(output, input, input_size);
    *output_size = input_size;
    return output;
#else

    size_t out_capacity = input_size * 5;
    char *output = malloc(out_capacity);
    if(! output ) return NULL;

    lzma_stream strm = LZMA_STREAM_INIT;

    if( lzma_stream_decoder(&strm, UINT64_MAX, 0) != LZMA_OK ) {
        free(output);
        return NULL;
    }

    strm.next_in = (uint8_t*)input;
    strm.avail_in = input_size;
    strm.next_out = (uint8_t*)output;
    strm.avail_out = out_capacity;

    lzma_ret ret = lzma_code(&strm, LZMA_FINISH);

    if( ret != LZMA_STREAM_END && ret != LZMA_OK ) {
        lzma_end(&strm);
        free(output);
        return NULL;
    }

    *output_size = out_capacity - strm.avail_out;
    lzma_end(&strm);
    return output;
#endif
}
