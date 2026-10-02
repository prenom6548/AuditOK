// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_EIS_H_
#define _BACKEND_EIS_H_

#include <wlr/backend.h>

struct wlr_output_layout;
struct wlr_input_device;

struct wlr_backend *eis_backend_create(struct wl_display *display,
                                       struct wlr_output_layout *layout);

bool wlr_backend_is_eis(struct wlr_backend *backend);

bool wlr_input_device_is_eis(struct wlr_input_device *device);

#endif /* _BACKEND_EIS_H_ */
