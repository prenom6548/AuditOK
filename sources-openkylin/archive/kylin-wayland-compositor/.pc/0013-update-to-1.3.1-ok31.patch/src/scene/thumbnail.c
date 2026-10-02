// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <drm_fourcc.h>
#include <wlr/types/wlr_output.h>

#include <kywc/log.h>

#include "effect/effect.h"
#include "output.h"
#include "render/renderer.h"
#include "scene/render.h"
#include "scene/thumbnail.h"
#include "server.h"

enum thumbnail_type {
    THUMBNAIL_TYPE_NONE = 0,
    THUMBNAIL_TYPE_NODE,
    THUMBNAIL_TYPE_VIEW,
    THUMBNAIL_TYPE_WORKSPACE,
    THUMBNAIL_TYPE_OUTPUT,
};

struct thumbnail {
    struct wl_list link;
    struct thumbnail_buffer *buffer;

    const struct thumbnail_interface *impl;
    void *user_data;

    struct client client;

    bool force_update, wants_update;
};

struct thumbnail_source_info {
    /* starting from the source, the view is root node */
    struct kywc_box bounding_box;
    struct view_padding padding;
    /* normalized */
    struct kywc_fbox effective_fbox;
    /* round corner radius on bounding_box */
    int radius[4];
};

struct thumbnail_buffer_interface {
    void (*get_bounding_box_from_source)(const struct thumbnail_buffer *buffer,
                                         struct kywc_box *bounding_box);
    void (*get_padding_from_source)(const struct thumbnail_buffer *buffer,
                                    struct view_padding *padding);
    void (*get_radius_from_source)(const struct thumbnail_buffer *buffer, int radius[static 4]);
    bool (*render)(struct thumbnail_buffer *buffer, struct scene_output *output);
    void (*destroy)(struct thumbnail_buffer *buffer);
};

struct thumbnail_buffer {
    struct wlr_buffer *buffer;
    struct wlr_buffer *shadow;

    struct wl_list thumbnails;
    const struct thumbnail_buffer_interface *impl;

    float scale;
    enum thumbnail_type type;
    bool single_plane;
    bool own_buffer, buffer_changed;
    struct thumbnail_source_info info;

    bool was_damaged, has_content;
    bool need_destroy, can_destroy;
};

struct node_thumbnail {
    struct thumbnail_buffer base;
    struct wl_list link;

    uint32_t options;
    struct scene_node *source_node;
    struct wl_listener source_damage;
    struct wl_listener source_destroy;
};

struct view_thumbnail {
    struct thumbnail_buffer base;
    struct wl_list link;
    uint32_t options;

    struct view *view;
    struct wl_listener view_unmap;

    struct scene_node *source_node;
    struct wl_listener source_damage;
};

struct workspace_thumbnail_entry {
    struct wl_list link;
    struct workspace_thumbnail *workspace_thumbnail;

    struct view *view;
    struct wl_listener view_move;
    struct wl_listener view_output;
    struct wl_listener workspace_leave;
    struct wl_listener view_unmap;
    struct wl_listener view_minimize;
    struct wl_listener view_damage;
};

struct workspace_thumbnail {
    struct thumbnail_buffer base;
    struct wl_list link;
    uint32_t options;

    struct wl_list entries; // workspace_thumbnail_entry

    struct workspace *workspace;
    struct wl_listener view_enter;
    struct wl_listener workspace_destroy;

    struct kywc_output *output;
    struct wl_listener output_destroy;
};

struct output_thumbnail_state {
    enum wl_output_transform transform;
    /* transformed output resolution */
    int trans_width, trans_height;
    int width, height;
    float scale;

    /* lx, ly in scene */
    struct kywc_box logical;
};

struct output_thumbnail {
    struct thumbnail_buffer base;
    struct wl_list link;

    bool state_readonly;
    struct output_thumbnail_state state;

    struct scene_output *output;
    struct wl_listener output_commit;
    struct wl_listener output_destroy;
};

struct thumbnail_output {
    struct wl_list link;

    struct scene_output *output;
    struct wl_listener frame;
    struct wl_listener destroy;
    struct wl_listener power;

    struct wl_event_source *timer; // fake frame when power off
    bool frame_pending;
};

static struct thumbnail_manager {
    struct wl_list node_thumbnails;
    struct wl_list view_thumbnails;
    struct wl_list workspace_thumbnails;
    struct wl_list output_thumbnails;

    struct wl_list outputs;
    struct wl_listener new_enabled_output;

    struct server *server;
    struct wl_listener destroy;
} *manager = NULL;

static void thumbnail_manager_schedule_frame(bool force)
{
    if (!manager->server->active) {
        return;
    }

    struct thumbnail_output *output;
    wl_list_for_each(output, &manager->outputs, link) {
        struct wlr_output *wlr_output = output->output->output;
        if (force && !wlr_output->enabled && !output->frame_pending) {
            int32_t refresh = wlr_output->refresh > 0 ? wlr_output->refresh : 60000;
            wl_event_source_timer_update(output->timer, 1000000 / refresh);
            output->frame_pending = true;
        }
        output_schedule_frame(wlr_output);
    }
}

static void thumbnail_buffer_init(struct thumbnail_buffer *buffer, enum thumbnail_type type,
                                  const struct thumbnail_buffer_interface *info_impl, float scale,
                                  bool single_plane, bool own_buffer)
{
    buffer->scale = scale;
    buffer->was_damaged = false; // damage from source
    buffer->can_destroy = true;
    buffer->type = type;
    buffer->single_plane = single_plane;
    buffer->own_buffer = own_buffer;
    buffer->impl = info_impl;

    wl_list_init(&buffer->thumbnails);
}

static struct node_thumbnail *find_node_thumbnail(struct scene_node *node, float scale,
                                                  uint32_t options)
{
    struct node_thumbnail *node_thumbnail;
    wl_list_for_each(node_thumbnail, &manager->node_thumbnails, link) {
        if (!node_thumbnail->base.own_buffer) {
            continue;
        }
        if (node_thumbnail->source_node == node && node_thumbnail->base.scale == scale &&
            node_thumbnail->options == options) {
            return node_thumbnail;
        }
    }
    return NULL;
}

static struct view_thumbnail *find_view_thumbnail(struct view *view, uint32_t options, float scale)
{
    struct view_thumbnail *view_thumbnail;
    wl_list_for_each(view_thumbnail, &manager->view_thumbnails, link) {
        if (!view_thumbnail->base.own_buffer) {
            continue;
        }
        if (view_thumbnail->view == view && view_thumbnail->options == options &&
            view_thumbnail->base.scale == scale) {
            return view_thumbnail;
        }
    }
    return NULL;
}

static struct workspace_thumbnail *find_workspace_thumbnail(struct workspace *workspace,
                                                            struct kywc_output *output,
                                                            uint32_t options, float scale)
{
    struct workspace_thumbnail *workspace_thumbnail;
    wl_list_for_each(workspace_thumbnail, &manager->workspace_thumbnails, link) {
        if (!workspace_thumbnail->base.own_buffer) {
            continue;
        }
        if (workspace_thumbnail->workspace == workspace && workspace_thumbnail->output == output &&
            workspace_thumbnail->options == options && workspace_thumbnail->base.scale == scale) {
            return workspace_thumbnail;
        }
    }
    return NULL;
}

static bool thumbnail_buffer_ensure_shadow_buffer(struct thumbnail_buffer *thumbnail_buffer)
{
    if (thumbnail_buffer->own_buffer ||
        renderer_check_render_buffer(manager->server->renderer, thumbnail_buffer->buffer)) {
        wlr_buffer_drop(thumbnail_buffer->shadow);
        thumbnail_buffer->shadow = NULL;
        return true;
    }

    struct wlr_buffer *buffer = thumbnail_buffer->shadow;
    int width = thumbnail_buffer->buffer->width;
    int height = thumbnail_buffer->buffer->height;

    bool need_create = !buffer || (buffer->width != width || buffer->height != height);
    if (!need_create) {
        return true;
    }

    wlr_buffer_drop(buffer);

    thumbnail_buffer->shadow =
        renderer_create_buffer(manager->server->renderer, manager->server->allocator, width, height,
                               DRM_FORMAT_ARGB8888, thumbnail_buffer->single_plane);
    if (!thumbnail_buffer->shadow) {
        kywc_log(KYWC_ERROR, "Failed to create wlr buffer for capture shadow");
        return false;
    }

    return true;
}

