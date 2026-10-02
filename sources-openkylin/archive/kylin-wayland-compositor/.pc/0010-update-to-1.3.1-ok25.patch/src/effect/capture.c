// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>

#include <drm_fourcc.h>
#include <wlr/util/region.h>

#include <kywc/log.h>

#include "effect/capture.h"
#include "effect_p.h"
#include "output.h"
#include "painter.h"
#include "render/renderer.h"
#include "security.h"
#include "server.h"

enum capture_type {
    CAPTURE_TYPE_OUTPUT = 0,
    CAPTURE_TYPE_AREA,
    CAPTURE_TYPE_FULLSCREEN,
};

struct capture {
    struct wl_list link;
    struct capture_buffer *buffer;

    const struct capture_interface *impl;
    void *user_data;

    bool force_update, wants_update;
};

struct capture_buffer {
    struct wl_list link;
    struct wlr_buffer *buffer;
    struct wlr_buffer *shadow;

    struct wl_list captures; // capture->link
    struct wl_list requests; // request->link

    enum capture_type type;
    uint32_t options;
    union {
        struct output *output;
        struct wlr_box area;
    };

    float scale;
    int width, height; // may != buffer size
    bool own_buffer, buffer_changed;

    bool was_damaged, has_content;
    bool need_destroy, can_destroy;
};

struct capture_request {
    struct wl_list link;
    struct wl_list output_link;
    struct capture_buffer *buffer;
    struct capture_output *output;

    struct wlr_fbox src_box;
    struct wlr_box dst_box;
    bool blit_done, cursor_locked;
};

struct capture_output {
    struct wl_list link;
    struct wl_list requests; // capture_request->output_link;

    struct output *output;
    struct wl_listener commit;
    struct wl_listener geometry;
    struct wl_listener disable;
    struct wl_listener power;

    struct wl_event_source *timer; // fake commit when power off
    bool commit_pending;
};

struct capture_manager {
    struct wl_list buffers; // capture_buffer->link
    struct wl_list outputs; // capture_output->link

    struct wl_listener new_enabled_output;

    struct server *server;
    struct wl_listener destroy;
};

static struct capture_manager *manager = NULL;

static struct capture_request *capture_request_create(struct output *output,
                                                      struct capture_buffer *buffer);

static void capture_request_destroy(struct capture_request *request);

static bool capture_clear_buffer(struct wlr_buffer *buffer, const pixman_region32_t *clear)
{
    /* get a renderer to clear the buffer */
    struct wlr_renderer *renderer = manager->server->renderer;
    if (!renderer_check_render_buffer(renderer, buffer)) {
        renderer = server_get_pixman_renderer(manager->server);
        if (!renderer) {
            return false;
        }
    }

    struct wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(renderer, buffer, NULL);
    if (!pass) {
        return false;
    }

    wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options){
                                       .color = { 0, 0, 0, 0 },
                                       .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                       .clip = clear,
                                   });
    wlr_render_pass_submit(pass);

    return true;
}

static void capture_buffer_calc_size(struct capture_buffer *capture_buffer)
{
    assert(!wl_list_empty(&capture_buffer->requests));

    if (capture_buffer->type == CAPTURE_TYPE_OUTPUT ||
        capture_buffer->type == CAPTURE_TYPE_FULLSCREEN) {
        pixman_region32_t region;
        pixman_region32_init(&region);
        struct capture_request *request;
        wl_list_for_each(request, &capture_buffer->requests, link) {
            pixman_region32_union_rect(&region, &region, request->dst_box.x, request->dst_box.y,
                                       request->dst_box.width, request->dst_box.height);
        }
        capture_buffer->width = region.extents.x2 - region.extents.x1;
        capture_buffer->height = region.extents.y2 - region.extents.y1;
        pixman_region32_fini(&region);
    } else if (capture_buffer->type == CAPTURE_TYPE_AREA) {
        capture_buffer->width = capture_buffer->area.width;
        capture_buffer->height = capture_buffer->area.height;
    }
}

static struct wlr_buffer *capture_buffer_allocate(struct capture_buffer *capture_buffer)
{
    struct wlr_buffer *buffer = capture_buffer->buffer;
    bool need_create = !buffer || (buffer->width != capture_buffer->width ||
                                   buffer->height != capture_buffer->height);
    if (!need_create) {
        return buffer;
    }

    buffer =
        renderer_create_buffer(manager->server->renderer, manager->server->allocator,
                               capture_buffer->width, capture_buffer->height, DRM_FORMAT_ARGB8888,
                               capture_buffer->options & CAPTURE_NEED_SINGLE_PLANE);
    if (!buffer) {
        kywc_log(KYWC_ERROR, "Failed to create wlr buffer for capture");
        return capture_buffer->buffer;
    }

