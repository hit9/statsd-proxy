/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy parser (statsd protocol).
 */

#ifndef _CW_PARSER_H
#define _CW_PARSER_H 1

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct parser_result {
    char *key;   /* parsed key */
    size_t len;  /* parsed key len */
    char *block; /* parsed block */
    size_t blen; /* parsed block len */
};

/**
 * example usage:
 *
 *   struct parser_result res;
 *   int n, n_parsed = 0;
 *   while ((n = parse(&res, data, len)) > 0) {
 *      // do operations...
 *      data += n;
 *      len -= n;
 *      n_parsed += n;
 *   }
 *   buf_lrm(buf, n_parsed);
 */
int parse(struct parser_result *result, char *data, size_t len);

#if defined(__cplusplus)
}
#endif

#endif
