// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/render/wlr_texture.h>

#include <kywc/boxes.h>
#include <kywc/log.h>

#include "effect/snapshot.h"
#include "effect_p.h"
#include "scene/surface.h"
#include "scene/thumbnail.h"
#include "server.h"
#include "util/macros.h"
#include "view/view.h"
#include "xwayland.h"

struct snapshot;

struct snapshot_impl {
    void (*snapshot_detach)(struct snapshot *snapshot);
    void (*snapshot_destroy)(struct snapshot *snapshot);
    float (*snapshot_get_scale)(const struct snapshot *snapshot);
    bool (*snapshot_ensure_buffer_bindable)(struct snapshot *snapshot);
    bool (*snapshot_mark_wants_update)(struct snapshot *snapshot, bool wants);
    void (*snapshot_render)(struct snapshot *snapshot, struct ky_scene_render_target *target);
};

struct snapshot {
    struct snapshot_impl *impl;
    struct thumbnail *thumbnail;
    struct wlr_texture *texture;
    struct wlr_buffer *buffer;
    struct snapshot *mix_snapshot;

    struct kywc_fbox src_box;
    /* logic coord */
    struct kywc_box dst_box;
    float opacity, blend_factor;
    uint32_t anchors, options;

    float scale;
    bool detached;
    bool wants_update;
    bool was_damaged;

    struct {
        struct wl_signal destroy;
    } events;

    struct wl_listener update;
    struct wl_listener destroy;

    struct wl_list binders;
};

enum snapshot_source_type { SNAPSHOT_SOURCE_TYPE_VIEW, SNAPSHOT_SOURCE_TYPE_NODE };

struct snapshot_source {
    enum snapshot_source_type type;
    union {
        struct view *view;
        struct ky_scene_node *node;
    };
};

struct node_snapshot {
    struct snapshot base;
    struct ky_scene_node *node;
    struct snapshot_source source;
    int blur_offset_x, blur_offset_y;

    bool need_blur;
    bool is_zerocopy;
    int blur_radius[4];
    float surface_scale;
    struct blur_info blur_info;
    bool without_subeffect_node;

    struct wl_listener node_damage;
    struct wl_listener node_destroy;
    struct wl_listener buffer_destroy;
};

struct snapshot_binder {
    struct wl_list link;
    struct snapshot *snapshot;
    struct ky_scene_buffer *scene_buffer;
    struct wl_listener scene_buffer_destroy;
};

static struct snapshot_manager {
    struct server *server;
    struct wl_listener destroy;
} *manager = NULL;

static void snapshot_binder_destroy(struct snapshot_binder *snapshot_binder)
{
    wl_list_remove(&snapshot_binder->scene_buffer_destroy.link);
    wl_list_remove(&snapshot_binder->link);
    free(snapshot_binder);
}

static void snapshot_binder_handle_scene_buffer_destroy(struct wl_listener *listener, void *data)
{
    struct snapshot_binder *binder = wl_container_of(listener, binder, scene_buffer_destroy);
    snapshot_binder_destroy(binder);
}

static struct snapshot_binder *snapshot_binder_create(struct snapshot *snapshot,
                                                      struct ky_scene_buffer *buffer)
{
    struct snapshot_binder *binder = calloc(1, sizeof(*binder));
    if (!binder) {
        return NULL;
    }

    binder->snapshot = snapshot;
    binder->scene_buffer = buffer;
    wl_list_insert(&snapshot->binders, &binder->link);

    binder->scene_buffer_destroy.notify = snapshot_binder_handle_scene_buffer_destroy;
    wl_signal_add(&buffer->node.events.destroy, &binder->scene_buffer_destroy);
    return binder;
}

static void node_snapshot_update_blur_info(struct node_snapshot *node_snapshot)
{
    if (!node_snapshot->need_blur) {
        return;
    }

    ky_scene_node_get_blur_info(node_snapshot->node, &node_snapshot->blur_info);
    ky_scene_node_get_radius(node_snapshot->node, KY_SCENE_ROUND_CORNERS_FOR_OUTERMOST,
                             node_snapshot->blur_radius);
}

