/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy threading entry.
 */

#ifndef _CW_PROXY_H
#define _CW_PROXY_H 1

#include "ctx.h"
#include "event.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define BUF_RECV_UNIT 64 * 1024      /* recv packet size limit: 64kb */
#define BUF_RECV_CAP_MAX 1024 * 1024 /* recv buffer memory limit: 1mb */
#define BUF_SEND_UNIT 32 * 1024      /* send packet size limit: 32kb */
#define BUF_SEND_CAP_MAX 1024 * 1024 /* send buffer memory limit: 1mb */

enum {
    PROXY_OK = 0,     /* operation is ok */
    PROXY_ENOMEM = 1, /* no memory error */
    PROXY_EBIND = 2,  /* socket bind error */
    PROXY_ELOOP = 3,  /* event loop error */
};

void *thread_start(void *arg);
int server_start(struct ctx *ctx);
void recv_buf(struct event_loop *loop, int fd, int mask, void *data);
int relay_buf(struct ctx *ctx);
void send_buf(struct ctx *ctx, struct sockaddr_in addr, struct buf *buf,
              char *addr_s);
void flush_buf(struct event_loop *loop, int id, void *data);

#if defined(__cplusplus)
}
#endif
#endif
