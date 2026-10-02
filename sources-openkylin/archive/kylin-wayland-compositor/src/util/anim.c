// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <math.h>
#include <string.h>

#include "util/anim.h"

// @see animtr.end_mask
#define MAX_NUM_CHANNELS 31

static float bezier_evaluate(float t, float p1, float p2)
{
    float c1 = 3.0f * p1;
    float c2 = 3.0f * p2 - 6.0f * p1;
    float c3 = 3.0f * p1 - 3.0f * p2 + 1.0f;
    return ((c3 * t + c2) * t + c1) * t;
}

static float bezier_derivative(float t, float p1, float p2)
{
    float c1 = 3.0f * p1;
    float c2 = 3.0f * p2 - 6.0f * p1;
    float c3 = 3.0f * p1 - 3.0f * p2 + 1.0f;
    return (3.0f * c3 * t + 2.0f * c2) * t + c1;
}

static float anim_easing_evaluate_bezier(const struct anim_easing *easing, float t)
{
    float cp1x = easing->easing.bezier.cp1[0];
    float cp1y = easing->easing.bezier.cp1[1];
    float cp2x = easing->easing.bezier.cp2[0];
    float cp2y = easing->easing.bezier.cp2[1];

    if (t <= 0.0f) {
        return 0.0f;
    }
    if (t >= 1.0f) {
        return 1.0f;
    }

    // Newton-Raphson method
    float t2 = t;
    for (int i = 0; i < 8; i++) {
        float bx = bezier_evaluate(t2, cp1x, cp2x);
        float diff = bx - t;
        if (fabsf(diff) < 1e-6f) {
            break;
        }

        float dbx = bezier_derivative(t2, cp1x, cp2x);
        if (fabsf(dbx) < 1e-6f) {
            break;
        }

        t2 -= diff / dbx;
        t2 = fmaxf(0.0f, fminf(1.0f, t2));
    }

    // Fallback to bisection method if Newton iteration fails to converge
    float bx_final = bezier_evaluate(t2, cp1x, cp2x);
    if (fabsf(bx_final - t) > 1e-5f) {
        float lo = 0.0f, hi = 1.0f;
        for (int i = 0; i < 16; i++) {
            float mid = (lo + hi) * 0.5f;
            float v = bezier_evaluate(mid, cp1x, cp2x);
            if (v < t) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        t2 = (lo + hi) * 0.5f;
    }

    return bezier_evaluate(t2, cp1y, cp2y);
}

static float hermite_evaluate(float t0, float y0, float out_tan, float t1, float y1, float in_tan,
                              float t)
{
    float dt = t1 - t0;
    if (fabsf(dt) < 1e-6f) {
        return y0;
    }

    float u = (t - t0) / dt;
    float u2 = u * u;
    float u3 = u2 * u;

    float h00 = 2.0f * u3 - 3.0f * u2 + 1.0f;
    float h10 = u3 - 2.0f * u2 + u;
    float h01 = -2.0f * u3 + 3.0f * u2;
    float h11 = u3 - u2;

    return h00 * y0 + h10 * out_tan * dt + h01 * y1 + h11 * in_tan * dt;
}

static float anim_easing_evaluate_hermite(const struct anim_easing *easing, float t)
{
    uint32_t num = easing->easing.hermite.points_num;
    const struct anim_curve_point *points = easing->easing.hermite.points;

    if (num == 0) {
        return t;
    }
    if (num == 1) {
        return points[0].value;
    }

    if (t <= points[0].time) {
        return points[0].value;
    }
    if (t >= points[num - 1].time) {
        return points[num - 1].value;
    }

    // Bisection method
    uint32_t lo = 0, hi = num - 1;
    while (hi - lo > 1) {
        uint32_t mid = (lo + hi) / 2;
        if (points[mid].time <= t) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    uint32_t i = lo;

    return hermite_evaluate(points[i].time, points[i].value, points[i].out_tangent,
                            points[i + 1].time, points[i + 1].value, points[i + 1].in_tangent, t);
}

static float anim_easing_evaluate(const struct anim_easing *easing, float t)
{
    switch (easing->type) {
    case ANIM_EASING_TYPE_FUNCTION:
        return easing->easing.function ? easing->easing.function(t) : t;
    case ANIM_EASING_TYPE_BEZIER:
        return anim_easing_evaluate_bezier(easing, t);
    case ANIM_EASING_TYPE_HERMITE:
        return easing->easing.hermite.points_num > 0 ? anim_easing_evaluate_hermite(easing, t) : t;
    default:
        return t;
    }
}

void anim_channel_set_easing_fn(struct anim_channel *anim_channel, anim_easing_function easing_fn,
                                float duration, float start, float end,
                                anim_channel_callback callback)
{
    anim_channel->easing.type = ANIM_EASING_TYPE_FUNCTION;
    anim_channel->easing.easing.function = easing_fn;

    anim_channel->duration = duration;
    anim_channel->start = start;
    anim_channel->end = end;
    anim_channel->callback = callback;
}

void animtr_start(struct animtr *animtr, struct anim *template, void *target, float speed)
{
    memset(animtr, 0, sizeof(struct animtr));
    animtr->template = template;
    animtr->target = target;
    animtr->speed = speed;
}

void animtr_update(struct animtr *animtr, float elapsed_time)
{
    struct anim *anim = animtr->template;
    float time = elapsed_time;

    assert(anim->channels_num <= MAX_NUM_CHANNELS);
    for (uint32_t i = 0; i < anim->channels_num; i++) {
        struct anim_channel *channel = &anim->channels[i];
        float duration = channel->duration / animtr->speed;

        if (time > 0.0f && time <= duration) {
            float t = time / duration;
            float eased_t = anim_easing_evaluate(&channel->easing, t);
            float value = channel->start + (channel->end - channel->start) * eased_t;
            channel->callback(animtr->target, value);
        }

        const uint32_t mask = 1U << i;
        if (!(animtr->end_mask & mask) && time > duration) {
            channel->callback(animtr->target, channel->end);
            animtr->end_mask |= mask;
        }
    }
}

bool animtr_is_end(struct animtr *animtr)
{
    uint32_t channels_num = animtr->template->channels_num;
    uint32_t mask = (1U << channels_num) - 1U;
    return (animtr->end_mask & mask) == mask;
}

float anim_easing_quad_in(float t)
{
    return t * t;
}

float anim_easing_quad_out(float t)
{
    float f = 1.0f - t;
    return 1.0f - f * f;
}

float anim_easing_quad_inout(float t)
{
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        float f = -2.0f * t + 2.0f;
        return 1.0f - 0.5f * f * f;
    }
}

float anim_easing_cubic_in(float t)
{
    return t * t * t;
}

float anim_easing_cubic_out(float t)
{
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}

float anim_easing_cubic_inout(float t)
{
    if (t < 0.5f)
        return 4.0f * t * t * t;
    else {
        float f = (2.0f * t) - 2.0f;
        return 0.5f * f * f * f + 1.0f;
    }
}