static void node_snapshot_get_blur_offset(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (!snapshot->thumbnail || !node_snapshot->base.wants_update || !node_snapshot->need_blur) {
        return;
    }

    thumbnail_get_node_offset(snapshot->thumbnail, node_snapshot->node,
                              &node_snapshot->blur_offset_x, &node_snapshot->blur_offset_y);
}

/* don't destroy the thumbnail in snapshot，just assigning thumbnail as a null pointer */
static void snapshot_remove_thumbnail(struct snapshot *snapshot)
{
    wl_list_remove(&snapshot->destroy.link);
    wl_list_remove(&snapshot->update.link);
    wl_list_init(&snapshot->destroy.link);
    wl_list_init(&snapshot->update.link);

    snapshot->wants_update = false;
    snapshot->thumbnail = NULL;
}

static void snapshot_push_binder_damage(struct snapshot *snapshot)
{
    struct snapshot_binder *binder;
    wl_list_for_each(binder, &snapshot->binders, link) {
        ky_scene_node_push_damage(&binder->scene_buffer->node, KY_SCENE_DAMAGE_HARMLESS, NULL);
    }
}

static void snapshot_update_binder_buffer(struct snapshot *snapshot)
{
    if (wl_list_empty(&snapshot->binders)) {
        return;
    }

    if (snapshot->impl->snapshot_ensure_buffer_bindable &&
        !snapshot->impl->snapshot_ensure_buffer_bindable(snapshot)) {
        return;
    }

    struct snapshot_binder *binder;
    wl_list_for_each(binder, &snapshot->binders, link) {
        ky_scene_buffer_set_buffer(binder->scene_buffer, snapshot->buffer);
    }
}

static void node_snapshot_handle_thumbnail_update(struct wl_listener *listener, void *data)
{
    struct snapshot *snapshot = wl_container_of(listener, snapshot, update);
    struct thumbnail_update_event *event = data;
    node_snapshot_get_blur_offset(snapshot);
    if (!event->buffer_changed) {
        snapshot_push_binder_damage(snapshot);
        return;
    }

    snapshot->buffer = event->buffer;
    wlr_texture_destroy(snapshot->texture);
    snapshot_update_binder_buffer(snapshot);
    snapshot->texture = wlr_texture_from_buffer(manager->server->renderer, event->buffer);
}

static void node_snapshot_handle_thumbnail_destroy(struct wl_listener *listener, void *data)
{
    struct snapshot *snapshot = wl_container_of(listener, snapshot, destroy);
    snapshot_remove_thumbnail(snapshot);
}

static bool snapshot_init_thumbnail(struct snapshot *snapshot, struct thumbnail *thumbnail)
{
    snapshot->detached = false;
    snapshot->wants_update = true;
    snapshot->thumbnail = thumbnail;

    snapshot->update.notify = node_snapshot_handle_thumbnail_update;
    thumbnail_add_update_listener(thumbnail, &snapshot->update);
    snapshot->destroy.notify = node_snapshot_handle_thumbnail_destroy;
    thumbnail_add_destroy_listener(thumbnail, &snapshot->destroy);

    thumbnail_update(thumbnail);
    /* if the update fails, a destroy message will be sent */
    if (!snapshot->thumbnail) {
        return false;
    }

    /* just in case */
    if (!snapshot->buffer) {
        return false;
    }

    return true;
}

static void node_snapshot_set_zerocopy_buffer(struct node_snapshot *node_snapshot,
                                              struct ky_scene_buffer *scene_buffer)
{
    struct snapshot *snapshot = &node_snapshot->base;
    if (snapshot->buffer == scene_buffer->buffer) {
        snapshot_push_binder_damage(snapshot);
        return;
    }

