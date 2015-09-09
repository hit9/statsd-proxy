/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "buf.h"
#include "ctx.h"
#include "ketama.h"
#include "log.h"

/* Create ctx, init client/server sockets and ketama ring. */
struct ctx *
ctx_new(struct ketama_node *nodes, size_t num_nodes, unsigned short port)
{
    assert(nodes != NULL);

    /* Create ctx */
    struct ctx *ctx = malloc(sizeof(struct ctx));

    if (ctx == NULL)
        return NULL;

    /* Create buf */
    struct buf *buf = buf_new(NULL);

    if (buf == NULL) {
        free(ctx);
        return NULL;
    }

    /* Create net address */
    struct sockaddr_in *addrs = malloc(sizeof(struct sockaddr_in) * num_nodes);

    if (addrs == NULL) {
        free(buf);
        free(ctx);
        return NULL;
    }

    /* Create ketama ring */
    struct ketama_ring *ring = ketama_ring_new(nodes, num_nodes);

    if (ring == NULL) {
        free(addrs);
        free(buf);
        free(ctx);
        return NULL;
    }

    /* Assign all attrs */
    ctx->buf = buf;
    ctx->cfd = -1;
    ctx->sfd = -1;
    ctx->ring = ring;
    ctx->port = port;
    ctx->addrs = addrs;
    ctx->nodes = nodes;
    ctx->num_nodes = num_nodes;
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

        if (ctx->addrs != NULL)
            free(ctx->addrs);

        free(ctx);
    }
}

/* Init ctx, init addrs and client/sockets */
int
ctx_init(struct ctx *ctx)
{
    assert(ctx != NULL);

    /* Fillin net address */
    assert(ctx->nodes != NULL);
    assert(ctx->addrs != NULL);
    struct sockaddr_in *addrs = ctx->addrs;
    int i;
    unsigned short bport = 8125;
    char bhost[17] = {0}; /* 255.255.255.255 (15) */
    struct ketama_node *nodes = ctx->nodes;

    for (i = 0; i < ctx->num_nodes; i++) {
        if (strlen(nodes[i].key) > 26)  /* 15 + 10 + 1 = 26 */
            return CTX_EBADFMT;

        if (sscanf(nodes[i].key, "%[^:]:%hu", bhost, &bport) != 2)
            return CTX_EBADFMT;

        bzero(&addrs[i], sizeof(struct sockaddr_in));
        addrs[i].sin_family = AF_INET;
        addrs[i].sin_addr.s_addr = inet_addr(bhost);
        addrs[i].sin_port = htons(bport);
        log_info("load node %s:%d done.", bhost, bport);
    }

    /* Init client socket */
    assert(ctx->cfd == -1);

    ctx->cfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (ctx->cfd < 0) {
        return CTX_ESOCKET;
    }

    /* Init server socket */
    assert(ctx->sfd == -1);

    ctx->sfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    int optval = 1;
    setsockopt(ctx->sfd, SOL_SOCKET, SO_REUSEPORT,
            (const void *)&optval , sizeof(int));

    if (ctx->sfd < 0) {
        close(ctx->cfd);
        return CTX_ESOCKET;
    }
    return CTX_OK;
}
