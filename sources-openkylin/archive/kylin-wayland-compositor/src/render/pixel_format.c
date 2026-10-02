// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <drm_fourcc.h>

#include <kywc/log.h>

#include "render/pixel_format.h"
#include "util/macros.h"

/*
 * The DRM formats are little endian while the GL formats are big endian,
 * so DRM_FORMAT_ARGB8888 is actually compatible with GL_BGRA_EXT.
 */
static const struct pixel_format formats[] = {
    {
        .drm_format = DRM_FORMAT_ARGB8888,
        .bytes_per_block = 4,
        .has_alpha = true,

        .gl_format = GL_BGRA_EXT,
        .gl_type = GL_UNSIGNED_BYTE,

        .pixman_format = PIXMAN_a8r8g8b8,
    },
    {
        .drm_format = DRM_FORMAT_XRGB8888,
        .bytes_per_block = 4,

        .gl_format = GL_BGRA_EXT,
        .gl_type = GL_UNSIGNED_BYTE,

        .pixman_format = PIXMAN_x8r8g8b8,
    },
    {
        .drm_format = DRM_FORMAT_XBGR8888,
        .bytes_per_block = 4,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_BYTE,

        .pixman_format = PIXMAN_x8b8g8r8,
    },
    {
        .drm_format = DRM_FORMAT_ABGR8888,
        .bytes_per_block = 4,
        .has_alpha = true,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_BYTE,

        .pixman_format = PIXMAN_a8b8g8r8,
    },
    {
        .drm_format = DRM_FORMAT_RGBX8888,
        .bytes_per_block = 4,

        .pixman_format = PIXMAN_r8g8b8x8,
    },
    {
        .drm_format = DRM_FORMAT_RGBA8888,
        .bytes_per_block = 4,
        .has_alpha = true,

        .pixman_format = PIXMAN_r8g8b8a8,
    },
    {
        .drm_format = DRM_FORMAT_BGRX8888,
        .bytes_per_block = 4,

        .pixman_format = PIXMAN_b8g8r8x8,
    },
    {
        .drm_format = DRM_FORMAT_BGRA8888,
        .bytes_per_block = 4,
        .has_alpha = true,

        .pixman_format = PIXMAN_b8g8r8a8,
    },
    {
        .drm_format = DRM_FORMAT_R8,
        .bytes_per_block = 1,
    },
    {
        .drm_format = DRM_FORMAT_GR88,
        .bytes_per_block = 2,
    },
    {
        .drm_format = DRM_FORMAT_RGB888,
        .bytes_per_block = 3,
    },
    {
        .drm_format = DRM_FORMAT_BGR888,
        .bytes_per_block = 3,

        .gl_format = GL_RGB,
        .gl_type = GL_UNSIGNED_BYTE,
    },
    /* little endian */
    {
        .drm_format = DRM_FORMAT_RGBX4444,
        .bytes_per_block = 2,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT_4_4_4_4,
    },
    {
        .drm_format = DRM_FORMAT_RGBA4444,
        .bytes_per_block = 2,
        .has_alpha = true,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT_4_4_4_4,
    },
    {
        .drm_format = DRM_FORMAT_BGRX4444,
        .bytes_per_block = 2,
    },
    {
        .drm_format = DRM_FORMAT_BGRA4444,
        .bytes_per_block = 2,
        .has_alpha = true,
    },
    {
        .drm_format = DRM_FORMAT_RGBX5551,
        .bytes_per_block = 2,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT_5_5_5_1,
    },
    {
        .drm_format = DRM_FORMAT_RGBA5551,
        .bytes_per_block = 2,
        .has_alpha = true,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT_5_5_5_1,
    },
    {
        .drm_format = DRM_FORMAT_BGRX5551,
        .bytes_per_block = 2,
    },
    {
        .drm_format = DRM_FORMAT_BGRA5551,
        .bytes_per_block = 2,
        .has_alpha = true,
    },
    {
        .drm_format = DRM_FORMAT_XRGB1555,
        .bytes_per_block = 2,
    },
    {
        .drm_format = DRM_FORMAT_ARGB1555,
        .bytes_per_block = 2,
        .has_alpha = true,
    },
    {
        .drm_format = DRM_FORMAT_RGB565,
        .bytes_per_block = 2,

        .gl_format = GL_RGB,
        .gl_type = GL_UNSIGNED_SHORT_5_6_5,

        .pixman_format = PIXMAN_r5g6b5,
    },
    {
        .drm_format = DRM_FORMAT_BGR565,
        .bytes_per_block = 2,

        .pixman_format = PIXMAN_b5g6r5,
    },
    {
        .drm_format = DRM_FORMAT_XRGB2101010,
        .bytes_per_block = 4,

        .pixman_format = PIXMAN_x2r10g10b10,
    },
    {
        .drm_format = DRM_FORMAT_ARGB2101010,
        .bytes_per_block = 4,
        .has_alpha = true,

        .pixman_format = PIXMAN_a2r10g10b10,
    },
    {
        .drm_format = DRM_FORMAT_XBGR2101010,
        .bytes_per_block = 4,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_INT_2_10_10_10_REV_EXT,

        .pixman_format = PIXMAN_x2b10g10r10,
    },
    {
        .drm_format = DRM_FORMAT_ABGR2101010,
        .bytes_per_block = 4,
        .has_alpha = true,

        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_INT_2_10_10_10_REV_EXT,

        .pixman_format = PIXMAN_a2b10g10r10,
    },
    {
        .drm_format = DRM_FORMAT_XBGR16161616F,
        .bytes_per_block = 8,

        .gl_format = GL_RGBA,
        .gl_type = GL_HALF_FLOAT_OES,
    },
    {
        .drm_format = DRM_FORMAT_ABGR16161616F,
        .bytes_per_block = 8,
        .has_alpha = true,

        .gl_format = GL_RGBA,
        .gl_type = GL_HALF_FLOAT_OES,
    },
    {
        .drm_format = DRM_FORMAT_XBGR16161616,
        .bytes_per_block = 8,

        .gl_internalformat = GL_RGBA16_EXT,
        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT,
    },
    {
        .drm_format = DRM_FORMAT_ABGR16161616,
        .bytes_per_block = 8,
        .has_alpha = true,

        .gl_internalformat = GL_RGBA16_EXT,
        .gl_format = GL_RGBA,
        .gl_type = GL_UNSIGNED_SHORT,

#if PIXMAN_VERSION >= PIXMAN_VERSION_ENCODE(0, 46, 0)
        .pixman_format = PIXMAN_a16b16g16r16,
#endif
    },
    {
        .drm_format = DRM_FORMAT_YVYU,
        .bytes_per_block = 4,
        .block_width = 2,
        .block_height = 1,
    },
    {
        .drm_format = DRM_FORMAT_VYUY,
        .bytes_per_block = 4,
        .block_width = 2,
        .block_height = 1,
    },
};