    capture_clear_buffer(buffer, NULL);

    return buffer;
}

static bool capture_buffer_ensure_shadow_buffer(struct capture_buffer *capture_buffer)
{
    /* no need to create shadow buffer when support render */
    if (capture_buffer->own_buffer ||
        renderer_check_render_buffer(manager->server->renderer, capture_buffer->buffer)) {
        wlr_buffer_drop(capture_buffer->shadow);
        capture_buffer->shadow = NULL;
        return true;
    }

    struct wlr_buffer *buffer = capture_buffer->shadow;
    int width = capture_buffer->buffer->width;
    int height = capture_buffer->buffer->height;

    bool need_create = !buffer || (buffer->width != width || buffer->height != height);
    if (!need_create) {
        return true;
    }

    /* make sure size is equal to capture_buffer->buffer size */
    wlr_buffer_drop(buffer);

    capture_buffer->shadow = renderer_create_buffer(
        manager->server->renderer, manager->server->allocator, width, height,
        render_buffer_is_opaque(capture_buffer->buffer) ? DRM_FORMAT_XRGB8888 : DRM_FORMAT_ARGB8888,
        capture_buffer->options & CAPTURE_NEED_SINGLE_PLANE);
    if (!capture_buffer->shadow) {
        kywc_log(KYWC_ERROR, "Failed to create wlr buffer for capture shadow");
        return false;
    }

    return true;
}

static bool capture_buffer_ensure_buffer(struct capture_buffer *capture_buffer)
{
    struct wlr_buffer *buffer = NULL;

    if (capture_buffer->own_buffer) {
        buffer = capture_buffer_allocate(capture_buffer);
    } else {
        struct capture *capture = wl_container_of(capture_buffer->captures.next, capture, link);
        buffer = capture->impl->buffer(
            capture, capture_buffer->buffer, capture_buffer->width, capture_buffer->height,
            capture_buffer->options & CAPTURE_NEED_SINGLE_PLANE, capture->user_data);
    }

    if (!buffer) {
        return false;
    }

    if (buffer == capture_buffer->buffer) {
        return true;
    }

    if (capture_buffer->own_buffer) {
        wlr_buffer_drop(capture_buffer->buffer);
    }
    capture_buffer->buffer = buffer;
    capture_buffer->buffer_changed = true;
    capture_buffer->has_content = false;

    if (!capture_buffer_ensure_shadow_buffer(capture_buffer)) {
        return false;
    }

    /* re-blit all requests */
    struct capture_request *request;
    wl_list_for_each(request, &capture_buffer->requests, link) {
        request->blit_done = false;
    }

    return true;
}

static void fix_src(struct wlr_fbox *src, int32_t width, int32_t height)
{
    double x1 = fmax(src->x, 0);
    double y1 = fmax(src->y, 0);
    double x2 = fmin(src->x + src->width, width);
    double y2 = fmin(src->y + src->height, height);

    src->x = x1;
    src->y = y1;
    src->width = x2 - x1;
    src->height = y2 - y1;
}

static bool capture_buffer_calc_box(struct capture_buffer *buffer, struct output *output,
                                    struct wlr_box *dst, struct wlr_fbox *src)
{
    float scale = 1.0;
    if (buffer->options & CAPTURE_NEED_UNSCALED) {
        scale = buffer->options & CAPTURE_PREFER_OUTPUT_SCALE ? output->wlr_output->scale
                                                              : output_manager_get_max_scale();
    }

    struct kywc_box *geo = &output->geometry;
    *dst = (struct wlr_box){ 0 };
    *src = (struct wlr_fbox){ 0 };

    if (buffer->type == CAPTURE_TYPE_OUTPUT) {
        /* calc the dst box in request */
        dst->width = geo->width * scale;
        dst->height = geo->height * scale;
    } else if (buffer->type == CAPTURE_TYPE_AREA) {
        *dst = (struct wlr_box){ geo->x * scale, geo->y * scale, geo->width * scale,
                                 geo->height * scale };
        if (!wlr_box_intersection(dst, dst, &buffer->area)) {
            return false;
        }

        float output_scale = output->wlr_output->scale;
        src->x = (dst->x - geo->x * scale) / scale * output_scale;
        src->y = (dst->y - geo->y * scale) / scale * output_scale;
        src->width = dst->width / scale * output_scale;
        src->height = dst->height / scale * output_scale;

        int width, height;
        /* translate to buffer coord, otherwise assert failed in wlr_render_pass_add_texture */
        wlr_output_transformed_resolution(output->wlr_output, &width, &height);
        wlr_fbox_transform(src, src, wlr_output_transform_invert(output->wlr_output->transform),
                           width, height);
        fix_src(src, output->wlr_output->width, output->wlr_output->height);

        dst->x -= buffer->area.x;
        dst->y -= buffer->area.y;
    } else if (buffer->type == CAPTURE_TYPE_FULLSCREEN) {
        /* assume that we always have zero coord */
        *dst = (struct wlr_box){ geo->x * scale, geo->y * scale, geo->width * scale,
                                 geo->height * scale };
    }

