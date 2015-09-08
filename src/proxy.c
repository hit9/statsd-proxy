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

int
server_start(void *arg)
{
    struct ctx *ctx = arg;

    assert(ctx != NULL);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(ctx->port);

    if (bind(ctx->sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return PROXY_EBIND;

    int n;
    struct buf *buf = ctx->buf;

    ctx->state = CTX_SERVER_RUNNING;

    for(;ctx->state != CTX_SERVER_STOPPED;) {
        while (1) {
            if (buf_grow(buf, buf->len + BUF_READ_UNIT) != BUF_OK)
                return PROXY_ENOMEM;

            if ((n = recv(ctx->sfd, buf->data + buf->len,
                            (buf->cap - buf->len) * sizeof(char), 0)) < 0)
                break;

            buf->len += n;
            log_info("%s\n", buf_str(buf));
            buf_clear(buf);  /* !important */
        }
    }
}
