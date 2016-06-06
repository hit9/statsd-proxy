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
#include "log.h"

struct config *config_new(void) {
    struct config *c = malloc(sizeof(struct config));

    if (c == NULL) return NULL;

    c->port = 8125;
    c->num_threads = 4;
    c->flush_interval = 10;

    int i;

    for (i = 0; i < KETAMA_NUM_NODES_MAX; i++) {
        if ((c->nodes[i].key =
                 malloc(KETAMA_NODE_KEY_LEN_MAX * sizeof(char))) == NULL) {
            for (i = 0; i < KETAMA_NUM_NODES_MAX; i++)
                if (c->nodes[i].key != NULL) free(c->nodes[i].key);
            return NULL;
        }
    }

    return c;
}

void config_free(struct config *c) {
    if (c != NULL) {
        int i;
        for (i = 0; i < KETAMA_NUM_NODES_MAX; i++)
            if (c->nodes[i].key != NULL) free(c->nodes[i].key);
        free(c);
    }
}

int config_init(struct config *c, const char *filename) {
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

    char s[CONFIG_VAL_LEN_MAX] = {0};

    while ((cfg_err = cfg_get(&cfg)) == CFG_OK) {
        memset(s, 0, CONFIG_VAL_LEN_MAX);
        memcpy(s, cfg.val, cfg.val_len);

        if (cfg.val_len > CONFIG_VAL_LEN_MAX) {
            log_error("value is too large at line %d", cfg.lineno);
            return CONFIG_EVALUE;
        }

        if (strncmp("port", cfg.key, cfg.key_len) == 0) {
            long port = strtol(s, NULL, 10);

            if (port <= 0 || port > 65535) {
                log_error("invalid port at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            c->port = (unsigned short)port;
            log_debug("load config.port => %hu", c->port);
        }

        if (strncmp("num_threads", cfg.key, cfg.key_len) == 0) {
            long num_threads = strtol(s, NULL, 10);

            if (num_threads <= 0 || num_threads > 1024) {
                log_error("invalid num_threads at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            c->num_threads = (unsigned short)num_threads;
            log_debug("load config.num_threads => %hu", c->num_threads);
        }

        if (strncmp("flush_interval", cfg.key, cfg.key_len) == 0) {
            long flush_interval = strtol(s, NULL, 10);

            if (flush_interval <= 0 || flush_interval > 1000) {
                log_error("invalid flush_interval at line %d", cfg.lineno);
                return CONFIG_EVALUE;
            }

            c->flush_interval = (uint32_t)flush_interval;
            log_debug("load config.flush_interval => %ldms", c->flush_interval);
        }

        if (strncmp("node", cfg.key, cfg.key_len) == 0) {
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
            (c->nodes[c->num_nodes]).idx = c->num_nodes;
            c->num_nodes++;
            log_debug("load config.node#%d udp://%s:%hu:%u", c->num_nodes, host,
                      port, weight);
        }
    }

    buf_free(buf);

    if (cfg_err == CFG_EBADFMT) {
        log_error("invalid syntax in %s, at line %d", filename, cfg.lineno);
        return CONFIG_EBADFMT;
    }

    log_debug("config load done.");
    return CONFIG_OK;
}