    buffer->scale = scale;
    return true;
}

static void capture_manager_schedule_frame(bool force)
{
    if (!manager->server->active) {
        return;
    }

    struct capture_output *output;
    wl_list_for_each(output, &manager->outputs, link) {
        struct wlr_output *wlr_output = output->output->wlr_output;
        if (force && !wlr_output->enabled && !output->commit_pending) {
            int32_t refresh = wlr_output->refresh > 0 ? wlr_output->refresh : 60000;
            wl_event_source_timer_update(output->timer, 1000000 / refresh);
            output->commit_pending = true;
        }
        output_schedule_frame(wlr_output);
    }
}

static void capture_buffer_destroy(struct capture_buffer *buffer)
{
    /* don't destroy if still have captures */
    if (!buffer->need_destroy && !wl_list_empty(&buffer->captures)) {
        return;
    }
    /* mark need_destroy if cannot be destroyed current */
    if (!buffer->can_destroy) {
        buffer->need_destroy = true;
        return;
    }

    struct capture_request *request, *request_tmp;
    wl_list_for_each_safe(request, request_tmp, &buffer->requests, link) {
        request->buffer = NULL;
        capture_request_destroy(request);
    }

    /* force destroy all captures */
    struct capture *capture, *tmp;
    wl_list_for_each_safe(capture, tmp, &buffer->captures, link) {
        capture->buffer = NULL;
        if (capture->impl && capture->impl->cancel) {
            capture->impl->cancel(capture, capture->user_data);
        }
        capture_destroy(capture);
    }

    if (buffer->own_buffer) {
        wlr_buffer_drop(buffer->buffer);
    }
    wlr_buffer_drop(buffer->shadow);

    wl_list_remove(&buffer->link);
    free(buffer);
}

static void capture_output_destroy(struct capture_output *output)
{
    wl_list_remove(&output->commit.link);
    wl_list_remove(&output->geometry.link);
    wl_list_remove(&output->disable.link);
    wl_list_remove(&output->power.link);
    wl_list_remove(&output->link);

    struct capture_request *request, *tmp;
    wl_list_for_each_safe(request, tmp, &output->requests, output_link) {
        capture_request_destroy(request);
    }

    wl_event_source_remove(output->timer);
    free(output);
}

static void capture_request_destroy(struct capture_request *request)
{
    wl_list_remove(&request->link);
    wl_list_remove(&request->output_link);

    /* destroy capture_buffer if no requests */
    struct capture_buffer *buffer = request->buffer;
    if (buffer) {
        if (wl_list_empty(&buffer->requests)) {
            buffer->need_destroy = true;
            capture_buffer_destroy(buffer);
        } else if (buffer->type == CAPTURE_TYPE_AREA && buffer->buffer) {
            /* clear the buffer when area type */
            pixman_region32_t clear;
            pixman_region32_init_rect(&clear, request->dst_box.x, request->dst_box.y,
                                      request->dst_box.width, request->dst_box.height);
            capture_clear_buffer(buffer->shadow ? buffer->shadow : buffer->buffer, &clear);
            pixman_region32_fini(&clear);
        }
    }

    struct wlr_output *wlr_output = request->output->output->wlr_output;
    wlr_output_lock_attach_render(wlr_output, false);
    if (request->cursor_locked) {
        wlr_output_lock_software_cursors(wlr_output, false);
    }

    free(request);
}

static void capture_output_handle_disable(struct wl_listener *listener, void *data)
{
    struct capture_output *capture_output = wl_container_of(listener, capture_output, disable);
    capture_output_destroy(capture_output);
    /* re-allocate buffer */
    struct capture_buffer *buffer;
    wl_list_for_each(buffer, &manager->buffers, link) {
        capture_buffer_calc_size(buffer);
    }
    capture_manager_schedule_frame(false);
}

