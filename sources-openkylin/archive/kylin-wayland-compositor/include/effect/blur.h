// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_BLUR_H_
#define _EFFECT_BLUR_H_

#include "render/pass.h"
#include "scene/render.h"

struct blur_render_options {
    const float *alpha;
    const struct kywc_box *dst_box;
    const pixman_region32_t *clip;
    const struct render_round_corner *radius;
    /* support different round corner radius */
    enum wl_output_transform transform;

    const struct blur_info *blur;
};

void blur_render_with_target(struct scene_render_target *target,
                             const struct blur_render_options *options);

struct blur_task_render_options {
    /**
     * input: OpenGL texture
     * keep the texture size as a power of 2. otherwise flickering
     */
    struct opengl_texture *in_texture;
    /**
     * output: OpenGL framebuffer
     * NOTE: width and height must match input texture.
     */
    uint32_t out_fbo;
    uint32_t iterations;
    float offset;

    /* if clip is null blur all pixels otherwise blur clip.boundbox region */
    const pixman_region32_t *clip;

    /**
     * if apply_blur is null, render blurred texture(in clip region) to out_fbo.
     * if apply_blur isn't null, render blurred texture to out_fbo according to parameters.
     */
    const struct blur_render_options *apply_blur;
};

struct blur_task *blur_task_create(void);

void blur_task_destroy(struct blur_task *task);

void blur_task_render(struct blur_task *task, struct blur_task_render_options *options);

#endif /* _EFFECT_BLUR_H_ */
