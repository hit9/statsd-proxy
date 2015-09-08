/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy config.
 */

#ifndef _CW_CONFIG_H
#define _CW_CONFIG_H 1

#include "ketama.h"

static unsigned short port = 8125;
static size_t num_threads = 24;
static size_t num_nodes = 36;
static struct ketama_node nodes[] = {
    { "10.0.12.101:8127", 1},
    { "10.0.12.101:8128", 1},
    { "10.0.12.101:8129", 1},
    { "10.0.12.101:8130", 1},
    { "10.0.12.101:8131", 1},
    { "10.0.12.101:8132", 1},
    { "10.0.12.101:8133", 1},
    { "10.0.12.101:8134", 1},
    { "10.0.12.101:8135", 1},
    { "10.0.12.101:8136", 1},
    { "10.0.12.101:8137", 1},
    { "10.0.12.101:8138", 1},
    { "10.0.12.101:8139", 1},
    { "10.0.12.101:8140", 1},
    { "10.0.12.101:8141", 1},
    { "10.0.12.101:8142", 1},
    { "10.0.12.101:8143", 1},
    { "10.0.12.101:8144", 1},
    { "10.0.12.101:8145", 1},
    { "10.0.12.101:8146", 1},
    { "10.0.12.101:8147", 1},
    { "10.0.12.101:8148", 1},
    { "10.0.12.101:8149", 1},
    { "10.0.12.101:8150", 1},
    { "10.0.12.101:8151", 1},
    { "10.0.12.101:8152", 1},
    { "10.0.12.101:8153", 1},
    { "10.0.12.101:8154", 1},
    { "10.0.12.101:8155", 1},
    { "10.0.12.101:8156", 1},
    { "10.0.12.101:8157", 1},
    { "10.0.12.101:8158", 1},
    { "10.0.12.101:8159", 1},
    { "10.0.12.101:8160", 1},
    { "10.0.12.101:8161", 1},
    { "10.0.12.101:8162", 1},
};

#endif
