// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <xf86drm.h>

#include <wlr/util/region.h>

#include <kywc/log.h>

#include "drm_p.h"
#include "output.h"
#include "render/allocator.h"
#include "render/opengl.h"
#include "render/pixman.h"
#include "render/renderer.h"

#include "shaders/tex_frag_str.h"
#include "shaders/tex_oes_frag_str.h"
#include "shaders/tex_vert_str.h"
#include "shaders/tex_yuv_frag_str.h"

#include "util/drm_format.h"

#define MAX_QUADS 86 // 4kb

static bool set_renderer_drm_formats(struct drm_renderer *renderer,
                                     const struct wlr_drm_format_set *set)
{
    if (!renderer || !set) {
        return false;
    }

    for (size_t idx = 0; idx < set->len; idx++) {
        struct wlr_drm_format *fmt = &set->formats[idx];
        wlr_drm_format_set_add(&renderer->formats, fmt->format, DRM_FORMAT_MOD_LINEAR);
    }

    wlr_drm_format_set_add(&renderer->formats, DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR);
    return true;
}

static bool drm_mgpu_renderer_with_pixman(struct drm_device *drm, struct drm_renderer *renderer)
{
    renderer->wlr_rend = pixman_renderer_create();
    if (!renderer->wlr_rend) {
        kywc_log(KYWC_ERROR, "Failed to create a pixman renderer for mGPU");
        return false;
    }

    renderer->allocator = allocator_create(drm->wlr_backend, renderer->wlr_rend);
    if (!renderer->allocator) {
        kywc_log(KYWC_ERROR, "Failed to create allocator for renderer");
        return false;
    }

    for (size_t i = 0; i < drm->num_planes; i++) {
        struct drm_plane *drm_plane = &drm->planes[i];
        set_renderer_drm_formats(renderer, &drm_plane->formats);
    }

    return true;
}

bool drm_mgpu_renderer_init(struct drm_device *drm, struct drm_renderer *renderer)
{
    kywc_log(KYWC_INFO, "DRM mgpu render init");

    renderer->wlr_rend = opengl_renderer_create_with_drm_fd(drm->fd);
    if (!renderer->wlr_rend) {
        return drm_mgpu_renderer_with_pixman(drm, renderer);
    }

    const struct wlr_drm_format_set *texture_formats = NULL;
    texture_formats = wlr_renderer_get_dmabuf_texture_formats(renderer->wlr_rend);
    if (!texture_formats) {
        kywc_log(KYWC_WARN, "Failed to query renderer texture formats");
        wlr_renderer_destroy(renderer->wlr_rend);
        return drm_mgpu_renderer_with_pixman(drm, renderer);
    }

    renderer->allocator = allocator_create(drm->wlr_backend, renderer->wlr_rend);
    for (size_t i = 0; i < texture_formats->len; i++) {
        const struct wlr_drm_format *fmt = &texture_formats->formats[i];
        for (size_t j = 0; j < fmt->len; j++) {
            uint64_t mod = fmt->modifiers[j];
            if (mod == DRM_FORMAT_MOD_INVALID) {
                continue;
            }
            wlr_drm_format_set_add(&renderer->formats, fmt->format, mod);
        }
    }

    return true;
}

static void surface_swapchain_destroy(struct surf_swapchain *surf_swapchain)
{
    if (!surf_swapchain) {
        return;
    }

    swapchain_destroy(surf_swapchain->swapchain);
    free(surf_swapchain);
}

void drm_mgpu_renderer_finish(struct drm_renderer *renderer)
{
    if (!renderer) {
        return;
    }

    wlr_allocator_destroy(renderer->allocator);
    wlr_renderer_destroy(renderer->wlr_rend);
    wlr_drm_format_set_finish(&renderer->formats);
}

void drm_surface_finish(struct drm_surface *surf)
{
    if (!surf || !surf->wlr_rend) {
        return;
    }

    if (surf->dumb_swapchain != surf->swapchain) {
        surface_swapchain_destroy(surf->swapchain);
    }
    surface_swapchain_destroy(surf->dumb_swapchain);

    *surf = (struct drm_surface){ 0 };
}

