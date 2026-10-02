// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <float.h>
#include <stdlib.h>

#include "render/opengl.h"
#include "render/profile.h"
#include "scene/decoration.h"
#include "scene/render.h"
#include "scene_p.h"
#include "theme.h"
#include "util/macros.h"
#include "util/matrix.h"

#include "decoration_frag.h"
#include "decoration_vert.h"
#include "shadow_frag.h"
#include "shadow_vert.h"

enum deco_update_cause {
    DECO_UPDATE_CAUSE_NONE = 0,
    DECO_UPDATE_CAUSE_SURFACE_SIZE = 1 << 0,
    DECO_UPDATE_CAUSE_MARGIN = 1 << 1,
    DECO_UPDATE_CAUSE_MARGIN_COLOR = 1 << 2,
    DECO_UPDATE_CAUSE_MASK = 1 << 3,
    DECO_UPDATE_CAUSE_CORNER_RADIUS = 1 << 4,
    DECO_UPDATE_CAUSE_BLURRED = 1 << 5,
    DECO_UPDATE_CAUSE_SURFACE_COLOR = 1 << 6,
};

struct scene_shadow {
    int offset_x;
    int offset_y;
    int spread;
    int blur;
    float color[4];
    struct kywc_box box;
};

struct scene_decoration {
    /* based on scene_rect */
    struct scene_rect rect;
    scene_node_destroy_func_t node_destroy;
    uint32_t pending_cause;

    /* mask for border, shadow, corner */
    uint32_t mask; // TODO: resize region ?

    // global opacity. applied to all color
    float opacity;
    enum decoration_border_type border_type;

    /* surface size */
    int surface_width;
    int surface_height;
    /* color in surface size */
    float surface_color[4];
    /* blurred in surface size */
    bool surface_blurred;

    /* margin */
    int border_thickness;
    int title_height;
    /* margin color */
    float border_color[4];
    float title_color[4];

    /* region used to resize */
    int resize_width;

    // window round rect
    // 0=right-bottom, 1=right-top, 2=left-bottom, 3=left-top
    int round_corner_radius[4];

    int shadows_size;
    struct scene_shadow shadows[THEME_MAX_SHADOW_LAYERS];

    /**
     * render_box = window_box + shadow_boxs
     * window_box = 0, 0, rect.width, rect.height
     */
    struct kywc_box render_box;

    /* surface sub inner border */
    pixman_region32_t surface_region;
    /* title union inner border */
    pixman_region32_t title_region;
    pixman_region32_t border_region;
    pixman_region32_t round_corner_region;
    /* clip region without round_corner_region */
    pixman_region32_t pure_clip_region;
    pixman_region32_t clip_region;
};

// opengl render
static struct {
    int32_t program;
    // vs
    GLint in_uv;
    GLint uv2ndc;
    GLint inverse_transform;
    // fs
    GLint anti_aliasing;
    GLint aspect;
    GLint rect;
    GLint round_radius;
    GLint border_thickness;
    GLint border_color;
    GLint title_height;
    GLint title_color;
} gl_shader = { 0 };

static struct {
    int32_t program;
    // vs
    GLint in_uv;
    GLint size;
    GLint uv2ndc;
    GLint inverse_transform;
    // fs
    GLint rect;
    GLint sigma;
    GLint round_radius;
    GLint color;
    GLint window_anti_aliasing;
    GLint window_aspect;
    GLint window_rect;
    GLint window_round_radius;
} gl_shader_shadow = { 0 };

static void scene_decoration_create_opengl_shader(struct opengl_renderer *renderer)
{
    gl_shader.program = opengl_create_program(renderer, decoration_vert, decoration_frag);
    if (gl_shader.program <= 0) {
        gl_shader.program = -1;
        gl_shader_shadow.program = -1;
        return;
    }

    gl_shader_shadow.program = opengl_create_program(renderer, shadow_vert, shadow_frag);
    if (gl_shader_shadow.program <= 0) {
        gl_shader.program = -1;
        gl_shader_shadow.program = -1;
        return;
    }

    gl_shader.in_uv = glGetAttribLocation(gl_shader.program, "inUV");
    gl_shader.uv2ndc = glGetUniformLocation(gl_shader.program, "uv2ndc");
    gl_shader.inverse_transform = glGetUniformLocation(gl_shader.program, "inverseTransform");

    gl_shader.anti_aliasing = glGetUniformLocation(gl_shader.program, "antiAliasing");
    gl_shader.aspect = glGetUniformLocation(gl_shader.program, "aspect");
    gl_shader.rect = glGetUniformLocation(gl_shader.program, "rect");
    gl_shader.round_radius = glGetUniformLocation(gl_shader.program, "roundRadius");
    gl_shader.border_thickness = glGetUniformLocation(gl_shader.program, "borderThickness");
    gl_shader.border_color = glGetUniformLocation(gl_shader.program, "borderColor");
    gl_shader.title_height = glGetUniformLocation(gl_shader.program, "titleHeight");
    gl_shader.title_color = glGetUniformLocation(gl_shader.program, "titleColor");

    gl_shader_shadow.in_uv = glGetAttribLocation(gl_shader_shadow.program, "inUV");
    gl_shader_shadow.uv2ndc = glGetUniformLocation(gl_shader_shadow.program, "uv2ndc");
    gl_shader_shadow.inverse_transform =
        glGetUniformLocation(gl_shader_shadow.program, "inverseTransform");
    gl_shader_shadow.size = glGetUniformLocation(gl_shader_shadow.program, "size");
    gl_shader_shadow.color = glGetUniformLocation(gl_shader_shadow.program, "color");
    gl_shader_shadow.sigma = glGetUniformLocation(gl_shader_shadow.program, "sigma");
    gl_shader_shadow.rect = glGetUniformLocation(gl_shader_shadow.program, "rect");
    gl_shader_shadow.round_radius = glGetUniformLocation(gl_shader_shadow.program, "roundRadius");
    gl_shader_shadow.window_anti_aliasing =
        glGetUniformLocation(gl_shader_shadow.program, "windowAntiAliasing");
    gl_shader_shadow.window_aspect = glGetUniformLocation(gl_shader_shadow.program, "windowAspect");
    gl_shader_shadow.window_rect = glGetUniformLocation(gl_shader_shadow.program, "windowRect");
    gl_shader_shadow.window_round_radius =
        glGetUniformLocation(gl_shader_shadow.program, "windowRoundRadius");
}

