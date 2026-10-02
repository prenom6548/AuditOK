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
#include "render/pass.h"
#include "scene/surface.h"
#include "scene/thumbnail.h"
#include "security/render.h"
#include "server.h"
#include "util/macros.h"
#include "view/view.h"
#include "xwayland/xwayland.h"

struct snapshot_impl {
    void (*snapshot_detach)(struct snapshot *snapshot);
    void (*snapshot_destroy)(struct snapshot *snapshot);
    float (*snapshot_get_scale)(const struct snapshot *snapshot);
    bool (*snapshot_ensure_blockable)(struct snapshot *snapshot);
    bool (*snapshot_ensure_buffer_bindable)(struct snapshot *snapshot);
    void (*snapshot_get_render_box)(const struct snapshot *snapshot, struct kywc_box *dst);
    void (*snapshot_render)(struct snapshot *snapshot, struct scene_render_target *target);
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

    struct wl_list binders;
};

enum snapshot_source_type { SNAPSHOT_SOURCE_TYPE_VIEW, SNAPSHOT_SOURCE_TYPE_NODE };

struct snapshot_source {
    enum snapshot_source_type type;
    union {
        struct view *view;
        struct scene_node *node;
    };
};

struct node_snapshot {
    struct snapshot base;
    struct scene_node *node;
    struct snapshot_source source;
    int blur_offset_x, blur_offset_y;

    bool need_blur;
    bool is_zerocopy;
    float surface_scale;
    struct blur_info blur_info;
    bool without_subeffect_node;
    struct kywc_box thumbnail_box;
    struct kywc_fbox view_src;
    int radius[4];

    struct wl_listener node_damage;
    struct wl_listener node_destroy;
    struct wl_listener buffer_destroy;
};

struct snapshot_binder {
    struct wl_list link;
    struct snapshot *snapshot;
    struct scene_buffer *scene_buffer;
    struct wl_listener scene_buffer_destroy;
};

static struct snapshot_manager {
    struct server *server;
    struct wl_listener destroy;
} *manager = NULL;

static struct node_snapshot *node_snapshot_try_from_snapshot(const struct snapshot *snapshot);

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
                                                      struct scene_buffer *buffer)
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

    /* keep the processing logic of node enabled consistent with thumbnail rendering */
    bool stash_enabled = node_snapshot->node->enabled;
    node_snapshot->node->enabled = true;
    scene_node_get_blur_info(node_snapshot->node, &node_snapshot->blur_info);
    node_snapshot->node->enabled = stash_enabled;
}

static void node_snapshot_update_thumbnail_info(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (!snapshot->thumbnail || !node_snapshot->base.wants_update) {
        return;
    }

    thumbnail_get_source_extends(snapshot->thumbnail, &node_snapshot->thumbnail_box);
    thumbnail_get_effective_box(snapshot->thumbnail, &node_snapshot->view_src);
    thumbnail_get_source_radius(snapshot->thumbnail, node_snapshot->radius);
    node_snapshot->blur_offset_x = -node_snapshot->thumbnail_box.x;
    node_snapshot->blur_offset_y = -node_snapshot->thumbnail_box.y;
}

static void snapshot_remove_thumbnail(struct snapshot *snapshot)
{
    if (snapshot->thumbnail) {
        thumbnail_destroy(snapshot->thumbnail);
        snapshot->thumbnail = NULL;
    }

    snapshot->wants_update = false;
}

static void snapshot_push_binder_damage(struct snapshot *snapshot)
{
    struct snapshot_binder *binder;
    wl_list_for_each(binder, &snapshot->binders, link) {
        scene_node_push_damage(&binder->scene_buffer->node, SCENE_DAMAGE_HARMLESS, NULL);
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
        scene_buffer_set_buffer(binder->scene_buffer, snapshot->buffer);
    }
}

static void node_snapshot_handle_thumbnail_update(struct thumbnail *thumbnail,
                                                  struct wlr_buffer *buffer, bool buffer_changed,
                                                  void *data)
{
    struct snapshot *snapshot = data;
    node_snapshot_update_thumbnail_info(snapshot);
    if (!buffer_changed) {
        snapshot_push_binder_damage(snapshot);
        return;
    }

    snapshot->buffer = buffer;
    wlr_texture_destroy(snapshot->texture);
    snapshot_update_binder_buffer(snapshot);
    snapshot->texture = wlr_texture_from_buffer(manager->server->renderer, buffer);
}

