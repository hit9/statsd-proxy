/**
 * Copyright (c) 2015, Chao Wang <hit9@icloud.com>
 *
 * Proxy thread context.
 */


#ifndef KERNEL_ADDED_OVERHEAD_FACTOR
#define KERNEL_ADDED_OVERHEAD_FACTOR 2  // socket(7) man page indicates that the kernel doubles the size of SO_RCVBUF when set by setsockopt()
#endif

#ifndef _CW_CTX_H
#define _CW_CTX_H 1

#include <netinet/in.h>
#include "buf.h"
#include "ketama.h"

#if defined(__cplusplus)
extern "C" {
#endif

enum {
    CTX_OK = 0,      /* operation is ok */
    CTX_ENOMEM = 1,  /* no memory error */
    CTX_EBADFMT = 2, /* invalid format */
    CTX_ESOCKET = 3, /* socket create error */
    CTX_ETFD = 4,    /* timer fd create error */
};

struct ctx {
    int cfd;                 /* client udp socket fd */
    int sfd;                 /* server udp socket fd */
    unsigned short port;     /* server port to bind */
    uint32_t flush_interval; /* buffer flush interval */
    long socket_receive_bufsize; /* socket receive buffer size in bytes */
    uint32_t socket_send_packet_size; /* socket send packet size in bytes */
    size_t num_nodes;        /* number of ketama nodes */
    struct ketama_node *
        nodes; /* ketama nodes ref (shared by multiple threads, read only) */
    struct ketama_ring *ring;  /* ketama ring */
    struct sockaddr_in *addrs; /* backend addresses */
    struct buf *buf;           /* buffer to read socket */
    struct buf **sbufs;        /* buffers to send to socket */
};

struct ctx *ctx_new(struct ketama_node *nodes, size_t num_nodes,
                    unsigned short port, uint32_t flush_interval, long socket_receive_bufsize, uint32_t socket_send_packet_size);
void ctx_free(struct ctx *ctx);
int ctx_init(struct ctx *ctx);

#if defined(__cplusplus)
}
#endif
#endif
