/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy config.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "cfg.h"
#include "config.h"
#include "ketama.h"
#include "log.h"

#define CONFIG_READ_UNIT    1024

struct config *
config_new(void)
{
    struct config *c = malloc(sizeof(struct config));

    if (c == NULL)
        return NULL;

    c->port = 8125;
    c->num_threads = 4;

    int i;

    for (i = 0; i < KETAMA_NUM_NODES_MAX; i++) {
        if ((c->nodes[i].key = malloc(
                        KETAMA_NODE_KEY_LEN_MAX * sizeof(char))) == NULL) {
            for (i = 0; i < KETAMA_NUM_NODES_MAX; i++)
                if (c->nodes[i].key != NULL)
                    free(c->nodes[i].key);
            return NULL;
        }
    }

    return c;
}

void
config_free(struct config *c)
{
    if (c != NULL) {
        int i;
        for (i = 0; i < KETAMA_NUM_NODES_MAX; i++)
            if (c->nodes[i].key != NULL)
                free(c->nodes[i].key);
        free(c);
    }
}


int
config_init(struct config *c, const char *filename)
{
    assert(c != NULL);

    struct buf *buf = buf_new(NULL);

    /* Read config file */
    int nread;
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        log_error("cannot open file %s", filename);
        return CONFIG_EFOPEN;
    }

    while (1) {
        if (buf_grow(buf, buf->len + CONFIG_READ_UNIT) != BUF_OK) {
            log_error("no memory error");
            return CONFIG_ENOMEM;
        }

        if ((nread = fread(buf->data + buf->len, sizeof(char),
                    buf->cap - buf->len, fp)) <= 0)
            break;

        buf->len += nread;
    }

    /* Parse and get values */
    struct cfg cfg;
    cfg.data = buf->data;
    cfg.len = buf->len;
    cfg.lineno = 1;

    int cfg_err;
    c->num_nodes = 0;

    while ((cfg_err = cfg_get(&cfg)) == CFG_OK) {
        size_t s_len = cfg.val_len + 1;
        char s[s_len];
        memset(s, s_len, 0);

        if (strncmp("port", cfg.key, cfg.key_len) == 0) {
            memcpy(s, cfg.val, cfg.val_len);
            long port = strtol(s, NULL, 10);

            if (port <= 0 || port > 65535) {
                log_error("invalid port at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            c->port = (unsigned short)port;
        }

        if (strncmp("num_threads", cfg.key, cfg.key_len) == 0) {
            memcpy(s, cfg.val, cfg.val_len);
            long num_threads = (size_t)strtol(s, NULL, 10);

            if (num_threads <= 0 || num_threads > 1024) {
                log_error("invalid num_threads at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            c->num_threads = (unsigned short)num_threads;
        }

        if (strncmp("node", cfg.key, cfg.key_len) == 0) {
            memcpy(s, cfg.val, cfg.val_len);

            if (strlen(s) >= KETAMA_NODE_KEY_LEN_MAX) {
                log_error("node address too large at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            char host[cfg.val_len];
            unsigned short port;
            unsigned int weight;

            if (sscanf(s, "%[^:]:%hu:%u", host, &port, &weight) != 3) {
                log_error("invalid node at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            sprintf((c->nodes[c->num_nodes]).key, "%s:%hu", host, port);
            (c->nodes[c->num_nodes]).weight = weight;
            c->num_nodes++;
            log_info("load udp://%s:%hu:%u done.", host, port, weight);
        }
    }

    buf_free(buf);

    if (cfg_err == CFG_EBADFMT) {
        log_error("invalid syntax in %s, at line %d", filename,
                cfg.lineno);
        return CONFIG_EBADFMT;
    }
    return CONFIG_OK;
}