static bool drm_format_intersect(struct wlr_drm_format *dst, const struct wlr_drm_format *a,
                                 const struct wlr_drm_format *b)
{
    assert(a->format == b->format);

    size_t capacity = a->len < b->len ? a->len : b->len;
    uint64_t *modifiers = malloc(sizeof(*modifiers) * capacity);
    if (!modifiers) {
        return false;
    }

    struct wlr_drm_format fmt = {
        .capacity = capacity,
        .len = 0,
        .modifiers = modifiers,
        .format = a->format,
    };

    for (size_t i = 0; i < a->len; i++) {
        for (size_t j = 0; j < b->len; j++) {
            if (a->modifiers[i] == b->modifiers[j]) {
                assert(fmt.len < fmt.capacity);
                fmt.modifiers[fmt.len++] = a->modifiers[i];
                break;
            }
        }
    }

    wlr_drm_format_finish(dst);
    *dst = fmt;

    return true;
}

static bool create_opengl_shader(struct opengl_renderer *gl_renderer, bool use_yuyv, bool oes)
{
    struct opengl_surface_shader *gl_shader =
        use_yuyv ? &gl_renderer->shaders.surface_yuv_tex : &gl_renderer->shaders.surface_tex;
    if (gl_shader->program) {
        return true;
    }

    GLuint prog = opengl_create_program(gl_renderer, tex_vert_str,
                                        use_yuyv ? tex_yuv_frag_str
                                        : oes    ? tex_oes_frag_str
                                                 : tex_frag_str);
    if (prog == 0) {
        return false;
    }

    gl_shader->program = prog;
    gl_shader->in_position = glGetAttribLocation(prog, "in_position");
    gl_shader->size = glGetUniformLocation(prog, "size");
    gl_shader->contrast = glGetUniformLocation(prog, "contrast");
    gl_shader->whitepoint = glGetUniformLocation(prog, "whitepoint");
    gl_shader->color_matrix = glGetUniformLocation(prog, "color_matrix");
    gl_shader->brightness = glGetUniformLocation(prog, "brightness");
    gl_shader->texture = glGetUniformLocation(prog, "tex");

    if (use_yuyv) {
        gl_shader->pixel_offset_x = glGetUniformLocation(prog, "pixel_offset_x");
    }

    return true;
}

static bool renderer_add_texture(struct wlr_renderer *renderer, struct wlr_texture *src_texture,
                                 struct drm_render_target *target)
{
    if (target->damage && !pixman_region32_not_empty(target->damage)) {
        return true;
    }

    int width = src_texture->width;
    int height = src_texture->height;
    if (width <= 0 || height <= 0) {
        return true;
    }

    struct opengl_renderer *gl_renderer = opengl_renderer_from_wlr_renderer(renderer);
    struct opengl_texture *texture = opengl_texture_from_wlr_texture(src_texture);
    if (!create_opengl_shader(gl_renderer, target->use_yuyv,
                              texture->target == GL_TEXTURE_EXTERNAL_OES)) {
        return false;
    }

    struct opengl_surface_shader *gl_shader = target->use_yuyv
                                                  ? &gl_renderer->shaders.surface_yuv_tex
                                                  : &gl_renderer->shaders.surface_tex;
    glUseProgram(gl_shader->program);

    glUniform2f(gl_shader->size, width, height);

    float contrast = 1.0f;
    glUniform1f(gl_shader->contrast, contrast);

    float whitepoint[3] = { 0 };
    drm_color_temp_to_rgb(target->color_temp, whitepoint);
    glUniform3fv(gl_shader->whitepoint, 1, whitepoint);

    glUniformMatrix4fv(gl_shader->color_matrix, 1, GL_FALSE, target->color_mat);
    glUniform1f(gl_shader->brightness, (float)target->brightness * 0.01f);

    if (target->use_yuyv) {
        float pixel_offset_x = 1.0f / src_texture->width;
        glUniform1f(gl_shader->pixel_offset_x, pixel_offset_x);
    }

