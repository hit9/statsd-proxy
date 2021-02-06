/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include "ctx.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "buf.h"
#include "ketama.h"
#include "log.h"

/* Create ctx, init client/server sockets and ketama ring. */
struct ctx *ctx_new(struct ketama_node *nodes, size_t num_nodes,
                    unsigned short port, uint32_t flush_interval, long socket_receive_bufsize, uint32_t socket_send_packet_size) {
    assert(nodes != NULL);

    /* Create ctx */
    struct ctx *ctx = malloc(sizeof(struct ctx));

    if (ctx == NULL) return NULL;

    /* Create recv buf (for this thread) */
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

    /* Create send bufs (for each node)
     * note: for threading safe reasons, we cannot assign `node.data` and
     * arg `nodes` */
    struct buf **sbufs = malloc(sizeof(struct buf *) * num_nodes);

    if (sbufs == NULL) {
        ketama_ring_free(ring);
        free(addrs);
        free(buf);
        free(ctx);
        return NULL;
    }

    int i;

    for (i = 0; i < num_nodes; i++) {
        if ((sbufs[i] = buf_new(NULL)) == NULL) {
            free(sbufs);
            ketama_ring_free(ring);
            free(addrs);
            free(buf);
            free(ctx);
            return NULL;
        }
    }

    /* Assign all attrs */
    ctx->buf = buf;
    ctx->cfd = -1;
    ctx->sfd = -1;
    ctx->ring = ring;
    ctx->port = port;
    ctx->addrs = addrs;
    ctx->nodes = nodes;
    ctx->sbufs = sbufs;
    ctx->num_nodes = num_nodes;
    ctx->flush_interval = flush_interval;
    ctx->socket_receive_bufsize = socket_receive_bufsize;
    ctx->socket_send_packet_size = socket_send_packet_size;
    return ctx;
}

/* Free ctx, close client/server sockets and free ketama ring. */
void ctx_free(struct ctx *ctx) {
    if (ctx != NULL) {
        if (ctx->cfd > 0) close(ctx->cfd);

        if (ctx->sfd > 0) close(ctx->sfd);

        if (ctx->ring != NULL) ketama_ring_free(ctx->ring);

        if (ctx->buf != NULL) buf_free(ctx->buf);

        if (ctx->addrs != NULL) free(ctx->addrs);

        if (ctx->sbufs != NULL) {
            int i;
            for (i = 0; i < ctx->num_nodes; i++) buf_free(ctx->sbufs[i]);
            free(ctx->sbufs);
        }

        free(ctx);
    }
}

/* Init ctx, init addrs and client/sockets */
int ctx_init(struct ctx *ctx) {
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
        if (strlen(nodes[i].key) > 26) /* 15 + 10 + 1 = 26 */
            return CTX_EBADFMT;

        if (sscanf(nodes[i].key, "%[^:]:%hu", bhost, &bport) != 2)
            return CTX_EBADFMT;

        bzero(&addrs[i], sizeof(struct sockaddr_in));
        addrs[i].sin_family = AF_INET;
        addrs[i].sin_addr.s_addr = inet_addr(bhost);
        addrs[i].sin_port = htons(bport);
    }

    /* Init client socket */
    assert(ctx->cfd == -1);

    ctx->cfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (ctx->cfd < 0) {
        return CTX_ESOCKET;
    }

    /* Init server socket */
    assert(ctx->sfd == -1);

    ctx->sfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    int optval = 1;
    setsockopt(ctx->sfd, SOL_SOCKET, SO_REUSEPORT, (const void *)&optval,
               sizeof(int));
    if (ctx->socket_receive_bufsize > 0) {
        log_debug("Got non-zero socket_receive_bufsize.  Setting SO_RCVBUF to %ld...", ctx->socket_receive_bufsize);
        setsockopt(ctx->sfd, SOL_SOCKET, SO_RCVBUF, &ctx->socket_receive_bufsize, sizeof(ctx->socket_receive_bufsize));
        long actual_bufsize;
        socklen_t buflen = sizeof(actual_bufsize);
        if (getsockopt(ctx->sfd, SOL_SOCKET, SO_RCVBUF, &actual_bufsize, &buflen) == 0) {
            if (actual_bufsize != ctx->socket_receive_bufsize * KERNEL_ADDED_OVERHEAD_FACTOR) {
                log_warn("unable to set socket_receive_bufsize to %ld.  buffer was reset to %ld", ctx->socket_receive_bufsize, actual_bufsize);
            }
            log_debug("SO_RCVBUF ultimately set to %ld", actual_bufsize);
        }
        else {
            log_warn("unable to read socket buffer size: %s", strerror(errno));
        }
    }

    if (ctx->sfd < 0) {
        close(ctx->cfd);
        return CTX_ESOCKET;
    }
    return CTX_OK;
}