static bool thumbnail_buffer_blit_shadow(struct thumbnail_buffer *thumbnail_buffer,
                                         struct wlr_buffer *buffer)
{
    struct wlr_buffer *src_buffer = thumbnail_buffer->shadow;
    if (!src_buffer) {
        return true;
    }

    void *data;
    uint32_t format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(buffer, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &data, &format,
                                          &stride)) {
        return false;
    }

    struct wlr_renderer *renderer = manager->server->renderer;
    bool ok = false;

    if (wlr_renderer_begin_with_buffer(renderer, src_buffer)) {
        ok = wlr_renderer_read_pixels(renderer, format, stride, src_buffer->width,
                                      src_buffer->height, 0, 0, 0, 0, data);
        wlr_renderer_end(renderer);
    }

    wlr_buffer_end_data_ptr_access(buffer);
    return ok;
}

static struct wlr_buffer *thumbnail_buffer_allocate(struct thumbnail_buffer *thumbnail_buffer,
                                                    int width, int height)
{
    struct wlr_buffer *buffer = thumbnail_buffer->buffer;
    bool need_create = !buffer || (buffer->width != width || buffer->height != height);
    if (!need_create) {
        return buffer;
    }

    buffer = renderer_create_buffer(manager->server->renderer, manager->server->allocator, width,
                                    height, DRM_FORMAT_ARGB8888, thumbnail_buffer->single_plane);
    if (!buffer) {
        kywc_log(KYWC_ERROR, "Failed create thumbnail buffer with %d x %d", width, height);
        return thumbnail_buffer->buffer;
    }

    return buffer;
}

static bool thumbnail_buffer_ensure_render_buffer(struct thumbnail_buffer *thumbnail_buffer,
                                                  int width, int height)
{
    struct wlr_buffer *buffer = NULL;

    if (thumbnail_buffer->own_buffer) {
        buffer = thumbnail_buffer_allocate(thumbnail_buffer, width, height);
    } else {
        struct thumbnail *thumbnail =
            wl_container_of(thumbnail_buffer->thumbnails.next, thumbnail, link);
        buffer = thumbnail->impl->buffer(thumbnail, thumbnail_buffer->buffer, width, height,
                                         thumbnail_buffer->single_plane, thumbnail->user_data);
    }

    if (!buffer) {
        return false;
    }

    if (buffer == thumbnail_buffer->buffer) {
        return true;
    }

    if (thumbnail_buffer->own_buffer) {
        wlr_buffer_drop(thumbnail_buffer->buffer);
    }
    thumbnail_buffer->buffer = buffer;
    thumbnail_buffer->buffer_changed = true;
    thumbnail_buffer->has_content = false;

    return thumbnail_buffer_ensure_shadow_buffer(thumbnail_buffer);
}

static void thumbnail_buffer_get_info_from_source(const struct thumbnail_buffer *thumbnail_buffer,
                                                  struct thumbnail_source_info *source_info)
{
    source_info->padding = (struct view_padding){ 0, 0, 0, 0 };
    source_info->effective_fbox = (struct kywc_fbox){ 0, 0, 0, 0 };
    source_info->bounding_box = (struct kywc_box){ 0, 0, 0, 0 };
    memset(source_info->radius, 0, sizeof(source_info->radius));

    if (thumbnail_buffer->impl->get_bounding_box_from_source) {
        thumbnail_buffer->impl->get_bounding_box_from_source(thumbnail_buffer,
                                                             &source_info->bounding_box);
    }

    struct kywc_box *bbox = &source_info->bounding_box;
    if (kywc_box_empty(bbox)) {
        return;
    }

    if (thumbnail_buffer->impl->get_radius_from_source) {
        thumbnail_buffer->impl->get_radius_from_source(thumbnail_buffer, source_info->radius);
    }

    struct view_padding *padding = &source_info->padding;
    if (thumbnail_buffer->impl->get_padding_from_source) {
        thumbnail_buffer->impl->get_padding_from_source(thumbnail_buffer, padding);
    }

    struct kywc_fbox *effective_fbox = &source_info->effective_fbox;
    if (thumbnail_buffer->type == THUMBNAIL_TYPE_NODE ||
        thumbnail_buffer->type == THUMBNAIL_TYPE_VIEW) {
        if (!padding->top && !padding->bottom && !padding->left && !padding->right) {
            *effective_fbox = (struct kywc_fbox){ 0, 0, 1.0f, 1.0f };
            return;
        }

        int view_width = bbox->width - padding->left - padding->right;
        int view_height = bbox->height - padding->top - padding->bottom;
        if (view_width <= 0 || view_height <= 0) {
            return;
        }

        effective_fbox->width = (float)view_width / bbox->width;
        effective_fbox->height = (float)view_height / bbox->height;
        effective_fbox->x = (float)padding->left / bbox->width;
        effective_fbox->y = (float)padding->top / bbox->height;
    }
}

static bool node_thumbnail_render(struct thumbnail_buffer *thumbnail_buffer,
                                  struct scene_output *scene_output)
{
    struct node_thumbnail *node_thumbnail = wl_container_of(thumbnail_buffer, node_thumbnail, base);
    struct thumbnail_source_info source_info;
    thumbnail_buffer_get_info_from_source(thumbnail_buffer, &source_info);
    struct kywc_box *bounding_box = &source_info.bounding_box;
    int buffer_width = round(bounding_box->width * thumbnail_buffer->scale);
    int buffer_height = round(bounding_box->height * thumbnail_buffer->scale);
    if (buffer_width <= 0 || buffer_height <= 0) {
        kywc_log(KYWC_DEBUG, "node thumbnail size is too small, %d x %d", buffer_width,
                 buffer_height);
        return false;
    }

    if (!thumbnail_buffer_ensure_render_buffer(thumbnail_buffer, buffer_width, buffer_height)) {
        return false;
    }

    struct wlr_renderer *renderer = scene_output->output->renderer;
    struct wlr_buffer *dst_buffer =
        thumbnail_buffer->shadow ? thumbnail_buffer->shadow : thumbnail_buffer->buffer;

    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(renderer, dst_buffer, NULL);
    if (!render_pass) {
        return false;
    }

    /* clear the target buffer */
    wlr_render_pass_add_rect(render_pass, &(struct wlr_render_rect_options){
                                              .color = { 0, 0, 0, 0 },
                                              .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                          });

    struct scene_render_target target = {
        .logical = *bounding_box,
        .scale = thumbnail_buffer->scale,
        .trans_width = buffer_width,
        .trans_height = buffer_height,
        .buffer = dst_buffer,
        .output = scene_output,
        .render_pass = render_pass,
        .options = SCENE_RENDER_DISABLE_VISIBILITY | SCENE_RENDER_DISABLE_BLUR |
                   SCENE_RENDER_DISABLE_EFFECT | SCENE_RENDER_ENABLE_FORCE_OPAQUE,
    };
    if (node_thumbnail->options & THUMBNAIL_DISABLE_ROUND_CORNER) {
        target.options |= SCENE_RENDER_DISABLE_ROUND_CORNER;
    }
    if (node_thumbnail->options & THUMBNAIL_ENABLE_SECURITY) {
        target.options |= SCENE_RENDER_ENABLE_SECURITY;
    }
    if (node_thumbnail->options & THUMBNAIL_ENABLE_STYLE_EFFECT) {
        target.options |= SCENE_RENDER_ENABLE_STYLE_EFFECT;
    }
    if (node_thumbnail->options & THUMBNAIL_WITHOUT_SUBEFFECT_NODE) {
        target.options |= SCENE_RENDER_DISABLE_SUBEFFECT_NODE;
    }
    scene_render_target_init_regions(&target);
    pixman_region32_init_rect(&target.damage, bounding_box->x, bounding_box->y, bounding_box->width,
                              bounding_box->height);

    struct scene_node *source_node = node_thumbnail->source_node;
    bool old_state = source_node->enabled;
    source_node->enabled = true;
    source_node->impl.render(source_node, 0, 0, &target);
    source_node->enabled = old_state;
    wlr_render_pass_submit(target.render_pass);
    scene_render_target_finish_regions(&target);

    thumbnail_buffer->info = source_info;
    return thumbnail_buffer_blit_shadow(thumbnail_buffer, thumbnail_buffer->buffer);
}

