/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "buf.h"
#include "ctx.h"
#include "ketama.h"

/* Create ctx, init client/server sockets and ketama ring. */
struct ctx *
ctx_new(struct ketama_node *nodes, size_t num_nodes, unsigned short port)
{
    assert(nodes != NULL);

    struct ctx *ctx = malloc(sizeof(struct ctx));

    if (ctx == NULL)
        return NULL;

    struct buf *buf = buf_new(NULL);

    if (buf == NULL) {
        free(ctx);
        return NULL;
    }

    struct ketama_ring *ring = ketama_ring_new(nodes, num_nodes);

    if (ring == NULL) {
        free(buf);
        free(ctx);
        return NULL;
    }

    int cfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (ctx->cfd < 0) {
        ketama_ring_free(ring);
        free(ctx);
        return NULL;
    }

    int sfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT,
            (const void *)&optval , sizeof(int));

    if (ctx->sfd < 0) {
        free(buf);
        ketama_ring_free(ring);
        close(cfd);
        free(ctx);
        return NULL;
    }

    ctx->buf = buf;
    ctx->cfd = cfd;
    ctx->sfd = sfd;
    ctx->ring = ring;
    ctx->port = port;
    return ctx;
}

/* Free ctx, close client/server sockets and free ketama ring. */
void
ctx_free(struct ctx *ctx)
{
    if (ctx != NULL) {

        if (ctx->cfd > 0)
            close(ctx->cfd);

        if (ctx->sfd > 0)
            close(ctx->sfd);

        if (ctx->ring != NULL)
            ketama_ring_free(ctx->ring);

        if (ctx->buf != NULL)
            buf_free(ctx->buf);

        free(ctx);
    }
}
