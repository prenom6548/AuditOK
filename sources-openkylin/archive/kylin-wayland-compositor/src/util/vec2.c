// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>

#include "util/vec2.h"

float vec2_length2(struct vec2 *vec)
{
    return vec->x * vec->x + vec->y * vec->y;
}

float vec2_length(struct vec2 *vec)
{
    return sqrtf(vec->x * vec->x + vec->y * vec->y);
}

float vec2_distance2(struct vec2 *a_vec, struct vec2 *b_vec)
{
    float x = b_vec->x - a_vec->x;
    float y = b_vec->y - a_vec->y;
    return x * x + y * y;
}

float vec2_distance(struct vec2 *a_vec, struct vec2 *b_vec)
{
    float x = b_vec->x - a_vec->x;
    float y = b_vec->y - a_vec->y;
    return sqrtf(x * x + y * y);
}

void vec2_normalize(struct vec2 *vec)
{
    float length = vec2_length(vec);
    vec->x /= length;
    vec->y /= length;
}

void vec2_normalize_to(struct vec2 *vec, struct vec2 *result)
{
    float length = vec2_length(vec);
    result->x = vec->x / length;
    result->y = vec->y / length;
}

void vec2_perpendicular(struct vec2 *vec, struct vec2 *result)
{
    result->x = vec->y;
    result->y = -vec->x;
}

void vec2_add(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x + b_vec->x;
    result->y = a_vec->y + b_vec->y;
}

void vec2_sub(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x - b_vec->x;
    result->y = a_vec->y - b_vec->y;
}

void vec2_mul(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x * b_vec->x;
    result->y = a_vec->y * b_vec->y;
}

void vec2_div(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x / b_vec->x;
    result->y = a_vec->y / b_vec->y;
}

void vec2_muls(struct vec2 *vec, float s)
{
    vec->x *= s;
    vec->y *= s;
}

void vec2_divs(struct vec2 *vec, float s)
{
    vec->x /= s;
    vec->y /= s;
}

void vec2_muls_to(struct vec2 *vec, float s, struct vec2 *result)
{
    result->x = vec->x * s;
    result->y = vec->y * s;
}

void vec2_divs_to(struct vec2 *vec, float s, struct vec2 *result)
{
    result->x = vec->x / s;
    result->y = vec->y / s;
}

void vec2_min(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x < b_vec->x ? a_vec->x : b_vec->x;
    result->y = a_vec->y < b_vec->y ? a_vec->y : b_vec->y;
}

void vec2_max(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result)
{
    result->x = a_vec->x > b_vec->x ? a_vec->x : b_vec->x;
    result->y = a_vec->y > b_vec->y ? a_vec->y : b_vec->y;
}
