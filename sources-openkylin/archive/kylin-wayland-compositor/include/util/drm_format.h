// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_DRM_FORMAT_H_
#define _UTIL_DRM_FORMAT_H_

#include <wlr/render/drm_format_set.h>

bool drm_format_init(struct wlr_drm_format *fmt, uint32_t format, uint64_t *modifiers,
                     size_t modifier_count);

bool drm_format_has(const struct wlr_drm_format *fmt, uint64_t modifier);

bool drm_format_add(struct wlr_drm_format *fmt, uint64_t modifier);

bool drm_format_copy(struct wlr_drm_format *dst, const struct wlr_drm_format *src);

#endif /* _UTIL_DRM_FORMAT_H_ */
