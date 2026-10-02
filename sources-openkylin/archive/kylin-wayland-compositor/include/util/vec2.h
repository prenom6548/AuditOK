// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_VEC2_H_
#define _UTIL_VEC2_H_

struct vec2 {
    float x;
    float y;
};

// length square
float vec2_length2(struct vec2 *vec);

float vec2_length(struct vec2 *vec);

// distance square
float vec2_distance2(struct vec2 *a_vec, struct vec2 *b_vec);

float vec2_distance(struct vec2 *a_vec, struct vec2 *b_vec);

void vec2_normalize(struct vec2 *vec);

void vec2_normalize_to(struct vec2 *vec, struct vec2 *result);

void vec2_perpendicular(struct vec2 *vec, struct vec2 *result);

void vec2_add(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

void vec2_sub(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

void vec2_mul(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

void vec2_div(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

void vec2_muls(struct vec2 *vec, float s);

void vec2_divs(struct vec2 *vec, float s);

void vec2_muls_to(struct vec2 *vec, float s, struct vec2 *result);

void vec2_divs_to(struct vec2 *vec, float s, struct vec2 *result);

void vec2_min(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

void vec2_max(struct vec2 *a_vec, struct vec2 *b_vec, struct vec2 *result);

#endif /* _UTIL_VEC2_H_ */