static void capture_output_handle_geometry(struct wl_listener *listener, void *data)
{
    struct capture_output *capture_output = wl_container_of(listener, capture_output, geometry);

    /* update requests src and dst box if output geometry changed */
    struct capture_request *request, *tmp;
    wl_list_for_each_safe(request, tmp, &capture_output->requests, output_link) {
        if (capture_buffer_calc_box(request->buffer, capture_output->output, &request->dst_box,
                                    &request->src_box)) {
            capture_buffer_calc_size(request->buffer);
        } else {
            capture_request_destroy(request);
        }
    }

    /* update capture_buffer with area type */
    struct capture_buffer *buffer;
    wl_list_for_each(buffer, &manager->buffers, link) {
        if (buffer->type != CAPTURE_TYPE_AREA) {
            continue;
        }

        bool request_found = false;
        wl_list_for_each(request, &buffer->requests, link) {
            if (request->output == capture_output) {
                request_found = true;
                break;
            }
        }

        if (!request_found) {
            capture_request_create(capture_output->output, buffer);
            /* no need to re-allocate buffer when area type */
        }
    }

    capture_manager_schedule_frame(false);
}

static bool capture_buffer_blit(struct capture_buffer *buffer, struct capture_request *request,
                                struct wlr_buffer *src_buffer, const pixman_region32_t *clip)
{
    struct wlr_output *wlr_output = request->output->output->wlr_output;

    kywc_log(KYWC_DEBUG, "Capture output %s copy (%f, %f) %f x %f to (%d, %d) %d x %d",
             wlr_output->name, request->src_box.x, request->src_box.y, request->src_box.width,
             request->src_box.height, request->dst_box.x, request->dst_box.y,
             request->dst_box.width, request->dst_box.height);

    struct wlr_renderer *renderer = wlr_output->renderer;
    /* render src buffer to shadow buffer, then read pixels to dst buffer */
    struct wlr_buffer *dst_buffer = buffer->shadow ? buffer->shadow : buffer->buffer;

    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(renderer, dst_buffer, NULL);
    if (!render_pass) {
        return false;
    }

    struct wlr_texture *src_tex = wlr_texture_from_buffer(renderer, src_buffer);
    struct wlr_render_texture_options options = {
        .texture = src_tex,
        .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
        .src_box = request->src_box,
        .dst_box = request->dst_box,
        .clip = clip,
        .transform = wlr_output_transform_invert(wlr_output->transform),
    };
    wlr_render_pass_add_texture(render_pass, &options);
    wlr_texture_destroy(src_tex);
    wlr_render_pass_submit(render_pass);

    if (dst_buffer == buffer->buffer) {
        return true;
    }

    struct wlr_box box = request->dst_box;
    /* if buffer size is not equal to desired size */
    if (dst_buffer->width != buffer->width || dst_buffer->height != buffer->height) {
        if (!wlr_box_intersection(
                &box, &box, &(struct wlr_box){ 0, 0, dst_buffer->width, dst_buffer->height })) {
            return true;
        }
    }

    void *data;
    uint32_t format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(buffer->buffer, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &data,
                                          &format, &stride)) {
        return false;
    }

    bool ok = false;
    if (wlr_renderer_begin_with_buffer(renderer, dst_buffer)) {
        ok = wlr_renderer_read_pixels(renderer, format, stride, box.width, box.height, box.x, box.y,
                                      box.x, box.y, data);
        wlr_renderer_end(renderer);
    }

    wlr_buffer_end_data_ptr_access(buffer->buffer);
    return ok;
}