    if (snapshot->buffer) {
        wl_list_remove(&node_snapshot->buffer_destroy.link);
        wl_list_init(&node_snapshot->buffer_destroy.link);
    }

    struct ky_scene_surface *scene_surface = ky_scene_surface_try_from_buffer(scene_buffer);
    node_snapshot->surface_scale =
        scene_surface ? ky_scene_surface_get_scale(scene_surface->surface) : 1.0f;

    snapshot->buffer = scene_buffer->buffer;
    snapshot_update_binder_buffer(snapshot);
    if (snapshot->buffer) {
        wl_signal_add(&snapshot->buffer->events.destroy, &node_snapshot->buffer_destroy);
    }
}

/* the dst_box must be logic coord */
static void snapshot_get_render_src_box(const struct snapshot *snapshot,
                                        const struct wlr_texture *texture,
                                        const struct kywc_box *dst_box, struct kywc_fbox *src_box)
{
    *src_box = snapshot->src_box;
    if (kywc_fbox_empty(src_box)) {
        src_box->height = texture->height;
        src_box->width = texture->width;
    }

    if (!snapshot->anchors) {
        return;
    }

    /**
     * scale factor use for logic size -> thumbnail buffer size,
     * render logic size content to dst box.
     */
    float scale = snapshot->impl->snapshot_get_scale ? snapshot->impl->snapshot_get_scale(snapshot)
                                                     : snapshot->scale;
    if (snapshot->anchors & SNAPSHOT_ANCHOR_TOP) {
        if (!(snapshot->anchors & SNAPSHOT_ANCHOR_BOTTOM)) {
            src_box->height = MIN(src_box->height, dst_box->height * scale);
        }
    } else {
        if ((snapshot->anchors & SNAPSHOT_ANCHOR_BOTTOM)) {
            int y2 = src_box->y + src_box->height;
            int new_y1 = y2 - dst_box->height * scale;
            src_box->y = MAX(src_box->y, new_y1);
            src_box->height = y2 - src_box->y;
        }
    }

    if (snapshot->anchors & SNAPSHOT_ANCHOR_LEFT) {
        if (!(snapshot->anchors & SNAPSHOT_ANCHOR_RIGHT)) {
            src_box->width = MIN(src_box->width, dst_box->width * scale);
        }
    } else {
        if ((snapshot->anchors & SNAPSHOT_ANCHOR_RIGHT)) {
            int x2 = src_box->x + src_box->width;
            int new_x1 = x2 - dst_box->width * scale;
            src_box->x = MAX(src_box->x, new_x1);
            src_box->width = x2 - src_box->x;
        }
    }
}

static void snapshot_get_render_dst_box(const struct snapshot *snapshot, int tex_width,
                                        int tex_heigt, struct kywc_box *dst_box)
{
    *dst_box = snapshot->dst_box;
    if (kywc_box_empty(dst_box)) {
        /* allow dst box is emtpy */
        return;
    }

    if (!(snapshot->options & SNAPSHOT_RENDER_ENABLE_VIEW_BOX) || !snapshot->thumbnail) {
        return;
    }

    int top = 0, left = 0, right = 0, bottom = 0;
    if (thumbnail_get_padding(snapshot->thumbnail, &top, &left, &right, &bottom)) {
        int width = tex_width - right - left;
        int height = tex_heigt - top - bottom;
        float width_scale = width == 0 ? 0 : 1.0 * dst_box->width / width;
        float height_scale = height == 0 ? 0 : 1.0 * dst_box->height / height;

        int scaled_left = width_scale * left;
        int scaled_right = width_scale * right;
        int scaled_top = ceil(height_scale * top);
        int scaled_bottom = ceil(height_scale * bottom);

        dst_box->x -= scaled_left;
        dst_box->y -= scaled_top;
        dst_box->width += scaled_right + scaled_left;
        dst_box->height += scaled_bottom + scaled_top;
    }
}