static const uint32_t opaque_pixel_formats[] = {
    DRM_FORMAT_YUYV,   DRM_FORMAT_UYVY,   DRM_FORMAT_NV12,   DRM_FORMAT_NV21,   DRM_FORMAT_NV16,
    DRM_FORMAT_NV61,   DRM_FORMAT_NV24,   DRM_FORMAT_NV42,   DRM_FORMAT_YUV410, DRM_FORMAT_YVU410,
    DRM_FORMAT_YUV411, DRM_FORMAT_YVU411, DRM_FORMAT_YUV420, DRM_FORMAT_YVU420, DRM_FORMAT_YUV422,
    DRM_FORMAT_YVU422, DRM_FORMAT_YUV444, DRM_FORMAT_YVU444,
#ifdef DRM_FORMAT_P010
    DRM_FORMAT_P010,
#endif
#ifdef DRM_FORMAT_P012
    DRM_FORMAT_P012,
#endif
#ifdef DRM_FORMAT_P016
    DRM_FORMAT_P016,
#endif
};

void pixel_formats_for_each(pixel_formats_iterator_func_t iterator, void *data)
{
    for (size_t i = 0; i < ARRAY_SIZE(formats); ++i) {
        if (iterator(&formats[i], data)) {
            break;
        }
    }
}

const struct pixel_format *pixel_format_from_drm(uint32_t fmt)
{
    for (size_t i = 0; i < ARRAY_SIZE(formats); ++i) {
        if (formats[i].drm_format == fmt) {
            return &formats[i];
        }
    }
    return NULL;
}

bool pixel_format_is_opaque(uint32_t fmt)
{
    const struct pixel_format *format = pixel_format_from_drm(fmt);
    if (format != NULL) {
        return !format->has_alpha;
    }

    for (size_t i = 0; i < ARRAY_SIZE(opaque_pixel_formats); ++i) {
        if (opaque_pixel_formats[i] == fmt) {
            return true;
        }
    }

    return false;
}