static void get_render_region_with_mask(struct scene_decoration *deco, int lx, int ly,
                                        struct scene_render_target *target, struct wlr_box *region)
{
    struct wlr_box render_box = {
        .x = lx - target->logical.x + deco->render_box.x,
        .y = ly - target->logical.y + deco->render_box.y,
        .width = deco->render_box.width,
        .height = deco->render_box.height,
    };

    struct wlr_box window_box = {
        .x = lx - target->logical.x,
        .y = ly - target->logical.y,
        .width = deco->rect.width,
        .height = deco->rect.height,
    };

    bool left_shadow_mask = deco->mask & DECORATION_MASK_LEFT;
    bool right_shadow_mask = deco->mask & DECORATION_MASK_RIGHT;
    if (left_shadow_mask && right_shadow_mask) {
        region->x = render_box.x;
        region->width = render_box.width;
    } else if (!left_shadow_mask && right_shadow_mask) {
        region->x = window_box.x;
        region->width = (render_box.x + render_box.width) - window_box.x;
    } else if (left_shadow_mask && !right_shadow_mask) {
        region->x = render_box.x;
        region->width = (window_box.x + window_box.width) - render_box.x;
    } else {
        region->x = window_box.x;
        region->width = window_box.width;
    } // x2 - x1
    bool top_shadow_mask = deco->mask & DECORATION_MASK_TOP;
    bool bottom_shadow_mask = deco->mask & DECORATION_MASK_BOTTOM;
    if (top_shadow_mask && bottom_shadow_mask) {
        region->y = render_box.y;
        region->height = render_box.height;
    } else if (!top_shadow_mask && bottom_shadow_mask) {
        region->y = window_box.y;
        region->height = (render_box.y + render_box.height) - window_box.y;
    } else if (top_shadow_mask && !bottom_shadow_mask) {
        region->y = render_box.y;
        region->height = (window_box.y + window_box.height) - render_box.y;
    } else {
        region->y = window_box.y;
        region->height = window_box.height;
    }

    scene_render_box(region, target);
}

static void
get_window_box_from_surface_box(struct scene_decoration *deco, struct scene_render_target *target,
                                int outer_border_width, int title_height, const struct wlr_box *box,
                                const struct kywc_box *surface_box, struct kywc_box *window_box)
{
    int outer_border_width2 = outer_border_width * 2;

    if (target->transform == WL_OUTPUT_TRANSFORM_90) {
        window_box->x = surface_box->y - outer_border_width - box->y;
        window_box->y = surface_box->x - title_height - outer_border_width - box->x;
        window_box->width = surface_box->height + outer_border_width2;
        window_box->height = surface_box->width + title_height + outer_border_width2;
        // rotation correct
        window_box->x = box->height - surface_box->height - outer_border_width2 - window_box->x;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_180) {
        window_box->x = surface_box->x - outer_border_width - box->x;
        window_box->y = surface_box->y - outer_border_width - box->y;
        window_box->width = surface_box->width + outer_border_width2;
        window_box->height = surface_box->height + title_height + outer_border_width2;
        // rotation correct
        window_box->x = box->width - surface_box->width - outer_border_width2 - window_box->x;
        window_box->y =
            box->height - surface_box->height - outer_border_width2 - title_height - window_box->y;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_270) {
        window_box->x = surface_box->y - outer_border_width - box->y;
        window_box->y = surface_box->x - outer_border_width - box->x;
        window_box->width = surface_box->height + outer_border_width2;
        window_box->height = surface_box->width + title_height + outer_border_width2;
        // rotation correct
        window_box->y =
            box->width - surface_box->width - outer_border_width2 - title_height - window_box->y;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_FLIPPED) {
        window_box->x = surface_box->x - outer_border_width - box->x;
        window_box->y = surface_box->y - title_height - outer_border_width - box->y;
        window_box->width = surface_box->width + outer_border_width2;
        window_box->height = surface_box->height + title_height + outer_border_width2;
        // rotation correct
        window_box->x = box->width - surface_box->width - outer_border_width2 - window_box->x;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_FLIPPED_90) {
        window_box->x = surface_box->y - outer_border_width - box->y;
        window_box->y = surface_box->x - title_height - outer_border_width - box->x;
        window_box->width = surface_box->height + outer_border_width2;
        window_box->height = surface_box->width + title_height + outer_border_width2;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_FLIPPED_180) {
        window_box->x = surface_box->x - outer_border_width - box->x;
        window_box->y = surface_box->y - outer_border_width - box->y;
        window_box->width = surface_box->width + outer_border_width2;
        window_box->height = surface_box->height + title_height + outer_border_width2;
        // rotation correct
        window_box->y =
            box->height - surface_box->height - outer_border_width2 - title_height - window_box->y;
    } else if (target->transform == WL_OUTPUT_TRANSFORM_FLIPPED_270) {
        window_box->x = surface_box->y - outer_border_width - box->y;
        window_box->y = surface_box->x - outer_border_width - box->x;
        window_box->width = surface_box->height + outer_border_width2;
        window_box->height = surface_box->width + title_height + outer_border_width2;
        // rotation correct
        window_box->x = box->height - surface_box->height - outer_border_width2 - window_box->x;
        window_box->y =
            box->width - surface_box->width - outer_border_width2 - title_height - window_box->y;
    } else {
        window_box->x = surface_box->x - outer_border_width - box->x;
        window_box->y = surface_box->y - title_height - outer_border_width - box->y;
        window_box->width = surface_box->width + outer_border_width2;
        window_box->height = surface_box->height + title_height + outer_border_width2;
    }
}