static void node_snapshot_render(struct snapshot *snapshot, struct ky_scene_render_target *target)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (snapshot->was_damaged && snapshot->wants_update) {
        snapshot->was_damaged = false;
        node_snapshot_update_blur_info(node_snapshot);
        if (node_snapshot->is_zerocopy) {
            node_snapshot_set_zerocopy_buffer(node_snapshot,
                                              ky_scene_buffer_from_node(node_snapshot->node));
        }
    }

    struct wlr_texture *texture = snapshot->texture;
    if (node_snapshot->is_zerocopy && snapshot->wants_update) {
        texture = ky_scene_buffer_get_texture(ky_scene_buffer_from_node(node_snapshot->node),
                                              target->output->output->renderer);
    }

    if (!texture) {
        return;
    }

    struct kywc_box geometry = { 0 };
    snapshot_get_render_dst_box(snapshot, texture->width, texture->height, &geometry);
    if (kywc_box_empty(&geometry)) {
        return;
    }

    struct kywc_fbox src_box = { 0 };
    snapshot_get_render_src_box(snapshot, texture, &geometry, &src_box);

    /* if data need blur is false, blur region is empty */
    bool need_blur = pixman_region32_not_empty(&node_snapshot->blur_info.region);
    const struct ky_scene_render_texture_options opts = {
        .texture = texture,
        .geometry_box = &geometry,
        .alpha = &snapshot->opacity,
        .src = &src_box,
        .blur = {
            .scale = &snapshot->scale,
            .alpha = &snapshot->opacity,
            .info = need_blur ? &node_snapshot->blur_info : NULL,
            .offset_x = node_snapshot->blur_offset_x,
            .offset_y = node_snapshot->blur_offset_y,
            .radius = &node_snapshot->blur_radius,
        },
    };
    ky_scene_render_target_add_texture(target, &opts);
}

static bool node_snapshot_mark_wants_update(struct snapshot *snapshot, bool wants)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (!node_snapshot->node) {
        kywc_log(KYWC_INFO, "snapshot node is null, the texture can't update");
        return false;
    }

    /* do the following operations only when there is zero copy */
    if (!node_snapshot->is_zerocopy) {
        return true;
    }

    /* use textures that are no longer updated during rendering */
    if (!wants && snapshot->buffer) {
        snapshot->texture = wlr_texture_from_buffer(manager->server->renderer, snapshot->buffer);
    }

    /* real time texture acquisition during rendering */
    if (wants) {
        wlr_texture_destroy(snapshot->texture);
        snapshot->texture = NULL;
    }
    return true;
}

static float node_snapshot_get_scale(const struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    return node_snapshot->is_zerocopy ? node_snapshot->surface_scale : snapshot->scale;
}

static bool node_snapshot_ensure_buffer_bindable(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (!node_snapshot->is_zerocopy) {
        return true;
    }

    if (snapshot->buffer && wlr_client_buffer_get(snapshot->buffer)) {
        return true;
    }

    if (snapshot->detached && !node_snapshot->source.node) {
        return false;
    }

    struct thumbnail *thumbnail = NULL;
    if (node_snapshot->source.type == SNAPSHOT_SOURCE_TYPE_VIEW) {
        uint32_t thumbnail_options = (snapshot->options & SNAPSHOT_DISABLE_DECOR) |
                                     (snapshot->options & SNAPSHOT_DISABLE_SHADOW) |
                                     (snapshot->options & SNAPSHOT_DISABLE_ROUND_CORNER);
        thumbnail = thumbnail_create_from_view(node_snapshot->source.view, thumbnail_options,
                                               snapshot->scale);
    } else {
        thumbnail = thumbnail_create_from_node(node_snapshot->node, snapshot->scale,
                                               node_snapshot->without_subeffect_node);
    }

    if (!thumbnail) {
        return false;
    }

    bool old_wants_update = snapshot->wants_update;
    node_snapshot->is_zerocopy = false;
    snapshot->wants_update = true;
    snapshot->thumbnail = thumbnail;

    snapshot->update.notify = node_snapshot_handle_thumbnail_update;
    thumbnail_add_update_listener(thumbnail, &snapshot->update);
    snapshot->destroy.notify = node_snapshot_handle_thumbnail_destroy;
    thumbnail_add_destroy_listener(thumbnail, &snapshot->destroy);

    thumbnail_update(thumbnail);
    /* if the update fails, a destroy message will be sent */
    if (!snapshot->thumbnail) {
        node_snapshot->is_zerocopy = true;
        snapshot->wants_update = old_wants_update;
        return false;
    }

    /* just in case */
    if (!snapshot->buffer) {
        thumbnail_destroy(thumbnail);
        node_snapshot->is_zerocopy = true;
        snapshot->wants_update = old_wants_update;
        return false;
    }

    return true;
}

