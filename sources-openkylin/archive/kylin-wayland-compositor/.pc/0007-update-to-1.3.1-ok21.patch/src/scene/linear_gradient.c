// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "render/opengl.h"
#include "scene/linear_gradient.h"
#include "scene/render.h"
#include "scene_p.h"
#include "util/macros.h"
#include "util/matrix.h"

#include "linear_gradient_frag.h"
#include "linear_gradient_vert.h"

struct ky_color_stop {
    float offset;
    float color[4];
};

struct ky_scene_linear_gradient {
    struct ky_scene_rect rect;
    ky_scene_node_destroy_func_t node_destroy;
    float x0, y0, x1, y1;
    struct ky_color_stop color_stops[2];
};

// opengl render
static int32_t gl_shader = 0;

struct gl_shader_location {
    // vs
    GLint in_uv;
    GLint uv2ndc;
    // fs
    GLint start;
    GLint end;
    GLint color_stop0_offset;
    GLint color_stop0_color;
    GLint color_stop1_offset;
    GLint color_stop1_color;
    GLint aspect;
    GLint rounded_corner_radius;
};
static struct gl_shader_location gl_locations = { 0 };

static int create_opengl_shader(struct ky_opengl_renderer *renderer)
{
    GLuint prog = ky_opengl_create_program(renderer, linear_gradient_vert, linear_gradient_frag);
    if (prog <= 0) {
        return -1;
    }

    gl_locations.in_uv = glGetAttribLocation(prog, "inUV");
    gl_locations.uv2ndc = glGetUniformLocation(prog, "uv2ndc");
    gl_locations.start = glGetUniformLocation(prog, "start");
    gl_locations.end = glGetUniformLocation(prog, "end");
    gl_locations.color_stop0_offset = glGetUniformLocation(prog, "colorStops[0].offset");
    gl_locations.color_stop0_color = glGetUniformLocation(prog, "colorStops[0].color");
    gl_locations.color_stop1_offset = glGetUniformLocation(prog, "colorStops[1].offset");
    gl_locations.color_stop1_color = glGetUniformLocation(prog, "colorStops[1].color");
    gl_locations.aspect = glGetUniformLocation(prog, "aspect");
    gl_locations.rounded_corner_radius = glGetUniformLocation(prog, "roundedCornerRadius");

    return prog;
}

static void scene_linear_gradient_opengl_render(struct ky_scene_linear_gradient *linear_gradient,
                                                int lx, int ly, bool render_with_visibility,
                                                struct ky_scene_render_target *target)
{
    struct ky_scene_node *node = &linear_gradient->rect.node;
    struct kywc_box geo = { lx, ly, linear_gradient->rect.width, linear_gradient->rect.height };