static void scene_decoration_opengl_render(struct scene_decoration *deco, int lx, int ly,
                                           struct scene_render_target *target,
                                           const struct wlr_box *box, const pixman_region32_t *clip)
{
    if (deco->opacity < 0.01f) {
        return;
    }

    struct wlr_box region_box = { 0 };
    get_render_region_with_mask(deco, lx, ly, target, &region_box);
    pixman_region32_t region;
    pixman_region32_init_rect(&region, region_box.x, region_box.y, region_box.width,
                              region_box.height);
    pixman_region32_intersect(&region, &region, clip);

    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(&region, &rects_len);
    if (rects_len == 0) {
        pixman_region32_fini(&region);
        return;
    }

    GLfloat verts[rects_len * 6 * 2];
    size_t vert_index = 0;
    for (int i = 0; i < rects_len; i++) {
        const pixman_box32_t *rect = &rects[i];
        verts[vert_index++] = (GLfloat)(rect->x1 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y1 - box->y) / box->height;
        verts[vert_index++] = (GLfloat)(rect->x2 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y1 - box->y) / box->height;
        verts[vert_index++] = (GLfloat)(rect->x1 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y2 - box->y) / box->height;
        verts[vert_index++] = (GLfloat)(rect->x2 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y1 - box->y) / box->height;
        verts[vert_index++] = (GLfloat)(rect->x2 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y2 - box->y) / box->height;
        verts[vert_index++] = (GLfloat)(rect->x1 - box->x) / box->width;
        verts[vert_index++] = (GLfloat)(rect->y2 - box->y) / box->height;
    }

    int outer_border_raw =
        deco->border_type == DECORATION_BORDER_TYPE_OUTER ? deco->border_thickness : 0;
    // surface rect as reference use framebuffer coord. avoid fractional-scale issuse
    struct kywc_box surface_box = {
        .x = lx + outer_border_raw,
        .y = ly + outer_border_raw + deco->title_height,
        .width = deco->surface_width,
        .height = deco->surface_height,
    };
    scene_render_target_box(target, &surface_box);

    float scale = target->scale;
    float width = box->width;
    float height = box->height;
    if (target->transform & WL_OUTPUT_TRANSFORM_90) {
        width = box->height;
        height = box->width;
    }
    // keep border ceil scale
    int border_thickness = floorf(deco->border_thickness * scale);
    int outer_border = deco->border_type == DECORATION_BORDER_TYPE_OUTER ? border_thickness : 0;
    int title_height = round(deco->title_height * scale);
    float half_height = height * 0.5f; // shader distance scale
    float round_corner_radius[4] = { 0 };
    if (!(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER)) {
        round_corner_radius[0] = deco->round_corner_radius[0] > 0
                                     ? deco->round_corner_radius[0] * scale + outer_border
                                     : 0.0f;
        round_corner_radius[1] = deco->round_corner_radius[1] > 0
                                     ? deco->round_corner_radius[1] * scale + outer_border
                                     : 0.0f;
        round_corner_radius[2] = deco->round_corner_radius[2] > 0
                                     ? deco->round_corner_radius[2] * scale + outer_border
                                     : 0.0f;
        round_corner_radius[3] = deco->round_corner_radius[3] > 0
                                     ? deco->round_corner_radius[3] * scale + outer_border
                                     : 0.0f;
    }
    struct kywc_box window_box = { 0 };
    get_window_box_from_surface_box(deco, target, outer_border, title_height, box, &surface_box,
                                    &window_box);
    struct kywc_box dst_box = {
        .x = box->x,
        .y = box->y,
        .width = box->width,
        .height = box->height,
    };
    struct mat3 uv2ndc;
    mat3_uvofbox_to_ndc(&uv2ndc, target->buffer->width, target->buffer->height, 0, &dst_box);

    struct mat3 inverseTransform;
    mat3_invert_output_transform(&inverseTransform, target->transform);

    float one_pixel_distance = 1.0 / half_height;
    // 1 pixel border thickness keep 1 pixel less soft aa
    float anti_aliasing = one_pixel_distance * 0.5f;
    float aspect = width / height;
    float width_distance = window_box.width / height;
    float height_distance = window_box.height / height;
    float offset_x_distance = width_distance * 0.5f + window_box.x / height;
    float offset_y_distance = height_distance * 0.5f + window_box.y / height;
    float round_radius[4] = {
        deco->mask & DECORATION_MASK_BOTTOM_RIGHT ? round_corner_radius[0] * one_pixel_distance
                                                  : 0.0f,
        deco->mask & DECORATION_MASK_TOP_RIGHT ? round_corner_radius[1] * one_pixel_distance : 0.0f,
        deco->mask & DECORATION_MASK_BOTTOM_LEFT ? round_corner_radius[2] * one_pixel_distance
                                                 : 0.0f,
        deco->mask & DECORATION_MASK_TOP_LEFT ? round_corner_radius[3] * one_pixel_distance : 0.0f
    };

    // round rect shadow canot specify every corner round_radius
    float shadow_round_radius = 0.f;
    if (deco->mask & DECORATION_MASK_BOTTOM_RIGHT) {
        shadow_round_radius = round_corner_radius[0];
    } else if (deco->mask & DECORATION_MASK_TOP_RIGHT) {
        shadow_round_radius = round_corner_radius[1];
    } else if (deco->mask & DECORATION_MASK_BOTTOM_LEFT) {
        shadow_round_radius = round_corner_radius[2];
    } else if (deco->mask & DECORATION_MASK_TOP_LEFT) {
        shadow_round_radius = round_corner_radius[3];
    }
    shadow_round_radius = MAX(shadow_round_radius, FLT_MIN);

    PROFILE_RENDER_ZONE(target->output->output->renderer, gzone, __func__);

    glEnable(GL_BLEND);
    glEnableVertexAttribArray(gl_shader.in_uv);
    glVertexAttribPointer(gl_shader.in_uv, 2, GL_FLOAT, GL_FALSE, 0, verts);

    // reverse multi draw shadow with shape mask
    glUseProgram(gl_shader_shadow.program);
    glUniformMatrix3fv(gl_shader_shadow.uv2ndc, 1, GL_FALSE, uv2ndc.matrix);
    glUniformMatrix3fv(gl_shader_shadow.inverse_transform, 1, GL_FALSE, inverseTransform.matrix);
    glUniform2f(gl_shader_shadow.size, width, height);
    for (int i = deco->shadows_size - 1; i >= 0; i--) {
        struct scene_shadow *shadow = &deco->shadows[i];
        int shadow_offset_x = roundf(shadow->offset_x * scale);
        int shadow_offset_y = roundf(shadow->offset_y * scale);
        float shadow_blur = shadow->blur * scale;
        // rect is before blur. box = rect expand spread
        struct wlr_box shadow_rect = {
            .x = window_box.x + shadow_offset_x - shadow->spread,
            .y = window_box.y + shadow_offset_y - shadow->spread,
            .width = window_box.width + shadow->spread * 2,
            .height = window_box.height + shadow->spread * 2,
        };

        glUniform4f(gl_shader_shadow.rect, shadow_rect.x, shadow_rect.y,
                    shadow_rect.x + shadow_rect.width, shadow_rect.y + shadow_rect.height);
        glUniform1f(gl_shader_shadow.round_radius, shadow_round_radius);
        // blur with to gaussian sigma. scale = 1.0 / (2.0 * sqrt(2.0 * log(2.0))) = 0.424660891
        glUniform1f(gl_shader_shadow.sigma, shadow_blur * 0.424660891f);
        float shadow_color[4] = {
            shadow->color[0] * deco->opacity,
            shadow->color[1] * deco->opacity,
            shadow->color[2] * deco->opacity,
            shadow->color[3] * deco->opacity,
        };
        glUniform4fv(gl_shader_shadow.color, 1, shadow_color);
        glUniform1f(gl_shader_shadow.window_anti_aliasing, anti_aliasing);
        glUniform1f(gl_shader_shadow.window_aspect, aspect);
        glUniform4f(gl_shader_shadow.window_rect, offset_x_distance, offset_y_distance,
                    width_distance, height_distance);
        glUniform4fv(gl_shader_shadow.window_round_radius, 1, round_radius);
        glDrawArrays(GL_TRIANGLES, 0, rects_len * 6);
    }

    // draw shape
    glUseProgram(gl_shader.program);
    glUniformMatrix3fv(gl_shader.uv2ndc, 1, GL_FALSE, uv2ndc.matrix);
    glUniformMatrix3fv(gl_shader.inverse_transform, 1, GL_FALSE, inverseTransform.matrix);
    glUniform1f(gl_shader.anti_aliasing, anti_aliasing);
    glUniform1f(gl_shader.aspect, aspect);
    glUniform4f(gl_shader.rect, offset_x_distance, offset_y_distance, width_distance,
                height_distance);
    glUniform4fv(gl_shader.round_radius, 1, round_radius);
    glUniform1f(gl_shader.border_thickness, border_thickness * one_pixel_distance);
    float border_color[4] = {
        deco->border_color[0] * deco->opacity,
        deco->border_color[1] * deco->opacity,
        deco->border_color[2] * deco->opacity,
        deco->border_color[3] * deco->opacity,
    };
    glUniform4fv(gl_shader.border_color, 1, border_color);
    glUniform1f(gl_shader.title_height, title_height / height);
    float title_color[4] = {
        deco->title_color[0] * deco->opacity,
        deco->title_color[1] * deco->opacity,
        deco->title_color[2] * deco->opacity,
        deco->title_color[3] * deco->opacity,
    };
    glUniform4fv(gl_shader.title_color, 1, title_color);
    glDrawArrays(GL_TRIANGLES, 0, rects_len * 6);

    glDisableVertexAttribArray(gl_shader.in_uv);
    glUseProgram(0);

    PROFILE_RENDER_ZONE_END(target->output->output->renderer);

    pixman_region32_fini(&region);
}

