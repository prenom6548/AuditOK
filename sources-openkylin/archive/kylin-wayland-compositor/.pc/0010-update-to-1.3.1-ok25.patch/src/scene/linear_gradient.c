// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "render/opengl.h"
#include "scene/linear_gradient.h"
#include "scene/render.h"
#include "scene_p.h"
#include "util/macros.h"

struct scene_linear_gradient {
    struct scene_rect rect;
    scene_node_destroy_func_t node_destroy;
    struct render_linear_gradient linear_gradient;
};

static void scene_linear_gradient_opengl_render(struct scene_linear_gradient *linear_gradient,
                                                int lx, int ly, bool render_with_visibility,
                                                struct scene_render_target *target)
{
    struct scene_node *node = &linear_gradient->rect.node;
    struct kywc_box geo = { lx, ly, linear_gradient->rect.width, linear_gradient->rect.height };

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

    struct wlr_box dst_box = {
        .x = geo.x - target->logical.x,
        .y = geo.y - target->logical.y,
        .width = geo.width,
        .height = geo.height,
    };
    float x0 = dst_box.x + linear_gradient->linear_gradient.x0 * dst_box.width;
    float y0 = dst_box.y + linear_gradient->linear_gradient.y0 * dst_box.height;
    scene_render_point(&x0, &y0, target);
    float x1 = dst_box.x + linear_gradient->linear_gradient.x1 * dst_box.width;
    float y1 = dst_box.y + linear_gradient->linear_gradient.y1 * dst_box.height;
    scene_render_point(&x1, &y1, target);
    scene_render_box(&dst_box, target);

    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
    scene_render_region(&render_region, target);

    pixman_region32_t region;
    pixman_region32_init_rect(&region, dst_box.x, dst_box.y, dst_box.width, dst_box.height);
    pixman_region32_intersect(&region, &region, &render_region);

    bool render_with_radius = !(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER);
    render_pass_add_rect(target->render_pass, &(struct render_rect_options){
        .base = {
            .box = dst_box,
            .clip = &region,
            .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
        },
        .radius = {
            .rb = render_with_radius ? node->radius[0] * target->scale : 0,
            .rt = render_with_radius ? node->radius[1] * target->scale : 0,
            .lb = render_with_radius ? node->radius[2] * target->scale : 0,
            .lt = render_with_radius ? node->radius[3] * target->scale : 0,
        },
        .linear_gradient = &linear_gradient->linear_gradient,
    });

    pixman_region32_fini(&region);
    pixman_region32_fini(&render_region);
}

static void scene_linear_gradient_render(struct scene_node *node, int lx, int ly,
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

    struct scene_linear_gradient *linear_gradient = scene_linear_gradient_from_node(node);
    struct scene_rect *rect = scene_rect_from_linear_gradient(linear_gradient);

    if (!scene_rect_render(node, (struct kywc_box){ lx, ly, rect->width, rect->height },
                           rect->color, render_with_visibility, target)) {
        return;
    }

    if (COLOR_INVALID(linear_gradient->linear_gradient.color_stops[0].color) ||
        COLOR_INVALID(linear_gradient->linear_gradient.color_stops[1].color)) {
        return;
    }

    if (wlr_renderer_is_opengl(target->output->output->renderer)) {
        scene_linear_gradient_opengl_render(linear_gradient, lx, ly, render_with_visibility,
                                            target);
    }
}

static void scene_linear_gradient_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }

    struct scene_linear_gradient *linear_gradient = scene_linear_gradient_from_node(node);
    linear_gradient->node_destroy(node);
}

struct scene_linear_gradient *scene_linear_gradient_create(struct scene_tree *parent, int width,
                                                           int height,
                                                           const float background_color[static 4])
{
    struct scene_linear_gradient *linear_gradient = calloc(1, sizeof(*linear_gradient));
    if (!linear_gradient) {
        return NULL;
    }

    scene_rect_init(&linear_gradient->rect, parent, width, height, background_color);
    linear_gradient->node_destroy = linear_gradient->rect.node.impl.destroy;
    linear_gradient->rect.node.impl.destroy = scene_linear_gradient_destroy;
    linear_gradient->rect.node.impl.render = scene_linear_gradient_render;

    return linear_gradient;
}

struct scene_node *scene_node_from_linear_gradient(struct scene_linear_gradient *linear_gradient)
{
    return &linear_gradient->rect.node;
}