static bool view_thumbnail_render(struct thumbnail_buffer *thumbnail_buffer,
                                  struct scene_output *scene_output)
{
    struct view_thumbnail *view_thumbnail = wl_container_of(thumbnail_buffer, view_thumbnail, base);
    struct thumbnail_source_info source_info;
    thumbnail_buffer_get_info_from_source(thumbnail_buffer, &source_info);
    struct kywc_box *bounding_box = &source_info.bounding_box;
    int buffer_width = round(bounding_box->width * thumbnail_buffer->scale);
    int buffer_height = round(bounding_box->height * thumbnail_buffer->scale);
    if (buffer_width <= 0 || buffer_height <= 0) {
        kywc_log(KYWC_DEBUG, "view thumbnail size is too small, %d x %d", buffer_width,
                 buffer_height);
        return false;
    }

    if (!thumbnail_buffer_ensure_render_buffer(thumbnail_buffer, buffer_width, buffer_height)) {
        return false;
    }

    struct wlr_renderer *renderer = scene_output->output->renderer;
    struct wlr_buffer *dst_buffer =
        thumbnail_buffer->shadow ? thumbnail_buffer->shadow : thumbnail_buffer->buffer;

    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(renderer, dst_buffer, NULL);
    if (!render_pass) {
        return false;
    }

    /* clear the target buffer */
    wlr_render_pass_add_rect(render_pass, &(struct wlr_render_rect_options){
                                              .color = { 0, 0, 0, 0 },
                                              .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                          });

    struct scene_render_target target = {
        .logical = *bounding_box,
        .scale = thumbnail_buffer->scale,
        .trans_width = buffer_width,
        .trans_height = buffer_height,
        .buffer = dst_buffer,
        .output = scene_output,
        .render_pass = render_pass,
        .options = SCENE_RENDER_DISABLE_VISIBILITY | SCENE_RENDER_DISABLE_BLUR |
                   SCENE_RENDER_DISABLE_EFFECT | SCENE_RENDER_ENABLE_FORCE_OPAQUE,
    };
    if (view_thumbnail->options & THUMBNAIL_DISABLE_ROUND_CORNER) {
        target.options |= SCENE_RENDER_DISABLE_ROUND_CORNER;
    }
    if (view_thumbnail->options & THUMBNAIL_ENABLE_SECURITY) {
        target.options |= SCENE_RENDER_ENABLE_SECURITY;
    }
    if (view_thumbnail->options & THUMBNAIL_ENABLE_STYLE_EFFECT) {
        target.options |= SCENE_RENDER_ENABLE_STYLE_EFFECT;
    }
    scene_render_target_init_regions(&target);
    pixman_region32_init_rect(&target.damage, bounding_box->x, bounding_box->y, bounding_box->width,
                              bounding_box->height);

    struct scene_node *source_node = view_thumbnail->source_node;
    bool old_state = source_node->enabled;
    source_node->enabled = true;
    source_node->impl.render(source_node, 0, 0, &target);
    source_node->enabled = old_state;
    wlr_render_pass_submit(target.render_pass);
    scene_render_target_finish_regions(&target);

    thumbnail_buffer->info = source_info;
    return thumbnail_buffer_blit_shadow(thumbnail_buffer, thumbnail_buffer->buffer);
}

static bool thumbnail_buffer_destroy(struct thumbnail_buffer *thumbnail_buffer)
{
    /* don't destroy if still have thumbnails */
    if (!thumbnail_buffer->need_destroy && !wl_list_empty(&thumbnail_buffer->thumbnails)) {
        return false;
    }
    /* mark need_destroy if cannot be destroyed current */
    if (!thumbnail_buffer->can_destroy) {
        thumbnail_buffer->need_destroy = true;
        return false;
    }

    /* force destroy all thumbnails */
    struct thumbnail *thumbnail, *tmp;
    wl_list_for_each_safe(thumbnail, tmp, &thumbnail_buffer->thumbnails, link) {
        thumbnail->buffer = NULL;
        if (thumbnail->impl && thumbnail->impl->cancel) {
            thumbnail->impl->cancel(thumbnail, thumbnail->user_data);
        }
        thumbnail_destroy(thumbnail);
    }

    if (thumbnail_buffer->own_buffer) {
        wlr_buffer_drop(thumbnail_buffer->buffer);
    }
    wlr_buffer_drop(thumbnail_buffer->shadow);

    return true;
}

static void node_thumbnail_destroy(struct thumbnail_buffer *thumbnail_buffer)
{
    if (!thumbnail_buffer_destroy(thumbnail_buffer)) {
        return;
    }

    struct node_thumbnail *node_thumbnail = wl_container_of(thumbnail_buffer, node_thumbnail, base);
    wl_list_remove(&node_thumbnail->source_destroy.link);
    wl_list_remove(&node_thumbnail->source_damage.link);
    wl_list_remove(&node_thumbnail->link);

    free(node_thumbnail);
}

static void view_thumbnail_destroy(struct thumbnail_buffer *thumbnail_buffer)
{
    if (!thumbnail_buffer_destroy(thumbnail_buffer)) {
        return;
    }

    struct view_thumbnail *view_thumbnail = wl_container_of(thumbnail_buffer, view_thumbnail, base);
    scene_node_force_damage_event(&view_thumbnail->view->tree->node, false);

    wl_list_remove(&view_thumbnail->view_unmap.link);
    wl_list_remove(&view_thumbnail->source_damage.link);
    wl_list_remove(&view_thumbnail->link);

    free(view_thumbnail);
}

static void node_thumbnail_handle_source_destroy(struct wl_listener *listener, void *data)
{
    struct node_thumbnail *node_thumbnail =
        wl_container_of(listener, node_thumbnail, source_destroy);
    /* force destroyed when source node destroy */
    node_thumbnail->base.need_destroy = true;
    node_thumbnail_destroy(&node_thumbnail->base);
}

static void view_thumbnail_handle_source_destroy(struct wl_listener *listener, void *data)
{
    struct view_thumbnail *view_thumbnail = wl_container_of(listener, view_thumbnail, view_unmap);
    /* force destroyed when source node destroy */
    view_thumbnail->base.need_destroy = true;
    view_thumbnail_destroy(&view_thumbnail->base);
}

