// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "effect/node_transform.h"
#include "effect/shake_view.h"
#include "input/event.h"
#include "render/renderer.h"
#include "scene/render.h"
#include "scene/surface.h"
#include "theme.h"
#include "util/time.h"
#include "view_p.h"

static float modal_color[4] = { 18.0 / 255, 18.0 / 255, 18.0 / 255, 51.0 / 255 };

struct modal_manager {
    struct wl_list modals;

    struct wl_listener new_mapped;
    struct wl_listener server_destroy;

    bool hardware_renderer;
};

struct modal {
    struct wl_list link;
    struct modal_manager *modal_manager;

    struct ky_scene_rect *modal_box;
    struct kywc_box geo;

    struct view *view;
    struct wl_listener view_modal;
    struct wl_listener view_unmap;

    struct wl_listener parent_size;
    struct wl_listener parent_opacity;
    struct wl_listener parent_unmap;
};

static bool modal_hover(struct seat *seat, struct ky_scene_node *node, double x, double y,
                        uint32_t time, bool first, bool hold, void *data)
{
    return false;
}

static void modal_leave(struct seat *seat, struct ky_scene_node *node, bool last, void *data) {}

static void modal_click(struct seat *seat, struct ky_scene_node *node, uint32_t button,
                        bool pressed, uint32_t time, enum click_state state, void *data)
{
    if (!pressed) {
        return;
    }

    struct modal *modal = data;

    struct view *view = view_find_descendant_modal(modal->view);
    view = view ? view : modal->view;
    /* active current view */
    kywc_view_activate(&view->base);
    view_set_focus(view, seat);

    if (modal->modal_manager->hardware_renderer) {
        view_add_shake_effect(view);
    }
}

static struct ky_scene_node *modal_get_root(void *data)
{
    struct modal *modal = data;
    return &modal->modal_box->node;
}

static const struct input_event_node_impl modal_impl = {
    .hover = modal_hover,
    .leave = modal_leave,
    .click = modal_click,
};

static void modal_add_rect_opacity_transform(struct ky_scene_node *node, int duration,
                                             struct kywc_box geo, float start_alpha,
                                             float end_alpha, int64_t start_time,
                                             enum animation_type alpha_type)
{
    struct transform_options options = {
        .start_time = start_time,
        .duration = duration,
        .start = { .geometry = geo, .alpha = start_alpha },
        .end = { .geometry = geo, .alpha = end_alpha },
        .animations = { .alpha = animation_manager_get(alpha_type) },
    };
    node_add_transform_effect(node, &options);
}

static void modal_deactive(struct modal *modal)
{
    wl_list_remove(&modal->parent_size.link);
    wl_list_init(&modal->parent_size.link);
    wl_list_remove(&modal->parent_opacity.link);
    wl_list_init(&modal->parent_opacity.link);
    wl_list_remove(&modal->parent_unmap.link);
    wl_list_init(&modal->parent_unmap.link);

    if (modal->modal_box) {
        modal_add_rect_opacity_transform(&modal->modal_box->node, 200, modal->geo, 1.0, 0,
                                         current_time_msec(), ANIMATION_TYPE_EASE);
        ky_scene_node_destroy(&modal->modal_box->node);
    }
}

static void handle_parent_unmap(struct wl_listener *listener, void *data)
{
    struct modal *modal = wl_container_of(listener, modal, parent_unmap);
    kywc_view_set_modal(&modal->view->base, false);
}

static void modal_box_set_round_corner(struct ky_scene_rect *modal_box, struct view *view);

static void handle_parent_size(struct wl_listener *listener, void *data)
{
    struct modal *modal = wl_container_of(listener, modal, parent_size);
    struct kywc_view *parent = &modal->view->parent->base;
    struct kywc_box geo = {
        .x = -parent->margin.off_x,
        .y = -parent->margin.off_y,
        .width = parent->geometry.width + parent->margin.off_width,
        .height = parent->geometry.height + parent->margin.off_height,
    };

    struct kywc_box geometry = { parent->geometry.x + geo.x, parent->geometry.y + geo.y, geo.width,
                                 geo.height };
    modal->geo = geometry;
    ky_scene_rect_set_size(modal->modal_box, geo.width, geo.height);
    ky_scene_node_set_position(&modal->modal_box->node, geo.x, geo.y);
    modal_box_set_round_corner(modal->modal_box, modal->view->parent);
}

static void modal_box_set_round_corner(struct ky_scene_rect *modal_box, struct view *view)
{
    struct ky_scene_buffer *buffer = ky_scene_buffer_try_from_surface(view->surface);
    if (!buffer) {
        return;
    }

    int radius[4] = { 0 };
    memcpy(radius, buffer->node.radius, sizeof(radius));

    /* set top corner if has ssd title */
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->ssd & KYWC_SSD_TITLE) {
        bool need_corner = !kywc_view->maximized && !kywc_view->fullscreen && !kywc_view->tiled;
        struct theme *theme = theme_manager_get_theme();
        radius[KY_SCENE_ROUND_CORNER_RT] = need_corner ? theme->window_radius : 0;
        radius[KY_SCENE_ROUND_CORNER_LT] = need_corner ? theme->window_radius : 0;
    }

    ky_scene_node_set_radius(&modal_box->node, radius);
}

static void modal_box_render(struct ky_scene_node *node, int lx, int ly,
                             struct ky_scene_render_target *target)
{
    /* When the modal disappears, obtain the rect thumbnail and render it */
    if (!node->enabled || (!node->has_effect && target->options & KY_SCENE_RENDER_DISABLE_EFFECT)) {
        return;
    }

