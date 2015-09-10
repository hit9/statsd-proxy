/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy config.
 */

#ifndef _CW_CONFIG_H
#define _CW_CONFIG_H 1

#include "ketama.h"

#define KETAMA_NUM_NODES_MAX        1024
#define KETAMA_NODE_KEY_LEN_MAX     32

enum {
    CONFIG_OK = 0,
    CONFIG_ENOMEM = 1,
    CONFIG_EFOPEN = 2,
    CONFIG_EBADFMT = 3,
    CONFIG_EVALUE = 4,
};

struct config {
    unsigned short port;
    size_t num_threads;
    size_t num_nodes;
    struct ketama_node nodes[KETAMA_NUM_NODES_MAX];
};

struct config *config_new(void);
int config_init(struct config *c, const char *filename);
void config_free(struct config *c);

#endif