static void node_thumbnail_handle_source_damage(struct wl_listener *listener, void *data)
{
    struct node_thumbnail *node_thumbnail =
        wl_container_of(listener, node_thumbnail, source_damage);
    if ((node_thumbnail->options & THUMBNAIL_WITHOUT_SUBEFFECT_NODE) &&
        scene_node_find_upper_effect_node(data, node_thumbnail->source_node)) {
        return;
    }

    node_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void view_thumbnail_handle_source_damage(struct wl_listener *listener, void *data)
{
    struct view_thumbnail *view_thumbnail =
        wl_container_of(listener, view_thumbnail, source_damage);
    view_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void node_thumbnail_get_bounding_box(const struct thumbnail_buffer *thumbnail_buffer,
                                            struct kywc_box *bounding_box)
{
    struct node_thumbnail *node_thumbnail = wl_container_of(thumbnail_buffer, node_thumbnail, base);
    enum scene_bounding_type type = node_thumbnail->options & THUMBNAIL_WITHOUT_SUBEFFECT_NODE
                                        ? SCENE_BOUNDING_WITHOUT_EFFECT_NODE
                                        : SCENE_BOUNDING_WITHOUT_EFFECT;
    scene_node_get_affected_bounding_box(node_thumbnail->source_node, type, bounding_box);
}

static void node_thumbnail_get_padding(const struct thumbnail_buffer *thumbnail_buffer,
                                       struct view_padding *padding)
{
    struct node_thumbnail *node_thumbnail = wl_container_of(thumbnail_buffer, node_thumbnail, base);
    *padding = (struct view_padding){ 0, 0, 0, 0 };
    struct scene_node *node = node_thumbnail->source_node;
    if (!node) {
        return;
    }

    struct view *view = node->role.data;
    if (node->role.type != SCENE_ROLE_VIEW || !view) {
        return;
    }

    if (!(node_thumbnail->options & THUMBNAIL_DISABLE_SHADOW)) {
        *padding = view->base.padding;
    }
}

static void node_thumbnail_get_radius(const struct thumbnail_buffer *thumbnail_buffer,
                                      int radius[static 4])
{
    struct node_thumbnail *node_thumbnail = wl_container_of(thumbnail_buffer, node_thumbnail, base);
    scene_node_get_radius(node_thumbnail->source_node, radius);
}

static const struct thumbnail_buffer_interface node_thumbnail_impl = {
    .get_bounding_box_from_source = node_thumbnail_get_bounding_box,
    .get_padding_from_source = node_thumbnail_get_padding,
    .get_radius_from_source = node_thumbnail_get_radius,
    .render = node_thumbnail_render,
    .destroy = node_thumbnail_destroy,
};

static struct node_thumbnail *node_thumbnail_get_or_create(struct scene_node *node,
                                                           uint32_t options, float scale,
                                                           bool own_buffer)
{
    struct node_thumbnail *node_thumbnail = NULL;
    if (!(options & THUMBNAIL_ENABLE_SECURITY) && own_buffer) {
        node_thumbnail = find_node_thumbnail(node, scale, options);
        if (node_thumbnail) {
            return node_thumbnail;
        }
    }

    node_thumbnail = calloc(1, sizeof(*node_thumbnail));
    if (!node_thumbnail) {
        return NULL;
    }

    thumbnail_buffer_init(&node_thumbnail->base, THUMBNAIL_TYPE_NODE, &node_thumbnail_impl, scale,
                          options & THUMBNAIL_ENABLE_SINGLE_PLANE, own_buffer);

    node_thumbnail->source_node = node;
    node_thumbnail->options = options;
    node_thumbnail->source_damage.notify = node_thumbnail_handle_source_damage;
    wl_signal_add(&node->events.damage, &node_thumbnail->source_damage);
    node_thumbnail->source_destroy.notify = node_thumbnail_handle_source_destroy;
    wl_signal_add(&node->events.destroy, &node_thumbnail->source_destroy);

    wl_list_insert(&manager->node_thumbnails, &node_thumbnail->link);

    return node_thumbnail;
}

static void view_thumbnail_get_source_bounding_box(const struct thumbnail_buffer *thumbnail_buffer,
                                                   struct kywc_box *box)
{
    struct view_thumbnail *view_thumbnail = wl_container_of(thumbnail_buffer, view_thumbnail, base);
    struct kywc_view *view = &view_thumbnail->view->base;
    *box = (struct kywc_box){ 0, 0, view->geometry.width, view->geometry.height };

    uint32_t options = view_thumbnail->options;
    if (options & THUMBNAIL_DISABLE_DECOR) {
        return;
    }

    if (view->ssd == KYWC_SSD_NONE) {
        if (options & THUMBNAIL_DISABLE_SHADOW) {
            return;
        }

        box->x -= view->padding.left;
        box->y -= view->padding.top;
        box->width += view->padding.right + view->padding.left;
        box->height += view->padding.bottom + view->padding.top;
        return;
    }

    if (options & THUMBNAIL_DISABLE_SHADOW) {
        box->x -= view->margin.off_x;
        box->y -= view->margin.off_y;
        box->width += view->margin.off_width;
        box->height += view->margin.off_height;
    } else {
        box->x -= view->margin.off_x + view->padding.left;
        box->y -= view->margin.off_y + view->padding.top;
        box->width += view->margin.off_width + view->padding.right + view->padding.left;
        box->height += view->margin.off_height + view->padding.bottom + view->padding.top;
    }
}

static void view_thumbnail_get_source_padding(const struct thumbnail_buffer *thumbnail_buffer,
                                              struct view_padding *padding)
{
    struct view_thumbnail *view_thumbnail = wl_container_of(thumbnail_buffer, view_thumbnail, base);
    *padding = (struct view_padding){ 0, 0, 0, 0 };
    if (!(view_thumbnail->options & THUMBNAIL_DISABLE_SHADOW)) {
        *padding = view_thumbnail->view->base.padding;
    }
}

static void view_thumbnail_get_source_radius(const struct thumbnail_buffer *thumbnail_buffer,
                                             int radius[static 4])
{
    struct view_thumbnail *view_thumbnail = wl_container_of(thumbnail_buffer, view_thumbnail, base);
    if (!view_thumbnail->source_node) {
        return;
    }

    scene_node_get_radius(view_thumbnail->source_node, radius);
}

static const struct thumbnail_buffer_interface view_thumbnail_impl = {
    .get_bounding_box_from_source = view_thumbnail_get_source_bounding_box,
    .get_padding_from_source = view_thumbnail_get_source_padding,
    .get_radius_from_source = view_thumbnail_get_source_radius,
    .render = view_thumbnail_render,
    .destroy = view_thumbnail_destroy,
};

static struct view_thumbnail *view_thumbnail_get_or_create(struct view *view, uint32_t options,
                                                           float scale, bool own_buffer)
{
    struct view_thumbnail *view_thumbnail = NULL;
    if (!(options & THUMBNAIL_ENABLE_SECURITY) && own_buffer) {
        view_thumbnail = find_view_thumbnail(view, options, scale);
        if (view_thumbnail) {
            return view_thumbnail;
        }
    }

    view_thumbnail = calloc(1, sizeof(*view_thumbnail));
    if (!view_thumbnail) {
        return NULL;
    }

    thumbnail_buffer_init(&view_thumbnail->base, THUMBNAIL_TYPE_VIEW, &view_thumbnail_impl, scale,
                          options & THUMBNAIL_ENABLE_SINGLE_PLANE, own_buffer);

    view_thumbnail->view = view;
    view_thumbnail->options = options;
    view_thumbnail->view_unmap.notify = view_thumbnail_handle_source_destroy;
    wl_signal_add(&view->base.events.unmap, &view_thumbnail->view_unmap);

    view_thumbnail->source_node = &view->tree->node;
    scene_node_force_damage_event(&view->tree->node, true);
    view_thumbnail->source_damage.notify = view_thumbnail_handle_source_damage;
    wl_signal_add(&view_thumbnail->source_node->events.damage, &view_thumbnail->source_damage);

    wl_list_insert(&manager->view_thumbnails, &view_thumbnail->link);

    return view_thumbnail;
}

static bool workspace_thumbnail_render(struct thumbnail_buffer *thumbnail_buffer,
                                       struct scene_output *scene_output)
{
    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(thumbnail_buffer, workspace_thumbnail, base);
    struct output *output = output_from_kywc_output(workspace_thumbnail->output);
    /* use output effective size */
    int buffer_width = round(output->geometry.width * thumbnail_buffer->scale);
    int buffer_height = round(output->geometry.height * thumbnail_buffer->scale);
    if (buffer_width <= 0 || buffer_height <= 0) {
        kywc_log(KYWC_DEBUG, "workspace thumbnail size is too small, %d x %d", buffer_width,
                 buffer_height);
        return false;
    }

    if (!thumbnail_buffer_ensure_render_buffer(thumbnail_buffer, buffer_width, buffer_height)) {
        return false;
    }

    struct wlr_renderer *renderer = scene_output->output->renderer;
    struct wlr_buffer *dst_buffer =
        thumbnail_buffer->shadow ? thumbnail_buffer->shadow : thumbnail_buffer->buffer;

    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(renderer, dst_buffer, NULL);
    if (!render_pass) {
        return false;
    }

    /* clear the target buffer */
    wlr_render_pass_add_rect(render_pass, &(struct wlr_render_rect_options){
                                              .color = { 0, 0, 0, 0 },
                                              .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                          });

    struct scene_render_target target = {
        .logical = { 0, 0, output->geometry.width, output->geometry.height },
        .scale = thumbnail_buffer->scale,
        .trans_width = buffer_width,
        .trans_height = buffer_height,
        .buffer = dst_buffer,
        .output = scene_output,
        .render_pass = render_pass,
        .options = SCENE_RENDER_DISABLE_VISIBILITY | SCENE_RENDER_DISABLE_BLUR |
                   SCENE_RENDER_DISABLE_EFFECT | SCENE_RENDER_ENABLE_FORCE_OPAQUE,
    };
    if (workspace_thumbnail->options & THUMBNAIL_DISABLE_ROUND_CORNER) {
        target.options |= SCENE_RENDER_DISABLE_ROUND_CORNER;
    }
    if (workspace_thumbnail->options & THUMBNAIL_ENABLE_SECURITY) {
        target.options |= SCENE_RENDER_ENABLE_SECURITY;
    }
    scene_render_target_init_regions(&target);
    pixman_region32_init_rect(&target.damage, 0, 0, output->geometry.width,
                              output->geometry.height);

    bool no_ssd = workspace_thumbnail->options & THUMBNAIL_DISABLE_DECOR;

    struct view_proxy *view_proxy;
    wl_list_for_each_reverse(view_proxy, &workspace_thumbnail->workspace->view_proxies,
                             workspace_link) {
        struct view *view = view_proxy->view;
        if (!view->base.mapped || view->base.minimized || view->base.skip_switcher ||
            view->output != workspace_thumbnail->output) {
            continue;
        }

        struct scene_node *node = no_ssd ? &view->surface_tree->node : &view->tree->node;
        int lx = view->base.geometry.x - output->geometry.x;
        int ly = view->base.geometry.y - output->geometry.y;
        node->impl.render(node, lx, ly, &target);
    }

    wlr_render_pass_submit(render_pass);
    scene_render_target_finish_regions(&target);

    thumbnail_buffer_get_info_from_source(thumbnail_buffer, &thumbnail_buffer->info);
    return thumbnail_buffer_blit_shadow(thumbnail_buffer, thumbnail_buffer->buffer);
}

static void workspace_thumbnail_entry_destroy(struct workspace_thumbnail_entry *entry)
{
    wl_list_remove(&entry->link);
    wl_list_remove(&entry->view_move.link);
    wl_list_remove(&entry->view_output.link);
    wl_list_remove(&entry->workspace_leave.link);
    wl_list_remove(&entry->view_unmap.link);
    wl_list_remove(&entry->view_minimize.link);
    wl_list_remove(&entry->view_damage.link);

    free(entry);
}

static void workspace_thumbnail_destroy(struct thumbnail_buffer *thumbnail_buffer)
{
    if (!thumbnail_buffer_destroy(thumbnail_buffer)) {
        return;
    }

    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(thumbnail_buffer, workspace_thumbnail, base);

    struct workspace_thumbnail_entry *entry, *tmp;
    wl_list_for_each_safe(entry, tmp, &workspace_thumbnail->entries, link) {
        workspace_thumbnail_entry_destroy(entry);
    }

    struct workspace *workspace = workspace_thumbnail->workspace;
    for (int i = 0; i < 3; i++) {
        scene_node_force_damage_event(&workspace->layers[i].tree->node, false);
    }

    wl_list_remove(&workspace_thumbnail->view_enter.link);
    wl_list_remove(&workspace_thumbnail->output_destroy.link);
    wl_list_remove(&workspace_thumbnail->workspace_destroy.link);
    wl_list_remove(&workspace_thumbnail->link);

    free(workspace_thumbnail);
}

static void workspace_thumbnail_handle_view_move(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, view_move);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;

    if (workspace_thumbnail->output != entry->view->output) {
        return;
    }
    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_view_output(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, view_output);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;

    wl_list_remove(&entry->view_minimize.link);
    wl_list_remove(&entry->view_damage.link);

    if (workspace_thumbnail->output == entry->view->output) {
        wl_signal_add(&entry->view->base.events.minimize, &entry->view_minimize);
        wl_signal_add(&entry->view->tree->node.events.damage, &entry->view_damage);
    } else {
        wl_list_init(&entry->view_minimize.link);
        wl_list_init(&entry->view_damage.link);
    }

    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_workspace_leave(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, workspace_leave);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;
    struct workspace *workspace = data;
    /* not leave the workspace */
    if (workspace_thumbnail->workspace != workspace) {
        return;
    }

    workspace_thumbnail_entry_destroy(entry);
    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, view_unmap);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;
    struct kywc_output *output = entry->view->output;

    workspace_thumbnail_entry_destroy(entry);
    if (workspace_thumbnail->output != output) {
        return;
    }

    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_view_minimize(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, view_minimize);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;
    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_view_damage(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail_entry *entry = wl_container_of(listener, entry, view_damage);
    struct workspace_thumbnail *workspace_thumbnail = entry->workspace_thumbnail;
    /* skip if view is minimized */
    if (entry->view->base.minimized) {
        return;
    }

    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(listener, workspace_thumbnail, output_destroy);
    workspace_thumbnail->base.need_destroy = true;
    workspace_thumbnail_destroy(&workspace_thumbnail->base);
}

static void workspace_thumbnail_handle_source_destroy(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(listener, workspace_thumbnail, workspace_destroy);
    /* force destroyed when source node destroy */
    workspace_thumbnail->base.need_destroy = true;
    workspace_thumbnail_destroy(&workspace_thumbnail->base);
}

static void workspace_thumbnail_create_entry(struct workspace_thumbnail *workspace_thumbnail,
                                             struct kywc_output *kywc_output, struct view *view)
{
    struct workspace_thumbnail_entry *entry = calloc(1, sizeof(*entry));
    if (!entry) {
        return;
    }

    entry->workspace_thumbnail = workspace_thumbnail;
    wl_list_insert(&workspace_thumbnail->entries, &entry->link);

    entry->view = view;
    entry->view_output.notify = workspace_thumbnail_handle_view_output;
    wl_signal_add(&view->events.output, &entry->view_output);
    entry->view_move.notify = workspace_thumbnail_handle_view_move;
    wl_signal_add(&view->base.events.position, &entry->view_move);
    entry->workspace_leave.notify = workspace_thumbnail_handle_workspace_leave;
    wl_signal_add(&view->events.workspace_leave, &entry->workspace_leave);
    entry->view_unmap.notify = workspace_thumbnail_handle_view_unmap;
    wl_signal_add(&view->base.events.unmap, &entry->view_unmap);

    entry->view_minimize.notify = workspace_thumbnail_handle_view_minimize;
    wl_list_init(&entry->view_minimize.link);
    entry->view_damage.notify = workspace_thumbnail_handle_view_damage;
    wl_list_init(&entry->view_damage.link);

    if (kywc_output == view->output) {
        workspace_thumbnail_handle_view_output(&entry->view_output, NULL);
    }
}

static void workspace_thumbnail_handle_view_enter(struct wl_listener *listener, void *data)
{
    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(listener, workspace_thumbnail, view_enter);

    struct view *view = data;
    workspace_thumbnail_create_entry(workspace_thumbnail, workspace_thumbnail->output, view);

    workspace_thumbnail->base.was_damaged = true;
    thumbnail_manager_schedule_frame(false);
}

static void workspace_thumbnail_create_entries(struct workspace_thumbnail *workspace_thumbnail,
                                               struct kywc_output *kywc_output)
{
    struct view_proxy *view_proxy;
    struct workspace *workspace = workspace_thumbnail->workspace;
    wl_list_for_each(view_proxy, &workspace->view_proxies, workspace_link) {
        if (view_proxy->view->base.mapped) {
            workspace_thumbnail_create_entry(workspace_thumbnail, kywc_output, view_proxy->view);
        }
    }
}

static void workspace_thumbnail_get_bounding_box(const struct thumbnail_buffer *thumbnail_buffer,
                                                 struct kywc_box *bounding_box)
{
    struct workspace_thumbnail *workspace_thumbnail =
        wl_container_of(thumbnail_buffer, workspace_thumbnail, base);
    struct output *output = output_from_kywc_output(workspace_thumbnail->output);
    *bounding_box = (struct kywc_box){ 0, 0, output->geometry.width, output->geometry.height };
}

static const struct thumbnail_buffer_interface workspace_thumbnail_buffer_impl = {
    .get_bounding_box_from_source = workspace_thumbnail_get_bounding_box,
    .render = workspace_thumbnail_render,
    .destroy = workspace_thumbnail_destroy,
};

static struct workspace_thumbnail *
workspace_thumbnail_get_or_create(struct workspace *workspace, struct kywc_output *kywc_output,
                                  uint32_t options, float scale, bool own_buffer)
{
    struct workspace_thumbnail *workspace_thumbnail = NULL;
    if (!(options & THUMBNAIL_ENABLE_SECURITY) && own_buffer) {
        workspace_thumbnail = find_workspace_thumbnail(workspace, kywc_output, options, scale);
        if (workspace_thumbnail) {
            return workspace_thumbnail;
        }
    }

    workspace_thumbnail = calloc(1, sizeof(*workspace_thumbnail));
    if (!workspace_thumbnail) {
        return NULL;
    }

    wl_list_init(&workspace_thumbnail->entries);
    wl_list_insert(&manager->workspace_thumbnails, &workspace_thumbnail->link);

    thumbnail_buffer_init(&workspace_thumbnail->base, THUMBNAIL_TYPE_WORKSPACE,
                          &workspace_thumbnail_buffer_impl, scale,
                          options & THUMBNAIL_ENABLE_SINGLE_PLANE, own_buffer);

    workspace_thumbnail->workspace = workspace;
    workspace_thumbnail->output = kywc_output;
    workspace_thumbnail->options = options;

    for (int i = 0; i < 3; i++) {
        scene_node_force_damage_event(&workspace->layers[i].tree->node, true);
    }

    workspace_thumbnail_create_entries(workspace_thumbnail, kywc_output);

    struct output *output = output_from_kywc_output(kywc_output);

    workspace_thumbnail->view_enter.notify = workspace_thumbnail_handle_view_enter;
    wl_signal_add(&workspace->events.view_enter, &workspace_thumbnail->view_enter);

    workspace_thumbnail->output_destroy.notify = workspace_thumbnail_handle_output_destroy;
    wl_signal_add(&output->scene_output->events.destroy, &workspace_thumbnail->output_destroy);

    workspace_thumbnail->workspace_destroy.notify = workspace_thumbnail_handle_source_destroy;
    wl_signal_add(&workspace->events.destroy, &workspace_thumbnail->workspace_destroy);

    return workspace_thumbnail;
}

static bool output_thumbnail_render(struct thumbnail_buffer *thumbnail_buffer,
                                    struct scene_output *scene_output)
{
    struct output_thumbnail *output_thumbnail =
        wl_container_of(thumbnail_buffer, output_thumbnail, base);
    struct scene_output *src_output = output_thumbnail->output;
    if (!output_thumbnail->state_readonly) {
        output_thumbnail->state.logical.x = src_output->geometry.x;
        output_thumbnail->state.logical.y = src_output->geometry.y;
    }

    const struct output_thumbnail_state *state = &output_thumbnail->state;
    int buffer_width = round(state->width * thumbnail_buffer->scale);
    int buffer_height = round(state->height * thumbnail_buffer->scale);
    if (buffer_width <= 0 || buffer_height <= 0) {
        kywc_log(KYWC_DEBUG, "output thumbnail size is too small, %d x %d", buffer_width,
                 buffer_height);
        return false;
    }

    if (!thumbnail_buffer_ensure_render_buffer(thumbnail_buffer, buffer_width, buffer_height)) {
        return false;
    }

    struct wlr_renderer *renderer = src_output->output->renderer;
    struct wlr_buffer *dst_buffer =
        thumbnail_buffer->shadow ? thumbnail_buffer->shadow : thumbnail_buffer->buffer;

    struct wlr_render_pass *render_pass =
        wlr_renderer_begin_buffer_pass(renderer, dst_buffer, NULL);
    if (!render_pass) {
        return false;
    }

    /* clear the target buffer */
    wlr_render_pass_add_rect(render_pass, &(struct wlr_render_rect_options){
                                              .color = { 0, 0, 0, 0 },
                                              .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                          });

    struct scene_render_target target = {
        .logical = state->logical,
        .scale = state->scale,
        .buffer = dst_buffer,
        .transform = state->transform,
        .trans_width = state->trans_width,
        .trans_height = state->trans_height,
        .output = src_output,
        .render_pass = render_pass,
        .options = SCENE_RENDER_DISABLE_VISIBILITY | SCENE_RENDER_ENABLE_FORCE_OPAQUE,
    };
    scene_render_target_init_regions(&target);
    pixman_region32_init_rect(&target.damage, target.logical.x, target.logical.y,
                              target.logical.width, target.logical.height);

    struct scene_node *root = &src_output->scene->tree.node;
    root->impl.render(root, root->x, root->y, &target);
    wlr_render_pass_submit(target.render_pass);
    scene_render_target_finish_regions(&target);

    thumbnail_buffer_get_info_from_source(thumbnail_buffer, &thumbnail_buffer->info);
    return thumbnail_buffer_blit_shadow(thumbnail_buffer, thumbnail_buffer->buffer);
}

static void output_thumbnail_destroy(struct thumbnail_buffer *thumbnail_buffer)
{
    if (!thumbnail_buffer_destroy(thumbnail_buffer)) {
        return;
    }

    struct output_thumbnail *output_thumbnail =
        wl_container_of(thumbnail_buffer, output_thumbnail, base);
    wl_list_remove(&output_thumbnail->output_destroy.link);
    wl_list_remove(&output_thumbnail->output_commit.link);
    wl_list_remove(&output_thumbnail->link);

    free(output_thumbnail);
}

static void output_thumbnail_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct output_thumbnail *output_thumbnail =
        wl_container_of(listener, output_thumbnail, output_destroy);
    /* force destroyed when output node destroy */
    output_thumbnail->base.need_destroy = true;
    output_thumbnail_destroy(&output_thumbnail->base);
}

