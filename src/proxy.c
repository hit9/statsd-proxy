/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 */

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ctx.h"
#include "ketama.h"
#include "proxy.h"
#include "log.h"

/* Start proxy in a thread. */
void *
thread_start(void *arg)
{
    struct ctx *ctx = arg;
    if (ctx_init(ctx) == CTX_OK)
        server_start(ctx); /* FIXME: deal the return codes */
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

    if (bind(ctx->sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return PROXY_EBIND;

    log_info("listening on 127.0.0.1:%d ..", ctx->port);

    int n;
    struct buf *buf = ctx->buf;

    ctx->state = CTX_SERVER_RUNNING;

    for(; ctx->state != CTX_SERVER_STOPPED; ) {
        while (1) {
            if (buf_grow(buf, buf->len + BUF_READ_UNIT) != BUF_OK)
                return PROXY_ENOMEM;

            if ((n = recv(ctx->sfd, buf->data + buf->len,
                            (buf->cap - buf->len) * sizeof(char), 0)) < 0)
                break;

            buf->len += n;

            if (relay_buf(ctx) != PROXY_OK)
                log_warn("failed to relay data..");

            buf_clear(buf);  /* !important */
        }
    }

    return PROXY_OK;
}

/* Relay buffer to backends. */
int
relay_buf(struct ctx *ctx)
{
    assert(ctx != NULL);

}
