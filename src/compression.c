#include <lzma.h>
#include <stdbool.h>
#include <stdlib.h>

bool runa_compress(char *input, int input_size, char **output, int *output_size) {
    size_t out_capacity = input_size + input_size / 3 + 128;
    *output = malloc(out_capacity);
    if(! *output ) return false;

    lzma_stream strm = LZMA_STREAM_INIT;

    lzma_ret ret = lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64);
    if( ret != LZMA_OK ) {
        free(*output);
        *output = NULL;
        return false;
    }

    strm.next_in = (uint8_t*)input;
    strm.avail_in = (size_t)input_size;

    strm.next_out = (uint8_t*)*output;
    strm.avail_out = out_capacity;

    while(true) {
        ret = lzma_code(&strm, LZMA_FINISH);

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
}

char *runa_decompress(char *input, int input_size, int *output_size) {
    size_t out_capacity = input_size * 5;
    char *output = malloc(out_capacity);

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret _ = lzma_stream_decoder(&strm, UINT64_MAX, 0);

    strm.next_in = input;
    strm.avail_in = input_size;

    strm.next_out = output;
    strm.avail_out = out_capacity;

    _ = lzma_code(&strm, LZMA_FINISH);
    *output_size = out_capacity - strm.avail_out;

    lzma_end(&strm);
    return output;
}
