// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _XWAYLAND_H_
#define _XWAYLAND_H_

#include <stdbool.h>

struct seat;
struct server;
struct wl_client;
struct view;
struct input;
struct wlr_surface;

#if HAVE_XWAYLAND

bool xwayland_manager_create(struct server *server);

void xwayland_manager_destroy(void);

bool xwayland_check_client(const struct wl_client *client);

bool xwayland_check_view(struct view *view);

int xwayland_unscale(int value);

int xwayland_scale(int value);

float xwayland_get_scale(void);

void xwayland_set_cursor(struct seat *seat);

void xwayland_update_workarea(void);

bool xwayland_is_dragging_x11(struct seat *seat);

void xwayland_send_touch_size(struct input *input, int touch_id, double touch_major,
                              double touch_minor);

void xwayland_send_touch_info(struct input *input, int major_minimum, int major_maximum,
                              int major_resolution, int minor_minimum, int minor_maximum,
                              int minor_resolution);

pid_t xwayland_get_pid_from_wlr_surface(struct wlr_surface *wlr_surface);

#else

// clang-format off

#define INLINE static __attribute__((unused)) inline

INLINE bool xwayland_manager_create(struct server *server) { return false; }

INLINE void xwayland_manager_destroy(void) {}

INLINE bool xwayland_check_client(const struct wl_client *client) { return false; }

INLINE bool xwayland_check_view(struct view *view) { return false; }

INLINE int xwayland_unscale(int value) { return value; }

INLINE int xwayland_scale(int value) { return value; }

INLINE float xwayland_get_scale(void) { return 1.0; }

INLINE void xwayland_set_cursor(struct seat *seat) {}

INLINE void xwayland_update_workarea(void) {}

INLINE bool xwayland_is_dragging_x11(struct seat *seat) { return false; }

INLINE void xwayland_send_touch_size(struct input *input, int touch_id, double touch_major, double touch_minor) {}

INLINE void xwayland_send_touch_info(struct input *input, int major_minimum, int major_maximum,
                              int major_resolution, int minor_minimum, int minor_maximum,
                              int minor_resolution) {}

INLINE pid_t xwayland_get_pid_from_wlr_surface(struct wlr_surface *wlr_surface) { return -1; }

// clang-format on

#undef INLINE

#endif

#endif /* _XWAYLAND_H_ */
