// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_ANIM_H_
#define _UTIL_ANIM_H_

#include <stdbool.h>
#include <stdint.h>

struct anim_curve_point {
    float time;
    float value;
    float in_tangent;
    float out_tangent;
};

enum anim_easing_type {
    ANIM_EASING_TYPE_FUNCTION = 0,
    ANIM_EASING_TYPE_BEZIER,
    ANIM_EASING_TYPE_HERMITE,
};

typedef float (*anim_easing_function)(float t);

struct anim_easing {
    enum anim_easing_type type;
    union {
        anim_easing_function function;
        struct {
            float cp1[2];
            float cp2[2];
        } bezier;
        struct {
            uint32_t points_num;
            struct anim_curve_point *points;
        } hermite;
    } easing;
};

typedef void (*anim_channel_callback)(void *target, float value);

struct anim_channel {
    float duration;
    float start;
    float end;
    struct anim_easing easing;
    anim_channel_callback callback;
};

struct anim {
    uint32_t channels_num;
    struct anim_channel *channels;
};

struct animtr {
    struct anim *template;
    void *target;
    float speed;
    // private
    uint32_t end_mask;
};

void anim_channel_set_easing_fn(struct anim_channel *anim_channel, anim_easing_function easing_fn,
                                float duration, float start, float end,
                                anim_channel_callback callback);

void animtr_start(struct animtr *animtr, struct anim *template, void *target, float speed);

void animtr_update(struct animtr *animtr, float elapsed_time);

bool animtr_is_end(struct animtr *animtr);

float anim_easing_quad_in(float t);

float anim_easing_quad_out(float t);

float anim_easing_quad_inout(float t);

float anim_easing_cubic_in(float t);

float anim_easing_cubic_out(float t);

float anim_easing_cubic_inout(float t);

#endif /* _UTIL_ANIM_H_ */
