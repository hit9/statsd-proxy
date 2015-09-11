/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ctx.h"
#include "ketama.h"
#include "log.h"
#include "proxy.h"
#include "parser.h"

/* Start proxy in a thread. */
void *
thread_start(void *arg)
{
    struct ctx *ctx = arg;
    int err;

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

/* Start server. */
int
server_start(struct ctx *ctx)
{
    assert(ctx != NULL);
    assert(ctx->sfd > 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(ctx->port);

    if (bind(ctx->sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return PROXY_EBIND;
    }

    log_info("listening on udp://127.0.0.1:%d..", ctx->port);

    int n;
    struct buf *buf = ctx->buf;

    ctx->state = CTX_SERVER_RUNNING;

    for(; ctx->state != CTX_SERVER_STOPPED; ) {
        if (buf_grow(buf, buf->len + BUF_RECV_UNIT) != BUF_OK)
            return PROXY_ENOMEM;

        if ((n = recv(ctx->sfd, buf->data + buf->len,
                        (buf->cap - buf->len) * sizeof(char), 0)) < 0) {
            log_warn("socket recv error, skipping..");
            break;
        }

        buf->len += n;

        relay_buf(ctx);

        if (buf->cap >= BUF_RECV_CAP_MAX) {
            buf_clear(buf);
        } else {
            buf->len = 0;
        }
    }

    return PROXY_OK;
}

/* Util to send buf to addr */
void
send_buf(struct ctx *ctx, struct sockaddr_in addr, struct buf *sbuf)
{
    assert(ctx != NULL);
    assert(sbuf != NULL);

    int n = sendto(ctx->cfd, sbuf->data, sbuf->len, 0,
            (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (n < 0)
        log_warn("send => an error occurred, skipping..");

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

    struct parser_result result;
    int n, n_parsed = 0;
    char *data = ctx->buf->data;
    size_t len = ctx->buf->len;
    struct ketama_node node;
    struct sockaddr_in addr;
    struct buf *sbuf = NULL;

    /* Aggregate net blocks for each node */
    while ((n = parse(&result, data, len)) > 0) {
        node = ketama_node_get(ctx->ring, result.key, result.len);

        sbuf = ctx->sbufs[node.idx];
        addr = ctx->addrs[node.idx];

        if (sbuf->len > 0 && buf_putc(sbuf, '\n') != BUF_OK)
            return PROXY_ENOMEM;

        if (buf_put(sbuf, result.block, result.blen) != BUF_OK)
            return PROXY_ENOMEM;

        if (sbuf->len >= BUF_SEND_UNIT) {
            log_info("send1 %s", buf_str(sbuf));
            send_buf(ctx, addr, sbuf);
        }

        data += n;
        len -= n;
        n_parsed += n;
    }

    /* Flush all node buffers even if they do not reach the send unit size */
    int i;

    for (i = 0; i < ctx->num_nodes; i++) {
        sbuf = ctx->sbufs[i];
        addr = ctx->addrs[i];
        send_buf(ctx, addr, sbuf);
    }
}
