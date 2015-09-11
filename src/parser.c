/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include "parser.h"

/* Parse statsd netblock string, and return once a key was
 * found.
 *
 * Return number chars parsed on success, -1 on failure.
 */
int
parse(struct parser_result *result, char *data, size_t len)
{
    int i;  /* to find the first block */
    int j;  /* to find key in one block */

    for (i = 0; i < len; i++) {
        if (data[i] == '\n') {
            for (j = 0; j < i; j++) {
                if (data[j] == ':') {
                    result->key = data;
                    result->len = j; /* exclude ':'*/
                    result->block = data;
                    result->blen = i; /* exclude '\n' */
                    return i + 1;
                }
            }
        }
    }

    /* no '\n' was found, single block */
    for (i = 0; i < len; i++) {
        if (data[i] == ':') {
            result->key = data;
            result->len = i;
            result->block = data;
            result->blen = len;
            return len;
        }
    }

    result->key = NULL;
    result->len = 0;
    result->block = NULL;
    result->blen = 0;
    return -1;
}
