// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/drm_format.h"

bool drm_format_init(struct wlr_drm_format *fmt, uint32_t format, uint64_t *modifiers,
                     size_t modifier_count)
{
    *fmt = (struct wlr_drm_format){
        .format = format,
    };

    if (!modifiers || modifier_count == 0) {
        return true;
    }

    fmt->modifiers = malloc(modifier_count * sizeof(*modifiers));
    if (!fmt->modifiers) {
        return false;
    }

    memcpy(fmt->modifiers, modifiers, modifier_count * sizeof(*modifiers));
    fmt->len = fmt->capacity = modifier_count;
    return true;
}

bool drm_format_has(const struct wlr_drm_format *fmt, uint64_t modifier)
{
    for (size_t i = 0; i < fmt->len; ++i) {
        if (fmt->modifiers[i] == modifier) {
            return true;
        }
    }
    return false;
}

bool drm_format_add(struct wlr_drm_format *fmt, uint64_t modifier)
{
    if (drm_format_has(fmt, modifier)) {
        return true;
    }

    if (fmt->len == fmt->capacity) {
        size_t capacity = fmt->capacity ? fmt->capacity * 2 : 4;

        uint64_t *new_modifiers = realloc(fmt->modifiers, sizeof(*fmt->modifiers) * capacity);
        if (!new_modifiers) {
            return false;
        }

        fmt->capacity = capacity;
        fmt->modifiers = new_modifiers;
    }

    fmt->modifiers[fmt->len++] = modifier;
    return true;
}

bool drm_format_copy(struct wlr_drm_format *dst, const struct wlr_drm_format *src)
{
    assert(src->len <= src->capacity);

    uint64_t *modifiers = malloc(sizeof(*modifiers) * src->len);
    if (!modifiers) {
        return false;
    }

    memcpy(modifiers, src->modifiers, sizeof(*modifiers) * src->len);

    wlr_drm_format_finish(dst);
    dst->capacity = src->len;
    dst->len = src->len;
    dst->format = src->format;
    dst->modifiers = modifiers;
    return true;
}