static void scene_decoration_update_round_corner_region(struct scene_decoration *deco)
{
    pixman_region32_clear(&deco->round_corner_region);

    if (gl_shader.program < 0) {
        return;
    }

    int width = deco->surface_width;
    int height = deco->surface_height;
    int outer_border =
        deco->border_type == DECORATION_BORDER_TYPE_OUTER ? deco->border_thickness : 0;

    // include border round_corner
    float round_corner_radius[4] = {
        deco->round_corner_radius[0] + outer_border,
        deco->round_corner_radius[1] + outer_border,
        deco->round_corner_radius[2] + outer_border,
        deco->round_corner_radius[3] + outer_border,
    };
    int x1 = 0;
    int x2 = width + 2 * outer_border;
    int y1 = 0;
    int y2 = height + deco->title_height + 2 * outer_border;
    if ((deco->mask & DECORATION_MASK_TOP_LEFT) && round_corner_radius[3] > 0) {
        pixman_region32_union_rect(&deco->round_corner_region, &deco->round_corner_region, x1, y1,
                                   round_corner_radius[3], round_corner_radius[3]);
    }
    if ((deco->mask & DECORATION_MASK_TOP_RIGHT) && round_corner_radius[1] > 0) {
        pixman_region32_union_rect(&deco->round_corner_region, &deco->round_corner_region,
                                   x2 - round_corner_radius[1], y1, round_corner_radius[1],
                                   round_corner_radius[1]);
    }
    if ((deco->mask & DECORATION_MASK_BOTTOM_LEFT) && round_corner_radius[2] > 0) {
        pixman_region32_union_rect(&deco->round_corner_region, &deco->round_corner_region, x1,
                                   y2 - round_corner_radius[2], round_corner_radius[2],
                                   round_corner_radius[2]);
    }
    if ((deco->mask & DECORATION_MASK_BOTTOM_RIGHT) && round_corner_radius[0] > 0) {
        pixman_region32_union_rect(&deco->round_corner_region, &deco->round_corner_region,
                                   x2 - round_corner_radius[0], y2 - round_corner_radius[0],
                                   round_corner_radius[0], round_corner_radius[0]);
    }
}

static void scene_decoration_update_region(struct scene_decoration *deco)
{
    pixman_region32_clear(&deco->surface_region);
    pixman_region32_clear(&deco->title_region);
    pixman_region32_clear(&deco->border_region);
    pixman_region32_clear(&deco->pure_clip_region);

    int title = deco->title_height;
    int border = deco->border_thickness;

    int width, height;
    if (deco->border_type == DECORATION_BORDER_TYPE_INNER) {
        int border2 = 2 * deco->border_thickness;
        width = deco->surface_width - border2;
        height = deco->surface_height - border2;
        pixman_region32_init_rect(&deco->title_region, 0, 0, deco->surface_width, title);
    } else {
        width = deco->surface_width;
        height = deco->surface_height;
        pixman_region32_init_rect(&deco->title_region, border, border, width, title);
    }
    pixman_region32_init_rect(&deco->surface_region, border, border + title, width, height);

    pixman_region32_t reg1, reg2;
    pixman_region32_init_rect(&reg1, 0, 0, deco->rect.width, deco->rect.height);
    pixman_region32_init_rect(&reg2, border, border, width, title + height);
    pixman_region32_subtract(&deco->border_region, &reg1, &reg2);
    pixman_region32_fini(&reg1);
    pixman_region32_fini(&reg2);

    pixman_region32_init_rect(&deco->pure_clip_region, deco->render_box.x, deco->render_box.y,
                              deco->render_box.width, deco->render_box.height);
    pixman_region32_subtract(&deco->pure_clip_region, &deco->pure_clip_region,
                             &deco->surface_region);
}

