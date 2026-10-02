// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_output.h>

#include "render/opengl.h"
#include "render/pass.h"
#include "render/profile.h"
#include "scene/render.h"
#include "scene_p.h"

struct scene_rect *scene_rect_from_node(struct scene_node *node)
{
    assert(node->type == SCENE_NODE_RECT);
    struct scene_rect *rect = wl_container_of(node, rect, node);
    return rect;
};

static struct scene_node *rect_accept_input(struct scene_node *node, int lx, int ly, double px,
                                            double py, double *rx, double *ry)
{
    /* skip disabled or input bypassed nodes */
    if (!node->enabled || node->input_bypassed) {
        return NULL;
    }

    struct scene_rect *rect = scene_rect_from_node(node);
    struct kywc_box box = { floor(px), floor(py), 1, 1 };
    struct kywc_box node_box = { lx, ly, rect->width, rect->height };
    if (!kywc_box_intersects(&node_box, &box)) {
        return NULL;
    }

    if (pixman_region32_not_empty(&node->input_region) &&
        !pixman_region32_contains_point(&node->input_region, box.x - lx, box.y - ly, NULL)) {
        return NULL;
    }

    *rx = px - lx;
    *ry = py - ly;
    return node;
}

static void rect_update_outputs(struct scene_node *node, int lx, int ly, struct wl_list *outputs,
                                struct scene_output *ignore, struct scene_output *force)
{
    // Do nothing
}

static void rect_get_opaque_region(struct scene_node *node, pixman_region32_t *opaque)
{
    pixman_region32_clear(opaque);
    if (!node->enabled) {
        return;
    }

    struct scene_rect *rect = scene_rect_from_node(node);
    if (rect->color[3] != 1) {
        return;
    }

    pixman_region32_union_rect(opaque, opaque, 0, 0, rect->width, rect->height);
    if (pixman_region32_not_empty(&node->clip_region)) {
        pixman_region32_intersect(opaque, opaque, &node->clip_region);
    }

    /* subtract (round corners + anti-aliasing edge) */
    pixman_region32_t corner;
    pixman_region32_init(&corner);
    scene_corner_region(&corner, rect->width, rect->height, node->radius);
    pixman_region32_subtract(opaque, opaque, &corner);
    pixman_region32_fini(&corner);
}

static void rect_collect_invisible(struct scene_rect *rect, struct kywc_box *ly_box,
                                   pixman_region32_t *invisible)
{
    pixman_region32_t opaque;
    pixman_region32_init(&opaque);
    rect_get_opaque_region(&rect->node, &opaque);
    if (!pixman_region32_not_empty(&opaque)) {
        pixman_region32_fini(&opaque);
        return;
    }

    pixman_region32_translate(&opaque, ly_box->x, ly_box->y);
    pixman_region32_union(invisible, invisible, &opaque);
    pixman_region32_fini(&opaque);
}