    pixman_region32_t render_region;
    if (render_with_visibility) {
        pixman_region32_init(&render_region);
        pixman_region32_intersect(&render_region, &node->visible_region, &target->damage);
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
    ky_scene_render_box(&dst_box, target);

    pixman_region32_translate(&render_region, -target->logical.x, -target->logical.y);
    ky_scene_render_region(&render_region, target);

    // batch opengl draw
    pixman_region32_t region;
    pixman_region32_init_rect(&region, dst_box.x, dst_box.y, dst_box.width, dst_box.height);
    pixman_region32_intersect(&region, &region, &render_region);

    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(&region, &rects_len);
    if (rects_len == 0) {
        pixman_region32_fini(&region);
        pixman_region32_fini(&render_region);
        return;
    }

    GLfloat verts[rects_len * 6 * 2];
    size_t vert_index = 0;
    for (int i = 0; i < rects_len; i++) {
        const pixman_box32_t *rect = &rects[i];
        verts[vert_index++] = (GLfloat)(rect->x1 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y1 - dst_box.y) / dst_box.height;
        verts[vert_index++] = (GLfloat)(rect->x2 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y1 - dst_box.y) / dst_box.height;
        verts[vert_index++] = (GLfloat)(rect->x1 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y2 - dst_box.y) / dst_box.height;
        verts[vert_index++] = (GLfloat)(rect->x2 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y1 - dst_box.y) / dst_box.height;
        verts[vert_index++] = (GLfloat)(rect->x2 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y2 - dst_box.y) / dst_box.height;
        verts[vert_index++] = (GLfloat)(rect->x1 - dst_box.x) / dst_box.width;
        verts[vert_index++] = (GLfloat)(rect->y2 - dst_box.y) / dst_box.height;
    }

    struct ky_mat3 uv2ndc;
    struct kywc_box dst_kywc_box = {
        .x = dst_box.x,
        .y = dst_box.y,
        .width = dst_box.width,
        .height = dst_box.height,
    };
    ky_mat3_uvofbox_to_ndc(&uv2ndc, target->buffer->width, target->buffer->height, 0,
                           &dst_kywc_box);

    glEnable(GL_BLEND);
    glUseProgram(gl_shader);
    glEnableVertexAttribArray(gl_locations.in_uv);
    glVertexAttribPointer(gl_locations.in_uv, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glUniformMatrix3fv(gl_locations.uv2ndc, 1, GL_FALSE, uv2ndc.matrix);
    float x0 = dst_box.x + linear_gradient->x0 * dst_box.width;
    float y0 = dst_box.y + linear_gradient->y0 * dst_box.height;
    ky_scene_render_point(&x0, &y0, target);
    x0 = (x0 - dst_box.x) / dst_box.width;
    y0 = (y0 - dst_box.y) / dst_box.height;
    glUniform2f(gl_locations.start, x0, y0);
    float x1 = dst_box.x + linear_gradient->x1 * dst_box.width;
    float y1 = dst_box.y + linear_gradient->y1 * dst_box.height;
    ky_scene_render_point(&x1, &y1, target);
    x1 = (x1 - dst_box.x) / dst_box.width;
    y1 = (y1 - dst_box.y) / dst_box.height;
    glUniform2f(gl_locations.end, x1, y1);
    glUniform1f(gl_locations.color_stop0_offset, linear_gradient->color_stops[0].offset);
    glUniform1f(gl_locations.color_stop1_offset, linear_gradient->color_stops[1].offset);
    glUniform4fv(gl_locations.color_stop0_color, 1, linear_gradient->color_stops[0].color);
    glUniform4fv(gl_locations.color_stop1_color, 1, linear_gradient->color_stops[1].color);
    float width = dst_box.width, height = dst_box.height;
    glUniform1f(gl_locations.aspect, width / height);
    bool render_with_radius = !(target->options & KY_SCENE_RENDER_DISABLE_ROUND_CORNER);
    float one_pixel_distance = 1.0f / height * 2.0f;
    float round_corner_radius[4] = {
        render_with_radius ? node->radius[0] * target->scale * one_pixel_distance : 0,
        render_with_radius ? node->radius[1] * target->scale * one_pixel_distance : 0,
        render_with_radius ? node->radius[2] * target->scale * one_pixel_distance : 0,
        render_with_radius ? node->radius[3] * target->scale * one_pixel_distance : 0
    };
    glUniform4f(gl_locations.rounded_corner_radius, round_corner_radius[0], round_corner_radius[1],
                round_corner_radius[2], round_corner_radius[3]);
    glDrawArrays(GL_TRIANGLES, 0, rects_len * 6);
    glUseProgram(0);
    glDisableVertexAttribArray(gl_locations.in_uv);

    pixman_region32_fini(&region);
    pixman_region32_fini(&render_region);
}

static void scene_linear_gradient_render(struct ky_scene_node *node, int lx, int ly,
                                         struct ky_scene_render_target *target)
{
    if (!node->enabled) {
        return;
    }

    bool render_with_visibility = !(target->options & KY_SCENE_RENDER_DISABLE_VISIBILITY);
    if (render_with_visibility && !pixman_region32_not_empty(&node->visible_region)) {
        return;
    }

    struct ky_scene_linear_gradient *linear_gradient = ky_scene_linear_gradientn_from_node(node);
    struct ky_scene_rect *rect = ky_scene_rect_from_linear_gradient(linear_gradient);

    if (!ky_scene_rect_render(node, (struct kywc_box){ lx, ly, rect->width, rect->height },
                              rect->color, render_with_visibility, target)) {
        return;
    }

    if (COLOR_INVALID(linear_gradient->color_stops[0].color) ||
        COLOR_INVALID(linear_gradient->color_stops[1].color)) {
        return;
    }

    if (gl_shader >= 0 && wlr_renderer_is_opengl(target->output->output->renderer)) {
        if (gl_shader == 0) {
            struct ky_opengl_renderer *renderer =
                ky_opengl_renderer_from_wlr_renderer(target->output->output->renderer);
            gl_shader = create_opengl_shader(renderer);
        }
        if (gl_shader > 0) {
            scene_linear_gradient_opengl_render(linear_gradient, lx, ly, render_with_visibility,
                                                target);
        } else {
            gl_shader = -1;
        }
    }
}

static void scene_linear_gradient_destroy(struct ky_scene_node *node)
{
    if (!node) {
        return;
    }

    struct ky_scene_linear_gradient *linear_gradient = ky_scene_linear_gradientn_from_node(node);
    linear_gradient->node_destroy(node);
}

struct ky_scene_linear_gradient *
ky_scene_linear_gradient_create(struct ky_scene_tree *parent, int width, int height,
                                const float background_color[static 4])
{
    struct ky_scene_linear_gradient *linear_gradient = calloc(1, sizeof(*linear_gradient));
    if (!linear_gradient) {
        return NULL;
    }

    ky_scene_rect_init(&linear_gradient->rect, parent, width, height, background_color);
    linear_gradient->node_destroy = linear_gradient->rect.node.impl.destroy;
    linear_gradient->rect.node.impl.destroy = scene_linear_gradient_destroy;
    linear_gradient->rect.node.impl.render = scene_linear_gradient_render;

    return linear_gradient;
}

struct ky_scene_node *
ky_scene_node_from_linear_gradient(struct ky_scene_linear_gradient *linear_gradient)
{
    return &linear_gradient->rect.node;
}

struct ky_scene_rect *
ky_scene_rect_from_linear_gradient(struct ky_scene_linear_gradient *linear_gradient)
{
    return &linear_gradient->rect;
}

struct ky_scene_linear_gradient *ky_scene_linear_gradientn_from_node(struct ky_scene_node *node)
{
    struct ky_scene_rect *rect = ky_scene_rect_from_node(node);
    struct ky_scene_linear_gradient *linear_gradient = wl_container_of(rect, linear_gradient, rect);
    return linear_gradient;
}

void ky_scene_linear_gradient_set_background_color(struct ky_scene_linear_gradient *linear_gradient,
                                                   const float color[static 4])
{
    ky_scene_rect_set_color(&linear_gradient->rect, color);
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

void ky_scene_linear_gradient_set_linear(struct ky_scene_linear_gradient *linear_gradient,
                                         float degree)
{
    if (gl_shader < 0) {
        return;
    }

    float aspect = (float)linear_gradient->rect.width / (float)linear_gradient->rect.height;
    float radian = (90.0f - degree) * M_PI / 180.0f;
    float dir_x = cosf(radian), dir_y = -sinf(radian) / aspect;

    float x0, y0, x1, y1;
    rect_center_ray_intersect(dir_x, dir_y, &x1, &y1);
    rect_center_ray_intersect(-dir_x, -dir_y, &x0, &y0);

    ky_scene_linear_gradient_set_linear_points(linear_gradient, x0, y0, x1, y1);
}

void ky_scene_linear_gradient_set_linear_points(struct ky_scene_linear_gradient *linear_gradient,
                                                float x0, float y0, float x1, float y1)
{
    if (gl_shader < 0) {
        return;
    }

    if (FLOAT_EQUAL(linear_gradient->x0, x0) && FLOAT_EQUAL(linear_gradient->y0, y0) &&
        FLOAT_EQUAL(linear_gradient->x1, x1) && FLOAT_EQUAL(linear_gradient->y1, y1)) {
        return;
    }

    linear_gradient->x0 = x0, linear_gradient->y0 = y0;
    linear_gradient->x1 = x1, linear_gradient->y1 = y1;
    ky_scene_node_push_damage(&linear_gradient->rect.node, KY_SCENE_DAMAGE_HARMLESS, NULL);
}

static void scene_linear_gradient_set_color_stop(struct ky_scene_linear_gradient *linear_gradient,
                                                 struct ky_color_stop *color_stop, float offset,
                                                 const float color[static 4])
{
    if (color_stop->offset == offset &&
        memcmp(color_stop->color, color, sizeof(color_stop->color)) == 0) {
        return;
    }

    color_stop->offset = offset;
    memcpy(color_stop->color, color, sizeof(color_stop->color));
    ky_scene_node_push_damage(&linear_gradient->rect.node, KY_SCENE_DAMAGE_HARMLESS, NULL);
}

void ky_scene_linear_gradient_set_color_stop_0(struct ky_scene_linear_gradient *linear_gradient,
                                               float offset, const float color[static 4])
{
    if (gl_shader < 0) {
        return;
    }

    struct ky_color_stop *color_stop = &linear_gradient->color_stops[0];
    scene_linear_gradient_set_color_stop(linear_gradient, color_stop, offset, color);
}

void ky_scene_linear_gradient_set_color_stop_1(struct ky_scene_linear_gradient *linear_gradient,
                                               float offset, const float color[static 4])
{
    if (gl_shader < 0) {
        return;
    }

    struct ky_color_stop *color_stop = &linear_gradient->color_stops[1];
    scene_linear_gradient_set_color_stop(linear_gradient, color_stop, offset, color);
}