struct scene_rect *scene_rect_from_linear_gradient(struct scene_linear_gradient *linear_gradient)
{
    return &linear_gradient->rect;
}

struct scene_linear_gradient *scene_linear_gradient_from_node(struct scene_node *node)
{
    struct scene_rect *rect = scene_rect_from_node(node);
    struct scene_linear_gradient *linear_gradient = wl_container_of(rect, linear_gradient, rect);
    return linear_gradient;
}

void scene_linear_gradient_set_background_color(struct scene_linear_gradient *linear_gradient,
                                                const float color[static 4])
{
    scene_rect_set_color(&linear_gradient->rect, color);
}

static void rect_center_ray_intersect(float dir_x, float dir_y, float *x, float *y)
{
    const float center_x = 0.5f, center_y = 0.5f;
    const float right = 1.0f, left = 0.0f, top = 1.0f, bottom = 0.0f;

    float t_x_positive = (right - center_x) / dir_x;
    float t_x_negative = (left - center_x) / dir_x;
    float t_y_positive = (top - center_y) / dir_y;
    float t_y_negative = (bottom - center_y) / dir_y;

    float t = INFINITY;
    if (dir_x > 0.0f) {
        t = fminf(t, t_x_positive);
    } else if (dir_x < 0.0f) {
        t = fminf(t, t_x_negative);
    }

    if (dir_y > 0.0f) {
        t = fminf(t, t_y_positive);
    } else if (dir_y < 0.0f) {
        t = fminf(t, t_y_negative);
    }

    *x = center_x + dir_x * t;
    *y = center_y + dir_y * t;
}

void scene_linear_gradient_set_linear(struct scene_linear_gradient *linear_gradient, float degree)
{
    float aspect = (float)linear_gradient->rect.width / (float)linear_gradient->rect.height;
    float radian = (90.0f - degree) * M_PI / 180.0f;
    float dir_x = cosf(radian), dir_y = -sinf(radian) / aspect;

    float x0, y0, x1, y1;
    rect_center_ray_intersect(dir_x, dir_y, &x1, &y1);
    rect_center_ray_intersect(-dir_x, -dir_y, &x0, &y0);

    scene_linear_gradient_set_linear_points(linear_gradient, x0, y0, x1, y1);
}

void scene_linear_gradient_set_linear_points(struct scene_linear_gradient *linear_gradient,
                                             float x0, float y0, float x1, float y1)
{
    if (FLOAT_EQUAL(linear_gradient->linear_gradient.x0, x0) &&
        FLOAT_EQUAL(linear_gradient->linear_gradient.y0, y0) &&
        FLOAT_EQUAL(linear_gradient->linear_gradient.x1, x1) &&
        FLOAT_EQUAL(linear_gradient->linear_gradient.y1, y1)) {
        return;
    }

    linear_gradient->linear_gradient.x0 = x0, linear_gradient->linear_gradient.y0 = y0;
    linear_gradient->linear_gradient.x1 = x1, linear_gradient->linear_gradient.y1 = y1;
    scene_node_push_damage(&linear_gradient->rect.node, SCENE_DAMAGE_HARMLESS, NULL);
}

static void scene_linear_gradient_set_color_stop(struct scene_linear_gradient *linear_gradient,
                                                 struct render_color_stop *color_stop, float offset,
                                                 const float color[static 4])
{
    if (color_stop->offset == offset &&
        memcmp(color_stop->color, color, sizeof(color_stop->color)) == 0) {
        return;
    }

    color_stop->offset = offset;
    memcpy(color_stop->color, color, sizeof(color_stop->color));
    scene_node_push_damage(&linear_gradient->rect.node, SCENE_DAMAGE_HARMLESS, NULL);
}

void scene_linear_gradient_set_color_stop_0(struct scene_linear_gradient *linear_gradient,
                                            float offset, const float color[static 4])
{
    struct render_color_stop *color_stop = &linear_gradient->linear_gradient.color_stops[0];
    scene_linear_gradient_set_color_stop(linear_gradient, color_stop, offset, color);
}

void scene_linear_gradient_set_color_stop_1(struct scene_linear_gradient *linear_gradient,
                                            float offset, const float color[static 4])
{
    struct render_color_stop *color_stop = &linear_gradient->linear_gradient.color_stops[1];
    scene_linear_gradient_set_color_stop(linear_gradient, color_stop, offset, color);
}