static void rect_collect_damage(struct scene_node *node, int lx, int ly, bool parent_enabled,
                                uint32_t damage_type, pixman_region32_t *damage,
                                pixman_region32_t *invisible, pixman_region32_t *affected)
{
    bool node_enabled = parent_enabled && node->enabled;
    /* node is still disabled, skip it */
    if (!node_enabled && !node->last_enabled) {
        node->damage_type = SCENE_DAMAGE_NONE;
        return;
    }

    struct scene_rect *rect = scene_rect_from_node(node);
    // if node state is changed, it must in the affected region
    if (rect->width > 0 && rect->height > 0 &&
        pixman_region32_contains_rectangle(
            affected, &(pixman_box32_t){ lx, ly, lx + rect->width, ly + rect->height }) ==
            PIXMAN_REGION_OUT) {
        if (node_enabled) {
            rect_collect_invisible(rect, &(struct kywc_box){ lx, ly, rect->width, rect->height },
                                   invisible);
        }
        return;
    }

    /**
     * we may need to do 3 things:
     * 1. add node damage region to the scene collected_damage
     * 2. update node visible region
     * 3. add node opaque region to the scene collected_invisible
     */

    // no damage if node state is not changed
    bool no_damage = node->last_enabled && node_enabled && damage_type == SCENE_DAMAGE_NONE;
    if (!no_damage) {
        /* node last visible region is added to damgae */
        if (node->last_enabled && (!node_enabled || (damage_type & SCENE_DAMAGE_HARMFUL))) {
            pixman_region32_union(damage, damage, &node->visible_region);
        }
    }

    // update node visible region always
    pixman_region32_clear(&node->visible_region);

    if (node_enabled && rect->color[3] != 0) {
        // current visible region
        bool has_clip_region = pixman_region32_not_empty(&node->clip_region);
        if (has_clip_region) {
            pixman_region32_intersect_rect(&node->visible_region, &node->clip_region, 0, 0,
                                           rect->width, rect->height);
            pixman_region32_translate(&node->visible_region, lx, ly);
        } else {
            pixman_region32_init_rect(&node->visible_region, lx, ly, rect->width, rect->height);
        }
        pixman_region32_subtract(&node->visible_region, &node->visible_region, invisible);

        if (!no_damage) {
            pixman_region32_union(damage, damage, &node->visible_region);
        }

        rect_collect_invisible(rect, &(struct kywc_box){ lx, ly, rect->width, rect->height },
                               invisible);
    }

    node->last_enabled = node_enabled;
    node->damage_type = SCENE_DAMAGE_NONE;
}

bool scene_rect_render(struct scene_node *node, struct kywc_box geo, float color[4],
                       bool render_with_visibility, struct scene_render_target *target)
{
    pixman_region32_t render_region;
    if (render_with_visibility) {
        pixman_region32_init(&render_region);
        pixman_region32_union(&render_region, &node->visible_region, &node->extend_render_region);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    } else {
        pixman_region32_init_rect(&render_region, 0, 0, geo.width, geo.height);
        if (pixman_region32_not_empty(&node->clip_region)) {
            pixman_region32_intersect(&render_region, &render_region, &node->clip_region);
        }
        pixman_region32_translate(&render_region, geo.x, geo.y);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    }

    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return false;
    }

    struct wlr_box dst_box = {
        .x = geo.x - target->logical.x,
        .y = geo.y - target->logical.y,
        .width = geo.width,
        .height = geo.height,
    };
    scene_render_box(&dst_box, target);

    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
    scene_render_region(&render_region, target);

    PROFILE_RENDER_ZONE(render_pass_get_renderer(target->render_pass), gzone, __func__);
    bool render_with_radius = !(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER);
    struct render_rect_options options = {
        .base = {
            .box = dst_box,
            .color = {
                .r = color[0],
                .g = color[1],
                .b = color[2],
                .a = color[3],
            },
            .clip = &render_region,
            .blend_mode = color[3] != 1 ?
                WLR_RENDER_BLEND_MODE_PREMULTIPLIED : WLR_RENDER_BLEND_MODE_NONE,
        },
        .transform = target->transform,
        .radius = {
            .rb = render_with_radius ? node->radius[0] * target->scale : 0,
            .rt = render_with_radius ? node->radius[1] * target->scale : 0,
            .lb = render_with_radius ? node->radius[2] * target->scale : 0,
            .lt = render_with_radius ? node->radius[3] * target->scale : 0,
        },
    };

    if (!(target->options & SCENE_RENDER_DISABLE_BLUR)) {
        struct scene_render_blur_options opts = {
            .node_lx = geo.x,
            .node_ly = geo.x,
            .round_mask = {
                .boundary = &(struct kywc_box){ dst_box.x, dst_box.y, dst_box.width, dst_box.height,},
                .radius = &options.radius,
            },
            .clip = &render_region,
            .blur = node->has_blur ? &node->blur : NULL,
        };
        scene_render_target_add_blur(target, &opts);
    }

    render_pass_add_rect(target->render_pass, &options);

    pixman_region32_fini(&render_region);
    PROFILE_RENDER_ZONE_END(render_pass_get_renderer(target->render_pass));

    return true;
}

