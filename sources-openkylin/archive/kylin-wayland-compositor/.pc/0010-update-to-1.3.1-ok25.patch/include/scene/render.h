// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _SCENE_RENDER_H_
#define _SCENE_RENDER_H_

#include "scene.h"

enum scene_render_option {
    SCENE_RENDER_DISABLE_VISIBILITY = 1 << 0,
    SCENE_RENDER_DISABLE_ROUND_CORNER = 1 << 1,
    SCENE_RENDER_DISABLE_BLUR = 1 << 2,
    SCENE_RENDER_DISABLE_EFFECT = 1 << 3,
    SCENE_RENDER_ENABLE_PRESENTATION = 1 << 4,
    SCENE_RENDER_ENABLE_SECURITY = 1 << 5,
    SCENE_RENDER_ENABLE_CURSORS = 1 << 6,
    SCENE_RENDER_DISABLE_SUBEFFECT_NODE = 1 << 7,
    /**
     * use force opaque when rendering a full apaque node.
     * don't use glDisable(GL_BLEND), which can prevent
     * textures that are marked as opaque but contain transparent pixels in the content
     * from generating thumbnails with transparent pixels.
     */
    SCENE_RENDER_ENABLE_FORCE_OPAQUE = 1 << 8,
};

/**
 * based on the set scale and transformation,
 * render the content within the logical range of the scene
 * to the offset position of the buffer.
 */
struct scene_render_target {
    /* the range of rendering in logical coord */
    struct kywc_box logical;

    /* geometry attributes */
    enum wl_output_transform transform;
    float scale;

    /* transformed offset in buffer */
    int offset_x, offset_y;
    /* transformed buffer size */
    int trans_width, trans_height;

    struct wlr_buffer *buffer;
    struct scene_output *output;
    struct wlr_render_pass *render_pass;

    /* current layout damage in logical coord */
    pixman_region32_t damage;
    /* expand layout damage in logical coord */
    pixman_region32_t expand_damage;
    /* excluded layout damage in logical coord */
    pixman_region32_t excluded_damage;
    /* excluded buffer damage in buffer-local coord */
    pixman_region32_t excluded_buffer_damage;

    /* options when render to this target */
    uint32_t options;
};

struct scene_render_blur_options {
    /* logic coord, (lx, ly) of blur node */
    int node_lx, node_ly;
    const float *alpha;
    /* logic coord, node blur info */
    const struct blur_info *blur;
    /* framebuffer coord */
    const pixman_region32_t *clip;
    struct {
        /**
         * framebuffer coord,  box of round box.
         * if boundary is NULL use blur region bounding box,
         * blur region can't empty in this situation.
         */
        const struct kywc_box *boundary;
        /* radius on boundary box corner, framebuffer pixel */
        const struct render_round_corner *radius;
    } round_mask;
};

struct scene_render_texture_options {
    struct wlr_texture *texture;
    const struct kywc_box *geometry_box;
    const struct render_texture_mix_options *mix_options;
    /**
     * texture round corner radius
     * 0 = right-bottom, 1 = right-top, 2 = left-bottom, 3 = left-top
     */
    const int (*radius)[4];
    /* the rotation angle is based on transform */
    const float *alpha, angle;
    /* the origin is the texture top-left */
    const struct kywc_fbox *src;
    /* transform applied to the source texture */
    enum wl_output_transform transform;

    /**
     * blur region don't support rotation
     * the origin is the top-left of surface in logical coord
     */
    struct {
        const struct blur_info *info;
        const float *alpha;
        /* the magnification factor of the blur area in the texture */
        const float *scale;
        /**
         * round corner radius on the tex_dst.
         * if blur is in this round corner region, blur don't be rendered.
         * 0 = right-bottom, 1 = right-top, 2 = left-bottom, 3 = left-top
         */
        const int (*radius)[4];
        /* logical coord of the blur area in the texture, not scale */
        int offset_x, offset_y;
    } blur;
};

/**
 * translate logical coord box to render target buffer coord
 */
void scene_render_box(struct wlr_box *box, struct scene_render_target *target);

void scene_render_region(pixman_region32_t *region, struct scene_render_target *target);

void scene_render_point(float *x, float *y, struct scene_render_target *target);

void scene_render_damage_in_target(struct scene *scene, struct scene_render_target *target);

void scene_render_target_box(const struct scene_render_target *target, struct kywc_box *box);

void scene_render_target_region(struct scene_render_target *target, pixman_region32_t *region);

void scene_render_target_add_software_cursors(struct scene_render_target *target);

/* blur region is logic coord */
void scene_render_target_add_blur(struct scene_render_target *target,
                                  const struct scene_render_blur_options *options);

void scene_render_target_add_texture_with_blur(struct scene_render_target *target,
                                               const struct scene_render_texture_options *opts);

#endif /* _SCENE_RENDER_H_ */