    struct ky_scene_rect *rect = ky_scene_rect_from_node(node);
    if (rect->color[3] == 0) {
        return;
    }

    bool render_with_visibility = !(target->options & KY_SCENE_RENDER_DISABLE_VISIBILITY);
    if (render_with_visibility && !pixman_region32_not_empty(&node->visible_region) &&
        !pixman_region32_not_empty(&node->extend_render_region)) {
        return;
    }

    ky_scene_rect_render(node, (struct kywc_box){ lx, ly, rect->width, rect->height }, rect->color,
                         render_with_visibility, target);
}

static void modal_active(struct modal *modal)
{
    if (!modal->view->parent || !modal->view->parent->surface) {
        return;
    }

    struct kywc_view *parent = &modal->view->parent->base;
    struct kywc_box geo = {
        .x = -parent->margin.off_x,
        .y = -parent->margin.off_y,
        .width = parent->geometry.width + parent->margin.off_width,
        .height = parent->geometry.height + parent->margin.off_height,
    };

    float opacity = parent->opacity;
    float color[4] = { modal_color[0] * opacity, modal_color[1] * opacity, modal_color[2] * opacity,
                       modal_color[3] * opacity };
    modal->modal_box =
        ky_scene_rect_create(modal->view->parent->tree, geo.width, geo.height, color);
    if (!modal->modal_box) {
        return;
    }

    modal->modal_box->node.impl.render = modal_box_render;
    ky_scene_node_raise_to_top(&modal->modal_box->node);
    ky_scene_node_set_position(&modal->modal_box->node, geo.x, geo.y);
    modal_box_set_round_corner(modal->modal_box, modal->view->parent);

    struct kywc_box geometry = { parent->geometry.x + geo.x, parent->geometry.y + geo.y, geo.width,
                                 geo.height };
    modal->geo = geometry;
    modal_add_rect_opacity_transform(&modal->modal_box->node, 300, modal->geo, 0, 1.0,
                                     current_time_msec(), ANIMATION_TYPE_EASE);

    input_event_node_create(&modal->modal_box->node, &modal_impl, modal_get_root, NULL, modal);

    view_add_parent_workspace(modal->view);

    wl_signal_add(&parent->events.opacity, &modal->parent_opacity);
    wl_signal_add(&parent->events.unmap, &modal->parent_unmap);
    wl_signal_add(&parent->events.size, &modal->parent_size);
}

static void handle_modal_view_unmap(struct wl_listener *listener, void *data)
{
    struct modal *modal = wl_container_of(listener, modal, view_unmap);
    if (modal->view->base.modal) {
        kywc_view_set_modal(&modal->view->base, false);
    }

    wl_list_remove(&modal->view_modal.link);
    wl_list_remove(&modal->view_unmap.link);
    wl_list_remove(&modal->parent_size.link);
    wl_list_remove(&modal->parent_opacity.link);
    wl_list_remove(&modal->parent_unmap.link);
    wl_list_remove(&modal->link);

    free(modal);
}

static void handle_view_modal(struct wl_listener *listener, void *data)
{
    struct modal *modal = wl_container_of(listener, modal, view_modal);
    if (modal->view->base.modal) {
        modal_active(modal);
    } else {
        modal_deactive(modal);
    }
}

static void handle_parent_opacity(struct wl_listener *listener, void *data)
{
    struct modal *modal = wl_container_of(listener, modal, parent_opacity);
    float opacity = modal->view->parent->base.opacity;
    float color[4] = { modal_color[0] * opacity, modal_color[1] * opacity, modal_color[2] * opacity,
                       modal_color[3] * opacity };
    ky_scene_rect_set_color(modal->modal_box, color);
}

static void handle_new_mapped_view(struct wl_listener *listener, void *data)
{
    struct modal_manager *modal_manager = wl_container_of(listener, modal_manager, new_mapped);
    struct kywc_view *kywc_view = data;

    struct modal *modal = calloc(1, sizeof(*modal));
    if (!modal) {
        return;
    }

    modal->modal_manager = modal_manager;
    wl_list_insert(&modal_manager->modals, &modal->link);

    modal->view_modal.notify = handle_view_modal;
    wl_signal_add(&kywc_view->events.modal, &modal->view_modal);
    modal->view_unmap.notify = handle_modal_view_unmap;
    wl_signal_add(&kywc_view->events.unmap, &modal->view_unmap);

    modal->parent_size.notify = handle_parent_size;
    wl_list_init(&modal->parent_size.link);
    modal->parent_opacity.notify = handle_parent_opacity;
    wl_list_init(&modal->parent_opacity.link);
    modal->parent_unmap.notify = handle_parent_unmap;
    wl_list_init(&modal->parent_unmap.link);

    modal->view = view_from_kywc_view(kywc_view);
    if (kywc_view->modal) {
        modal_active(modal);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct modal_manager *modal_manager = wl_container_of(listener, modal_manager, server_destroy);
    wl_list_remove(&modal_manager->server_destroy.link);
    wl_list_remove(&modal_manager->new_mapped.link);
    free(modal_manager);
}

bool modal_manager_create(struct server *server)
{
    struct modal_manager *modal_manager = calloc(1, sizeof(*modal_manager));
    if (!modal_manager) {
        return false;
    }

    wl_list_init(&modal_manager->modals);
    modal_manager->hardware_renderer = !ky_renderer_is_software(server->renderer);

    modal_manager->new_mapped.notify = handle_new_mapped_view;
    kywc_view_add_new_mapped_listener(&modal_manager->new_mapped);
    modal_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &modal_manager->server_destroy);

    return true;
}