static void get_state_from_wlr_output(struct output_thumbnail_state *state,
                                      struct wlr_output *wlr_output)
{
    state->scale = wlr_output->scale;
    state->width = wlr_output->width;
    state->height = wlr_output->height;
    state->transform = wlr_output->transform;
    wlr_output_transformed_resolution(wlr_output, &state->trans_width, &state->trans_height);

    state->logical.width = state->trans_width / state->scale;
    state->logical.height = state->trans_height / state->scale;
}

static void output_thumbnail_handle_output_commit(struct wl_listener *listener, void *data)
{
    struct output_thumbnail *output_thumbnail =
        wl_container_of(listener, output_thumbnail, output_commit);
    struct wlr_output_event_commit *event = data;

    if (!output_thumbnail->state_readonly &&
        (event->state->committed & WLR_OUTPUT_STATE_MODE ||
         event->state->committed & WLR_OUTPUT_STATE_SCALE ||
         event->state->committed & WLR_OUTPUT_STATE_TRANSFORM)) {
        get_state_from_wlr_output(&output_thumbnail->state, event->output);
    }

    if (!(event->state->committed & WLR_OUTPUT_STATE_BUFFER)) {
        return;
    }

    bool has_damage = event->state->committed & WLR_OUTPUT_STATE_DAMAGE &&
                      pixman_region32_not_empty(&event->state->damage);
    if (has_damage) {
        output_thumbnail->base.was_damaged = true;
        thumbnail_manager_schedule_frame(false);
    }
}