uint32_t pixel_format_pixels_per_block(const struct pixel_format *format)
{
    uint32_t pixels = format->block_width * format->block_height;
    return pixels > 0 ? pixels : 1;
}

static int32_t div_round_up(int32_t dividend, int32_t divisor)
{
    int32_t quotient = dividend / divisor;
    if (dividend % divisor != 0) {
        quotient++;
    }
    return quotient;
}

int32_t pixel_format_min_stride(const struct pixel_format *format, int32_t width)
{
    int32_t pixels_per_block = (int32_t)pixel_format_pixels_per_block(format);
    int32_t bytes_per_block = (int32_t)format->bytes_per_block;
    if (width > INT32_MAX / bytes_per_block) {
        kywc_log(KYWC_DEBUG, "Invalid width %d (overflow)", width);
        return 0;
    }
    return div_round_up(width * bytes_per_block, pixels_per_block);
}

bool pixel_format_check_stride(const struct pixel_format *format, int32_t stride, int32_t width)
{
    int32_t bytes_per_block = (int32_t)format->bytes_per_block;
    if (stride % bytes_per_block != 0) {
        kywc_log(KYWC_DEBUG, "Invalid stride %d (incompatible with %d bytes-per-block)", stride,
                 bytes_per_block);
        return false;
    }

    int32_t min_stride = pixel_format_min_stride(format, width);
    if (min_stride <= 0) {
        return false;
    } else if (stride < min_stride) {
        kywc_log(KYWC_DEBUG, "Invalid stride %d (too small for %d bytes-per-block and width %d)",
                 stride, bytes_per_block, width);
        return false;
    }

    return true;
}

const struct pixel_format *pixel_format_from_gl(GLint gl_format, GLint gl_type, bool alpha)
{
    for (size_t i = 0; i < ARRAY_SIZE(formats); ++i) {
        if (formats[i].gl_format == gl_format && formats[i].gl_type == gl_type &&
            formats[i].has_alpha == alpha) {
            return &formats[i];
        }
    }
    return NULL;
}

bool pixel_format_is_support_by_gl(const struct pixel_format *format,
                                   const struct opengl_renderer *renderer)
{
    if (format->gl_type == 0) {
        return false;
    }
    if (format->gl_type == GL_UNSIGNED_INT_2_10_10_10_REV_EXT &&
        !renderer->exts.EXT_texture_type_2_10_10_10_REV) {
        return false;
    }
    if (format->gl_type == GL_HALF_FLOAT_OES && !renderer->exts.OES_texture_half_float_linear) {
        return false;
    }
    if (format->gl_type == GL_UNSIGNED_SHORT && !renderer->exts.EXT_texture_norm16) {
        return false;
    }
    /*
     * Note that we don't need to check for GL_EXT_texture_format_BGRA8888
     * here, since we've already checked if we have it at renderer creation
     * time and bailed out if not. We do the check there because Wayland
     * requires all compositors to support SHM buffers in that format.
     */
    return true;
}

const uint32_t *pixel_format_get_drm_formats_for_gl(size_t *len,
                                                    const struct opengl_renderer *renderer)
{
    static uint32_t shm_formats[ARRAY_SIZE(formats)];
    size_t j = 0;
    for (size_t i = 0; i < ARRAY_SIZE(formats); i++) {
        if (!pixel_format_is_support_by_gl(&formats[i], renderer)) {
            continue;
        }
        shm_formats[j++] = formats[i].drm_format;
    }
    *len = j;
    return shm_formats;
}

const uint32_t *pixel_format_get_drm_formats_for_pixman(size_t *len)
{
    static uint32_t drm_formats[ARRAY_SIZE(formats)];
    size_t j = 0;
    for (size_t i = 0; i < ARRAY_SIZE(formats); i++) {
        if (formats[i].pixman_format == 0) {
            continue;
        }
        drm_formats[j++] = formats[i].drm_format;
    }
    *len = j;
    return drm_formats;
}

const struct pixel_format *pixel_format_from_pixman(pixman_format_code_t fmt)
{
    for (size_t i = 0; i < ARRAY_SIZE(formats); ++i) {
        if (formats[i].pixman_format == fmt) {
            return &formats[i];
        }
    }
    return NULL;
}
