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
#include <getopt.h>

#include "log.h"
#include "proxy.h"

#include "config.h"

#define STATSD_PROXY_VERSION    "0.0.5"

void version(void);
void usage(void);
void start(struct config *config);

int
main(int argc, char *argv[])
{
    log_open("statsd-proxy", NULL);

    char *filename;

    const char *short_opt = "hvdf:";
    struct option long_opt[] = {
        {"help",       no_argument,       NULL, 'h'},
        {"version",    no_argument,       NULL, 'v'},
        {"debug",      no_argument,       NULL, 'd'},
        {"file",       required_argument, NULL, 'f'},
        {NULL,         0,                 NULL, 0},
    };

    int c;
    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch (c) {
            case 'h':
            case ':':
            case '?':
                usage();
                break;
            case 'v':
                version();
                break;
            case 'f':
                filename = optarg;
                break;
            case 'd':
                log_setlevel(LOG_DEBUG);
                break;
            default:
                usage();
        };
    }

    if (argc == 1 || optind < argc)
        usage();

    struct config *config = config_new();

    if (config == NULL)
        exit(1);

    if (config_init(config, filename) != CONFIG_OK)
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
    fprintf(stderr, "  ./statsd-proxy -f ./path/to/config.cfg\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help        Show this message\n");
    fprintf(stderr, "  -v, --version     Show version\n");
    fprintf(stderr, "  -d, --debug       Enable debug logging\n");
    fprintf(stderr, "Copyright (c) https://github.com/hit9/statsd-proxy\n");
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
        if ((ctxs[i] = ctx_new(config->nodes,
                        config->num_nodes,
                        config->port,
                        config->flush_interval)) == NULL)
            exit(1);
        pthread_create(&threads[i], NULL, &thread_start, ctxs[i]);
    }

    for (i = 0; i < config->num_threads; i++) {
         pthread_join(threads[i], NULL);
         ctx_free(ctxs[i]);
    }
}
