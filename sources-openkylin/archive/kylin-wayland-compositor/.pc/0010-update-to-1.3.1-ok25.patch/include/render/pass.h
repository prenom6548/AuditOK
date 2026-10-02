// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _RENDER_PASS_H_
#define _RENDER_PASS_H_

#include <wlr/render/pass.h>

struct kywc_box;

struct render_round_corner {
    float rb, rt, lb, lt;
};

struct render_color_stop {
    float offset;
    float color[4];
};

struct render_linear_gradient {
    float x0, y0, x1, y1;
    struct render_color_stop color_stops[2];
};

/* only mix the overlapping parts of the target positions of the two textures */
struct render_texture_mix_options {
    /* source texture */
    struct wlr_texture *texture;
    /* destination coordinates in the target */
    const struct kywc_box *dst_box;
    float blend_factor;
};

struct render_texture_options {
    struct wlr_render_texture_options base;
    struct render_round_corner radius;
    bool enable_force_opaque;
    float rotation_angle;
    bool repeated;

    const struct render_texture_mix_options *mix_options;
};

struct render_rect_options {
    struct wlr_render_rect_options base;
    enum wl_output_transform transform;
    struct render_round_corner radius;
    float rotation_angle;
    float stroke_width;
    const struct render_linear_gradient *linear_gradient;
};

struct render_pass_impl {
    bool (*submit)(struct wlr_render_pass *pass, uint32_t quirks);
    void (*add_texture)(struct wlr_render_pass *pass, const struct render_texture_options *options);
    void (*add_rect)(struct wlr_render_pass *pass, const struct render_rect_options *options);
};

bool render_pass_options_has_radius(const struct render_round_corner *radius);

bool render_pass_submit(struct wlr_render_pass *render_pass, uint32_t quirks);

void render_pass_add_texture(struct wlr_render_pass *render_pass,
                             const struct render_texture_options *options);

void render_pass_add_rect(struct wlr_render_pass *render_pass,
                          const struct render_rect_options *options);

void render_pass_save(struct wlr_render_pass *render_pass);

void render_pass_restore(struct wlr_render_pass *render_pass);

struct wlr_renderer *render_pass_get_renderer(struct wlr_render_pass *render_pass);

#endif /* _RENDER_PASS_H_ */