static bool capture_request_blit_buffer(struct capture_request *request, struct wlr_buffer *src)
{
    struct capture_buffer *buffer = request->buffer;
    struct output *output = request->output->output;

    if (!capture_buffer_ensure_buffer(buffer)) {
        return false;
    }

    struct kywc_box *logical = &output->geometry;
    float scale = 1.0;

    struct kywc_box *viewport_src = &output->scene_output->viewport.src;
    bool use_viewport = kywc_box_not_empty(viewport_src) && !kywc_box_equal(logical, viewport_src);
    // use viewport src when zoom is enabled
    if (use_viewport) {
        // zoom scale
        scale = (float)output->geometry.width / viewport_src->width;
        logical = viewport_src;
    }

    // scene layout logical coord region to copy
    pixman_region32_t clip;

    if (buffer->type == CAPTURE_TYPE_AREA) {
        pixman_region32_init_rect(&clip, request->dst_box.x + buffer->area.x,
                                  request->dst_box.y + buffer->area.y, request->dst_box.width,
                                  request->dst_box.height);
        wlr_region_scale(&clip, &clip, 1.0 / buffer->scale);
        // calc the logical region in viewport
        pixman_region32_translate(&clip, -output->geometry.x, -output->geometry.y);
        wlr_region_scale(&clip, &clip, 1.0 / scale);
        pixman_region32_translate(&clip, logical->x, logical->y);
    } else {
        pixman_region32_init_rect(&clip, logical->x, logical->y, logical->width, logical->height);
    }

    /* remove security region in clip */
    bool touched = security_check_output(output, &clip);
    /* no need to blit when clear all */
    bool need_blit = !touched || pixman_region32_not_empty(&clip);
    bool ok = true;

    // clear the buffer if security touched
    if (touched) {
        /* clip to buffer coord in dst buffer */
        switch (buffer->type) {
        case CAPTURE_TYPE_OUTPUT:
            pixman_region32_translate(&clip, -logical->x, -logical->y);
            wlr_region_scale(&clip, &clip, scale * buffer->scale);
            break;
        case CAPTURE_TYPE_FULLSCREEN:
            pixman_region32_translate(&clip, -logical->x, -logical->y);
            wlr_region_scale(&clip, &clip, scale * buffer->scale);
            pixman_region32_translate(&clip, request->dst_box.x, request->dst_box.y);
            break;
        case CAPTURE_TYPE_AREA:
            pixman_region32_translate(&clip, -logical->x, -logical->y);
            wlr_region_scale(&clip, &clip, scale);
            pixman_region32_translate(&clip, output->geometry.x, output->geometry.y);
            wlr_region_scale(&clip, &clip, buffer->scale);
            pixman_region32_translate(&clip, -buffer->area.x, -buffer->area.y);
            break;
        }

        pixman_region32_t clear;
        /* buffer coord in output without transform */
        pixman_region32_init_rect(&clear, request->dst_box.x, request->dst_box.y,
                                  request->dst_box.width, request->dst_box.height);
        pixman_region32_intersect(&clip, &clip, &clear);
        pixman_region32_subtract(&clear, &clear, &clip);

        /* only clear shadow, will copy shadow to buffer when blit */
        struct wlr_buffer *clear_buffer =
            need_blit && buffer->shadow ? buffer->shadow : buffer->buffer;
        ok = capture_clear_buffer(clear_buffer, &clear);
        pixman_region32_fini(&clear);
        if (!ok) {
            goto out;
        }
    }

    if (need_blit) {
        ok = capture_buffer_blit(buffer, request, src, touched ? &clip : NULL);
    }

out:
    pixman_region32_fini(&clip);
    return ok;
}

static bool capture_request_mark_done(struct capture_request *request)
{
    request->blit_done = true;

    struct capture_buffer *buffer = request->buffer;
    if (buffer->has_content) {
        return true;
    }

    struct capture_request *req;
    wl_list_for_each(req, &buffer->requests, link) {
        if (!req->blit_done) {
            return false;
        }
    }
    return true;
}

static void capture_buffer_update(struct capture_buffer *buffer)
{
    if (!buffer->has_content) {
        return;
    }

    struct capture *capture, *tmp;
    if (!buffer->was_damaged) {
        buffer->can_destroy = false;
        wl_list_for_each_safe(capture, tmp, &buffer->captures, link) {
            if (capture->wants_update && capture->force_update) {
                capture->force_update = false;
                if (capture->impl && capture->impl->update) {
                    capture->impl->update(capture, buffer->buffer, true, capture->user_data);
                }
            }
        }
        /* capture may need be destroyed in update */
        buffer->can_destroy = true;
        capture_buffer_destroy(buffer);
        return;
    }

    /* now all requests are done, call update for all captures */
    buffer->can_destroy = false;
    wl_list_for_each_safe(capture, tmp, &buffer->captures, link) {
        if (capture->wants_update) {
            bool buffer_changed = buffer->buffer_changed || capture->force_update;
            capture->force_update = false;
            if (capture->impl && capture->impl->update) {
                capture->impl->update(capture, buffer->buffer, buffer_changed, capture->user_data);
            }
        } else {
            capture->force_update |= buffer->buffer_changed;
        }
    }

    buffer->buffer_changed = false;
    buffer->was_damaged = false;
    buffer->can_destroy = true;
    capture_buffer_destroy(buffer);
}

static bool capture_buffer_need_update(struct capture_buffer *buffer, bool *force_update)
{
    bool need_update = false;
    struct capture *capture;
    wl_list_for_each(capture, &buffer->captures, link) {
        need_update |= capture->wants_update;
        if (capture->wants_update) {
            *force_update |= capture->force_update;
        }
    }
    return need_update;
}

static void capture_buffer_update_damage(struct capture_request *request, bool has_damage,
                                         const pixman_region32_t *damage)
{
    struct capture_buffer *buffer = request->buffer;
    if (!has_damage || buffer->was_damaged) {
        return;
    }

    if (buffer->type != CAPTURE_TYPE_AREA) {
        buffer->was_damaged = true;
        return;
    }

    /* check damage region and src_box */
    pixman_region32_t box;
    pixman_region32_init_rect(&box, request->src_box.x, request->src_box.y, request->src_box.width,
                              request->src_box.height);
    pixman_region32_intersect(&box, &box, damage);
    if (pixman_region32_not_empty(&box)) {
        buffer->was_damaged = true;
    }
    pixman_region32_fini(&box);
}

