// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <string.h>

#include "render/pass.h"
#include "scene/render.h"
#include "scene/stroke_rect.h"
#include "scene_p.h"

struct scene_stroke_rect {
    struct scene_rect rect;
    scene_node_destroy_func_t node_destroy;

    int radius[4];
    int stroke_width;
};

/**
 * update input region and clip region
 */
static void stroke_rect_update_region(struct scene_stroke_rect *scene_stroke_rect)
{
    pixman_region32_t region;
    pixman_region32_init(&region);

    int width = scene_stroke_rect->rect.width;
    int height = scene_stroke_rect->rect.height;

    if (width > 0 && height > 0) {
        pixman_region32_init_rect(&region, 0, 0, width, height);

        int off = scene_stroke_rect->stroke_width;
        pixman_region32_t reg;
        pixman_region32_init_rect(&reg, off, off, width - 2 * off, height - 2 * off);
        pixman_region32_subtract(&region, &region, &reg);
        pixman_region32_fini(&reg);

        // left-top
        if (scene_stroke_rect->radius[3] > 0) {
            pixman_region32_union_rect(&region, &region, 0, 0, scene_stroke_rect->radius[3],
                                       scene_stroke_rect->radius[3]);
        }
        // right-top
        if (scene_stroke_rect->radius[1] > 0) {
            pixman_region32_union_rect(&region, &region, width - scene_stroke_rect->radius[1], 0,
                                       scene_stroke_rect->radius[1], scene_stroke_rect->radius[1]);
        }
        // left-bottom
        if (scene_stroke_rect->radius[2] > 0) {
            pixman_region32_union_rect(&region, &region, 0, height - scene_stroke_rect->radius[2],
                                       scene_stroke_rect->radius[2], scene_stroke_rect->radius[2]);
        }
        // right-bottom
        if (scene_stroke_rect->radius[0] > 0) {
            pixman_region32_union_rect(&region, &region, width - scene_stroke_rect->radius[0],
                                       height - scene_stroke_rect->radius[0],
                                       scene_stroke_rect->radius[0], scene_stroke_rect->radius[0]);
        }
    }

    scene_node_set_input_region(&scene_stroke_rect->rect.node, &region);
    // affect damage region
    scene_node_set_clip_region(&scene_stroke_rect->rect.node, &region);

    pixman_region32_fini(&region);
}

static void stroke_rect_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }

    struct scene_stroke_rect *stroke_rect = scene_stroke_rect_from_node(node);
    stroke_rect->node_destroy(node);
}

static void stroke_rect_render(struct scene_node *node, int lx, int ly,
                               struct scene_render_target *target)
{
    if (!node->enabled) {
        return;
    }

    bool render_with_visibility = !(target->options & SCENE_RENDER_DISABLE_VISIBILITY);
    if (render_with_visibility && !pixman_region32_not_empty(&node->visible_region) &&
        !pixman_region32_not_empty(&node->extend_render_region)) {
        return;
    }

    struct scene_stroke_rect *stroke_rect = scene_stroke_rect_from_node(node);

    struct kywc_box geo = { lx, ly, stroke_rect->rect.width, stroke_rect->rect.height };

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
        return;
    }
    scene_render_target_region(target, &render_region);

    struct wlr_box dst_box = {
        .x = geo.x - target->logical.x,
        .y = geo.y - target->logical.y,
        .width = geo.width,
        .height = geo.height,
    };
    scene_render_box(&dst_box, target);

    bool render_with_radius = !(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER);
    render_pass_add_rect(target->render_pass, &(struct render_rect_options){
        .base = {
            .box = dst_box,
            .clip = &render_region,
            .color = {
                stroke_rect->rect.color[0],
                stroke_rect->rect.color[1],
                stroke_rect->rect.color[2],
                stroke_rect->rect.color[3],
            },
        },
        .radius = {
            .rb = render_with_radius ? stroke_rect->radius[0] * target->scale : 0,
            .rt = render_with_radius ? stroke_rect->radius[1] * target->scale : 0,
            .lb = render_with_radius ? stroke_rect->radius[2] * target->scale : 0,
            .lt = render_with_radius ? stroke_rect->radius[3] * target->scale : 0,
        },
        .stroke_width = stroke_rect->stroke_width * target->scale,
    });
}

struct scene_stroke_rect *scene_stroke_rect_create(struct scene_tree *parent, int width, int height,
                                                   const float color[static 4], int stroke_width)
{
    struct scene_stroke_rect *stroke_rect = calloc(1, sizeof(*stroke_rect));
    if (!stroke_rect) {
        return NULL;
    }

    scene_rect_init(&stroke_rect->rect, parent, width, height, color);
    stroke_rect->node_destroy = stroke_rect->rect.node.impl.destroy;
    stroke_rect->rect.node.impl.destroy = stroke_rect_destroy;
    stroke_rect->rect.node.impl.render = stroke_rect_render;

    stroke_rect->stroke_width = stroke_width;
    stroke_rect_update_region(stroke_rect);

    return stroke_rect;
}

struct scene_node *scene_node_from_stroke_rect(struct scene_stroke_rect *stroke_rect)
{
    return &stroke_rect->rect.node;
}

struct scene_rect *scene_rect_from_stroke_rect(struct scene_stroke_rect *stroke_rect)
{
    return &stroke_rect->rect;
}

struct scene_stroke_rect *scene_stroke_rect_from_node(struct scene_node *node)
{
    struct scene_rect *rect = scene_rect_from_node(node);
    struct scene_stroke_rect *stroke_rect = wl_container_of(rect, stroke_rect, rect);
    return stroke_rect;
}

void scene_stroke_rect_set_color(struct scene_stroke_rect *stroke_rect, const float color[static 4])
{
    scene_rect_set_color(&stroke_rect->rect, color);
}

void scene_stroke_rect_set_size(struct scene_stroke_rect *stroke_rect, int width, int height)
{
    if (stroke_rect->rect.width == width && stroke_rect->rect.height == height) {
        return;
    }

    scene_rect_set_size(&stroke_rect->rect, width, height);
    stroke_rect_update_region(stroke_rect);
}

void scene_stroke_rect_set_stroke_width(struct scene_stroke_rect *stroke_rect, int width)
{
    if (stroke_rect->stroke_width == width) {
        return;
    }

    stroke_rect->stroke_width = width;
    stroke_rect_update_region(stroke_rect);
}

void scene_stroke_rect_set_radius(struct scene_stroke_rect *stroke_rect, const int radius[static 4])
{
    if (memcmp(stroke_rect->radius, radius, sizeof(stroke_rect->radius)) == 0) {
        return;
    }

    memcpy(stroke_rect->radius, radius, sizeof(stroke_rect->radius));
    stroke_rect_update_region(stroke_rect);
}