    glDisable(GL_BLEND);
    // bind texture
    glUniform1i(gl_shader->texture, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(texture->target, texture->tex);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // draw damage
    pixman_region32_t clipped;
    pixman_region32_init_rect(&clipped, 0, 0, width, height);
    if (target->damage) {
        float scale = target->output->base.state.scale;
        if (target->use_yuyv && floor(scale) == scale) {
            wlr_region_expand(&clipped, target->damage, 1);
            pixman_region32_intersect_rect(&clipped, &clipped, 0, 0, width, height);
        } else {
            pixman_region32_intersect_rect(&clipped, target->damage, 0, 0, width, height);
        }
    }

    glEnableVertexAttribArray(gl_shader->in_position);

    int rects_len = 0;
    const pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &rects_len);
    for (int i = 0; i < rects_len;) {
        int batch = rects_len - i < MAX_QUADS ? rects_len - i : MAX_QUADS;
        int batch_end = batch + i;

        size_t vert_index = 0;
        GLfloat verts[MAX_QUADS * 6 * 2];
        for (; i < batch_end; i++) {
            const pixman_box32_t *rect = &rects[i];
            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y1;
            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y1;
            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y2;
            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y1;
            verts[vert_index++] = (GLfloat)rect->x2;
            verts[vert_index++] = (GLfloat)rect->y2;
            verts[vert_index++] = (GLfloat)rect->x1;
            verts[vert_index++] = (GLfloat)rect->y2;
        }

        glVertexAttribPointer(gl_shader->in_position, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glDrawArrays(GL_TRIANGLES, 0, batch * 6);
    }

    glDisableVertexAttribArray(gl_shader->in_position);
    glBindTexture(texture->target, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    pixman_region32_fini(&clipped);
    return true;
}

static bool readpixel_from_buffer(struct wlr_renderer *wlr_rend, struct wlr_buffer *source,
                                  struct wlr_buffer *dst, const pixman_region32_t *damage)
{
    if (damage && !pixman_region32_not_empty(damage)) {
        return true;
    }

    size_t stride;
    uint32_t format = 0;
    void *dst_ptr = NULL;
    if (!wlr_buffer_begin_data_ptr_access(dst, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &dst_ptr, &format,
                                          &stride)) {
        kywc_log(KYWC_ERROR, "Failed to get buffer data %p", dst);
        return false;
    }

    struct wlr_dmabuf_attributes attribs = { .format = format };
    if (!wlr_buffer_get_dmabuf(source, &attribs) && !wlr_renderer_is_pixman_renderer(wlr_rend)) {
        wlr_buffer_end_data_ptr_access(dst);
        return false;
    }

    if (!wlr_renderer_begin_with_buffer(wlr_rend, source)) {
        wlr_buffer_end_data_ptr_access(dst);
        return false;
    }

    int x = 0, y = 0;
    int width = source->width;
    int height = source->height;

    if (damage) {
        pixman_region32_t clipped;
        pixman_region32_init(&clipped);
        pixman_region32_intersect_rect(&clipped, damage, 0, 0, width, height);
        y = clipped.extents.y1;
        height = clipped.extents.y2 - clipped.extents.y1;
        pixman_region32_fini(&clipped);
    }

    if (!wlr_renderer_read_pixels(wlr_rend, attribs.format, stride, width, height, x, y, x, y,
                                  dst_ptr)) {
        kywc_log(KYWC_ERROR, "Faild read pixels from primary renderer");
    }

    wlr_renderer_end(wlr_rend);
    wlr_buffer_end_data_ptr_access(dst);

    return true;
}

static struct wlr_buffer *blit_buffer_with_opengl(struct drm_surface *surf,
                                                  struct drm_render_target *target)
{
    struct wlr_renderer *renderer = target->wlr_rend;
    struct wlr_texture *tex = wlr_texture_from_buffer(renderer, target->source);
    if (!tex) {
        kywc_log(KYWC_ERROR, "Failed to import source buffer into renderer");
        return NULL;
    }

    struct wlr_buffer *dst = swapchain_acquire(surf->swapchain->swapchain, NULL);
    if (!dst) {
        kywc_log(KYWC_ERROR, "Failed to acquire swapchain buffer");
        goto error_tex;
    }

    struct wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(renderer, dst, NULL);
    if (!pass) {
        kywc_log(KYWC_ERROR, "Failed to begin render pass on destination buffer");
        goto error_dst;
    }

    if (target->rgb_clear) {
        struct render_rect_options option = (struct render_rect_options){
            .base={
                .box = { .width = dst->width, .height = dst->height },
                .clip = target->damage,
                .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
            },
        };
        render_pass_add_rect(pass, &option);
    }

    renderer_add_texture(renderer, tex, target);

    if (!render_pass_submit(pass, target->output->quirks)) {
        kywc_log(KYWC_ERROR, "Failed to submit render pass");
        goto error_dst;
    }

    wlr_texture_destroy(tex);

    return dst;

error_dst:
    wlr_buffer_unlock(dst);
error_tex:
    wlr_texture_destroy(tex);
    return NULL;
}

bool drm_plane_pick_render_format(struct wlr_renderer *renderer,
                                  const struct wlr_drm_format_set *display_formats,
                                  struct wlr_drm_format *format, uint32_t fmt)
{
    const struct wlr_drm_format *render_format = renderer_get_render_format(renderer, fmt);
    if (render_format == NULL) {
        kywc_log(KYWC_ERROR, "Failed to get render formats");
        return false;
    }

    const struct wlr_drm_format *display_format = wlr_drm_format_set_get(display_formats, fmt);
    if (display_format == NULL) {
        kywc_log(KYWC_ERROR, "Output plane doesn't support format 0x%" PRIX32, fmt);
        return false;
    }

    if (!drm_format_intersect(format, display_format, render_format)) {
        kywc_log(KYWC_ERROR,
                 "Failed to intersect display and render modifiers for format 0x%" PRIX32 "", fmt);
        return false;
    }

    if (format->len == 0) {
        wlr_drm_format_finish(format);
        kywc_log(KYWC_WARN, "Failed to pick output format");
        return false;
    }

    return true;
}

static bool test_swapchain(struct wlr_output *output, struct swapchain *swapchain,
                           const struct wlr_output_state *state)
{
    struct wlr_buffer *buffer = swapchain_acquire(swapchain, NULL);
    if (buffer == NULL) {
        return false;
    }

    struct wlr_output_state copy = *state;
    copy.committed |= WLR_OUTPUT_STATE_BUFFER;
    copy.buffer = buffer;
    bool ok = wlr_output_test_state(output, &copy);
    wlr_buffer_unlock(buffer);
    return ok;
}

static struct surf_swapchain *surface_swapchain_create(struct wlr_allocator *allocator,
                                                       enum allocator_buffer_type type,
                                                       struct swapchain_option *option)
{
    struct surf_swapchain *surf_swapchain = calloc(1, sizeof(*surf_swapchain));
    if (!surf_swapchain) {
        return NULL;
    }

    struct wlr_drm_format *render_format = option->drm_format;
    struct wlr_output *wlr_output = option->output;

    char *format_name = drmGetFormatName(render_format->format);
    kywc_log(KYWC_DEBUG, "Choosing plane buffer format %s (0x%08" PRIX32 ") for output '%s'",
             format_name ? format_name : "<unknown>", render_format->format, wlr_output->name);
    free(format_name);

    struct swapchain *swapchain = allocator_create_swapchain(allocator, type, option->width,
                                                             option->height, render_format, 0);
    if (!swapchain) {
        wlr_drm_format_finish(render_format);
        free(swapchain);
        return NULL;
    }

    if (!option->state) {
        surf_swapchain->swapchain = swapchain;
        surf_swapchain->width = option->width;
        surf_swapchain->height = option->height;
        return surf_swapchain;
    }

    if (!test_swapchain(wlr_output, swapchain, option->state)) {
        kywc_log(KYWC_DEBUG, "Output test failed on '%s', retrying without modifiers",
                 option->output->name);
        swapchain_destroy(swapchain);
        swapchain = NULL;

        if (render_format->len != 1 || render_format->modifiers[0] != DRM_FORMAT_MOD_LINEAR) {
            if (!drm_format_has(render_format, DRM_FORMAT_MOD_INVALID)) {
                kywc_log(KYWC_DEBUG, "Implicit modifiers not supported");
                wlr_drm_format_finish(render_format);
            }
        }

        render_format->len = 0;
        if (!drm_format_add(render_format, DRM_FORMAT_MOD_INVALID)) {
            kywc_log(KYWC_DEBUG, "Failed to add implicit modifier to format");
            goto error;
        }

        swapchain = allocator_create_swapchain(allocator, type, option->width, option->height,
                                               render_format, 0);
        if (!test_swapchain(wlr_output, swapchain, option->state)) {
            kywc_log(KYWC_DEBUG, "Output test failed on '%s', retrying without modifiers",
                     option->output->name);
            goto error;
        }
    }

    surf_swapchain->width = option->width;
    surf_swapchain->height = option->height;
    surf_swapchain->swapchain = swapchain;
    return surf_swapchain;

error:
    wlr_drm_format_finish(render_format);
    swapchain_destroy(swapchain);
    free(surf_swapchain);
    return NULL;
}

bool drm_plane_configure_surface_swapchain(struct drm_plane *plane, struct drm_connector *conn,
                                           struct drm_renderer *mgpu_renderer, int width,
                                           int height, uint32_t format,
                                           const struct wlr_output_state *state, bool use_yuyv,
                                           bool buffer_match_plane)
{
    struct drm_surface *surf = &plane->multi_surf;
    if (surf->swapchain && surf->swapchain->swapchain && surf->swapchain->width == width &&
        surf->swapchain->height == height) {
        return true;
    }

    if (surf->dumb_swapchain != surf->swapchain) {
        surface_swapchain_destroy(surf->swapchain);
    }

    surface_swapchain_destroy(surf->dumb_swapchain);
    surf->swapchain = NULL;
    surf->dumb_swapchain = NULL;

    /* both primary and secondary gpu use pixman */
    bool both_pixman = false;
    struct wlr_drm_format render_format = { 0 };
    struct wlr_renderer *renderer = conn->output.renderer;
    struct wlr_allocator *allocator = conn->output.allocator;
    const struct wlr_drm_format_set *display_formats = &plane->formats;
    struct surf_swapchain *swapchain = NULL;
    struct surf_swapchain *dumb_swapchain = NULL;

    if (mgpu_renderer != NULL) {
        display_formats = &mgpu_renderer->formats;
        if (!wlr_renderer_is_pixman_renderer(mgpu_renderer->wlr_rend)) {
            renderer = mgpu_renderer->wlr_rend;
            allocator = mgpu_renderer->allocator;
        } else if (wlr_renderer_is_pixman_renderer(renderer)) {
            both_pixman = true;
            goto dumb_alloc;
        }
    } else if (wlr_renderer_is_pixman_renderer(renderer) &&
               allocator->buffer_caps & WLR_BUFFER_CAP_DMABUF) {
        surf->wlr_rend = renderer;
        return true;
    }

    struct wlr_drm_format_set yuv_display_formats = { 0 };
    if (!mgpu_renderer && use_yuyv) {
        wlr_drm_format_set_add(&yuv_display_formats, format, DRM_FORMAT_MOD_LINEAR);
        display_formats = &yuv_display_formats;
    }

    if (!buffer_match_plane) {
        const struct wlr_drm_format *_render_format = renderer_get_render_format(renderer, format);
        if (_render_format) {
            drm_format_copy(&render_format, _render_format);
        }
    } else {
        drm_plane_pick_render_format(renderer, display_formats, &render_format, format);
    }
    wlr_drm_format_set_finish(&yuv_display_formats);

    if (render_format.len == 0) {
        kywc_log(KYWC_WARN, "Renderer pick renderer format failed");
        return false;
    }

    enum allocator_buffer_type type = wlr_renderer_is_pixman_renderer(renderer)
                                          ? ALLOCATOR_BUFFER_TYPE_DUMB
                                          : ALLOCATOR_BUFFER_TYPE_GBM;

    struct swapchain_option option = {
        .width = width,
        .height = height,
        .output = &conn->output,
        .drm_format = &render_format,
        .state = use_yuyv ? NULL : state,
    };
    swapchain = surface_swapchain_create(allocator, type, &option);
    if (!swapchain) {
        return false;
    }
    wlr_drm_format_finish(&render_format);

    if (!mgpu_renderer && type == ALLOCATOR_BUFFER_TYPE_DUMB) {
        dumb_swapchain = swapchain;
    }

dumb_alloc:
    if ((mgpu_renderer && wlr_renderer_is_pixman_renderer(mgpu_renderer->wlr_rend)) ||
        !buffer_match_plane) {
        struct wlr_renderer *dumb_renderer =
            mgpu_renderer ? mgpu_renderer->wlr_rend : conn->output.renderer;
        struct wlr_allocator *dumb_allocator =
            mgpu_renderer ? mgpu_renderer->allocator : conn->output.allocator;

        const struct wlr_drm_format *_drm_format = wlr_drm_format_set_get(&plane->formats, format);
        uint32_t disp_format = format;
        if (!_drm_format) {
            disp_format = DRM_FORMAT_XRGB8888;
        }

        struct wlr_drm_format drm_format = { 0 };
        if (!buffer_match_plane) {
            _drm_format = wlr_drm_format_set_get(&plane->formats, disp_format);
            if (!_drm_format) {
                surface_swapchain_destroy(swapchain);
                return false;
            }
            drm_format_copy(&drm_format, _drm_format);
        } else {
            if (!drm_plane_pick_render_format(dumb_renderer, &plane->formats, &drm_format,
                                              disp_format)) {
                kywc_log(KYWC_WARN, "surface dumb format format failed");
                surface_swapchain_destroy(swapchain);
                return false;
            }
        }

        struct swapchain_option option = {
            .width = width,
            .height = height,
            .output = &conn->output,
            .drm_format = &drm_format,
            .state = NULL,
        };
        dumb_swapchain =
            surface_swapchain_create(dumb_allocator, ALLOCATOR_BUFFER_TYPE_DUMB, &option);
        if (!dumb_swapchain) {
            surface_swapchain_destroy(swapchain);
            return false;
        }
        wlr_drm_format_finish(&drm_format);

        if (both_pixman) {
            swapchain = dumb_swapchain;
        }
    }

    surf->swapchain = swapchain;
    surf->dumb_swapchain = dumb_swapchain;
    surf->wlr_rend = renderer;

    return true;
}

struct wlr_buffer *drm_surface_blit(struct drm_surface *surface, struct drm_render_target *target)
{
    if (wlr_renderer_is_pixman_renderer(surface->wlr_rend) && !surface->swapchain &&
        !surface->dumb_swapchain) {
        // do not blit
        return wlr_buffer_lock(target->source);
    }

    struct wlr_buffer *buffer = target->source;
    if (wlr_renderer_is_opengl(surface->wlr_rend)) {
        buffer = blit_buffer_with_opengl(surface, target);
    } else {
        buffer = wlr_buffer_lock(buffer);
    }

    if (!surface->dumb_swapchain) {
        return buffer;
    }

    struct wlr_buffer *dumb_buffer = swapchain_acquire(surface->dumb_swapchain->swapchain, NULL);
    if (!dumb_buffer) {
        kywc_log(KYWC_ERROR, "Failed to acquire multi-GPU dumb swapchain buffer");
        goto error;
    }

    if (!readpixel_from_buffer(target->wlr_rend, buffer, dumb_buffer, target->damage)) {
        goto error_pixel;
    }

    wlr_buffer_unlock(buffer);
    return dumb_buffer;

error_pixel:
    wlr_buffer_unlock(dumb_buffer);
error:
    wlr_buffer_unlock(buffer);
    return NULL;
}
