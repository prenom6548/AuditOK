// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <pixman.h>
#include <stdlib.h>
#include <time.h>

#include <wlr/types/wlr_matrix.h>

#include <kywc/boxes.h>
#include <kywc/log.h>

#include "render/opengl.h"
#include "render/profile.h"
#include "util/debug.h"
#include "util/matrix.h"
#include "util/quirks.h"

#define MAX_QUADS 86 // 4kb

static const struct wlr_render_pass_impl wlr_render_pass_impl;

bool wlr_render_pass_is_opengl(struct wlr_render_pass *render_pass)
{
    return render_pass->impl == &wlr_render_pass_impl;
}

struct opengl_render_pass *opengl_render_pass_from_wlr_render_pass(struct wlr_render_pass *wlr_pass)
{
    assert(wlr_pass->impl == &wlr_render_pass_impl);
    struct opengl_render_pass *pass = wl_container_of(wlr_pass, pass, base);
    return pass;
}

static bool _render_pass_submit(struct wlr_render_pass *wlr_pass, uint32_t quirks)
{
    PROFILE_ZONE(zone, __func__);

    struct opengl_render_pass *pass = opengl_render_pass_from_wlr_render_pass(wlr_pass);
    struct opengl_renderer *renderer = pass->renderer;
    struct opengl_render_timer *timer = pass->timer;

    opengl_push_debug(renderer);

    if (timer) {
        // clear disjoint flag
        GLint64 disjoint;
        glGetInteger64v(GL_GPU_DISJOINT_EXT, &disjoint);
        // set up the query
        glQueryCounterEXT(timer->id, GL_TIMESTAMP_EXT);
        // get end-of-CPU-work time in GL time domain
        glGetInteger64v(GL_TIMESTAMP_EXT, &timer->gl_cpu_end);
        // get end-of-CPU-work time in CPU time domain
        clock_gettime(CLOCK_MONOTONIC, &timer->cpu_end);
    }

    if (quirks & QUIRKS_MASK_EXPLICIT_SYNC) {
        glFinish();
    } else {
        glFlush();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    opengl_pop_debug(renderer);
    egl_restore_context(&pass->prev_ctx);

    if (pass->buffer) {
        wlr_buffer_unlock(pass->buffer->buffer);
    }
    free(pass);

    PROFILE_ZONE_END(zone);
    return true;
}

static bool render_pass_handle_submit(struct wlr_render_pass *wlr_pass)
{
    return _render_pass_submit(wlr_pass, 0);
}

static void render(struct wlr_renderer *renderer, const struct kywc_box *box,
                   const pixman_region32_t *clip, GLint attrib)
{
    pixman_region32_t region;
    pixman_region32_init_rect(&region, box->x, box->y, box->width, box->height);

    if (clip) {
        pixman_region32_intersect(&region, &region, clip);
    }

    int rects_len;
    const pixman_box32_t *rects = pixman_region32_rectangles(&region, &rects_len);
    if (rects_len == 0) {
        pixman_region32_fini(&region);
        return;
    }

    PROFILE_RENDER_ZONE(renderer, gzone_render, __func__);
    glEnableVertexAttribArray(attrib);

    for (int i = 0; i < rects_len;) {
        int batch = rects_len - i < MAX_QUADS ? rects_len - i : MAX_QUADS;
        int batch_end = batch + i;

        size_t vert_index = 0;
        GLfloat verts[MAX_QUADS * 6 * 2];
        for (; i < batch_end; i++) {
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

        glVertexAttribPointer(attrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glDrawArrays(GL_TRIANGLES, 0, batch * 6);
    }

    glDisableVertexAttribArray(attrib);
    PROFILE_RENDER_ZONE_END(renderer);
    pixman_region32_fini(&region);
}

static void setup_blending(enum wlr_render_blend_mode mode)
{
    switch (mode) {
    case WLR_RENDER_BLEND_MODE_PREMULTIPLIED:
        glEnable(GL_BLEND);
        break;
    case WLR_RENDER_BLEND_MODE_NONE:
        glDisable(GL_BLEND);
        break;
    }
}

static void render_pass_handle_add_texture(struct wlr_render_pass *wlr_pass,
                                           const struct wlr_render_texture_options *wlr_options)
{
    struct render_texture_options options = {
        .base = *wlr_options,
        .radius = { 0 },
        .repeat_mode = RENDER_TEXTURE_REPEAT_NONE,
        .rotation_angle = 0,
    };
    opengl_render_pass_add_texture(wlr_pass, &options);
}

static void render_pass_handle_add_rect(struct wlr_render_pass *wlr_pass,
                                        const struct wlr_render_rect_options *wlr_options)
{
    struct render_rect_options options = {
        .base = *wlr_options,
        .radius = { 0 },
        .rotation_angle = 0,
    };
    opengl_render_pass_add_rect(wlr_pass, &options);
}

static const struct wlr_render_pass_impl wlr_render_pass_impl = {
    .submit = render_pass_handle_submit,
    .add_texture = render_pass_handle_add_texture,
    .add_rect = render_pass_handle_add_rect,
};

static bool opengl_render_pass_submit(struct wlr_render_pass *wlr_pass, uint32_t quirks)
{
    return _render_pass_submit(wlr_pass, quirks);
}

static void mix_set_texture(GLuint u_uv2texcoord, const struct opengl_texture *texture,
                            const struct kywc_fbox *src_fbox, struct mat3 *uv_rotation)
{
    struct mat3 uv2texcoord;
    struct mat3 tex_matrix;
    mat3_init_scale(&tex_matrix, src_fbox->width, src_fbox->height);
    mat3_translate(&tex_matrix, src_fbox->x, src_fbox->y);
    mat3_multiply(&uv2texcoord, &tex_matrix, uv_rotation);

    glBindTexture(texture->target, texture->tex);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glUniformMatrix3fv(u_uv2texcoord, 1, GL_FALSE, uv2texcoord.matrix);
}

static void opengl_render_pass_add_mix_texture(struct opengl_render_pass *pass,
                                               struct opengl_renderer *renderer,
                                               struct opengl_texture *texture,
                                               const struct wlr_box *dst_box,
                                               const struct wlr_fbox *src_fbox, float alpha,
                                               const struct render_texture_options *options)
{
    const struct render_texture_mix_options *mix_options = options->mix_options;
    struct opengl_tex_mix_shader *shader = &renderer->shaders.tex_mix;

    int tex_w = texture->wlr_texture.width;
    int tex_h = texture->wlr_texture.height;
    struct kywc_fbox src_kywcfbox = {
        .x = src_fbox->x / tex_w,
        .y = src_fbox->y / tex_h,
        .width = src_fbox->width / tex_w,
        .height = src_fbox->height / tex_h,
    };
    struct kywc_box dst_kywcbox = {
        .x = dst_box->x,
        .y = dst_box->y,
        .width = dst_box->width,
        .height = dst_box->height,
    };

    const struct kywc_box *dst_box2 = mix_options->dst_box;
    float width2 = dst_box2->width, height2 = dst_box2->height;
    struct kywc_fbox src_kywcfbox2 = {
        (dst_kywcbox.x - dst_box2->x) / width2,
        (dst_kywcbox.y - dst_box2->y) / height2,
        dst_kywcbox.width / width2,
        dst_kywcbox.height / height2,
    };

    /**
     * dst_box and dst_box2 are both in the same coordinate system (output framebuffer coordinate
     * system), needs recalculation only if the output coordinate axes are swapped (90/270).
     * the direction change and translation of the coordinate axis do not affect the relative
     * position of the starting point, so the calculation of the starting point does not need to be
     * changed.
     */
    if (options->base.transform & WL_OUTPUT_TRANSFORM_90) {
        src_kywcfbox2.x = (dst_kywcbox.y - dst_box2->y) / height2;
        src_kywcfbox2.y = (dst_kywcbox.x - dst_box2->x) / width2;
        src_kywcfbox2.width = dst_kywcbox.height / height2;
        src_kywcfbox2.height = dst_kywcbox.width / width2;
    }

    struct mat3 uv2ndc;
    struct mat3 uv_rotation;
    mat3_invert_output_transform(&uv_rotation, options->base.transform);
    mat3_uvofbox_to_ndc(&uv2ndc, pass->target_width, pass->target_height, options->rotation_angle,
                        &dst_kywcbox);

    opengl_push_debug(renderer);
    PROFILE_RENDER_ZONE(&renderer->wlr_renderer, gzone, __func__);
    setup_blending(!texture->has_alpha && alpha == 1.0 ? WLR_RENDER_BLEND_MODE_NONE
                                                       : options->base.blend_mode);
    glUseProgram(shader->program);
    glUniform1f(shader->alpha, alpha);
    glUniform1f(shader->blend_factor, mix_options->blend_factor);
    glUniformMatrix3fv(shader->uv2ndc, 1, GL_FALSE, uv2ndc.matrix);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(shader->tex_a, 0);
    mix_set_texture(shader->uv2texcoord, texture, &src_kywcfbox, &uv_rotation);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(shader->tex_b, 1);
    mix_set_texture(shader->uv2texcoord2, opengl_texture_from_wlr_texture(mix_options->texture),
                    &src_kywcfbox2, &uv_rotation);

    render(&renderer->wlr_renderer, &dst_kywcbox, options->base.clip, shader->uvpos_attrib);

    glBindTexture(texture->target, 0);
    PROFILE_RENDER_ZONE_END(&renderer->wlr_renderer);
    opengl_pop_debug(renderer);
}

static struct opengl_texture *create_copy_screen_texture(struct opengl_render_pass *pass,
                                                         struct wlr_fbox *src_fbox)
{
    struct opengl_renderer *renderer = pass->renderer;
    struct opengl_texture *gl_texture =
        opengl_texture_create(renderer, src_fbox->width, src_fbox->height);
    if (!gl_texture) {
        return NULL;
    }

    glGenTextures(1, &gl_texture->tex);
    glBindTexture(GL_TEXTURE_2D, gl_texture->tex);
    gl_texture->target = GL_TEXTURE_2D;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    bool has_alpha = pass->buffer ? pass->buffer->has_alpha : pass->texture->has_alpha;
    glCopyTexImage2D(GL_TEXTURE_2D, 0, has_alpha ? GL_RGBA : GL_RGB, src_fbox->x, src_fbox->y,
                     src_fbox->width, src_fbox->height, 0);

    return gl_texture;
}

void opengl_render_pass_add_texture(struct wlr_render_pass *wlr_pass,
                                    const struct render_texture_options *options)
{
    struct opengl_render_pass *pass = opengl_render_pass_from_wlr_render_pass(wlr_pass);
    const struct wlr_render_texture_options *wlr_options = &options->base;
    struct opengl_renderer *renderer = pass->renderer;
    bool use_exshader =
        render_pass_options_has_radius(&options->radius) || options->enable_force_opaque;

    struct wlr_box dst_box;
    struct wlr_fbox src_fbox;
    wlr_render_texture_options_get_src_box(wlr_options, &src_fbox);
    wlr_render_texture_options_get_dst_box(wlr_options, &dst_box);
    float alpha = wlr_render_texture_options_get_alpha(wlr_options);

    bool need_copy = false;
    if (pass->buffer) {
        struct wlr_texture *wlr_texture =
            wlr_texture_from_buffer(&renderer->wlr_renderer, pass->buffer->buffer);
        need_copy = wlr_options->texture == wlr_texture;
        wlr_texture_destroy(wlr_texture);
    } else if (pass->texture) {
        need_copy = wlr_options->texture == &pass->texture->wlr_texture;
    }

    struct opengl_texture *copy_texture = NULL;
    struct opengl_texture *texture = NULL;
    if (need_copy) {
        copy_texture = create_copy_screen_texture(pass, &src_fbox);
        if (!copy_texture) {
            return;
        }

        src_fbox.x = src_fbox.y = 0;
        texture = copy_texture;
    } else {
        texture = opengl_texture_from_wlr_texture(wlr_options->texture);
        if (options->mix_options) {
            opengl_render_pass_add_mix_texture(pass, renderer, texture, &dst_box, &src_fbox, alpha,
                                               options);
            return;
        }
    }

    struct opengl_tex_ex_shader *shader = NULL;
    switch (texture->target) {
    case GL_TEXTURE_2D:
        if (texture->has_alpha) {
            shader = use_exshader ? &renderer->shaders.tex_rgba_ex : &renderer->shaders.tex_rgba;
        } else {
            shader = use_exshader ? &renderer->shaders.tex_rgbx_ex : &renderer->shaders.tex_rgbx;
        }
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        assert(renderer->exts.OES_egl_image_external);
        shader = use_exshader ? &renderer->shaders.tex_ext_ex : &renderer->shaders.tex_ext;
        break;
    default:
        abort();
    }

    uint32_t tex_w = texture->wlr_texture.width;
    uint32_t tex_h = texture->wlr_texture.height;

    int width = dst_box.width;
    int height = dst_box.height;
    if (wlr_options->transform & WL_OUTPUT_TRANSFORM_90) {
        width = dst_box.height;
        height = dst_box.width;
    }

    struct mat3 uv_rotation;
    mat3_invert_output_transform(&uv_rotation, options->base.transform);

    struct mat3 texcoord_transform;
    if (options->repeat_mode == RENDER_TEXTURE_REPEAT_NONE) {
        struct kywc_fbox src_kywcbox = {
            .x = src_fbox.x / tex_w,
            .y = src_fbox.y / tex_h,
            .width = src_fbox.width / tex_w,
            .height = src_fbox.height / tex_h,
        };
        mat3_init_scale_translate(&texcoord_transform, src_kywcbox.width, src_kywcbox.height,
                                  src_kywcbox.x, src_kywcbox.y);
    } else {
        // UV transform is the inverse texture transform
        // UV coordinates range from 0 to 1; scaled up to pixel units
        float scale_x = width / src_fbox.width;
        float scale_y = height / src_fbox.height;
        float offset_x = -src_fbox.x / src_fbox.width;
        float offset_y = -src_fbox.y / src_fbox.height;
        mat3_init_scale_translate(&texcoord_transform, scale_x, scale_y, offset_x, offset_y);
    }

    struct mat3 uv2texcoord;
    mat3_multiply(&uv2texcoord, &texcoord_transform, &uv_rotation);

    struct kywc_box dst_kywcbox = {
        .x = dst_box.x,
        .y = dst_box.y,
        .width = dst_box.width,
        .height = dst_box.height,
    };
    struct mat3 uv2ndc;
    mat3_uvofbox_to_ndc(&uv2ndc, pass->target_width, pass->target_height, options->rotation_angle,
                        &dst_kywcbox);

    opengl_push_debug(renderer);
    PROFILE_RENDER_ZONE(&renderer->wlr_renderer, gzone, __func__);

    if (use_exshader || options->repeat_mode == RENDER_TEXTURE_REPEAT_DECAL) {
        // radius clip always need blend
        setup_blending(WLR_RENDER_BLEND_MODE_PREMULTIPLIED);
    } else {
        setup_blending(!texture->has_alpha && alpha == 1.0 ? WLR_RENDER_BLEND_MODE_NONE
                                                           : options->base.blend_mode);
    }

    glUseProgram(shader->program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture->target, texture->tex);

    switch (options->repeat_mode) {
    case RENDER_TEXTURE_REPEAT_NORAML:
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    case RENDER_TEXTURE_REPEAT_MIRROR:
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        break;
    case RENDER_TEXTURE_REPEAT_DECAL:
        if (renderer->exts.EXT_texture_border_clamp) {
            glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT);
            glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT);
            GLfloat borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            glTexParameterfv(texture->target, GL_TEXTURE_BORDER_COLOR, borderColor);
        } else {
            glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        break;
    case RENDER_TEXTURE_REPEAT_NONE:
    case RENDER_TEXTURE_REPEAT_CLAMP:
    default:
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    }

    switch (wlr_options->filter_mode) {
    case WLR_SCALE_FILTER_BILINEAR:
        glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case WLR_SCALE_FILTER_NEAREST:
        glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    }

    glUniformMatrix3fv(shader->uv2texcoord, 1, GL_FALSE, uv2texcoord.matrix);
    glUniformMatrix3fv(shader->uv2ndc, 1, GL_FALSE, uv2ndc.matrix);
    glUniform1i(shader->tex, 0);
    glUniform1f(shader->alpha, alpha);

    if (use_exshader) {
        glUniformMatrix3fv(shader->uv_rotation, 1, GL_FALSE, uv_rotation.matrix);
        // avoid texture alpha < 1
        bool force_opaque = (!texture->has_alpha && alpha == 1.0) ||
                            options->base.blend_mode == WLR_RENDER_BLEND_MODE_NONE;
        glUniform1i(shader->force_opaque, force_opaque);
        glUniform1f(shader->aspect, width / (float)height);
        float one_pixel_distance = 1.0f / height; // shader distance scale
        glUniform1f(shader->anti_aliasing, one_pixel_distance * 0.5f);
        glUniform4f(shader->round_corner_radius, options->radius.rb * one_pixel_distance,
                    options->radius.rt * one_pixel_distance,
                    options->radius.lb * one_pixel_distance,
                    options->radius.lt * one_pixel_distance);
    }
    render(&renderer->wlr_renderer, &dst_kywcbox, options->base.clip, shader->uv_attrib);

    glBindTexture(texture->target, 0);

    PROFILE_RENDER_ZONE_END(&renderer->wlr_renderer);

    if (copy_texture) {
        opengl_texture_destroy(copy_texture);
    }

    opengl_pop_debug(renderer);
}

static void opengl_render_pass_add_linear_gradient_rect(struct wlr_render_pass *wlr_pass,
                                                        const struct render_rect_options *options)
{
    struct opengl_render_pass *pass = opengl_render_pass_from_wlr_render_pass(wlr_pass);
    struct opengl_renderer *renderer = pass->renderer;

    bool is_stroke = options->stroke_width > 0.001f;
    struct opengl_linear_gradient_shader *shader;
    if (is_stroke) {
        shader = &renderer->shaders.linear_gradient_stroke;
    } else {
        shader = &renderer->shaders.linear_gradient;
    }

    struct wlr_box box = { .width = pass->target_width, .height = pass->target_height };
    if (!wlr_box_empty(&options->base.box)) {
        box = options->base.box;
    }

    struct kywc_box dst_box = {
        .x = box.x,
        .y = box.y,
        .width = box.width,
        .height = box.height,
    };
    int width = dst_box.width;
    int height = dst_box.height;
    if (options->transform & WL_OUTPUT_TRANSFORM_90) {
        width = dst_box.height;
        height = dst_box.width;
    }

    opengl_push_debug(renderer);
    PROFILE_RENDER_ZONE(&renderer->wlr_renderer, gzone, __func__);

    const struct render_linear_gradient *linear_gradient = options->linear_gradient;

    bool has_radius = render_pass_options_has_radius(&options->radius);
    if (has_radius) {
        setup_blending(WLR_RENDER_BLEND_MODE_PREMULTIPLIED);
    } else {
        float alpha0 = linear_gradient->color_stops[0].color[3];
        float alpha1 = linear_gradient->color_stops[1].color[3];
        setup_blending((alpha0 == 1.0f && alpha1 == 1.0f) ? WLR_RENDER_BLEND_MODE_NONE
                                                          : options->base.blend_mode);
    }

    glUseProgram(shader->program);

    struct mat3 uv2ndc;
    mat3_uvofbox_to_ndc(&uv2ndc, pass->target_width, pass->target_height, options->rotation_angle,
                        &dst_box);
    glUniformMatrix3fv(shader->uv2ndc, 1, GL_FALSE, uv2ndc.matrix);

    struct mat3 uv_rotation;
    mat3_invert_output_transform(&uv_rotation, options->transform);
    glUniformMatrix3fv(shader->uv_rotation, 1, GL_FALSE, uv_rotation.matrix);

    // linear gradient params
    float x0 = (linear_gradient->x0 - dst_box.x) / dst_box.width;
    float y0 = (linear_gradient->y0 - dst_box.y) / dst_box.height;
    glUniform2f(shader->start, x0, y0);
    float x1 = (linear_gradient->x1 - dst_box.x) / dst_box.width;
    float y1 = (linear_gradient->y1 - dst_box.y) / dst_box.height;
    glUniform2f(shader->end, x1, y1);
    glUniform1f(shader->color_stop0_offset, linear_gradient->color_stops[0].offset);
    glUniform1f(shader->color_stop1_offset, linear_gradient->color_stops[1].offset);
    glUniform4fv(shader->color_stop0_color, 1, linear_gradient->color_stops[0].color);
    glUniform4fv(shader->color_stop1_color, 1, linear_gradient->color_stops[1].color);

    // round rect mask
    glUniform1f(shader->aspect, width / (float)height);
    float one_pixel_distance = 1.0f / height;
    glUniform1f(shader->anti_aliasing, one_pixel_distance * 0.5f);
    glUniform4f(shader->round_corner_radius, options->radius.rb * one_pixel_distance,
                options->radius.rt * one_pixel_distance, options->radius.lb * one_pixel_distance,
                options->radius.lt * one_pixel_distance);

    if (is_stroke) {
        glUniform1f(shader->stroke_width, options->stroke_width / (float)height);
    }

    render(&renderer->wlr_renderer, &dst_box, options->base.clip, shader->in_uv);

    PROFILE_RENDER_ZONE_END(&renderer->wlr_renderer);
    opengl_pop_debug(renderer);
}

void opengl_render_pass_add_rect(struct wlr_render_pass *wlr_pass,
                                 const struct render_rect_options *options)
{
    if (options->linear_gradient) {
        opengl_render_pass_add_linear_gradient_rect(wlr_pass, options);
        return;
    }

    struct opengl_render_pass *pass = opengl_render_pass_from_wlr_render_pass(wlr_pass);
    struct opengl_renderer *renderer = pass->renderer;

    bool is_stroke = options->stroke_width > 0.001f;
    bool has_radius = render_pass_options_has_radius(&options->radius);
    struct opengl_rect_ex_shader *shader;
    if (is_stroke) {
        shader = &renderer->shaders.quad_stroke;
    } else if (has_radius) {
        shader = &renderer->shaders.quad_ex;
    } else {
        shader = &renderer->shaders.quad;
    }
    const struct wlr_render_color *color = &options->base.color;
    struct wlr_box box = { .width = pass->target_width, .height = pass->target_height };
    if (!wlr_box_empty(&options->base.box)) {
        box = options->base.box;
    }

    opengl_push_debug(renderer);
    PROFILE_RENDER_ZONE(&renderer->wlr_renderer, gzone, __func__);

    enum wlr_render_blend_mode blend_mode =
        (is_stroke || has_radius)
            ? WLR_RENDER_BLEND_MODE_PREMULTIPLIED
            : (color->a == 1.0 ? WLR_RENDER_BLEND_MODE_NONE : options->base.blend_mode);
    if (blend_mode == WLR_RENDER_BLEND_MODE_NONE && options->base.clip == NULL && box.x == 0 &&
        box.y == 0 && box.width == pass->target_width && box.height == pass->target_height) {
        glClearColor(color->r, color->g, color->b, color->a);
        glClear(GL_COLOR_BUFFER_BIT);
        goto final;
    }

    setup_blending(blend_mode);

    glUseProgram(shader->program);

    struct kywc_box dst_box = {
        .x = box.x,
        .y = box.y,
        .width = box.width,
        .height = box.height,
    };
    int width = dst_box.width;
    int height = dst_box.height;
    if (options->transform & WL_OUTPUT_TRANSFORM_90) {
        width = dst_box.height;
        height = dst_box.width;
    }

    struct mat3 uv2ndc;
    mat3_uvofbox_to_ndc(&uv2ndc, pass->target_width, pass->target_height, options->rotation_angle,
                        &dst_box);
    glUniformMatrix3fv(shader->uv2ndc, 1, GL_FALSE, uv2ndc.matrix);
    glUniform4f(shader->color, color->r, color->g, color->b, color->a);

    if (is_stroke || has_radius) {
        struct mat3 uv_rotation;
        mat3_invert_output_transform(&uv_rotation, options->transform);
        glUniformMatrix3fv(shader->uv_rotation, 1, GL_FALSE, uv_rotation.matrix);

        glUniform1f(shader->aspect, width / (float)height);
        float one_pixel_distance = 1.0f / height; // shader distance scale
        glUniform1f(shader->anti_aliasing, one_pixel_distance * 0.5f);
        glUniform4f(shader->round_corner_radius, options->radius.rb * one_pixel_distance,
                    options->radius.rt * one_pixel_distance,
                    options->radius.lb * one_pixel_distance,
                    options->radius.lt * one_pixel_distance);
    }
    if (is_stroke) {
        glUniform1f(shader->stroke_width, options->stroke_width / (float)height);
    }
    render(&renderer->wlr_renderer, &dst_box, options->base.clip, shader->uv_attrib);
final:
    PROFILE_RENDER_ZONE_END(&renderer->wlr_renderer);
    opengl_pop_debug(renderer);
}

static const struct render_pass_impl render_pass_impl = {
    .submit = opengl_render_pass_submit,
    .add_texture = opengl_render_pass_add_texture,
    .add_rect = opengl_render_pass_add_rect,
};

static const char *reset_status_str(GLenum status)
{
    switch (status) {
    case GL_GUILTY_CONTEXT_RESET_KHR:
        return "guilty";
    case GL_INNOCENT_CONTEXT_RESET_KHR:
        return "innocent";
    case GL_UNKNOWN_CONTEXT_RESET_KHR:
        return "unknown";
    default:
        return "<invalid>";
    }
}

static struct opengl_render_pass *opengl_begin_pass(struct opengl_renderer *renderer, int fbo,
                                                    int width, int64_t height,
                                                    struct egl_context *prev_ctx,
                                                    struct opengl_render_timer *timer)
{
    if (renderer->exts.KHR_robustness) {
        GLenum status = glGetGraphicsResetStatusKHR();
        if (status != GL_NO_ERROR) {
            kywc_log(KYWC_ERROR, "GPU reset (%s)", reset_status_str(status));
            wl_signal_emit_mutable(&renderer->wlr_renderer.events.lost, NULL);
            return NULL;
        }
    }

    struct opengl_render_pass *pass = calloc(1, sizeof(*pass));
    if (!pass) {
        return NULL;
    }

    pass->impl = &render_pass_impl;
    wlr_render_pass_init(&pass->base, &wlr_render_pass_impl);
    pass->renderer = renderer;
    pass->target_width = width;
    pass->target_height = height;
    pass->timer = timer;
    pass->prev_ctx = *prev_ctx;

    /* save initial state for store/restore (values we're about to set) */
    pass->saved_state.fbo = fbo;
    pass->saved_state.viewport[0] = 0;
    pass->saved_state.viewport[1] = 0;
    pass->saved_state.viewport[2] = width;
    pass->saved_state.viewport[3] = height;
    pass->saved_state.blend_src_rgb = GL_ONE;
    pass->saved_state.blend_dst_rgb = GL_ONE_MINUS_SRC_ALPHA;
    pass->saved_state.scissor_test_enabled = false;

    opengl_push_debug(renderer);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, pass->target_width, pass->target_height);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_SCISSOR_TEST);
    opengl_pop_debug(renderer);

    return pass;
}