static void scene_decoration_update(struct scene_decoration *deco, uint32_t cause)
{
    int width = deco->surface_width;
    int height = deco->surface_height;
    int title = deco->title_height;
    int border = deco->border_thickness;

    if (width <= 0 || height <= 0) {
        deco->pending_cause |= cause;
        return;
    }

    uint32_t pending_cause = deco->pending_cause | cause;
    deco->pending_cause = DECO_UPDATE_CAUSE_NONE;

    if (pending_cause & (DECO_UPDATE_CAUSE_SURFACE_SIZE | DECO_UPDATE_CAUSE_MARGIN)) {
        struct kywc_box window = { 0 };
        if (deco->border_type == DECORATION_BORDER_TYPE_INNER) {
            window.width = width;
            window.height = title + height;
        } else {
            int border2 = 2 * border;
            window.width = width + border2;
            window.height = title + height + border2;
        }

        int x_min = window.x;
        int y_min = window.y;
        int x_max = window.x + window.width;
        int y_max = window.y + window.height;

        for (int i = 0; i < deco->shadows_size; i++) {
            struct scene_shadow *shadow = &deco->shadows[i];
            int radius2 = (shadow->spread + shadow->blur) * 2;
            shadow->box = kywc_box_adjusted(
                &window, shadow->offset_x - shadow->spread - shadow->blur,
                shadow->offset_y - shadow->spread - shadow->blur, radius2, radius2);

            x_min = MIN(x_min, shadow->box.x);
            y_min = MIN(y_min, shadow->box.y);
            x_max = MAX(x_max, shadow->box.x + shadow->box.width);
            y_max = MAX(y_max, shadow->box.y + shadow->box.height);
        }

        struct kywc_box render = { x_min, y_min, x_max - x_min, y_max - y_min };
        if (!kywc_box_equal(&render, &deco->render_box)) {
            scene_node_push_damage(&deco->rect.node, SCENE_DAMAGE_BOTH, NULL);
            deco->render_box = render;
            scene_node_push_damage(&deco->rect.node, SCENE_DAMAGE_BOTH, NULL);
            /* clear some masks once the region is damaged */
            pending_cause &= ~(DECO_UPDATE_CAUSE_SURFACE_COLOR | DECO_UPDATE_CAUSE_MARGIN_COLOR);
        }

        scene_rect_set_size(&deco->rect, window.width, window.height);
        scene_decoration_update_region(deco);
    }

    if (pending_cause & (DECO_UPDATE_CAUSE_SURFACE_SIZE | DECO_UPDATE_CAUSE_MARGIN |
                         DECO_UPDATE_CAUSE_CORNER_RADIUS | DECO_UPDATE_CAUSE_MASK)) {
        scene_decoration_update_round_corner_region(deco);
        /* update clip region with corner region */
        pixman_region32_clear(&deco->clip_region);
        pixman_region32_union(&deco->clip_region, &deco->pure_clip_region,
                              &deco->round_corner_region);
    }

    if (pending_cause & (DECO_UPDATE_CAUSE_SURFACE_SIZE | DECO_UPDATE_CAUSE_BLURRED)) {
        scene_node_set_blur_region(&deco->rect.node,
                                   deco->surface_blurred ? &deco->surface_region : NULL);
        pending_cause &= ~DECO_UPDATE_CAUSE_BLURRED;
    }

    pending_cause &= ~(DECO_UPDATE_CAUSE_SURFACE_SIZE | DECO_UPDATE_CAUSE_MARGIN);

    if (pending_cause == DECO_UPDATE_CAUSE_SURFACE_COLOR) {
        scene_node_push_damage(&deco->rect.node, SCENE_DAMAGE_BOTH, &deco->surface_region);
        pending_cause &= ~DECO_UPDATE_CAUSE_SURFACE_COLOR;
    }

    if (pending_cause != DECO_UPDATE_CAUSE_NONE) {
        bool damage_whole =
            pending_cause & ~(DECO_UPDATE_CAUSE_MASK | DECO_UPDATE_CAUSE_MARGIN_COLOR);
        scene_node_push_damage(&deco->rect.node, SCENE_DAMAGE_BOTH,
                               damage_whole ? NULL : &deco->clip_region);
    }
}

struct scene_decoration *scene_decoration_from_node(struct scene_node *node)
{
    struct scene_rect *rect = scene_rect_from_node(node);
    struct scene_decoration *decoration = wl_container_of(rect, decoration, rect);
    return decoration;
}

struct scene_node *scene_node_from_decoration(struct scene_decoration *decoration)
{
    return &decoration->rect.node;
}

static struct scene_node *scene_decoration_accept_input(struct scene_node *node, int lx, int ly,
                                                        double px, double py, double *rx,
                                                        double *ry)
{
    if (!node->enabled || node->input_bypassed) {
        return NULL;
    }

    int x = floor(px) - lx;
    int y = floor(py) - ly;
    if (pixman_region32_not_empty(&node->input_region) &&
        !pixman_region32_contains_point(&node->input_region, x, y, NULL)) {
        return NULL;
    }

    struct scene_decoration *deco = scene_decoration_from_node(node);
    int border = deco->border_thickness;
    int resize = deco->resize_width;

    struct kywc_box window_box = { .width = deco->rect.width, .height = deco->rect.height };
    int off = resize > 0 ? resize : border;
    int lt = border - off;
    int rb = (off - border) * 2;
    struct kywc_box input_region_extents = kywc_box_adjusted(&window_box, lt, lt, rb, rb);

    pixman_region32_t input_region;
    pixman_region32_init_rect(&input_region, input_region_extents.x, input_region_extents.y,
                              input_region_extents.width, input_region_extents.height);
    pixman_region32_subtract(&input_region, &input_region, &deco->surface_region);
    if (!pixman_region32_contains_point(&input_region, x, y, NULL)) {
        pixman_region32_fini(&input_region);
        return NULL;
    }
    pixman_region32_fini(&input_region);

    *rx = px - lx;
    *ry = py - ly;
    return node;
}

