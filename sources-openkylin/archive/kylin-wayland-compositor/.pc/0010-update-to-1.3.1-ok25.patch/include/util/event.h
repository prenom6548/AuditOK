// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_EVENT_H_
#define _UTIL_EVENT_H_

typedef int (*event_func_t)(void *data);

struct event_context *event_loop_create(void);

void event_loop_destroy(struct event_context *ctx);

int event_loop_add_source(struct event_context *ctx, int fd, event_func_t func, void *data);

int event_loop_dispatch(struct event_context *ctx, int timeout);

#endif /* _UTIL_EVENT_H_ */
