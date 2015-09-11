/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy for etsy/statsd.
 */

#ifndef __linux__
#error "statsd-proxy requires linux3.9+"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "log.h"
#include "proxy.h"

#include "config.h"

#define STATSD_PROXY_VERSION    "0.0.4"

void version(void);
void usage(void);
void start(struct config *config);

int
main(int argc, const char *argv[])
{
    if (argc != 2)
        usage();

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        usage();

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
        version();

    if (argv[1][0] == '-')
        usage();

    log_open("statsd-proxy", NULL);
    log_setlevel(LOG_INFO);

    struct config *config = config_new();

    if (config == NULL)
        exit(1);

    if (config_init(config, argv[1]) != CONFIG_OK)
        exit(1);

    start(config);

    config_free(config);
    log_close();
    return 0;
}

void
version(void)
{
    fprintf(stderr, "statsd-proxy@%s\n", STATSD_PROXY_VERSION);
    exit(1);
}

void
usage(void)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  ./statsd-proxy /path/to/config.cfg\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help        Show this message\n");
    fprintf(stderr, "  -v, --version     Show version\n");
    exit(1);
}

void
start(struct config *config)
{
    assert(config != NULL);
    assert(config->num_threads > 0);
    assert(config->port > 0);
    assert(config->nodes != NULL);
    assert(config->num_nodes > 0);

    pthread_t threads[config->num_threads];
    struct ctx *ctxs[config->num_threads];

    int i;

    for (i = 0; i < config->num_threads; i++) {
        if ((ctxs[i] = ctx_new(config->nodes, config->num_nodes, config->port)) == NULL)
            exit(1);
        pthread_create(&threads[i], NULL, &thread_start, ctxs[i]);
    }

    log_debug("server started.");

    for (i = 0; i < config->num_threads; i++) {
         pthread_join(threads[i], NULL);
         ctx_free(ctxs[i]);
    }
}
