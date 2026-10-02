// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_COLOR_H_
#define _UTIL_COLOR_H_

#include <stdint.h>

struct color {
    uint8_t r, g, b; // [0, 255]
    float a;         // [0, 1.0]
};

// covert to 0xRRGGBB
static inline uint32_t color_to_uint24(struct color *src)
{
    return src->r << 16 | src->g << 8 | src->b;
}

static inline void color_to_float(float dst[static 4], struct color *src)
{
    dst[0] = src->r / 255.0;
    dst[1] = src->g / 255.0;
    dst[2] = src->b / 255.0;
    dst[3] = src->a;
}

// premultiplied alpha
static inline void color_float_pa(float dst[static 4], float src[static 4])
{
    dst[0] = src[0] * src[3];
    dst[1] = src[1] * src[3];
    dst[2] = src[2] * src[3];
    dst[3] = src[3];
}

static inline void color_float_pax(float dst[static 4], float src[static 4], float a)
{
    dst[0] = src[0] * a;
    dst[1] = src[1] * a;
    dst[2] = src[2] * a;
    dst[3] = a;
}

static inline void color_uint24_to_float(float dst[static 4], uint32_t src)
{
    dst[0] = ((src >> 16) & 0xff) / 255.0;
    dst[1] = ((src >> 8) & 0xff) / 255.0;
    dst[2] = (src & 0xff) / 255.0;
    dst[3] = 1.0;
}

#endif /* _UTIL_COLOR_H */