static void capture_output_handle_requests(struct capture_output *output,
                                           struct wlr_buffer *source_buffer, bool has_damage,
                                           const pixman_region32_t *damage)
{
    /* do blit for every request */
    struct capture_request *request, *tmp;
    wl_list_for_each_safe(request, tmp, &output->requests, output_link) {
        capture_buffer_update_damage(request, has_damage, damage);

        struct capture_buffer *buffer = request->buffer;
        bool force_update = false;
        /* skip this request if all captures don't want update */
        if (!capture_buffer_need_update(buffer, &force_update)) {
            continue;
        }

        /* source is not damaged, update only force update */
        if (!buffer->was_damaged) {
            if (!force_update) {
                continue;
            } else if (buffer->has_content && buffer->own_buffer) {
                /* make sure buffer hook is called before update when private */
                capture_buffer_update(buffer);
                continue;
            }
        }

        if (!source_buffer || capture_request_blit_buffer(request, source_buffer)) {
            /* mark this request in capture_buffer is done */
            buffer->has_content = capture_request_mark_done(request);
            capture_buffer_update(buffer);
        } else {
            /* destroy capture_buffer */
            buffer->need_destroy = true;
            capture_buffer_destroy(buffer);
        }
    }
}

static void capture_output_handle_commit(struct wl_listener *listener, void *data)
{
    struct capture_output *capture_output = wl_container_of(listener, capture_output, commit);
    struct wlr_output_event_commit *event = data;

    if (wl_list_empty(&capture_output->requests)) {
        return;
    }
    if (!(event->state->committed & WLR_OUTPUT_STATE_BUFFER)) {
        return;
    }

    bool has_damage = event->state->committed & WLR_OUTPUT_STATE_DAMAGE &&
                      pixman_region32_not_empty(&event->state->damage);
    capture_output_handle_requests(capture_output, event->state->buffer, has_damage,
                                   &event->state->damage);
}

static void capture_output_handle_power(struct wl_listener *listener, void *data)
{
    struct capture_output *capture_output = wl_container_of(listener, capture_output, power);
    if (!capture_output->output->base.state.power) {
        capture_manager_schedule_frame(true);
    }
}

static int capture_output_handle_timeout(void *data)
{
    struct capture_output *capture_output = data;

    if (!wl_list_empty(&capture_output->requests)) {
        struct wlr_buffer *buffer = capture_output->output->scene_output->buffer;
        capture_output_handle_requests(capture_output, buffer, false, NULL);
    }

    capture_output->commit_pending = false;
    return 0;
}

static struct capture_output *capture_output_get(struct output *output)
{
    struct capture_output *capture_output;
    wl_list_for_each(capture_output, &manager->outputs, link) {
        if (capture_output->output == output) {
            return capture_output;
        }
    }
    return NULL;
}

static struct capture_output *capture_output_create(struct output *output)
{
    struct capture_output *capture_output = calloc(1, sizeof(*capture_output));
    if (!capture_output) {
        return NULL;
    }

    capture_output->timer = wl_event_loop_add_timer(manager->server->event_loop,
                                                    capture_output_handle_timeout, capture_output);
    if (!capture_output->timer) {
        free(capture_output);
        return NULL;
    }

    wl_list_init(&capture_output->requests);
    wl_list_insert(&manager->outputs, &capture_output->link);

    capture_output->output = output;
    capture_output->commit.notify = capture_output_handle_commit;
    wl_signal_add(&output->wlr_output->events.commit, &capture_output->commit);
    capture_output->geometry.notify = capture_output_handle_geometry;
    wl_signal_add(&output->events.geometry, &capture_output->geometry);
    capture_output->disable.notify = capture_output_handle_disable;
    wl_signal_add(&output->events.disable, &capture_output->disable);
    capture_output->power.notify = capture_output_handle_power;
    wl_signal_add(&output->base.events.power, &capture_output->power);

    capture_output_handle_power(&capture_output->power, NULL);

    return capture_output;
}

static struct capture_request *capture_request_create(struct output *output,
                                                      struct capture_buffer *buffer)
{
    /* create capture requests according to capture type */
    struct wlr_fbox fbox;
    struct wlr_box box;

    if (!capture_buffer_calc_box(buffer, output, &box, &fbox)) {
        return NULL;
    }

    struct capture_output *capture_output = capture_output_get(output);
    if (!capture_output) {
        return NULL;
    }

    struct capture_request *request = calloc(1, sizeof(*request));
    if (!request) {
        return NULL;
    }

    request->buffer = buffer;
    request->output = capture_output;
    request->src_box = fbox;
    request->dst_box = box;
    request->cursor_locked = buffer->options & CAPTURE_NEED_CURSOR;
    wl_list_insert(&buffer->requests, &request->link);
    wl_list_insert(&capture_output->requests, &request->output_link);