static void scene_decoration_get_opaque_region(struct scene_node *node, pixman_region32_t *opaque)
{
    pixman_region32_clear(opaque);
    if (!node->enabled) {
        return;
    }

    /**
     * the border is not included in the calculation of opaque areas
     * because when the border is opaque but the window is transparent,
     * it is easy to cut the visible area of the lower window,
     * causing fragmentation of the visible area.
     */
    struct scene_decoration *deco = scene_decoration_from_node(node);
    bool title_is_opaque = deco->title_height > 0 && (deco->title_color[3] * deco->opacity == 1);
    bool surface_is_opaque = (deco->surface_color[3] * deco->opacity) == 1;
    bool has_opaque_region = title_is_opaque || surface_is_opaque;
    if (!has_opaque_region) {
        return;
    }

    if (title_is_opaque) {
        pixman_region32_union(opaque, opaque, &deco->title_region);
    }
    if (surface_is_opaque) {
        pixman_region32_union(opaque, opaque, &deco->surface_region);
    }
    pixman_region32_subtract(opaque, opaque, &deco->round_corner_region);

    if (pixman_region32_not_empty(&node->clip_region)) {
        pixman_region32_intersect(opaque, opaque, &node->clip_region);
    }
}

static void decoration_collect_invisible(struct scene_decoration *deco, int lx, int ly,
                                         pixman_region32_t *invisible)
{
    pixman_region32_t opaque_region;
    pixman_region32_init(&opaque_region);
    scene_decoration_get_opaque_region(&deco->rect.node, &opaque_region);
    if (!pixman_region32_not_empty(&opaque_region)) {
        pixman_region32_fini(&opaque_region);
        return;
    }

    pixman_region32_translate(&opaque_region, lx, ly);
    pixman_region32_union(invisible, invisible, &opaque_region);
    pixman_region32_fini(&opaque_region);
}

static void scene_decoration_collect_damage(struct scene_node *node, int lx, int ly,
                                            bool parent_enabled, uint32_t damage_type,
                                            pixman_region32_t *damage, pixman_region32_t *invisible,
                                            pixman_region32_t *affected)
{
    bool node_enabled = parent_enabled && node->enabled;
    /* node is still disabled, skip it */
    if (!node_enabled && !node->last_enabled) {
        node->damage_type = SCENE_DAMAGE_NONE;
        return;
    }

    struct scene_decoration *deco = scene_decoration_from_node(node);
    // if node state is changed, it must in the affected region
    if (deco->render_box.width > 0 && deco->render_box.height > 0 &&
        pixman_region32_contains_rectangle(
            affected, &(pixman_box32_t){ lx + deco->render_box.x, ly + deco->render_box.y,
                                         lx + deco->render_box.x + deco->render_box.width,
                                         ly + deco->render_box.y + deco->render_box.height }) ==
            PIXMAN_REGION_OUT) {
        if (node_enabled) {
            decoration_collect_invisible(deco, lx, ly, invisible);
        }
        return;
    }

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

    if (node_enabled) {
        bool has_clip_region = pixman_region32_not_empty(&node->clip_region);
        if (has_clip_region) {
            pixman_region32_intersect_rect(&node->visible_region, &node->clip_region,
                                           deco->render_box.x, deco->render_box.y,
                                           deco->render_box.width, deco->render_box.height);
            pixman_region32_translate(&node->visible_region, lx, ly);
        } else {
            pixman_region32_init_rect(&node->visible_region, lx + deco->render_box.x,
                                      ly + deco->render_box.y, deco->render_box.width,
                                      deco->render_box.height);
        }

        pixman_region32_subtract(&node->visible_region, &node->visible_region, invisible);

        if (!no_damage) {
            pixman_region32_union(damage, damage, &node->visible_region);
        }

        decoration_collect_invisible(deco, lx, ly, invisible);
    }

    node->last_enabled = node_enabled;
    node->damage_type = SCENE_DAMAGE_NONE;
}

static void scene_decoration_blur_render(struct scene_decoration *deco, int lx, int ly,
                                         struct scene_render_target *target,
                                         const pixman_region32_t *clip)
{
    if ((deco->surface_color[3] * deco->opacity) == 1 || !deco->surface_blurred ||
        target->options & SCENE_RENDER_DISABLE_BLUR) {
        return;
    }

    struct kywc_box blur_box = {
        .x = lx + deco->surface_region.extents.x1,
        .y = ly + deco->surface_region.extents.y1,
        .width = deco->surface_width,
        .height = deco->surface_height,
    };
    scene_render_target_box(target, &blur_box);

    struct render_round_corner round_corner_radius = { 0 };
    if (!(target->options & SCENE_RENDER_DISABLE_ROUND_CORNER)) {
        round_corner_radius.rb =
            deco->round_corner_radius[0] > 0 ? deco->round_corner_radius[0] * target->scale : 0.0f;
        round_corner_radius.rt =
            deco->round_corner_radius[1] > 0 ? deco->round_corner_radius[1] * target->scale : 0.0f;
        round_corner_radius.lb =
            deco->round_corner_radius[2] > 0 ? deco->round_corner_radius[2] * target->scale : 0.0f;
        round_corner_radius.lt =
            deco->round_corner_radius[3] > 0 ? deco->round_corner_radius[3] * target->scale : 0.0f;
    }

    struct scene_render_blur_options opts = {
        .node_lx = lx,
        .node_ly = ly,
        .round_mask = {
            .boundary = NULL,
            .radius = &round_corner_radius,
        },
        .clip = clip,
        .blur = deco->surface_blurred ? &deco->rect.node.blur : NULL,
    };
    scene_render_target_add_blur(target, &opts);
}