static void node_snapshot_handle_thumbnail_cancel(struct thumbnail *thumbnail, void *data)
{
    struct snapshot *snapshot = data;
    snapshot->thumbnail = NULL;
    snapshot_remove_thumbnail(snapshot);
}

static const struct thumbnail_interface snapshot_impl = {
    .update = node_snapshot_handle_thumbnail_update,
    .cancel = node_snapshot_handle_thumbnail_cancel,
};

static bool snapshot_init_thumbnail(struct snapshot *snapshot, struct thumbnail *thumbnail)
{
    snapshot->detached = false;
    snapshot->wants_update = true;
    snapshot->thumbnail = thumbnail;

    thumbnail_update(thumbnail);
    /* if the update fails, a destroy message will be sent */
    if (!snapshot->thumbnail) {
        return false;
    }

    /* just in case */
    return snapshot->buffer != NULL;
}

static void node_snapshot_set_zerocopy_buffer(struct node_snapshot *node_snapshot,
                                              struct scene_buffer *scene_buffer)
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

    struct scene_surface *scene_surface = scene_surface_try_from_buffer(scene_buffer);
    node_snapshot->surface_scale =
        scene_surface ? scene_surface_get_scale(scene_surface->surface) : 1.0f;
    scene_node_get_affected_bounding_box(&scene_buffer->node, SCENE_BOUNDING_WITHOUT_EFFECT,
                                         &node_snapshot->thumbnail_box);

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

static void snapshot_get_full_box(const struct snapshot *snapshot,
                                  const struct kywc_box *render_box, struct kywc_box *full_box)
{
    struct node_snapshot *node_snapshot = node_snapshot_try_from_snapshot(snapshot);
    if (!node_snapshot) {
        return;
    }

    *full_box = *render_box;
    if (kywc_fbox_not_empty(&snapshot->src_box)) {
        /* TODO: */
        return;
    }

    if ((snapshot->anchors == SNAPSHOT_ANCHOR_NONE || snapshot->anchors == SNAPSHOT_ANCHOR_ALL)) {
        return;
    }

    /* calculate the required area for rendering the entire texture */
    int full_height = node_snapshot->thumbnail_box.height;
    int render_height = render_box->height;
    if (snapshot->anchors & SNAPSHOT_ANCHOR_TOP) {
        if (!(snapshot->anchors & SNAPSHOT_ANCHOR_BOTTOM)) {
            render_height = MAX(render_height, full_height);
        }
    } else if (snapshot->anchors & SNAPSHOT_ANCHOR_BOTTOM) {
        render_height = MAX(render_height, full_height);
        full_box->y = render_box->y + render_box->height - render_height;
    }
    full_box->height = render_height;

    int full_width = node_snapshot->thumbnail_box.width;
    int render_width = render_box->width;
    if (snapshot->anchors & SNAPSHOT_ANCHOR_LEFT) {
        if (!(snapshot->anchors & SNAPSHOT_ANCHOR_RIGHT)) {
            render_width = MAX(render_width, full_width);
        }
    } else if (snapshot->anchors & SNAPSHOT_ANCHOR_RIGHT) {
        render_width = MAX(render_width, full_width);
        full_box->x = render_box->x + render_box->width - render_width;
    }
    full_box->width = render_width;

    if (render_width == 0 || render_height == 0) {
        *full_box = (struct kywc_box){ 0 };
        return;
    }
}

static void view_box_from_full_box(struct snapshot *snapshot, struct kywc_box *full_box,
                                   struct kywc_box *view_box)
{
    *view_box = *full_box;
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    struct kywc_fbox *view_src = &node_snapshot->view_src;
    if (view_src->width <= 0.0f || view_src->height <= 0.0f || view_src->width > 1.0f ||
        view_src->height > 1.0f) {
        return;
    }

    view_box->x = full_box->x + full_box->width * view_src->x;
    view_box->y = full_box->y + full_box->height * view_src->y;
    view_box->width = full_box->width * view_src->width;
    view_box->height = full_box->height * view_src->height;
}