struct opengl_render_pass *opengl_begin_buffer_pass(struct opengl_buffer *buffer,
                                                    struct egl_context *prev_ctx,
                                                    struct opengl_render_timer *timer)
{
    struct wlr_buffer *wlr_buffer = buffer->buffer;
    struct opengl_render_pass *pass = opengl_begin_pass(
        buffer->renderer, buffer->fbo, wlr_buffer->width, wlr_buffer->height, prev_ctx, timer);
    if (pass) {
        pass->buffer = buffer;
        wlr_buffer_lock(wlr_buffer);
    }

    return pass;
}

struct opengl_render_pass *opengl_begin_texture_pass(struct opengl_texture *texture,
                                                     struct opengl_render_timer *timer)
{
    assert(texture->fbo);
    struct opengl_renderer *renderer = texture->renderer;
    struct egl_context prev_ctx = { 0 };
    if (!egl_make_current(renderer->egl, &prev_ctx)) {
        return NULL;
    }

    if (timer) {
        clock_gettime(CLOCK_MONOTONIC, &timer->cpu_start);
    }

    struct opengl_render_pass *pass =
        opengl_begin_pass(texture->renderer, texture->fbo, texture->wlr_texture.width,
                          texture->wlr_texture.height, &prev_ctx, timer);
    if (pass) {
        pass->texture = texture;
    }

    return pass;
}

GLuint opengl_render_pass_get_fbo(struct opengl_render_pass *pass)
{
    assert(pass->buffer || pass->texture);
    return pass->buffer ? pass->buffer->fbo : pass->texture->fbo;
}

void opengl_render_pass_save(struct opengl_render_pass *pass)
{
    /* save current opengl state (not the initial state) */
}

void opengl_render_pass_restore(struct opengl_render_pass *pass)
{
    /* restore opengl state to the state when store was called */
    egl_make_current(pass->renderer->egl, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, pass->saved_state.fbo);
    glViewport(pass->saved_state.viewport[0], pass->saved_state.viewport[1],
               pass->saved_state.viewport[2], pass->saved_state.viewport[3]);
    glBlendFunc(pass->saved_state.blend_src_rgb, pass->saved_state.blend_dst_rgb);
    if (pass->saved_state.scissor_test_enabled) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}