static void scene_decoration_render(struct scene_node *node, int lx, int ly,
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

    struct scene_decoration *deco = scene_decoration_from_node(node);

    pixman_region32_t render_region;
    if (render_with_visibility) {
        pixman_region32_init(&render_region);
        pixman_region32_union(&render_region, &node->visible_region, &node->extend_render_region);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    } else {
        pixman_region32_init_rect(&render_region, deco->render_box.x, deco->render_box.y,
                                  deco->render_box.width, deco->render_box.height);
        if (pixman_region32_not_empty(&node->clip_region)) {
            pixman_region32_intersect(&render_region, &render_region, &node->clip_region);
        }
        pixman_region32_translate(&render_region, lx, ly);
        pixman_region32_intersect(&render_region, &render_region, &target->damage);
    }

    if (!pixman_region32_not_empty(&render_region)) {
        pixman_region32_fini(&render_region);
        return;
    }
    scene_render_target_region(target, &render_region);

    /* actual render region exclude the blur region */
    pixman_region32_t clip_region;
    pixman_region32_init(&clip_region);
    pixman_region32_copy(&clip_region, &deco->clip_region);
    pixman_region32_translate(&clip_region, lx - target->logical.x, ly - target->logical.y);
    scene_render_region(&clip_region, target);
    pixman_region32_intersect(&clip_region, &clip_region, &render_region);

    bool need_render = pixman_region32_not_empty(&clip_region);
    struct wlr_box dst_box = {
        .x = lx - target->logical.x + deco->render_box.x,
        .y = ly - target->logical.y + deco->render_box.y,
        .width = deco->render_box.width,
        .height = deco->render_box.height,
    };
    if (need_render) {
        scene_render_box(&dst_box, target);
    }

    // try opengl render if opengl is used
    if (wlr_renderer_is_opengl(target->output->output->renderer) && gl_shader.program >= 0 &&
        gl_shader_shadow.program >= 0) {
        if (gl_shader.program == 0 && gl_shader_shadow.program == 0) {
            struct opengl_renderer *renderer =
                opengl_renderer_from_wlr_renderer(target->output->output->renderer);
            scene_decoration_create_opengl_shader(renderer);
        }
        if (gl_shader.program > 0 && gl_shader_shadow.program > 0) {
            scene_decoration_blur_render(deco, lx, ly, target, &render_region);
            if (need_render) {
                scene_decoration_opengl_render(deco, lx, ly, target, &dst_box, &clip_region);
            }
        }
    }

    float surface_color_alpha = deco->surface_color[3] * deco->opacity;
    if (surface_color_alpha != 0) {
        pixman_region32_t surface;
        pixman_region32_init(&surface);
        pixman_region32_copy(&surface, &deco->surface_region);
        pixman_region32_translate(&surface, lx - target->logical.x, ly - target->logical.y);
        scene_render_region(&surface, target);
        pixman_region32_intersect(&surface, &surface, &render_region);

        struct wlr_box surface_box = {
            .x = lx - target->logical.x + deco->surface_region.extents.x1,
            .y = ly - target->logical.y + deco->surface_region.extents.y1,
            .width = deco->surface_width,
            .height = deco->surface_height,
        };
        scene_render_box(&surface_box, target);

        render_pass_add_rect(target->render_pass, &(struct render_rect_options){
            .base = {
            .box = surface_box,
            .color = {
                .r = deco->surface_color[0] * deco->opacity,
                .g = deco->surface_color[1] * deco->opacity,
                .b = deco->surface_color[2] * deco->opacity,
                .a = surface_color_alpha,
            },
            .clip = &surface,
            .blend_mode = WLR_RENDER_BLEND_MODE_PREMULTIPLIED,
            },
            .radius = {
                .rb = deco->round_corner_radius[0] * target->scale,
                .rt = deco->title_height ? 0 : deco->round_corner_radius[1] * target->scale,
                .lb = deco->round_corner_radius[2] * target->scale,
                .lt = deco->title_height ? 0 : deco->round_corner_radius[3] * target->scale,
            },
        });
        pixman_region32_fini(&surface);
    }

    if ((gl_shader.program > 0 && gl_shader_shadow.program > 0) || !need_render) {
        pixman_region32_fini(&render_region);
        pixman_region32_fini(&clip_region);
        return;
    }

    /* draw title with title color */
    if (deco->title_height > 0) {
        pixman_region32_t title;
        pixman_region32_init(&title);
        pixman_region32_copy(&title, &deco->title_region);
        pixman_region32_translate(&title, lx - target->logical.x, ly - target->logical.y);
        scene_render_region(&title, target);
        pixman_region32_intersect(&title, &title, &clip_region);

        float title_color_alpha = deco->title_color[3] * deco->opacity;
        wlr_render_pass_add_rect(target->render_pass, &(struct wlr_render_rect_options){
            .box = dst_box,
            .color = {
                .r = deco->title_color[0] * deco->opacity,
                .g = deco->title_color[1] * deco->opacity,
                .b = deco->title_color[2] * deco->opacity,
                .a = title_color_alpha,
            },
            .clip = &title,
            .blend_mode = title_color_alpha != 1 ?
                WLR_RENDER_BLEND_MODE_PREMULTIPLIED : WLR_RENDER_BLEND_MODE_NONE,
        });
        pixman_region32_fini(&title);
    }

    /* draw border with border color */
    if (deco->border_thickness > 0) {
        pixman_region32_t border;
        pixman_region32_init(&border);
        pixman_region32_copy(&border, &deco->border_region);
        pixman_region32_translate(&border, lx - target->logical.x, ly - target->logical.y);
        scene_render_region(&border, target);
        pixman_region32_intersect(&border, &border, &clip_region);

        float border_color_alpha = deco->border_color[3] * deco->opacity;
        wlr_render_pass_add_rect(target->render_pass, &(struct wlr_render_rect_options){
            .box = dst_box,
            .color = {
                .r = deco->border_color[0] * deco->opacity,
                .g = deco->border_color[1] * deco->opacity,
                .b = deco->border_color[2] * deco->opacity,
                .a = border_color_alpha,
            },
            .clip = &border,
            .blend_mode = border_color_alpha != 1 ?
                WLR_RENDER_BLEND_MODE_PREMULTIPLIED : WLR_RENDER_BLEND_MODE_NONE,
        });
        pixman_region32_fini(&border);
    }

    pixman_region32_fini(&render_region);
    pixman_region32_fini(&clip_region);
}

static void scene_decoration_get_bounding_box(struct scene_node *node, struct wlr_box *box)
{
    struct scene_decoration *deco = scene_decoration_from_node(node);
    *box = (struct wlr_box){ deco->render_box.x, deco->render_box.y, deco->render_box.width,
                             deco->render_box.height };
}

static void scene_decoration_destroy(struct scene_node *node)
{
    if (!node) {
        return;
    }

    struct scene_decoration *decoration = scene_decoration_from_node(node);
    pixman_region32_fini(&decoration->surface_region);
    pixman_region32_fini(&decoration->title_region);
    pixman_region32_fini(&decoration->border_region);
    pixman_region32_fini(&decoration->round_corner_region);
    pixman_region32_fini(&decoration->pure_clip_region);
    pixman_region32_fini(&decoration->clip_region);

    decoration->node_destroy(node);
}

struct scene_decoration *scene_decoration_create(struct scene_tree *parent)
{
    return scene_decoration_create_with_border_type(parent, DECORATION_BORDER_TYPE_OUTER);
}

struct scene_decoration *
scene_decoration_create_with_border_type(struct scene_tree *parent,
                                         enum decoration_border_type border_type)
{
    struct scene_decoration *decoration = calloc(1, sizeof(*decoration));
    if (!decoration) {
        return NULL;
    }

