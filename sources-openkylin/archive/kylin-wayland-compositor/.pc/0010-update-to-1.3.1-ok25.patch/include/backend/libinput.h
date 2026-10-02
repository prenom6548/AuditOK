// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_LIBINPUT_H_
#define _BACKEND_LIBINPUT_H_

#include <libinput.h>

#include <wlr/backend.h>

struct wlr_input_device;
struct wlr_touch;

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

bool wlr_libinput_get_touch_size_value(struct wlr_touch *touch, int32_t touch_id, double *major,
                                       double *minor);

bool wlr_libinput_get_touch_size_abs(struct wlr_touch *touch, int32_t *major_minimum,
                                     int32_t *major_maximum, int32_t *major_resolution,
                                     int32_t *minor_minimum, int32_t *minor_maximum,
                                     int32_t *minor_resolution);

#endif /* _BACKEND_LIBINPUT_H_ */