static void node_snapshot_get_render_box(const struct snapshot *snapshot, struct kywc_box *dst_box)
{
    if (!(snapshot->options & SNAPSHOT_RENDER_ENABLE_VIEW_BOX)) {
        return;
    }

    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    struct kywc_fbox *view_src = &node_snapshot->view_src;
    if (view_src->width <= 0.0f || view_src->height <= 0.0f || view_src->width > 1.0f ||
        view_src->height > 1.0f) {
        return;
    }

    /* invert normalized view_src to expand from view box to full box */
    double full_w = (double)dst_box->width / view_src->width;
    double full_h = (double)dst_box->height / view_src->height;
    double full_x = (double)dst_box->x - full_w * view_src->x;
    double full_y = (double)dst_box->y - full_h * view_src->y;

    dst_box->x = round(full_x);
    dst_box->y = round(full_y);
    dst_box->width = round(full_w);
    dst_box->height = round(full_h);
}

static void node_snapshot_render(struct snapshot *snapshot, struct scene_render_target *target)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    if (snapshot->was_damaged && snapshot->wants_update) {
        snapshot->was_damaged = false;
        node_snapshot_update_blur_info(node_snapshot);
        if (node_snapshot->is_zerocopy) {
            scene_node_get_radius(node_snapshot->node, node_snapshot->radius);
            node_snapshot_set_zerocopy_buffer(node_snapshot,
                                              scene_buffer_from_node(node_snapshot->node));
        }
    }

    struct wlr_texture *texture = snapshot->texture;
    struct wlr_renderer *renderer = target->output->output->renderer;
    if (node_snapshot->is_zerocopy && snapshot->wants_update) {
        texture = scene_buffer_get_texture(scene_buffer_from_node(node_snapshot->node), renderer);
    } else if (!texture) {
        texture = wlr_texture_from_buffer(renderer, snapshot->buffer);
        snapshot->texture = texture;
    }

    if (!texture) {
        return;
    }

    struct kywc_box geometry = { 0 };
    snapshot_get_render_box(snapshot, &geometry);
    if (kywc_box_empty(&geometry)) {
        return;
    }

    struct kywc_fbox src_box = { 0 };
    snapshot_get_render_src_box(snapshot, texture, &geometry, &src_box);

    struct kywc_box full_box = { 0 }, view_box = { 0 };
    snapshot_get_full_box(snapshot, &geometry, &full_box);
    view_box_from_full_box(snapshot, &full_box, &view_box);

    pixman_region32_t content_clip;
    pixman_region32_init_rect(&content_clip, geometry.x, geometry.y, geometry.width,
                              geometry.height);
    pixman_region32_intersect_rect(&content_clip, &content_clip, view_box.x, view_box.y,
                                   view_box.width, view_box.height);
    pixman_region32_translate(&content_clip, -geometry.x, -geometry.y);

    struct kywc_box mix_dst = { 0 };
    struct snapshot *mix_snapshot = snapshot->mix_snapshot;
    if (mix_snapshot && mix_snapshot->texture) {
        snapshot_get_render_box(mix_snapshot, &mix_dst);
        scene_render_target_box(target, &mix_dst);
    }

    /* if data need blur is false, blur region is empty */
    bool need_blur = pixman_region32_not_empty(&node_snapshot->blur_info.region);
    const struct scene_render_texture_options opts = {
        .texture = texture,
        .buffer = snapshot->buffer,
        .geometry_box = &geometry,
        .content_clip = &content_clip,
        .alpha = &snapshot->opacity,
        .radius = &node_snapshot->radius,
        .src = &src_box,
        .blur = {
            .scale = &snapshot->scale,
            .alpha = &snapshot->opacity,
            .info = need_blur ? &node_snapshot->blur_info : NULL,
            .offset_x = node_snapshot->blur_offset_x,
            .offset_y = node_snapshot->blur_offset_y,
            .radius = &node_snapshot->radius,
        },
        .mix_options = (mix_snapshot && mix_snapshot->texture) ?
            &(struct render_texture_mix_options) {
                .texture = mix_snapshot->texture,
                .blend_factor = snapshot->blend_factor,
                .dst_box = &mix_dst,
            } : NULL,
    };
    scene_render_target_add_texture_with_blur(target, &opts);
    pixman_region32_fini(&content_clip);
}

