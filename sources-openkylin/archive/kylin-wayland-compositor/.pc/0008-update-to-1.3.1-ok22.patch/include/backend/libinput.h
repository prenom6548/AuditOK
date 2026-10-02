// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_LIBINPUT_H_
#define _BACKEND_LIBINPUT_H_

#include <libinput.h>

#include <wlr/backend.h>

struct wlr_input_device;

struct pointer_raw_tap_event {
    uint32_t time_msec;
    uint32_t fingers;
};

struct wlr_backend *libinput_backend_create(struct wl_display *display,
                                            struct wlr_session *session);

bool wlr_backend_is_libinput(struct wlr_backend *backend);

bool wlr_input_device_is_libinput(struct wlr_input_device *device);

struct libinput_device *wlr_libinput_get_device_handle(struct wlr_input_device *wlr_dev);

void wlr_libinput_add_raw_tap_listener(struct wlr_input_device *wlr_dev,
                                       struct wl_listener *listener);

#endif /* _BACKEND_LIBINPUT_H_ */