    decoration->opacity = 1.0f;
    decoration->border_type = border_type;
    float color[4] = { 0.0f, 0.0f, 0.0f, 0.25f };
    scene_rect_init(&decoration->rect, parent, 0, 0, color);
    scene_node_set_role(&decoration->rect.node, SCENE_ROLE_DECO, NULL, decoration);
    memset(decoration->surface_color, 0, sizeof(decoration->surface_color));
    memcpy(decoration->title_color, color, sizeof(decoration->title_color));
    memcpy(decoration->border_color, color, sizeof(decoration->border_color));

    decoration->node_destroy = decoration->rect.node.impl.destroy;
    decoration->rect.node.impl.destroy = scene_decoration_destroy;
    decoration->rect.node.impl.accept_input = scene_decoration_accept_input;
    decoration->rect.node.impl.collect_damage = scene_decoration_collect_damage;
    decoration->rect.node.impl.get_opaque_region = scene_decoration_get_opaque_region;
    decoration->rect.node.impl.render = scene_decoration_render;
    decoration->rect.node.impl.get_bounding_box = scene_decoration_get_bounding_box;
    /* no need to update_region and push_damage, it is invisible */

    pixman_region32_init(&decoration->surface_region);
    pixman_region32_init(&decoration->title_region);
    pixman_region32_init(&decoration->border_region);
    pixman_region32_init(&decoration->round_corner_region);
    pixman_region32_init(&decoration->pure_clip_region);
    pixman_region32_init(&decoration->clip_region);

    return decoration;
}

void scene_decoration_set_surface_size(struct scene_decoration *decoration, int width, int height)
{
    if (decoration->surface_width == width && decoration->surface_height == height) {
        return;
    }

    decoration->surface_width = width;
    decoration->surface_height = height;
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_SURFACE_SIZE);
}

void scene_decoration_set_round_corner_radius(struct scene_decoration *decoration,
                                              const int round_corner_radius[static 4])
{
    if (gl_shader.program < 0) {
        return;
    }

    if (memcmp(decoration->round_corner_radius, round_corner_radius,
               sizeof(decoration->round_corner_radius)) == 0) {
        return;
    }

    memcpy(decoration->round_corner_radius, round_corner_radius,
           sizeof(decoration->round_corner_radius));
    scene_node_set_radius(&decoration->rect.node, round_corner_radius);
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_CORNER_RADIUS);
}

void scene_decoration_set_margin(struct scene_decoration *decoration, int title_height,
                                 int border_thickness)
{
    if (decoration->title_height == title_height &&
        decoration->border_thickness == border_thickness) {
        return;
    }

    decoration->title_height = title_height;
    decoration->border_thickness = border_thickness;
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_MARGIN);
}

void scene_decoration_set_margin_color(struct scene_decoration *decoration,
                                       const float title_color[static 4],
                                       const float border_color[static 4])
{
    if (memcmp(decoration->title_color, title_color, sizeof(decoration->title_color)) == 0 &&
        memcmp(decoration->border_color, border_color, sizeof(decoration->border_color)) == 0) {
        return;
    }

    memcpy(decoration->title_color, title_color, sizeof(decoration->title_color));
    memcpy(decoration->border_color, border_color, sizeof(decoration->border_color));
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_MARGIN_COLOR);
}

void scene_decoration_set_mask(struct scene_decoration *decoration, uint32_t masks)
{
    if (decoration->mask == masks) {
        return;
    }

    decoration->mask = masks;
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_MASK);
}

void scene_decoration_set_shadow_count(struct scene_decoration *decoration, int size)

{
    if (gl_shader.program < 0) {
        return;
    }

    if (size < 0 || size > THEME_MAX_SHADOW_LAYERS) {
        return;
    }

    if (decoration->shadows_size == size) {
        return;
    }

    decoration->shadows_size = size;
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_MARGIN | DECO_UPDATE_CAUSE_MARGIN_COLOR);
}

void scene_decoration_set_shadow(struct scene_decoration *decoration, int index, int offset_x,
                                 int offset_y, int spread, int blur, const float color[static 4])
{
    if (gl_shader.program < 0) {
        return;
    }

    if (index < 0 || index >= THEME_MAX_SHADOW_LAYERS) {
        return;
    }

    uint32_t update = DECO_UPDATE_CAUSE_NONE;
    struct scene_shadow *shadow = &decoration->shadows[index];
    if (shadow->offset_x != offset_x || shadow->offset_y != offset_y || shadow->spread != spread ||
        shadow->blur != blur) {
        shadow->offset_x = offset_x;
        shadow->offset_y = offset_y;
        shadow->spread = spread;
        shadow->blur = blur;
        update |= DECO_UPDATE_CAUSE_MARGIN;
    }

    if (memcmp(shadow->color, color, sizeof(shadow->color))) {
        memcpy(shadow->color, color, sizeof(shadow->color));
        update |= DECO_UPDATE_CAUSE_MARGIN_COLOR;
    }

    if (update != DECO_UPDATE_CAUSE_NONE) {
        scene_decoration_update(decoration, update);
    }
}

void scene_decoration_set_resize_width(struct scene_decoration *decoration, int resize_with)
{
    if (decoration->resize_width == resize_with) {
        return;
    }

    decoration->resize_width = resize_with;
}

void scene_decoration_set_surface_blurred(struct scene_decoration *decoration, bool blurred)
{
    if (decoration->surface_blurred == blurred) {
        return;
    }

    decoration->surface_blurred = blurred;
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_BLURRED);
}

void scene_decoration_set_surface_color(struct scene_decoration *decoration,
                                        const float color[static 4])
{
    if (memcmp(decoration->surface_color, color, sizeof(decoration->surface_color)) == 0) {
        return;
    }

    memcpy(decoration->surface_color, color, sizeof(decoration->surface_color));
    scene_decoration_update(decoration, DECO_UPDATE_CAUSE_SURFACE_COLOR);
}

void scene_decoration_set_opacity(struct scene_decoration *decoration, float opacity)
{
    if (fabsf(decoration->opacity - opacity) < 0.01f) {
        return;
    }

    decoration->opacity = opacity;
    scene_decoration_update(decoration,
                            DECO_UPDATE_CAUSE_MARGIN_COLOR | DECO_UPDATE_CAUSE_SURFACE_COLOR);
}