static bool node_snapshot_ensure_blockable(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    /* do the following operations only when there is zero copy */
    if (!node_snapshot->is_zerocopy) {
        return true;
    }

    if (!node_snapshot->node) {
        kywc_log(KYWC_DEBUG, "snapshot node is null, the texture can't update");
        return false;
    }

    /* detach from source buffer, can't convert to texture. texture will lock the buffer. */
    return snapshot->impl->snapshot_ensure_buffer_bindable(snapshot);
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
    if (node_snapshot->source.type == SNAPSHOT_SOURCE_TYPE_VIEW && !node_snapshot->is_zerocopy) {
        uint32_t thumbnail_options = 0;
        if (snapshot->options & SNAPSHOT_DISABLE_DECOR) {
            thumbnail_options |= THUMBNAIL_DISABLE_DECOR;
        }
        if (snapshot->options & SNAPSHOT_DISABLE_SHADOW) {
            thumbnail_options |= THUMBNAIL_DISABLE_SHADOW;
        }
        if (snapshot->options & SNAPSHOT_DISABLE_ROUND_CORNER) {
            thumbnail_options |= THUMBNAIL_DISABLE_ROUND_CORNER;
        }
        if (snapshot->options & SNAPSHOT_ENABLE_STYLE_EFFECT) {
            thumbnail_options |= THUMBNAIL_ENABLE_STYLE_EFFECT;
        }
        thumbnail = thumbnail_create_from_view(node_snapshot->source.view, thumbnail_options,
                                               snapshot->scale, &snapshot_impl, node_snapshot);
    } else {
        uint32_t thumbnail_options = 0;
        if (snapshot->options & SNAPSHOT_DISABLE_ROUND_CORNER) {
            thumbnail_options |= THUMBNAIL_DISABLE_ROUND_CORNER;
        }
        if (snapshot->options & SNAPSHOT_ENABLE_STYLE_EFFECT) {
            thumbnail_options |= THUMBNAIL_ENABLE_STYLE_EFFECT;
        }
        if (node_snapshot->without_subeffect_node) {
            thumbnail_options |= THUMBNAIL_WITHOUT_SUBEFFECT_NODE;
        }
        thumbnail = thumbnail_create_from_node(node_snapshot->node, thumbnail_options,
                                               snapshot->scale, &snapshot_impl, node_snapshot);
    }

    if (!thumbnail) {
        return false;
    }

    bool old_wants_update = snapshot->wants_update;
    node_snapshot->is_zerocopy = false;
    snapshot->wants_update = true;
    snapshot->thumbnail = thumbnail;

    thumbnail_update(thumbnail);
    /* if the update fails, a destroy message will be sent */
    if (!snapshot->thumbnail) {
        node_snapshot->is_zerocopy = true;
        snapshot->wants_update = old_wants_update;
        return false;
    }

    /* just in case */
    if (!snapshot->buffer) {
        snapshot_remove_thumbnail(snapshot);
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
    node_snapshot->source.node = NULL;
}

static void node_snapshot_destroy(struct snapshot *snapshot)
{
    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    wl_list_remove(&node_snapshot->buffer_destroy.link);
    pixman_region32_fini(&node_snapshot->blur_info.region);
}

static struct snapshot_impl node_snapshot_impl = {
    .snapshot_ensure_buffer_bindable = node_snapshot_ensure_buffer_bindable,
    .snapshot_ensure_blockable = node_snapshot_ensure_blockable,
    .snapshot_get_render_box = node_snapshot_get_render_box,
    .snapshot_get_scale = node_snapshot_get_scale,
    .snapshot_render = node_snapshot_render,
    .snapshot_detach = node_snapshot_detach,
    .snapshot_destroy = node_snapshot_destroy,
};

static struct node_snapshot *node_snapshot_try_from_snapshot(const struct snapshot *snapshot)
{
    if (snapshot->impl != &node_snapshot_impl) {
        return NULL;
    }

    struct node_snapshot *node_snapshot = wl_container_of(snapshot, node_snapshot, base);
    return node_snapshot;
}

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
    if (node_snapshot->node->type == SCENE_NODE_BUFFER) {
        node_snapshot->base.was_damaged = true;
        if (node_snapshot->is_zerocopy && node_snapshot->base.wants_update) {
            node_snapshot_set_zerocopy_buffer(node_snapshot,
                                              scene_buffer_from_node(node_snapshot->node));
        }
        return;
    }

    struct scene_node *damage_node = data;
    if (node_snapshot->without_subeffect_node && damage_node != node_snapshot->node &&
        (damage_node->has_effect ||
         scene_node_find_upper_effect_node(damage_node, node_snapshot->node))) {
        return;
    }

    node_snapshot->base.was_damaged = true;
}

