/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <strings.h>
#include "ctx.h"
#include "event.h"
#include "ketama.h"
#include "log.h"
#include "proxy.h"
#include "parser.h"

int set_timerfd(int tfd, uint32_t ms);

/* Start proxy in a thread. */
void *
thread_start(void *arg)
{
    struct ctx *ctx = arg;
    int err = -1;

    if ((err = ctx_init(ctx)) == CTX_OK)
        err = server_start(ctx);

    switch (err) {
        case PROXY_ENOMEM:
            log_error("no memory");
            break;
        case PROXY_EBIND:
            log_error("failed to bind server on port %d", ctx->port);
            break;
        default:
            log_error("fatal error occurred");
    }
    exit(1);
}

/* Recv into buffer on data */
void
recv_buf(struct event_loop *loop, int fd, int mask, void *data)
{
    struct ctx *ctx = data;
    int n;
    struct buf *buf = ctx->buf;

    while (1) {
        if (buf_grow(buf, buf->len + BUF_RECV_UNIT) != BUF_OK) {
            log_error("no memory");
            exit(1);
        }

        if ((n = recv(ctx->sfd, buf->data + buf->len,
                        (buf->cap - buf->len) * sizeof(char), 0)) < 0)
            break;

        buf->len += n;

        if (relay_buf(ctx) != PROXY_OK) {
            log_error("no memory");
            exit(1);
        }
    }

    if (buf->cap >= BUF_RECV_CAP_MAX) {
        buf_clear(buf);
    } else {
        buf->len = 0;
    }
}

/* Start server. */
int
server_start(struct ctx *ctx)
{
    assert(ctx != NULL);
    assert(ctx->sfd > 0);
    assert(ctx->tfd > 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(ctx->port);

    if (bind(ctx->sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return PROXY_EBIND;

    log_info("serving on udp://127.0.0.1:%d..", ctx->port);

    if (set_timerfd(ctx->tfd, ctx->flush_interval) < 0) {
        log_error("failed to set timer for next interval!");
        exit(1);
    }

    struct event_loop *loop = event_loop_new(2);

    if (loop == NULL)
        return PROXY_ENOMEM;

    event_add_in(loop, ctx->sfd, &recv_buf, (void *)ctx);
    event_add_in(loop, ctx->tfd, &flush_buf, (void *)ctx);
    event_loop_start(loop);  /* block forerver */
    event_loop_free(loop);
    return PROXY_OK;
}

/* Util to send buf to addr */
void
send_buf(struct ctx *ctx, struct sockaddr_in addr, struct buf *sbuf,
        char *addr_s)
{
    assert(ctx != NULL);
    assert(sbuf != NULL);

    int n = sendto(ctx->cfd, sbuf->data, sbuf->len, 0,
            (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (n < 0)
        log_warn("send => an error occurred, skipping..");

    log_debug("flush %d bytes =>  %s", n, addr_s);

    if (sbuf->cap >= BUF_SEND_CAP_MAX) {
        buf_clear(sbuf);
    } else {
        sbuf->len = 0;
    }
}

/* Relay buffer to backends. Note: all network etc. errors on single
 * relay will be ignored. */
int
relay_buf(struct ctx *ctx)
{
    assert(ctx != NULL);
    assert(ctx->buf != NULL);
    assert(ctx->sbufs != NULL);

    if (ctx->buf->len == 0)
        return PROXY_OK;

    struct parser_result result;
    int n, n_parsed = 0;
    char *data = ctx->buf->data;
    size_t len = ctx->buf->len;
    struct ketama_node *node;
    struct sockaddr_in addr;
    struct buf *sbuf = NULL;

    while ((n = parse(&result, data, len)) > 0) {
        node = ketama_node_iget(ctx->ring, result.key, result.len);

        sbuf = ctx->sbufs[node->idx];
        addr = ctx->addrs[node->idx];

        if (sbuf->len > 0 && buf_putc(sbuf, '\n') != BUF_OK)
            return PROXY_ENOMEM;

        if (buf_put(sbuf, result.block, result.blen) != BUF_OK)
            return PROXY_ENOMEM;

        /* flush buffer if this buf is large enough */
        if (sbuf->len >= BUF_SEND_UNIT)
            send_buf(ctx, addr, sbuf, node->key);

        data += n;
        len -= n;
        n_parsed += n;
    }

    buf_lrm(ctx->buf, n_parsed);
    return PROXY_OK;
}

/* Flush buffers on interval. */
void
flush_buf(struct event_loop *loop, int fd, int mask, void *data)
{
    struct ctx *ctx = data;
    struct buf *sbuf;
    struct sockaddr_in addr;
    struct ketama_node *node;
    int i;

    for (i = 0; i < ctx->num_nodes; i++) {
        sbuf = ctx->sbufs[i];
        addr = ctx->addrs[i];
        node = &(ctx->nodes[i]);
        if (sbuf->len > 0)
            send_buf(ctx, addr, sbuf, node->key);
    }

    if (set_timerfd(ctx->tfd, ctx->flush_interval) < 0) {
        log_error("failed to set timer for next interval!");
        exit(1);
    }
}

int
set_timerfd(int tfd, uint32_t ms)
{
    /* Restart timer */
    struct itimerspec new_value;
    struct itimerspec old_value;

    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));

    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms - ts.tv_sec * 1000) * 1000000;

    new_value.it_value = ts;
    new_value.it_interval = ts;
    return timerfd_settime(tfd, 0, &new_value, &old_value);
}
