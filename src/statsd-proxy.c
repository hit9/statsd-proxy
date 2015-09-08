/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy for etsy/statsd.
 */

#include <stdio.h>
#include <pthread.h>

#include "log.h"
#include "proxy.h"

#include "config.h"

int
main(int argc, const char *argv[])
{
    log_open("statsd-proxy", NULL);

    pthread_t threads[num_threads];
    struct ctx *ctxs[num_threads];

    int i;

    for (i = 0; i < num_threads; i++) {
        ctxs[i] = ctx_new(nodes, num_nodes, port);
        pthread_create(&threads[i], NULL, &thread_start, ctxs[i]);
    }

    for (i = 0; i < num_threads; i++) {
         pthread_join(threads[i], NULL);
         ctx_free(ctxs[i]);
    }

    log_close();
    return 0;
}