    if (request->cursor_locked) {
        wlr_output_lock_software_cursors(output->wlr_output, true);
    }

    /* Locks the output to only use rendering instead of direct scan-out */
    wlr_output_lock_attach_render(output->wlr_output, true);

    return request;
}

static bool create_request(struct kywc_output *kywc_output, int index, void *data)
{
    struct output *output = output_from_kywc_output(kywc_output);
    struct capture_buffer *buffer = data;
    capture_request_create(output, buffer);
    return false;
}

static struct capture_buffer *get_capture_buffer(enum capture_type type, uint32_t options,
                                                 struct output *output, struct wlr_box *area)
{
    struct capture_buffer *buffer;
    wl_list_for_each(buffer, &manager->buffers, link) {
        if (!buffer->own_buffer) {
            continue;
        }
        /* skip other options check */
        if (buffer->type != type ||
            (buffer->options ^ options) &
                (CAPTURE_NEED_UNSCALED | CAPTURE_NEED_SINGLE_PLANE | CAPTURE_PREFER_OUTPUT_SCALE)) {
            continue;
        }
        if (type == CAPTURE_TYPE_FULLSCREEN) {
            return buffer;
        } else if (type == CAPTURE_TYPE_OUTPUT && buffer->output == output) {
            return buffer;
        } else if (type == CAPTURE_TYPE_AREA && wlr_box_equal(&buffer->area, area)) {
            return buffer;
        }
    }

    return NULL;
}

static struct capture_buffer *capture_buffer_get_or_create(enum capture_type type, uint32_t options,
                                                           struct output *output,
                                                           struct wlr_box *area, bool private)
{
    struct capture_buffer *buffer = NULL;

    if (!private) {
        buffer = get_capture_buffer(type, options, output, area);
        if (buffer) {
            return buffer;
        }
    }

    buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    buffer->type = type;
    buffer->options = options;
    buffer->can_destroy = true;
    buffer->own_buffer = !private; // capture will not allocate buffer when false

    wl_list_init(&buffer->captures);
    wl_list_init(&buffer->requests);
    wl_list_init(&buffer->link);

    if (type == CAPTURE_TYPE_OUTPUT) {
        buffer->output = output;
        capture_request_create(buffer->output, buffer);
    } else if (type == CAPTURE_TYPE_AREA) {
        buffer->area = *area;
        output_manager_for_each_output(create_request, true, buffer);
    } else if (type == CAPTURE_TYPE_FULLSCREEN) {
        output_manager_for_each_output(create_request, true, buffer);
    }

    if (wl_list_empty(&buffer->requests)) {
        buffer->need_destroy = true;
        capture_buffer_destroy(buffer);
        return NULL;
    }

    capture_buffer_calc_size(buffer);
    wl_list_insert(&manager->buffers, &buffer->link);
    return buffer;
}

static struct capture *capture_create(enum capture_type type, uint32_t options,
                                      struct output *output, struct wlr_box *area,
                                      const struct capture_interface *impl, void *data)
{
    struct capture *capture = calloc(1, sizeof(*capture));
    if (!capture) {
        return NULL;
    }

    // clear CAPTURE_PREFER_OUTPUT_SCALE when CAPTURE_NEED_UNSCALED is not set
    if (!(options & CAPTURE_NEED_UNSCALED)) {
        options &= ~CAPTURE_PREFER_OUTPUT_SCALE;
    }

    struct capture_buffer *buffer =
        capture_buffer_get_or_create(type, options, output, area, impl && impl->buffer);
    if (!buffer) {
        free(capture);
        return NULL;
    }

    capture->buffer = buffer;
    wl_list_insert(&buffer->captures, &capture->link);
    capture->impl = impl;
    capture->user_data = data;
    capture->force_update = capture->wants_update = true;
    /* buffer update is needed */
    capture_manager_schedule_frame(true);

    return capture;
}

struct capture *capture_create_from_output(struct output *output, uint32_t options,
                                           const struct capture_interface *impl, void *data)
{
    if (!manager || !output->scene_output) {
        return NULL;
    }

    return capture_create(CAPTURE_TYPE_OUTPUT, options, output, NULL, impl, data);
}

struct capture *capture_create_from_area(struct wlr_box *rect, uint32_t options,
                                         const struct capture_interface *impl, void *data)
{
    if (!manager || wlr_box_empty(rect)) {
        return NULL;
    }

    return capture_create(CAPTURE_TYPE_AREA, options, NULL, rect, impl, data);
}