static void node_snapshot_detach(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    wl_list_remove(&node_snapshot->node_damage.link);
    wl_list_remove(&node_snapshot->node_destroy.link);
    wl_list_init(&node_snapshot->node_damage.link);
    wl_list_init(&node_snapshot->node_destroy.link);
    node_snapshot->node = NULL;
}

static void node_snapshot_destroy(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    wl_list_remove(&node_snapshot->buffer_destroy.link);
    pixman_region32_fini(&node_snapshot->blur_info.region);
}

static struct snapshot_impl node_snapshot_impl = {
    .snapshot_ensure_buffer_bindable = node_snapshot_ensure_buffer_bindable,
    .snapshot_mark_wants_update = node_snapshot_mark_wants_update,
    .snapshot_get_scale = node_snapshot_get_scale,
    .snapshot_render = node_snapshot_render,
    .snapshot_detach = node_snapshot_detach,
    .snapshot_destroy = node_snapshot_destroy,
};

static void node_snapshot_handle_buffer_destroy(struct wl_listener *listener, void *data)
{
    struct node_snapshot *node_snapshot = wl_container_of(listener, node_snapshot, buffer_destroy);
    wl_list_remove(&node_snapshot->buffer_destroy.link);
    wl_list_init(&node_snapshot->buffer_destroy.link);

    node_snapshot->base.buffer = NULL;
}

static void node_snapshot_handle_node_damage(struct wl_listener *listener, void *data)
{
    struct node_snapshot *node_snapshot = wl_container_of(listener, node_snapshot, node_damage);
    if (node_snapshot->node->type == KY_SCENE_NODE_BUFFER) {
        node_snapshot->base.was_damaged = true;
        if (node_snapshot->is_zerocopy && node_snapshot->base.wants_update) {
            node_snapshot_set_zerocopy_buffer(node_snapshot,
                                              ky_scene_buffer_from_node(node_snapshot->node));
        }
        return;
    }

    struct ky_scene_node *damage_node = data;
    if (node_snapshot->without_subeffect_node && damage_node != node_snapshot->node &&
        (damage_node->has_effect ||
         ky_scene_node_find_upper_effect_node(damage_node, node_snapshot->node))) {
        return;
    }

    node_snapshot->base.was_damaged = true;
}

static void node_snapshot_handle_node_destroy(struct wl_listener *listener, void *data)
{
    struct node_snapshot *node_snapshot = wl_container_of(listener, node_snapshot, node_destroy);
    node_snapshot->source.node = NULL;
    snapshot_detach(&node_snapshot->base);
}

void snapshot_destroy(struct snapshot *snapshot)
{
    wl_signal_emit_mutable(&snapshot->events.destroy, NULL);
    assert(wl_list_empty(&snapshot->events.destroy.listener_list));

    struct snapshot_binder *binder, *tmp;
    wl_list_for_each_safe(binder, tmp, &snapshot->binders, link) {
        snapshot_binder_destroy(binder);
    }

    snapshot_detach(snapshot);
    wlr_texture_destroy(snapshot->texture);

    if (snapshot->impl->snapshot_destroy) {
        snapshot->impl->snapshot_destroy(snapshot);
    }

    free(snapshot);
}

