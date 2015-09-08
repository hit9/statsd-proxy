/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy config.
 */

#ifndef _CW_CONFIG_H
#define _CW_CONFIG_H 1

#include "ketama.h"

unsigned short port = 8125;
size_t num_threads = 4;
size_t num_nodes = 1;
struct ketama_node nodes[] = {
    { "127.0.0.1:8126", 1, NULL, 0 }
};

#endif