struct capture *capture_create_from_fullscreen(uint32_t options,
                                               const struct capture_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    return capture_create(CAPTURE_TYPE_FULLSCREEN, options, NULL, NULL, impl, data);
}

void capture_destroy(struct capture *capture)
{
    if (!capture) {
        return;
    }

    wl_list_remove(&capture->link);

    /* capture_buffer may not be destroyed caused by can_destroy == false */
    if (capture->buffer) {
        capture_buffer_destroy(capture->buffer);
    }

    free(capture);
}

void capture_mark_wants_update(struct capture *capture, bool wants, bool force)
{
    capture->wants_update = wants;
    capture->force_update = force;

    /* should send update if buffer was damaged */
    if (wants) {
        capture_manager_schedule_frame(force);
    }
}

void capture_get_buffer_size(struct capture *capture, int *width, int *height)
{
    if (!capture) {
        *width = *height = 0;
    } else {
        *width = capture->buffer->width;
        *height = capture->buffer->height;
    }
}

/**
 * helpers to read buffer, and write to png file in another thread.
 */
void capture_read_buffer(struct wlr_buffer *buffer, uint32_t format, uint32_t stride,
                         struct wlr_box *box, void *data)
{
    struct wlr_box src_box = { 0, 0, buffer->width, buffer->height };
    if (!wlr_box_intersection(&src_box, &src_box, box)) {
        return;
    }

    struct wlr_renderer *renderer = manager->server->renderer;
    if (!renderer_check_render_buffer(renderer, buffer)) {
        renderer = server_get_pixman_renderer(manager->server);
    }

    if (!renderer || !wlr_renderer_begin_with_buffer(renderer, buffer)) {
        return;
    }

    wlr_renderer_read_pixels(renderer, format, stride, src_box.width, src_box.height, src_box.x,
                             src_box.y, 0, 0, data);
    wlr_renderer_end(renderer);
}

struct capture_data {
    struct wlr_buffer *buffer;
    void (*done)(const char *path, void *data);
    char *path;
    void *user_data;
};

static void capture_write_image(struct wlr_buffer *buffer, const char *path,
                                void (*done)(const char *path, void *data), void *user_data)
{
    painter_buffer_write_to_file(buffer, path);
    wlr_buffer_drop(buffer);

    if (done) {
        done(path, user_data);
    }
}

static void write_image(void *job, void *gdata, int index)
{
    kywc_log(KYWC_DEBUG, "%s: in thread %d", __func__, index);
    struct capture_data *data = job;
    capture_write_image(data->buffer, data->path, data->done, data->user_data);
    free(data->path);
    free(data);
}

void capture_write_file(struct wlr_buffer *buffer, const char *path,
                        void (*done)(const char *path, void *data), void *user_data)
{
    uint32_t format;
    size_t stride;
    void *dst_ptr;

    struct wlr_buffer *dst_buf = painter_create_buffer(buffer->width, buffer->height, 1.0);
    wlr_buffer_begin_data_ptr_access(dst_buf, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &dst_ptr, &format,
                                     &stride);
    capture_read_buffer(buffer, format, stride,
                        &(struct wlr_box){ 0, 0, buffer->width, buffer->height }, dst_ptr);
    wlr_buffer_end_data_ptr_access(dst_buf);
    kywc_log(KYWC_DEBUG, "Capture copy buffer to memory");

    struct capture_data *data = malloc(sizeof(*data));
    if (!data) {
        capture_write_image(dst_buf, path, done, user_data);
        return;
    }

    data->buffer = dst_buf;
    data->path = strdup(path);
    data->done = done;
    data->user_data = user_data;

    if (!queue_add_job(manager->server->queue, data, NULL, write_image, NULL)) {
        free(data->path);
        free(data);
        capture_write_image(dst_buf, path, done, user_data);
    }
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct kywc_output *kywc_output = data;
    struct output *output = output_from_kywc_output(kywc_output);

    if (!capture_output_create(output)) {
        return;
    }

    struct capture_buffer *buffer, *tmp;
    wl_list_for_each_safe(buffer, tmp, &manager->buffers, link) {
        if (buffer->type == CAPTURE_TYPE_OUTPUT) {
            continue;
        }
        if (capture_request_create(output, buffer)) {
            capture_buffer_calc_size(buffer);
        }
    }
    /* blit done is reset, must blit again */
    capture_manager_schedule_frame(false);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->destroy.link);
    wl_list_remove(&manager->new_enabled_output.link);
    free(manager);
    manager = NULL;
}

bool capture_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->buffers);
    wl_list_init(&manager->outputs);

    manager->new_enabled_output.notify = handle_new_enabled_output;
    output_manager_add_new_enabled_listener(&manager->new_enabled_output);

    manager->server = server;
    manager->destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->destroy);

    return true;
}