struct ky_scene_buffer *view_try_get_single_buffer(struct view *view)
{
    if (view->base.ssd != KYWC_SSD_NONE || !xwayland_check_view(view)) {
        return NULL;
    }

    return ky_scene_buffer_try_from_surface(view->surface);
}

static struct snapshot *snapshot_create(struct snapshot_source *source, float scale,
                                        uint32_t options)
{
    struct node_snapshot *node_snapshot = calloc(1, sizeof(*node_snapshot));
    if (!node_snapshot) {
        return NULL;
    }

    node_snapshot->base.scale = scale;
    node_snapshot->base.opacity = 1.0f;
    node_snapshot->base.options = options;
    node_snapshot->base.impl = &node_snapshot_impl;
    wl_signal_init(&node_snapshot->base.events.destroy);
    wl_list_init(&node_snapshot->base.update.link);
    wl_list_init(&node_snapshot->base.destroy.link);
    wl_list_init(&node_snapshot->base.binders);

    if (source->type == SNAPSHOT_SOURCE_TYPE_VIEW) {
        struct ky_scene_buffer *buffer = view_try_get_single_buffer(source->view);
        node_snapshot->node = buffer ? &buffer->node : &source->view->tree->node;
    } else if (source->type == SNAPSHOT_SOURCE_TYPE_NODE) {
        node_snapshot->node = source->node;
    }
    node_snapshot->source = *source;
    node_snapshot->surface_scale = 1.0f;
    node_snapshot->need_blur = !(options & SNAPSHOT_RENDER_DISABLE_BLUR);
    node_snapshot->without_subeffect_node = !(options & SNAPSHOT_ENABLE_SUBEFFECT_NODE);
    node_snapshot->buffer_destroy.notify = node_snapshot_handle_buffer_destroy;
    wl_list_init(&node_snapshot->buffer_destroy.link);
    pixman_region32_init(&node_snapshot->blur_info.region);
    node_snapshot_update_blur_info(node_snapshot);

    if (node_snapshot->node->type == KY_SCENE_NODE_BUFFER) {
        struct ky_scene_buffer *buffer = ky_scene_buffer_from_node(node_snapshot->node);
        /**
         * don't care about scale, all rendering results are derived from the original buffer.
         * if options are set, they can be converted into rendering parameters and applied
         * during rendering.
         */
        if (!ky_scene_buffer_is_clipped(buffer)) {
            node_snapshot->is_zerocopy = true;
            /* update texture */
            node_snapshot->base.was_damaged = true;
            node_snapshot->base.wants_update = true;
            /* TODO: set up src_box based on options */
            node_snapshot_set_zerocopy_buffer(node_snapshot, buffer);
            goto final;
        }
    }

    struct thumbnail *thumbnail = NULL;
    if (source->type == SNAPSHOT_SOURCE_TYPE_VIEW) {
        uint32_t thumbnail_options = (options & (SNAPSHOT_DISABLE_DECOR | SNAPSHOT_DISABLE_SHADOW |
                                                 SNAPSHOT_DISABLE_ROUND_CORNER));
        thumbnail = thumbnail_create_from_view(source->view, thumbnail_options, scale);
    } else {
        thumbnail = thumbnail_create_from_node(node_snapshot->node, scale,
                                               node_snapshot->without_subeffect_node);
    }

    if (!thumbnail || !snapshot_init_thumbnail(&node_snapshot->base, thumbnail)) {
        if (node_snapshot->base.thumbnail) {
            thumbnail_destroy(thumbnail);
        }
        pixman_region32_fini(&node_snapshot->blur_info.region);
        free(node_snapshot);
        return NULL;
    }

final:
    node_snapshot->node_damage.notify = node_snapshot_handle_node_damage;
    wl_signal_add(&node_snapshot->node->events.damage, &node_snapshot->node_damage);
    node_snapshot->node_destroy.notify = node_snapshot_handle_node_destroy;
    wl_signal_add(&node_snapshot->node->events.destroy, &node_snapshot->node_destroy);

    return &node_snapshot->base;
}

