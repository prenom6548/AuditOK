// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _BACKEND_DRM_H_
#define _BACKEND_DRM_H_

#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_output.h>

struct kywc_output_state;

struct wlr_backend *drm_backend_create(struct wlr_session *session, struct wlr_device *dev,
                                       struct wlr_backend *parent);

struct drm_backend_monitor *drm_backend_monitor_create(struct wlr_backend *multi,
                                                       struct wlr_backend *primary_drm,
                                                       struct wlr_session *session);

int drm_backend_get_non_master_fd(struct wlr_backend *backend);

bool wlr_backend_is_drm(struct wlr_backend *backend);

bool wlr_output_is_drm(struct wlr_output *wlr_output);

struct wlr_output_mode *wlr_output_add_mode(struct wlr_output *wlr_output, int32_t width,
                                            int32_t height, int32_t refresh);

const void *wlr_output_get_edid(struct wlr_output *wlr_output, size_t *length);

bool wlr_output_get_vrr_capable(struct wlr_output *wlr_output);

bool wlr_output_get_rgb_range(struct wlr_output *wlr_output, uint32_t *rgb_range);

bool wlr_output_get_overscan(struct wlr_output *wlr_output, uint32_t *overscan);

bool wlr_output_get_scaling_mode(struct wlr_output *wlr_output, uint32_t *scaling_mode);

bool wlr_output_is_hardware_color_allowed(struct wlr_output *wlr_output,
                                          const struct kywc_output_state *state);

bool drm_output_yuyv_enabled(struct wlr_output *wlr_output, const struct kywc_output_state *state);

void wlr_output_set_color_changed(struct wlr_output *wlr_output, bool changed);

void wlr_output_set_buffer_damage(struct wlr_output *wlr_output, const pixman_region32_t *damage);

void wlr_output_set_rgb_range(struct wlr_output *wlr_output, uint32_t rgb_range);

void wlr_output_set_overscan(struct wlr_output *wlr_output, uint32_t overscan);

void wlr_output_set_scaling_mode(struct wlr_output *wlr_output, uint32_t scaling_mode);

#endif /* _BACKEND_DRM_H_ */