static bool output_state_is_same(struct output_thumbnail_state *thumbnail_state,
                                 struct kywc_output_state *output_state)
{
    if (thumbnail_state == NULL && output_state == NULL) {
        return true;
    } else if (thumbnail_state == NULL || output_state == NULL) {
        return false;
    }

    if (thumbnail_state->scale != output_state->scale ||
        thumbnail_state->width != output_state->width ||
        thumbnail_state->height != output_state->height ||
        thumbnail_state->logical.x != output_state->lx ||
        thumbnail_state->logical.y != output_state->ly ||
        thumbnail_state->transform != output_state->transform) {
        return false;
    }

    return true;
}

static struct output_thumbnail *find_output_thumbnail(struct scene_output *output,
                                                      struct kywc_output_state *state, float scale)
{
    struct output_thumbnail *output_thumbnail;
    wl_list_for_each(output_thumbnail, &manager->output_thumbnails, link) {
        if (!output_thumbnail->base.own_buffer) {
            continue;
        }
        if (output_thumbnail->output == output && output_thumbnail->base.scale == scale &&
            output_state_is_same(&output_thumbnail->state, state)) {
            return output_thumbnail;
        }
    }

    return NULL;
}

static void output_thumbnail_get_bounding_box(const struct thumbnail_buffer *thumbnail_buffer,
                                              struct kywc_box *bounding_box)
{
    struct output_thumbnail *output_thumbnail =
        wl_container_of(thumbnail_buffer, output_thumbnail, base);
    const struct output_thumbnail_state *state = &output_thumbnail->state;
    *bounding_box = (struct kywc_box){ 0, 0, state->logical.width, state->logical.height };
}

static const struct thumbnail_buffer_interface output_thumbnail_impl = {
    .get_bounding_box_from_source = output_thumbnail_get_bounding_box,
    .render = output_thumbnail_render,
    .destroy = output_thumbnail_destroy,
};