struct snapshot *snapshot_create_from_view(struct view *view, float scale, uint32_t options)
{
    struct snapshot_source source = {
        .type = SNAPSHOT_SOURCE_TYPE_VIEW,
        .view = view,
    };
    return snapshot_create(&source, scale, options);
}

struct snapshot *snapshot_create_from_node(struct ky_scene_node *node, float scale,
                                           uint32_t options)
{
    struct snapshot_source source = {
        .type = SNAPSHOT_SOURCE_TYPE_NODE,
        .node = node,
    };
    return snapshot_create(&source, scale, options);
}

struct ky_scene_buffer *snapshot_create_scene_buffer(struct ky_scene_tree *parent,
                                                     struct snapshot *snapshot)
{
    if (snapshot->impl->snapshot_ensure_buffer_bindable &&
        !snapshot->impl->snapshot_ensure_buffer_bindable(snapshot)) {
        return NULL;
    }

    struct ky_scene_buffer *buffer = ky_scene_buffer_create(parent, snapshot->buffer);
    if (!buffer) {
        return NULL;
    }

    if (!snapshot_binder_create(snapshot, buffer)) {
        ky_scene_node_destroy(&buffer->node);
        return NULL;
    }

    return buffer;
}

void snapshot_set_dst_box(struct snapshot *snapshot, const struct kywc_box *box)
{
    if (box != NULL) {
        snapshot->dst_box = *box;
    } else {
        snapshot->dst_box = (struct kywc_box){ 0 };
    }
}

void snapshot_set_anchors(struct snapshot *snapshot, uint32_t anchors)
{
    snapshot->anchors = anchors;
}

void snapshot_set_opacity(struct snapshot *snapshot, float opacity)
{
    snapshot->opacity = opacity;
}

void snapshot_set_mixed_snapshot(struct snapshot *snapshot, struct snapshot *mix_snapshot,
                                 float blend_factor)
{
    snapshot->blend_factor = blend_factor;
    snapshot->mix_snapshot = mix_snapshot;
}

void snapshot_get_render_box(struct snapshot *snapshot, struct kywc_box *dst_box)
{
    if (!snapshot->buffer) {
        *dst_box = snapshot->dst_box;
        return;
    }

    snapshot_get_render_dst_box(snapshot, snapshot->buffer->width, snapshot->buffer->height,
                                dst_box);
}

void snapshot_mark_wants_update(struct snapshot *snapshot, bool wants)
{
    if (snapshot->wants_update == wants) {
        return;
    }

    if (snapshot->impl->snapshot_mark_wants_update) {
        if (!snapshot->impl->snapshot_mark_wants_update(snapshot, wants)) {
            return;
        }
    }

    snapshot->wants_update = wants;
    if (!snapshot->thumbnail) {
        return;
    }

    thumbnail_mark_wants_update(snapshot->thumbnail, wants);
}

void snapshot_detach(struct snapshot *snapshot)
{
    if (snapshot->detached) {
        return;
    }

    snapshot->was_damaged = false;
    snapshot_mark_wants_update(snapshot, false);

    if (snapshot->impl->snapshot_detach) {
        snapshot->impl->snapshot_detach(snapshot);
    }

    snapshot->detached = true;
    if (!snapshot->thumbnail) {
        return;
    }

    thumbnail_destroy(snapshot->thumbnail);
}

void snapshot_add_destroy_listener(struct snapshot *snapshot, struct wl_listener *listener)
{
    wl_signal_add(&snapshot->events.destroy, listener);
}

void snapshot_render(struct snapshot *snapshot, struct ky_scene_render_target *target)
{
    assert(snapshot->impl->snapshot_render);
    snapshot->impl->snapshot_render(snapshot, target);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->destroy.link);
    free(manager);
    manager = NULL;
}

bool snapshot_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->server = server;
    manager->destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->destroy);

    return true;
}