static void rect_render(struct scene_node *node, int lx, int ly, struct scene_render_target *target)
{
    if (!node->enabled) {
        return;
    }

    struct scene_rect *rect = scene_rect_from_node(node);
    if (rect->color[3] == 0) {
        return;
    }

    bool render_with_visibility = !(target->options & SCENE_RENDER_DISABLE_VISIBILITY);
    if (render_with_visibility && !pixman_region32_not_empty(&node->visible_region) &&
        !pixman_region32_not_empty(&node->extend_render_region)) {
        return;
    }

    scene_rect_render(node, (struct kywc_box){ lx, ly, rect->width, rect->height }, rect->color,
                      render_with_visibility, target);
}

static void rect_get_bounding_box(struct scene_node *node, struct wlr_box *box)
{
    struct scene_rect *rect = scene_rect_from_node(node);
    *box = (struct wlr_box){ 0, 0, rect->width, rect->height };
}

static void rect_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }

    struct scene_rect *rect = scene_rect_from_node(node);
    if (node->last_enabled) {
        struct scene *scene = scene_from_node(node);
        scene_add_damage(scene, &node->visible_region);
    }

    rect->node_destroy(node);
}

void scene_rect_init(struct scene_rect *rect, struct scene_tree *parent, int width, int height,
                     const float color[static 4])
{
    *rect = (struct scene_rect){ 0 };
    scene_node_init(&rect->node, parent);

    rect->node.type = SCENE_NODE_RECT;

    rect->node_destroy = rect->node.impl.destroy;
    rect->node.impl.destroy = rect_destroy;

    rect->node.impl.accept_input = rect_accept_input;
    rect->node.impl.update_outputs = rect_update_outputs;
    rect->node.impl.collect_damage = rect_collect_damage;
    rect->node.impl.get_opaque_region = rect_get_opaque_region;
    rect->node.impl.render = rect_render;
    rect->node.impl.get_bounding_box = rect_get_bounding_box;

    rect->width = width;
    rect->height = height;
    memcpy(rect->color, color, sizeof(rect->color));
}

struct scene_rect *scene_rect_create(struct scene_tree *parent, int width, int height,
                                     const float color[static 4])
{
    struct scene_rect *scene_rect = calloc(1, sizeof(*scene_rect));
    if (!scene_rect) {
        return NULL;
    }

    scene_rect_init(scene_rect, parent, width, height, color);
    scene_node_push_damage(&scene_rect->node, SCENE_DAMAGE_HARMFUL, NULL);

    return scene_rect;
}

void scene_rect_set_size(struct scene_rect *rect, int width, int height)
{
    if (rect->width == width && rect->height == height) {
        return;
    }

    bool update_later = false;
    if ((rect->width > width || rect->height > height)) {
        scene_node_push_damage(&rect->node, SCENE_DAMAGE_BOTH, NULL);
    }
    if (rect->width < width || rect->height < height) {
        update_later = true;
    }

    rect->width = width;
    rect->height = height;

    if (update_later) {
        scene_node_push_damage(&rect->node, SCENE_DAMAGE_BOTH, NULL);
    }
}

void scene_rect_set_color(struct scene_rect *rect, const float color[static 4])
{
    if (memcmp(rect->color, color, sizeof(rect->color)) == 0) {
        return;
    }

    bool harmful = rect->color[3] == 0 || color[3] == 0 || rect->color[3] == 1 || color[3] == 1;
    memcpy(rect->color, color, sizeof(rect->color));

    scene_node_push_damage(&rect->node, harmful ? SCENE_DAMAGE_BOTH : SCENE_DAMAGE_HARMLESS, NULL);
}
