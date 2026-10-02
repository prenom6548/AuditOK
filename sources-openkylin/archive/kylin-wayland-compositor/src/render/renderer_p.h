// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDERER_P_H_
#define _RENDERER_P_H_

#include <wlr/render/wlr_renderer.h>

struct server;
struct wlr_drm_format;

bool wayland_drm_create(struct wl_display *display, struct wlr_renderer *renderer, int master_fd);

bool renderer_config_init(struct server *server);

bool single_pixel_buffer_manager_create(struct wl_display *display);

#endif /* _RENDERER_P_H_ */
