// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_EVENT_H_
#define _UTIL_EVENT_H_

enum {
    EVENT_READABLE = 1 << 0,
    EVENT_WRITABLE = 1 << 1,
    EVENT_HANGUP = 1 << 2,
    EVENT_ERROR = 1 << 3
};

typedef int (*event_func_t)(uint32_t mask, void *data);

struct event_context *event_loop_create(void);

void event_loop_destroy(struct event_context *ctx);

int event_loop_add_source(struct event_context *ctx, int fd, event_func_t func, void *data);

void event_loop_remove_source(struct event_context *ctx, int fd);

int event_loop_dispatch(struct event_context *ctx, int timeout);

#endif /* _UTIL_EVENT_H_ */
