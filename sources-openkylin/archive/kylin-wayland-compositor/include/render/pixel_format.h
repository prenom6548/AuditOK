// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDER_PIXEL_FORMAT_H_
#define _RENDER_PIXEL_FORMAT_H_

#include "render/opengl.h"

struct pixel_format {
    uint32_t drm_format;
    /* Bytes per block (including padding) */
    uint32_t bytes_per_block;
    /* Size of a block in pixels (zero for 1×1) */
    uint32_t block_width, block_height;
    /* Support alpha channel */
    bool has_alpha;

    // optional field, if empty then internalformat = format
    GLint gl_internalformat;
    GLint gl_format, gl_type;

    pixman_format_code_t pixman_format;
};

typedef bool (*pixel_formats_iterator_func_t)(const struct pixel_format *format, void *data);

void pixel_formats_for_each(pixel_formats_iterator_func_t iterator, void *data);

const struct pixel_format *pixel_format_from_drm(uint32_t fmt);

bool pixel_format_is_opaque(uint32_t fmt);

uint32_t pixel_format_pixels_per_block(const struct pixel_format *format);

bool pixel_format_check_stride(const struct pixel_format *format, int32_t stride, int32_t width);

int32_t pixel_format_min_stride(const struct pixel_format *format, int32_t width);

/* opengl */
const struct pixel_format *pixel_format_from_gl(GLint gl_format, GLint gl_type, bool alpha);

bool pixel_format_is_support_by_gl(const struct pixel_format *format,
                                   const struct opengl_renderer *renderer);

const uint32_t *pixel_format_get_drm_formats_for_gl(size_t *len,
                                                    const struct opengl_renderer *renderer);

/* pixman */
const uint32_t *pixel_format_get_drm_formats_for_pixman(size_t *len);

const struct pixel_format *pixel_format_from_pixman(pixman_format_code_t fmt);

#endif /* _RENDER_PIXEL_FORMAT_H_ */