static struct output_thumbnail *
output_thumbnail_get_or_create(struct scene_output *output, struct kywc_output_state *output_state,
                               float scale, bool own_buffer)
{
    struct output_thumbnail *output_thumbnail = NULL;
    if (own_buffer) {
        output_thumbnail = find_output_thumbnail(output, output_state, scale);
        if (output_thumbnail) {
            return output_thumbnail;
        }
    }

    output_thumbnail = calloc(1, sizeof(*output_thumbnail));
    if (!output_thumbnail) {
        return NULL;
    }

    thumbnail_buffer_init(&output_thumbnail->base, THUMBNAIL_TYPE_OUTPUT, &output_thumbnail_impl,
                          scale, false, own_buffer);

    output_thumbnail->output = output;
    output_thumbnail->output_commit.notify = output_thumbnail_handle_output_commit;
    wl_signal_add(&output->output->events.commit, &output_thumbnail->output_commit);
    output_thumbnail->output_destroy.notify = output_thumbnail_handle_output_destroy;
    wl_signal_add(&output->events.destroy, &output_thumbnail->output_destroy);

    wl_list_insert(&manager->output_thumbnails, &output_thumbnail->link);

    struct output_thumbnail_state *state = &output_thumbnail->state;
    if (!output_state) {
        struct wlr_output *wlr_output = output_thumbnail->output->output;
        state->logical.x = output_thumbnail->output->geometry.x;
        state->logical.y = output_thumbnail->output->geometry.y;
        get_state_from_wlr_output(state, wlr_output);

        output_thumbnail->state_readonly = false;
        return output_thumbnail;
    }

    state->scale = output_state->scale;
    state->width = output_state->width;
    state->height = output_state->height;
    state->transform = output_state->transform;
    if (state->transform % 2 == 0) {
        state->trans_width = state->width;
        state->trans_height = state->height;
    } else {
        state->trans_height = state->width;
        state->trans_width = state->height;
    }

    state->logical.x = output_state->lx;
    state->logical.y = output_state->ly;
    state->logical.width = state->trans_width / state->scale;
    state->logical.height = state->trans_height / state->scale;

    output_thumbnail->state_readonly = true;
    return output_thumbnail;
}

struct thumbnail *thumbnail_create_from_node(struct scene_node *node, uint32_t options, float scale,
                                             const struct thumbnail_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    struct thumbnail *thumbnail = calloc(1, sizeof(*thumbnail));
    if (!thumbnail) {
        return NULL;
    }

    struct node_thumbnail *node_thumbnail =
        node_thumbnail_get_or_create(node, options, scale, !impl || !impl->buffer);
    if (!node_thumbnail) {
        free(thumbnail);
        return NULL;
    }

    thumbnail->buffer = &node_thumbnail->base;
    wl_list_insert(&node_thumbnail->base.thumbnails, &thumbnail->link);
    thumbnail->impl = impl;
    thumbnail->user_data = data;
    thumbnail->force_update = thumbnail->wants_update = true;
    /* buffer update is needed */
    thumbnail_manager_schedule_frame(true);

    return thumbnail;
}

struct thumbnail *thumbnail_create_from_view(struct view *view, uint32_t options, float scale,
                                             const struct thumbnail_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    struct thumbnail *thumbnail = calloc(1, sizeof(*thumbnail));
    if (!thumbnail) {
        return NULL;
    }

    struct view_thumbnail *view_thumbnail =
        view_thumbnail_get_or_create(view, options, scale, !impl || !impl->buffer);
    if (!view_thumbnail) {
        free(thumbnail);
        return NULL;
    }

    if (impl->client) {
        impl->client(thumbnail, &thumbnail->client, data);
    }

    thumbnail->buffer = &view_thumbnail->base;
    wl_list_insert(&view_thumbnail->base.thumbnails, &thumbnail->link);
    thumbnail->impl = impl;
    thumbnail->user_data = data;
    thumbnail->force_update = thumbnail->wants_update = true;
    /* buffer update is needed */
    thumbnail_manager_schedule_frame(true);

    return thumbnail;
}

struct thumbnail *thumbnail_create_from_workspace(struct workspace *workspace,
                                                  struct kywc_output *kywc_output, uint32_t options,
                                                  float scale,
                                                  const struct thumbnail_interface *impl,
                                                  void *data)
{
    if (!manager) {
        return NULL;
    }

    struct thumbnail *thumbnail = calloc(1, sizeof(*thumbnail));
    if (!thumbnail) {
        return NULL;
    }

    struct workspace_thumbnail *workspace_thumbnail = workspace_thumbnail_get_or_create(
        workspace, kywc_output, options, scale, !impl || !impl->buffer);
    if (!workspace_thumbnail) {
        free(thumbnail);
        return NULL;
    }

    if (impl->client) {
        impl->client(thumbnail, &thumbnail->client, data);
    }

    thumbnail->buffer = &workspace_thumbnail->base;
    wl_list_insert(&workspace_thumbnail->base.thumbnails, &thumbnail->link);
    thumbnail->impl = impl;
    thumbnail->user_data = data;
    thumbnail->force_update = thumbnail->wants_update = true;
    /* buffer update is needed */
    thumbnail_manager_schedule_frame(true);

    return thumbnail;
}

struct thumbnail *thumbnail_create_from_output(struct scene_output *output,
                                               struct kywc_output_state *output_state, float scale,
                                               const struct thumbnail_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    struct thumbnail *thumbnail = calloc(1, sizeof(*thumbnail));
    if (!thumbnail) {
        return NULL;
    }

    struct output_thumbnail *output_thumbnail =
        output_thumbnail_get_or_create(output, output_state, scale, !impl || !impl->buffer);
    if (!output_thumbnail) {
        free(thumbnail);
        return NULL;
    }

    thumbnail->buffer = &output_thumbnail->base;
    wl_list_insert(&output_thumbnail->base.thumbnails, &thumbnail->link);
    thumbnail->impl = impl;
    thumbnail->user_data = data;
    thumbnail->force_update = thumbnail->wants_update = true;
    /* buffer update is needed */
    output_schedule_frame(output->output);

    return thumbnail;
}

void thumbnail_destroy(struct thumbnail *thumbnail)
{
    if (!thumbnail) {
        return;
    }

    wl_list_remove(&thumbnail->link);

    /* thumbnail_buffer may not be destroyed caused by can_destroy == false */
    if (thumbnail->buffer) {
        thumbnail->buffer->impl->destroy(thumbnail->buffer);
    }

    free(thumbnail);
}

void thumbnail_mark_wants_update(struct thumbnail *thumbnail, bool wants, bool force)
{
    thumbnail->wants_update = wants;
    thumbnail->force_update = force;

    /* should send update if buffer was damaged */
    if (wants) {
        thumbnail_manager_schedule_frame(false);
    }
}

void thumbnail_get_buffer_size(struct thumbnail *thumbnail, int *width, int *height)
{
    struct thumbnail_buffer *thumbnail_buffer = thumbnail->buffer;
    struct kywc_box *bounding_box = &thumbnail_buffer->info.bounding_box;
    if (!thumbnail_buffer->buffer) {
        if (!thumbnail_buffer->impl->get_bounding_box_from_source) {
            *width = *height = 0;
            return;
        }

        thumbnail_buffer->impl->get_bounding_box_from_source(thumbnail_buffer, bounding_box);
    }

    *width = round(bounding_box->width * thumbnail_buffer->scale);
    *height = round(bounding_box->height * thumbnail_buffer->scale);
}

void thumbnail_get_source_extends(struct thumbnail *thumbnail, struct kywc_box *bbox)
{
    struct thumbnail_buffer *thumbnail_buffer = thumbnail->buffer;
    *bbox = (struct kywc_box){ 0, 0, 0, 0 };
    if (thumbnail_buffer->buffer) {
        *bbox = thumbnail_buffer->info.bounding_box;
    }
}

void thumbnail_get_source_radius(struct thumbnail *thumbnail, int radius[static 4])
{
    struct thumbnail_buffer *thumbnail_buffer = thumbnail->buffer;
    int len = sizeof(*radius) * 4;
    memset(radius, 0, len);
    if (thumbnail_buffer->buffer) {
        memcpy(radius, thumbnail_buffer->info.radius, len);
    }
}

