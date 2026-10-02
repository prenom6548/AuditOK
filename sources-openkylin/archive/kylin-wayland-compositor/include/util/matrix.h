// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_MATRIX_H_
#define _UTIL_MATRIX_H_

#include <wayland-server-protocol.h>

#include <kywc/boxes.h>

// Column Major (base glsl)
struct mat3 {
    float matrix[9];
};

void mat3_identity(struct mat3 *mat3);

void mat3_init_scale(struct mat3 *mat3, float sx, float sy);

void mat3_init_translate(struct mat3 *mat3, float tx, float ty);

void mat3_init_scale_translate(struct mat3 *mat3, float sx, float sy, float tx, float ty);

// Right Multiplication
void mat3_multiply(struct mat3 *result, const struct mat3 *a_mat, const struct mat3 *b_mat);

void mat3_translate(struct mat3 *mat3, float x, float y);

void mat3_scale(struct mat3 *mat3, float x, float y);

// counter clock wise
void mat3_rotate(struct mat3 *mat3, float rad);

void mat3_flip_x(struct mat3 *mat3);

void mat3_flip_y(struct mat3 *mat3);

// use framebuffer coord not logic coord. see scene_render_box
void mat3_framebuffer_to_ndc(struct mat3 *mat3, int width, int height);

/* not include translate & scale
    // example
    struct mat3 transform;
    mat3_identity(&transform);

    // keep scale->rotate->translate order
    mat3_scale(&transform, target->scale, target->scale);
    // mat3_rotate(&transform, 3.14 * 0.25); // if need rotate
    mat3_translate(&transform, lx - target->logical.x, ly - target->logical.y);

    struct mat3 to_ndc;
    mat3_logic_to_ndc(&to_ndc, target->logical.width, target->logical.height, target->transform);

    struct mat3 transform_ndc;
    mat3_multiply(&transform_ndc, &to_ndc, &transform);

    // vertex shader
    gl_Position = vec4(transform_ndc * vec3(position, 1.0), 1.0);
*/
void mat3_logic_to_ndc(struct mat3 *mat3, int width, int height,
                       enum wl_output_transform transform);

// for invert rotate uv
void mat3_invert_output_transform(struct mat3 *mat, enum wl_output_transform transform);

void mat3_uvofbox_to_ndc(struct mat3 *uv2ndc, int buffer_w, int buffer_h, float rotation_angle,
                         const struct kywc_box *box);

#endif /* _UTIL_MATRIX_H_ */
