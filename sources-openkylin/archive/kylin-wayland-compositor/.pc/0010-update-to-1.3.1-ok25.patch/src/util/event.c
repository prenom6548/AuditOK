// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "util/event.h"

struct event_source {
    struct event_source *prev, *next;
    event_func_t func;
    void *user_data;
};

struct event_context {
    int epoll_fd;
    struct event_source source;
};

/* stolen from wayland */
static int set_cloexec_or_close(int fd)
{
    if (fd == -1) {
        return -1;
    }

    long flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        goto err;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        goto err;
    }

    return fd;

err:
    close(fd);
    return -1;
}

static int create_epoll(void)
{
    int fd;

#ifdef EPOLL_CLOEXEC
    fd = epoll_create1(EPOLL_CLOEXEC);
    if (fd >= 0) {
        return fd;
    }
    if (errno != EINVAL) {
        return -1;
    }
#endif

    fd = epoll_create(1);
    return set_cloexec_or_close(fd);
}

struct event_context *event_loop_create(void)
{
    struct event_context *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) {
        return NULL;
    }

    ctx->epoll_fd = create_epoll();
    if (ctx->epoll_fd == -1) {
        free(ctx);
        return NULL;
    }

    ctx->source.prev = ctx->source.next = &ctx->source;

    return ctx;
}

void event_loop_destroy(struct event_context *ctx)
{
    if (!ctx) {
        return;
    }

    struct event_source *head = &ctx->source, *temp;
    while (head->next != &ctx->source) {
        temp = head->next;
        head->next = temp->next;
        free(temp);
    }
    free(ctx);
}

int event_loop_add_source(struct event_context *ctx, int fd, event_func_t func, void *data)
{
    struct event_source *source = malloc(sizeof *source);
    if (!source) {
        return -1;
    }

    struct epoll_event ep = {
        .events = EPOLLIN,
        .data.ptr = source,
    };
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, fd, &ep) < 0) {
        free(source);
        return -1;
    }

    struct event_source *head = &ctx->source;
    source->prev = head;
    source->next = head->next;
    head->next->prev = source;
    head->next = source;

    source->func = func;
    source->user_data = data;

    return 0;
}

int event_loop_dispatch(struct event_context *ctx, int timeout)
{
    struct epoll_event ep[32];
    int count = epoll_wait(ctx->epoll_fd, ep, 32, timeout);
    if (count < 0 && errno != EINTR && errno != EAGAIN) {
        return -1;
    }

    struct event_source *source;
    for (int i = 0; i < count; i++) {
        source = ep[i].data.ptr;
        if (source->func(source->user_data) != 0) {
            return -1;
        }
    }

    return 0;
}