bool thumbnail_get_source_padding(struct thumbnail *thumbnail, struct view_padding *padding)
{
    struct thumbnail_buffer *thumbnail_buffer = thumbnail->buffer;
    if (!thumbnail_buffer->buffer || (thumbnail_buffer->type != THUMBNAIL_TYPE_NODE &&
                                      thumbnail_buffer->type != THUMBNAIL_TYPE_VIEW)) {
        *padding = (struct view_padding){ 0 };
        return false;
    }

    *padding = thumbnail_buffer->info.padding;
    return true;
}

void thumbnail_get_effective_box(struct thumbnail *thumbnail, struct kywc_fbox *box)
{
    struct thumbnail_buffer *thumbnail_buffer = thumbnail->buffer;
    if (!thumbnail_buffer->buffer) {
        *box = (struct kywc_fbox){ 0, 0, 0, 0 };
        return;
    }

    if (thumbnail_buffer->type != THUMBNAIL_TYPE_NODE &&
        thumbnail_buffer->type != THUMBNAIL_TYPE_VIEW) {
        *box = (struct kywc_fbox){ 0, 0, 1.0f, 1.0f };
        return;
    }

    *box = thumbnail_buffer->info.effective_fbox;
}

static bool thumbnail_buffer_need_update(struct thumbnail_buffer *buffer, bool *force_update)
{
    bool need_update = false;
    struct thumbnail *thumbnail;
    wl_list_for_each(thumbnail, &buffer->thumbnails, link) {
        need_update |= thumbnail->wants_update;
        if (thumbnail->force_update) {
            *force_update |= thumbnail->force_update;
        }
    }
    return need_update;
}

static bool thumbnail_buffer_render(struct thumbnail_buffer *thumbnail_buffer,
                                    struct thumbnail_output *output)
{
    bool force_update = false;
    /* skip rendering when no thumbnail want update */
    if (!thumbnail_buffer_need_update(thumbnail_buffer, &force_update)) {
        return true;
    }

    struct thumbnail *thumbnail, *tmp;

    if (!thumbnail_buffer->was_damaged) {
        if (!force_update) {
            return true;
        } else if (thumbnail_buffer->has_content && thumbnail_buffer->own_buffer) {
            /* thumbnail_buffer cannot be destroyed in here */
            thumbnail_buffer->can_destroy = false;
            wl_list_for_each_safe(thumbnail, tmp, &thumbnail_buffer->thumbnails, link) {
                if (thumbnail->wants_update && thumbnail->force_update) {
                    thumbnail->force_update = false;
                    if (thumbnail->impl && thumbnail->impl->update) {
                        thumbnail->impl->update(thumbnail, thumbnail_buffer->buffer, true,
                                                thumbnail->user_data);
                    }
                }
            }
            /* thumbnail may need be destroyed in update */
            thumbnail_buffer->can_destroy = true;
            thumbnail_buffer->impl->destroy(thumbnail_buffer);
            return true;
        }
        // need render if force_update && !has_content
    }

    /* destroy it when render failed */
    if (!thumbnail_buffer->impl->render(thumbnail_buffer, output->output)) {
        thumbnail_buffer->need_destroy = true;
        thumbnail_buffer->impl->destroy(thumbnail_buffer);
        return false;
    }

    /* mark buffer is not damaged */
    thumbnail_buffer->was_damaged = false;
    thumbnail_buffer->has_content = true;

    thumbnail_buffer->can_destroy = false;
    wl_list_for_each_safe(thumbnail, tmp, &thumbnail_buffer->thumbnails, link) {
        if (thumbnail->wants_update) {
            bool changed = thumbnail_buffer->buffer_changed || thumbnail->force_update;
            thumbnail->force_update = false;
            if (thumbnail->impl && thumbnail->impl->update) {
                thumbnail->impl->update(thumbnail, thumbnail_buffer->buffer, changed,
                                        thumbnail->user_data);
            }
        } else {
            thumbnail->force_update |= thumbnail_buffer->buffer_changed;
        }
    }

    thumbnail_buffer->buffer_changed = false;
    thumbnail_buffer->can_destroy = true;
    thumbnail_buffer->impl->destroy(thumbnail_buffer);

    return true;
}

static void thumbnail_output_handle_frame(struct wl_listener *listener, void *data)
{
    struct thumbnail_output *output = wl_container_of(listener, output, frame);

    struct node_thumbnail *node_thumbnail, *tmp;
    wl_list_for_each_safe(node_thumbnail, tmp, &manager->node_thumbnails, link) {
        thumbnail_buffer_render(&node_thumbnail->base, output);
    }

    struct view_thumbnail *view_thumbnail, *view_tmp;
    wl_list_for_each_safe(view_thumbnail, view_tmp, &manager->view_thumbnails, link) {
        thumbnail_buffer_render(&view_thumbnail->base, output);
    }

    struct workspace_thumbnail *workspace_thumbnail, *_tmp;
    wl_list_for_each_safe(workspace_thumbnail, _tmp, &manager->workspace_thumbnails, link) {
        thumbnail_buffer_render(&workspace_thumbnail->base, output);
    }

    struct output_thumbnail *output_thumbnail, *output_tmp;
    wl_list_for_each_safe(output_thumbnail, output_tmp, &manager->output_thumbnails, link) {
        thumbnail_buffer_render(&output_thumbnail->base, output);
    }
}

void thumbnail_update(struct thumbnail *thumbnail)
{
    struct thumbnail_output *output;
    wl_list_for_each(output, &manager->outputs, link) {
        if (thumbnail->buffer->type == THUMBNAIL_TYPE_WORKSPACE) {
            thumbnail_output_handle_frame(&output->frame, NULL);
        } else {
            thumbnail_buffer_render(thumbnail->buffer, output);
        }
        break;
    }
}

static void thumbnail_output_handle_destroy(struct wl_listener *listener, void *data)
{
    struct thumbnail_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->power.link);
    wl_list_remove(&output->link);
    wl_event_source_remove(output->timer);
    free(output);
}

static void thumbnail_output_handle_power(struct wl_listener *listener, void *data)
{
    struct thumbnail_output *output = wl_container_of(listener, output, power);
    struct kywc_output *kywc_output = &output_from_wlr_output(output->output->output)->base;
    if (!kywc_output->state.power) {
        thumbnail_manager_schedule_frame(true);
    }
}

static int thumbnail_output_handle_timeout(void *data)
{
    struct thumbnail_output *thumbnail_output = data;
    thumbnail_output_handle_frame(&thumbnail_output->frame, NULL);
    thumbnail_output->frame_pending = false;
    return 0;
}

static void handle_new_enabled_output(struct wl_listener *listener, void *data)
{
    struct thumbnail_output *thumbnail_output = calloc(1, sizeof(*thumbnail_output));
    if (!thumbnail_output) {
        return;
    }

    thumbnail_output->timer = wl_event_loop_add_timer(
        manager->server->event_loop, thumbnail_output_handle_timeout, thumbnail_output);
    if (!thumbnail_output->timer) {
        free(thumbnail_output);
        return;
    }

    wl_list_insert(&manager->outputs, &thumbnail_output->link);

    struct kywc_output *kywc_output = data;
    thumbnail_output->output = output_from_kywc_output(kywc_output)->scene_output;
    thumbnail_output->frame.notify = thumbnail_output_handle_frame;
    wl_signal_add(&thumbnail_output->output->events.frame, &thumbnail_output->frame);
    thumbnail_output->destroy.notify = thumbnail_output_handle_destroy;
    wl_signal_add(&thumbnail_output->output->events.destroy, &thumbnail_output->destroy);
    thumbnail_output->power.notify = thumbnail_output_handle_power;
    wl_signal_add(&kywc_output->events.power, &thumbnail_output->power);

    thumbnail_output_handle_power(&thumbnail_output->power, NULL);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->destroy.link);
    wl_list_remove(&manager->new_enabled_output.link);
    free(manager);
    manager = NULL;
}

bool thumbnail_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->node_thumbnails);
    wl_list_init(&manager->view_thumbnails);
    wl_list_init(&manager->workspace_thumbnails);
    wl_list_init(&manager->output_thumbnails);

    wl_list_init(&manager->outputs);
    manager->new_enabled_output.notify = handle_new_enabled_output;
    output_manager_add_new_enabled_listener(&manager->new_enabled_output);

    manager->server = server;
    manager->destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->destroy);

    return true;
}
