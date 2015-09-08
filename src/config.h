/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy config.
 */

#ifndef _CW_CONFIG_H
#define _CW_CONFIG_H 1

#include "ketama.h"

static unsigned short port = 8125;
static size_t num_threads = 4;
static size_t num_nodes = 2;
static struct ketama_node nodes[] = {
    { "127.0.0.1:8126", 1},
    { "127.0.0.1:8127", 1},
};

#endif