static void node_snapshot_handle_node_destroy(struct wl_listener *listener, void *data)
{
    struct node_snapshot *node_snapshot = wl_container_of(listener, node_snapshot, node_destroy);
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

struct scene_buffer *view_try_get_single_buffer(struct view *view)
{
    if (view->base.ssd != KYWC_SSD_NONE || !xwayland_check_view(view)) {
        return NULL;
    }

    if (security_check_node(&view->tree->node)) {
        return NULL;
    }

    return scene_buffer_try_from_surface(view->surface);
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
    wl_list_init(&node_snapshot->base.binders);

    struct scene_buffer *buffer = NULL;
    if (source->type == SNAPSHOT_SOURCE_TYPE_VIEW) {
        node_snapshot->node = &source->view->tree->node;
        buffer = view_try_get_single_buffer(source->view);
    } else if (source->type == SNAPSHOT_SOURCE_TYPE_NODE) {
        node_snapshot->node = source->node;
        buffer =
            source->node->type == SCENE_NODE_BUFFER ? scene_buffer_from_node(source->node) : NULL;
    }
    node_snapshot->source = *source;
    node_snapshot->surface_scale = 1.0f;
    node_snapshot->need_blur = !(options & SNAPSHOT_RENDER_DISABLE_BLUR);
    node_snapshot->without_subeffect_node = !(options & SNAPSHOT_ENABLE_SUBEFFECT_NODE);
    node_snapshot->view_src = (struct kywc_fbox){ 0, 0, 1.0f, 1.0f };
    node_snapshot->buffer_destroy.notify = node_snapshot_handle_buffer_destroy;
    wl_list_init(&node_snapshot->buffer_destroy.link);
    pixman_region32_init(&node_snapshot->blur_info.region);
    node_snapshot_update_blur_info(node_snapshot);

    /**
     * don't care about scale, all rendering results are derived from the original buffer.
     * if options are set, they can be converted into rendering parameters and applied
     * during rendering.
     * however, some thumbnail options (e.g. THUMBNAIL_ENABLE_STYLE_EFFECT) require
     * additional rendering processing, which cannot be achieved by zerocopy.
     */
    uint32_t have_buffer_options = options & SNAPSHOT_ENABLE_STYLE_EFFECT;
    if (buffer && !scene_buffer_is_clipped(buffer) && !have_buffer_options) {
        node_snapshot->node = &buffer->node;
        node_snapshot->is_zerocopy = true;
        /* update texture */
        node_snapshot->base.was_damaged = true;
        node_snapshot->base.wants_update = true;
        /* TODO: set up src_box based on options */
        node_snapshot_set_zerocopy_buffer(node_snapshot, buffer);
        scene_node_get_radius(node_snapshot->node, node_snapshot->radius);
        goto final;
    }

    struct thumbnail *thumbnail = NULL;
    if (source->type == SNAPSHOT_SOURCE_TYPE_VIEW) {
        uint32_t thumbnail_options = 0;
        if (options & SNAPSHOT_DISABLE_DECOR) {
            thumbnail_options |= THUMBNAIL_DISABLE_DECOR;
        }
        if (options & SNAPSHOT_DISABLE_SHADOW) {
            thumbnail_options |= THUMBNAIL_DISABLE_SHADOW;
        }
        if (options & SNAPSHOT_DISABLE_ROUND_CORNER) {
            thumbnail_options |= THUMBNAIL_DISABLE_ROUND_CORNER;
        }
        if (options & SNAPSHOT_ENABLE_STYLE_EFFECT) {
            thumbnail_options |= THUMBNAIL_ENABLE_STYLE_EFFECT;
        }
        thumbnail = thumbnail_create_from_view(source->view, thumbnail_options, scale,
                                               &snapshot_impl, node_snapshot);
    } else {
        uint32_t thumbnail_options = 0;
        if (options & SNAPSHOT_DISABLE_ROUND_CORNER) {
            thumbnail_options |= THUMBNAIL_DISABLE_ROUND_CORNER;
        }
        if (options & SNAPSHOT_ENABLE_STYLE_EFFECT) {
            thumbnail_options |= THUMBNAIL_ENABLE_STYLE_EFFECT;
        }
        if (node_snapshot->without_subeffect_node) {
            thumbnail_options |= THUMBNAIL_WITHOUT_SUBEFFECT_NODE;
        }
        thumbnail = thumbnail_create_from_node(node_snapshot->node, thumbnail_options, scale,
                                               &snapshot_impl, node_snapshot);
    }

    if (!thumbnail || !snapshot_init_thumbnail(&node_snapshot->base, thumbnail)) {
        snapshot_remove_thumbnail(&node_snapshot->base);
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

struct snapshot *snapshot_create_from_node(struct scene_node *node, float scale, uint32_t options)
{
    struct snapshot_source source = {
        .type = SNAPSHOT_SOURCE_TYPE_NODE,
        .node = node,
    };
    return snapshot_create(&source, scale, options);
}

static const char *snapshot_role_desc(const struct scene_node *node, void *data)
{
    static char *snapshot_info[] = { "node snapshot", "node zerocopy snapshot",
                                     "unknown snapshot" };

    struct node_snapshot *node_snapshot = node_snapshot_try_from_snapshot(data);
    if (node_snapshot && node_snapshot->is_zerocopy) {
        return snapshot_info[1];
    } else if (node_snapshot) {
        return snapshot_info[0];
    }

    return snapshot_info[2];
}

static const struct scene_node_role_interface snapshot_role_impl = {
    .desc = snapshot_role_desc,
};

struct scene_buffer *snapshot_create_scene_buffer(struct scene_tree *parent,
                                                  struct snapshot *snapshot)
{
    if (snapshot->impl->snapshot_ensure_buffer_bindable &&
        !snapshot->impl->snapshot_ensure_buffer_bindable(snapshot)) {
        return NULL;
    }

    struct scene_buffer *buffer = scene_buffer_create(parent, snapshot->buffer);
    if (!buffer) {
        return NULL;
    }

    if (!snapshot_binder_create(snapshot, buffer)) {
        scene_node_destroy(&buffer->node);
        return NULL;
    }

    scene_node_set_role(&buffer->node, SCENE_ROLE_SNAPSHOT, &snapshot_role_impl, snapshot);
    return buffer;
}

void snapshot_set_scene_buffer_radius(struct snapshot *snapshot, struct scene_buffer *buffer)
{
    struct node_snapshot *node_snapshot = node_snapshot_try_from_snapshot(snapshot);
    if (!node_snapshot) {
        return;
    }

    struct snapshot_binder *binder;
    wl_list_for_each(binder, &snapshot->binders, link) {
        if (binder->scene_buffer == buffer) {
            scene_node_set_radius(&buffer->node, node_snapshot->radius);
            break;
        }
    }
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

    *dst_box = snapshot->dst_box;
    if (kywc_box_empty(dst_box)) {
        /* allow dst box is empty */
        return;
    }

    if (snapshot->impl->snapshot_get_render_box) {
        snapshot->impl->snapshot_get_render_box(snapshot, dst_box);
    }
}

void snapshot_get_view_box(struct snapshot *snapshot, struct kywc_box *view_box)
{
    struct kywc_box render_box = snapshot->dst_box;
    *view_box = snapshot->dst_box;
    if ((snapshot->anchors == SNAPSHOT_ANCHOR_NONE || snapshot->anchors == SNAPSHOT_ANCHOR_ALL) &&
        (snapshot->options & SNAPSHOT_RENDER_ENABLE_VIEW_BOX)) {
        return;
    }

    struct wlr_buffer *buffer = snapshot->buffer;
    if (buffer) {
        snapshot_get_render_box(snapshot, &render_box);
    }

    if (!buffer || kywc_box_empty(&render_box)) {
        return;
    }

    struct kywc_box full_box = { 0 };
    snapshot_get_full_box(snapshot, &render_box, &full_box);
    view_box_from_full_box(snapshot, &full_box, view_box);
}

void snapshot_mark_wants_update(struct snapshot *snapshot, bool wants)
{
    if (snapshot->wants_update == wants || snapshot->detached) {
        return;
    }

    if (!wants && snapshot->impl->snapshot_ensure_blockable &&
        !snapshot->impl->snapshot_ensure_blockable(snapshot)) {
        return;
    }

    snapshot->wants_update = wants;
    if (!snapshot->thumbnail) {
        return;
    }

    thumbnail_mark_wants_update(snapshot->thumbnail, wants, false);
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
    snapshot_remove_thumbnail(snapshot);
}

void snapshot_add_destroy_listener(struct snapshot *snapshot, struct wl_listener *listener)
{
    wl_signal_add(&snapshot->events.destroy, listener);
}

void snapshot_render(struct snapshot *snapshot, struct scene_render_target *target)
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
