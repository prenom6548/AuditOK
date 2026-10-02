// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <wayland-server-core.h>

struct session;

struct wlr_backend *backend_autocreate(struct wl_display *display, struct session **session_ptr);

bool backend_support_timeline(struct wlr_backend *backend);

#endif /* _BACKEND_H_ */
