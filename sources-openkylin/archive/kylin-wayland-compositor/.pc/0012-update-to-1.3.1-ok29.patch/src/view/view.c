// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_foreign_registry.h>
#include <wlr/types/wlr_xdg_foreign_v1.h>
#include <wlr/types/wlr_xdg_foreign_v2.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "effect/action.h"
#include "effect/magic_lamp.h"
#include "effect/scale.h"
#include "effect/slide.h"
#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "scene/surface.h"
#include "server.h"
#include "theme.h"
#include "util/macros.h"
#include "view_p.h"
#include "xwayland/xwayland.h"

static struct view_manager *view_manager = NULL;

struct view *view_manager_get_activated(void)
{
    return view_manager->activated.view;
}

void view_manager_add_activate_view_listener(struct wl_listener *listener)
{
    wl_signal_add(&view_manager->events.activate_view, listener);
}

void view_manager_show_tile_assist(struct view *view, struct seat *seat,
                                   struct kywc_output *kywc_output)
{
    if (view_manager->impl.show_tile_assist) {
        view_manager->impl.show_tile_assist(view, seat, kywc_output);
    }
}

void view_manager_for_each_stack_view(view_iterator_func_t iterator, struct view *view, bool above,
                                      void *data)
{
    int start_layer = above ? (LAYER_UNKNOWN + 1) : (LAYER_NUMBER - 1);
    if (view) {
        if (wl_list_empty(&view->layer_link)) {
            kywc_log(KYWC_ERROR, "view was not insert layer!");
            return;
        }
        struct wl_list *start_link = above ? view->layer_link.next : view->layer_link.prev;
        struct wl_list *end_link = &view->view_layer->views;

        struct wl_list *link = start_link;
        while (link != end_link) {
            struct view *tmp = wl_container_of(link, tmp, layer_link);
            if (iterator(&tmp->base, data)) {
                return;
            }
            link = above ? link->next : link->prev;
        }

        start_layer = above ? (view->view_layer->layer + 1) : (view->view_layer->layer - 1);
    }

    int end_layer = above ? LAYER_NUMBER : LAYER_UNKNOWN;
    int direction = above ? 1 : -1;
    for (int layer = start_layer; layer != end_layer; layer += direction) {
        struct view_layer *view_layer;
        if (layer == LAYER_BELOW || layer == LAYER_ABOVE || layer == LAYER_NORMAL) {
            view_layer = view_manager_get_layer(layer, true);
        } else {
            view_layer = view_manager_get_layer(layer, false);
        }

        if (above) {
            wl_list_for_each(view, &view_layer->views, layer_link) {
                if (iterator(&view->base, data)) {
                    return;
                }
            }
        } else {
            wl_list_for_each_reverse(view, &view_layer->views, layer_link) {
                if (iterator(&view->base, data)) {
                    return;
                }
            }
        }
    }
}

struct view_layer *view_manager_get_layer(enum layer layer, bool in_workspace)
{
    if (!in_workspace) {
        return &view_manager->layers[layer];
    }

    switch (layer) {
    case LAYER_BELOW:
    case LAYER_NORMAL:
    case LAYER_ABOVE:
        return workspace_layer(workspace_manager_get_current(), layer);
    default:
        return &view_manager->layers[layer];
    }
}

struct view_layer *view_manager_get_layer_by_node(struct scene_node *node, bool in_workspace)
{
    struct scene_tree *tree = node->parent;

    while (tree && (!in_workspace || tree->node.role.type != SCENE_ROLE_WORKSPACE) &&
           tree->node.role.type != SCENE_ROLE_LAYER) {
        tree = tree->node.parent;
    }

    return tree->node.role.data;
}

static void view_insert_layer(struct view *view, struct view_layer *view_layer, bool top)
{
    view->view_layer = view_layer;
    if (!view->base.mapped) {
        return;
    }

    /* the view was already there */
    if (!wl_list_empty(&view_layer->views)) {
        if (top && (wl_container_of(view_layer->views.prev, view, layer_link) == view)) {
            return;
        }
        if (!top && (wl_container_of(view_layer->views.next, view, layer_link) == view)) {
            return;
        }
    }

    wl_list_remove(&view->layer_link);
    struct wl_list *list = top ? view_layer->views.prev : &view_layer->views;
    wl_list_insert(list, &view->layer_link);
    wl_signal_emit_mutable(&view->events.stack, NULL);
}

static void view_move_to_layer(struct view *view, struct view_layer *view_layer, bool top)
{
    if (view->view_layer == view_layer) {
        if (view->current_proxy) {
            struct view_proxy *view_proxy;
            wl_list_for_each(view_proxy, &view->view_proxies, view_link) {
                wl_list_remove(&view_proxy->workspace_link);
                if (top) {
                    wl_list_insert(&view_proxy->workspace->view_proxies,
                                   &view_proxy->workspace_link);
                    scene_node_raise_to_top(&view_proxy->tree->node);
                } else {
                    wl_list_insert(view_proxy->workspace->view_proxies.prev,
                                   &view_proxy->workspace_link);
                    scene_node_lower_to_bottom(&view_proxy->tree->node);
                }
            }
        } else {
            if (top) {
                scene_node_raise_to_top(&view->tree->node);
            } else {
                scene_node_lower_to_bottom(&view->tree->node);
            }
        }

        view_insert_layer(view, view_layer, top);
        return;
    }

    bool target_in_workspace = view_layer->layer == LAYER_BELOW ||
                               view_layer->layer == LAYER_NORMAL ||
                               view_layer->layer == LAYER_ABOVE;
    if (view->current_proxy && (view->tree->node.parent != view->current_proxy->tree)) {
        /* move in workspace (fullscreen restore) */
        scene_node_reparent(&view->tree->node, view->current_proxy->tree);
    } else if (view->current_proxy && target_in_workspace) {
        /* move workspace view to workspace layer */
        struct view_proxy *proxy;
        wl_list_for_each(proxy, &view->view_proxies, view_link) {
            struct view_layer *layer = workspace_layer(proxy->workspace, view_layer->layer);
            scene_node_reparent(&proxy->tree->node, layer->tree);
            if (top) {
                scene_node_raise_to_top(&proxy->tree->node);
            } else {
                scene_node_lower_to_bottom(&proxy->tree->node);
            }
        }
    } else {
        /* move view out of workspace */
        scene_node_reparent(&view->tree->node, view_layer->tree);
        if (top) {
            scene_node_raise_to_top(&view->tree->node);
        } else {
            scene_node_lower_to_bottom(&view->tree->node);
        }
    }

    view_insert_layer(view, view_layer, top);
}

static void view_update_output(struct view *view, struct kywc_output *output)
{
    /* use fallback output if no valid output */
    if (!output) {
        output = output_manager_get_fallback();
    }

    if (view->output == output) {
        return;
    }

    view->output = output;
    wl_list_remove(&view->output_destroy.link);
    wl_signal_add(&output->events.destroy, &view->output_destroy);
    wl_signal_emit_mutable(&view->events.output, NULL);
}

static void view_handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct view *view = wl_container_of(listener, view, output_destroy);
    view_update_output(view, NULL);
}

static void view_handle_outputs_update(struct wl_listener *listener, void *data)
{
    struct view *view = wl_container_of(listener, view, outputs_update);
    struct scene_outputs_update_event *event = data;
    /* skip update if the primary is empty, will clear when output destroy */
    if (!event->primary) {
        return;
    }
    view_update_output(view, &output_from_wlr_output(event->primary->output)->base);
}

void view_fix_geometry(struct view *view, struct kywc_box *geo, struct kywc_box *src_box,
                       struct kywc_box *dst_box)
{
    struct kywc_view *kywc_view = &view->base;
    /* actual view geometry with margin */
    int x = geo->x - kywc_view->margin.off_x;
    int y = geo->y - kywc_view->margin.off_y;
    int w = geo->width + kywc_view->margin.off_width;
    int h = geo->height + kywc_view->margin.off_height;

    if (w > dst_box->width) {
        geo->x = dst_box->x + kywc_view->margin.off_x;
        geo->width = dst_box->width - kywc_view->margin.off_width;
    } else if (src_box->x == x) {
        geo->x = dst_box->x + kywc_view->margin.off_x;
    } else if (src_box->x + src_box->width == x + w) {
        geo->x = dst_box->x + dst_box->width - w + kywc_view->margin.off_x;
    } else {
        double frac_x = (double)dst_box->width / src_box->width;
        geo->x = round((x - src_box->x) * frac_x) + dst_box->x + kywc_view->margin.off_x;
        geo->x = CLAMP(geo->x, dst_box->x - geo->width + VIEW_LEFT_GAP + kywc_view->margin.off_x,
                       dst_box->x + dst_box->width - VIEW_RIGHT_GAP + kywc_view->margin.off_x);
    }

    if (h > dst_box->height) {
        geo->y = dst_box->y + kywc_view->margin.off_y;
        geo->height = dst_box->height - kywc_view->margin.off_height;
    } else if (src_box->y == y) {
        geo->y = dst_box->y + kywc_view->margin.off_y;
    } else if (src_box->y + src_box->height == y + h) {
        geo->y = dst_box->y + dst_box->height - h + kywc_view->margin.off_y;
    } else {
        double frac_y = (double)dst_box->height / src_box->height;
        geo->y = round((y - src_box->y) * frac_y) + dst_box->y + kywc_view->margin.off_y;
        geo->y = CLAMP(geo->y, dst_box->y + kywc_view->margin.off_y,
                       dst_box->y + dst_box->height - VIEW_BOTTOM_GAP + kywc_view->margin.off_y);
    }
}

static void view_fix_tiled_geometry(struct view *view, struct kywc_box *geo,
                                    struct kywc_output *kywc_output)
{
    if (view_manager->impl.fix_geometry) {
        view_manager->impl.fix_geometry(view, geo, kywc_output);
    }
}

void view_move_to_output(struct view *view, struct kywc_box *src_box, struct kywc_box *preferred,
                         struct kywc_output *kywc_output)
{
    struct kywc_view *kywc_view = &view->base;
    struct output *dst = output_from_kywc_output(kywc_output);
    struct kywc_box *dst_box = &dst->usable_area;

    /* default to view output usable area */
    if (src_box == NULL) {
        src_box = &output_from_kywc_output(view->output)->usable_area;
    }

    if (kywc_view->fullscreen) {
        view_fix_geometry(view, &view->saved.geometry, src_box, dst_box);
        view_do_resize(view, &dst->geometry);
        return;
    }

    if (kywc_view->maximized) {
        struct kywc_box geo;
        view_get_tiled_geometry(view, &geo, kywc_output, KYWC_TILE_ALL);
        view_fix_geometry(view, &view->saved.geometry, src_box, dst_box);
        view_do_resize(view, &geo);
        return;
    }

    if (kywc_view->tiled) {
        struct kywc_box geo;
        view_fix_tiled_geometry(view, &geo, kywc_output);
        view_fix_geometry(view, &view->saved.geometry, src_box, dst_box);
        view_do_resize(view, &geo);
        return;
    }

    if (!KYWC_VIEW_IS_MOVABLE(kywc_view)) {
        return;
    }

    struct kywc_box geo = view_action_change_size(view->pending.configure_action)
                              ? view->pending.configure_geometry
                              : kywc_view->geometry;
    if (kywc_box_not_empty(preferred)) {
        geo.width = preferred->width;
        geo.height = preferred->height;
    }

    view_fix_geometry(view, &geo, src_box, dst_box);
    /* resize to dst */
    if (KYWC_VIEW_IS_RESIZABLE(kywc_view)) {
        view_do_resize(view, &geo);
    } else {
        view_do_move(view, geo.x, geo.y);
    }
}

void sub_view_move_to_output(struct view *view, struct kywc_output *kywc_output)
{
    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        sub_view_move_to_output(child, kywc_output);
        view_move_to_output(child, NULL, NULL, kywc_output);
    }
}

static void view_handle_update_capabilities(struct wl_listener *listener, void *data);

static void view_handle_panel_surface_destroy(struct wl_listener *listener, void *data);

static const char *view_handle_desc(const struct scene_node *node, void *data)
{
    struct view *view = data;
    return view->current_proxy ? view->base.title : view->base.app_id;
}

static const struct scene_node_role_interface view_role_impl = {
    .desc = view_handle_desc,
};

void view_init(struct view *view, const struct view_impl *impl, void *data)
{
    struct kywc_view *kywc_view = &view->base;

    wl_signal_init(&kywc_view->events.premap);
    wl_signal_init(&kywc_view->events.map);
    wl_signal_init(&kywc_view->events.unmap);
    wl_signal_init(&kywc_view->events.destroy);
    wl_signal_init(&kywc_view->events.activate);
    wl_signal_init(&kywc_view->events.maximize);
    wl_signal_init(&kywc_view->events.minimize);
    wl_signal_init(&kywc_view->events.fullscreen);
    wl_signal_init(&kywc_view->events.tile);
    wl_signal_init(&kywc_view->events.above);
    wl_signal_init(&kywc_view->events.below);
    wl_signal_init(&kywc_view->events.sticky);
    wl_signal_init(&kywc_view->events.skip_taskbar);
    wl_signal_init(&kywc_view->events.skip_switcher);
    wl_signal_init(&kywc_view->events.demands_attention);
    wl_signal_init(&kywc_view->events.modal);
    wl_signal_init(&kywc_view->events.capabilities);
    wl_signal_init(&kywc_view->events.title);
    wl_signal_init(&kywc_view->events.app_id);
    wl_signal_init(&kywc_view->events.position);
    wl_signal_init(&kywc_view->events.size);
    wl_signal_init(&kywc_view->events.decoration);
    wl_signal_init(&kywc_view->events.opacity);

    wl_list_init(&view->children);
    wl_list_init(&view->parent_link);
    wl_list_init(&view->view_proxies);
    wl_list_init(&view->layer_link);
    wl_signal_init(&view->events.parent);
    wl_signal_init(&view->events.workspace);
    wl_signal_init(&view->events.workspace_enter);
    wl_signal_init(&view->events.workspace_leave);
    wl_signal_init(&view->events.output);
    wl_signal_init(&view->events.icon_update);
    wl_signal_init(&view->events.position);
    wl_signal_init(&view->events.update_capabilities);
    wl_signal_init(&view->events.stack);

    view->impl = impl;
    view->data = data;
    kywc_view->uuid = kywc_identifier_uuid_generate();
    kywc_view->opacity = 1.0;
    view->focused_seat = input_manager_get_default_seat();
    wl_list_insert(&view_manager->views, &view->link);

    /* create view tree and disable it */
    struct view_layer *layer = view_manager_get_layer(LAYER_NORMAL, true);
    view->tree = scene_tree_create(layer->tree);
    scene_node_set_role(&view->tree->node, SCENE_ROLE_VIEW, &view_role_impl, view);
    scene_node_set_enabled(&view->tree->node, false);

    struct output *output = input_current_output(input_manager_get_default_seat());
    view->output = output ? &output->base : output_manager_get_fallback();
    view->output_destroy.notify = view_handle_output_destroy;
    wl_signal_add(&view->output->events.destroy, &view->output_destroy);
    view->outputs_update.notify = view_handle_outputs_update;
    wl_list_init(&view->outputs_update.link);

    /* initialize minimized_geometry */
    view->minimized_geometry.panel_surface_destroy.notify = view_handle_panel_surface_destroy;
    wl_list_init(&view->minimized_geometry.panel_surface_destroy.link);

    kywc_view->role = KYWC_VIEW_ROLE_UNDEF;
    view_set_role(view, KYWC_VIEW_ROLE_NORMAL);
    view->update_capabilities.notify = view_handle_update_capabilities;
    view_add_update_capabilities_listener(view, &view->update_capabilities);
    wl_signal_emit_mutable(&view_manager->events.new_view, kywc_view);
}

void view_get_tiled_geometry(struct view *view, struct kywc_box *geometry,
                             struct kywc_output *kywc_output, enum kywc_tile tile)
{
    if (!tile || tile > KYWC_TILE_BOTTOM_RIGHT) {
        struct output *output = output_from_kywc_output(kywc_output);
        struct kywc_view *kywc_view = &view->base;
        int32_t min_width = kywc_view->min_width + kywc_view->margin.off_width;
        int32_t min_height = kywc_view->min_height + kywc_view->margin.off_height;
        struct kywc_box *usable = &output->usable_area;
        int32_t half_width = usable->width / 2;
        int32_t half_height = usable->height / 2;
        bool use_min_width = half_width < min_width;
        bool use_min_height = half_height < min_height;

        if (tile == KYWC_TILE_NONE) {
            *geometry = view->saved.geometry;
        } else if (tile == KYWC_TILE_CENTER) {
            geometry->x = use_min_width
                              ? usable->x + half_width - min_width / 2 + kywc_view->margin.off_x
                              : usable->x + usable->width / 4 + kywc_view->margin.off_x;
            geometry->y = use_min_height
                              ? usable->y + half_height - min_height / 2 + kywc_view->margin.off_y
                              : usable->y + usable->height / 4 + kywc_view->margin.off_y;
            geometry->width = use_min_width ? min_width - kywc_view->margin.off_width
                                            : half_width - kywc_view->margin.off_width;
            geometry->height = use_min_height ? min_height - kywc_view->margin.off_height
                                              : half_height - kywc_view->margin.off_height;
        } else if (tile == KYWC_TILE_ALL) {
            geometry->x = usable->x + kywc_view->margin.off_x;
            geometry->y = usable->y + kywc_view->margin.off_y;
            geometry->width = usable->width - kywc_view->margin.off_width;
            geometry->height = usable->height - kywc_view->margin.off_height;
        }
        return;
    }

    if (view_manager->impl.get_tiled_geometry) {
        view_manager->impl.get_tiled_geometry(view, geometry, kywc_output, tile);
        return;
    }
}

struct wlr_buffer *view_get_icon_buffer(struct view *view, float scale)
{
    struct theme *theme = theme_manager_get_theme();
    struct wlr_buffer *buf = NULL;
    struct wlr_buffer *theme_buf = theme_icon_get_buffer(view->icon, theme->icon_size, scale);
    if (view->icon_name) {
        buf = theme_buf;
    }

    if (!buf && view->impl->get_icon_buffer) {
        buf = view->impl->get_icon_buffer(view, theme->icon_size, scale);
    }

    if (!buf) {
        buf = theme_buf;
    }
    return buf;
}

/* set surface round corner by ssd type, tiled, fullscreen and maximized state */
void view_update_round_corner(struct view *view)
{
    struct kywc_view *kywc_view = &view->base;
    if (!kywc_view->mapped) {
        return;
    }

    struct scene_buffer *buffer = scene_buffer_try_from_surface(view->surface);
    if (!buffer) {
        return;
    }

    int radius[4] = { 0 };
    if (!kywc_view->has_round_corner) {
        scene_node_set_radius(&buffer->node, radius);
        return;
    }

    /* don't draw top round corner if ssd has title */
    struct theme *theme = theme_manager_get_theme();
    bool need_corner = !kywc_view->maximized && !kywc_view->fullscreen && !kywc_view->tiled;
    if (!view_manager->state.csd_round_corner) {
        need_corner &= kywc_view->ssd != KYWC_SSD_NONE;
    }
    bool need_top_corner = need_corner && !(kywc_view->ssd & KYWC_SSD_TITLE);

    radius[SCENE_ROUND_CORNER_RB] = need_corner ? theme->window_radius : 0;
    radius[SCENE_ROUND_CORNER_RT] = need_top_corner ? theme->window_radius : 0;
    radius[SCENE_ROUND_CORNER_LB] = need_corner ? theme->window_radius : 0;
    radius[SCENE_ROUND_CORNER_LT] = need_top_corner ? theme->window_radius : 0;
    scene_node_set_radius(&buffer->node, radius);
}

void view_set_icon(struct view *view, bool buffer_changed)
{
    bool use_icon_name = false;
    bool use_icon_buffer = false;
    struct icon *old = view->icon;

    if (view->icon_name) {
        view->icon = theme_icon_from_app_id(view->icon_name);
        use_icon_name = !theme_icon_is_fallback(view->icon);
    }

    if (!use_icon_name && buffer_changed) {
        struct theme *theme = theme_manager_get_theme();
        struct wlr_buffer *buf =
            view->impl->get_icon_buffer(view, theme->icon_size, view->output->state.scale);
        /* set icon fallback if has any icon buffer */
        if (buf) {
            view->icon = theme_icon_from_app_id(NULL);
            use_icon_buffer = true;
        }
    }

    if (!use_icon_name && !use_icon_buffer) {
        view->icon = theme_icon_from_app_id(view->base.app_id);
        if (theme_icon_is_fallback(view->icon)) {
            view->icon = theme_icon_from_app_id(view->base.exec);
        }
    }

    if (use_icon_buffer || old != view->icon) {
        wl_signal_emit_mutable(&view->events.icon_update, NULL);
    }
}

static void view_end_keyboard_grab(struct view *view)
{
    struct view *descendant = view_find_descendant_modal(view);
    if (descendant) {
        view = descendant;
    }

    if (!KYWC_VIEW_IS_FOCUSABLE(&view->base)) {
        return;
    }

    struct seat *seat = view->focused_seat;
    if (!seat_is_dragging(seat)) {
        wlr_seat_keyboard_end_grab(seat->wlr_seat);
    }
}

static void action_effect_options_adjust(enum action_effect_options_step step,
                                         enum effect_action action,
                                         struct action_effect_options *options, void *user_data)
{
    switch (step) {
    case ACTION_EFFECT_OPTIONS_ADD:
        if (options->effect_type == ACTION_EFFECT_FADE) {
            struct view *view = user_data;
            options->surface = view->surface;
            if (action == EFFECT_ACTION_MAP) {
                options->width_scale = 0.9f;
                options->height_scale = 0.9f;
                options->duration = 300;
            } else if (action == EFFECT_ACTION_UNMAP) {
                options->width_scale = 0.8f;
                options->height_scale = 0.8f;
                options->duration = 260;
            }
            options->scale = view->surface ? scene_surface_get_scale(options->surface) : 1.0f;
        }
        break;
    case ACTION_EFFECT_OPTIONS_SURFACE:
    case ACTION_EFFECT_OPTIONS_CONFIRM:
        break;
    }
}

void view_map(struct view *view)
{
    struct kywc_view *kywc_view = &view->base;

    wl_signal_emit_mutable(&kywc_view->events.premap, NULL);

    view_set_icon(view, false);

    if (view_manager->mode->impl->view_map) {
        view_manager->mode->impl->view_map(view);
    }
    /* assume that request_minimize may emitted before map */
    scene_node_set_enabled(&view->tree->node, !kywc_view->minimized);

    kywc_view->mapped = true;

    if (view->pending.action) {
        /* add a fallback geometry */
        if (kywc_view->maximized || kywc_view->fullscreen) {
            view_get_tiled_geometry(view, &view->saved.geometry, view->output, KYWC_TILE_CENTER);
        }
        if (view->impl->configure) {
            view->impl->configure(view);
        }
    }

    struct scene_buffer *buffer = scene_buffer_try_from_surface(view->surface);
    if (buffer->primary_output) {
        view_update_output(view, &output_from_wlr_output(buffer->primary_output->output)->base);
    }
    wl_signal_add(&buffer->events.outputs_update, &view->outputs_update);

    kywc_view->has_initial_position = false;
    kywc_view_activate(kywc_view);
    view_end_keyboard_grab(view);
    view_set_focus(view, view->focused_seat);

    view_update_round_corner(view);

    if (view->parent && view->parent->base.mapped) {
        kywc_view->skip_taskbar = true;
        kywc_view->skip_switcher = true;
    }

    kywc_log(KYWC_DEBUG, "Kywc_view %p map", kywc_view);

    if (!view_add_slide_effect(view, true) && kywc_view->role == KYWC_VIEW_ROLE_NORMAL) {
        if (view_manager->impl.get_startup_geometry) {
            view_manager->impl.get_startup_geometry(view, view_manager);
            kywc_log(KYWC_DEBUG, "%s startup_geometry is %d, %d, %d, %d", kywc_view->app_id,
                     view->startup_geometry.x, view->startup_geometry.y,
                     view->startup_geometry.width, view->startup_geometry.height);
        }
        if (kywc_box_not_empty(&view->startup_geometry)) {
            view_add_scale_effect(view, SCALE_MINIMIZE);
        } else {
            view_add_action_effect(view, EFFECT_ACTION_MAP, ACTION_EFFECT_FADE,
                                   action_effect_options_adjust, view);
        }
    }
    /* clear startup geometry */
    view->startup_geometry = (struct kywc_box){ 0 };

    if (view->base.modal) {
        view_update_descendant_capabilities(
            view_find_ancestor(view), KYWC_VIEW_MINIMIZABLE | KYWC_VIEW_MAXIMIZABLE |
                                          KYWC_VIEW_CLOSEABLE | KYWC_VIEW_FULLSCREENABLE |
                                          KYWC_VIEW_MOVABLE | KYWC_VIEW_RESIZABLE |
                                          KYWC_VIEW_MINIMIZE_BUTTON | KYWC_VIEW_MAXIMIZE_BUTTON);
    }

    wl_signal_emit_mutable(&kywc_view->events.map, NULL);
    wl_signal_emit_mutable(&view_manager->events.new_mapped_view, kywc_view);

    struct view_proxy *proxy;
    wl_list_for_each(proxy, &view->view_proxies, view_link) {
        wl_signal_emit_mutable(&proxy->workspace->events.view_enter, view);
    }

    if (view->current_proxy && !kywc_view->minimized) {
        if (view_manager->show_desktop_enabled) {
            view_manager_show_desktop(false, false);
        }
        if (view_manager->show_activated_only_enabled) {
            view_manager_show_activated_only(false, false);
        }
    }

    cursor_rebase_all(false);
}

static void view_remove_relationship(struct view *view)
{
    wl_list_remove(&view->parent_link);
    wl_list_init(&view->parent_link);
    if (view->parent) {
        view_update_capabilities(view->parent, KYWC_VIEW_MINIMIZABLE | KYWC_VIEW_MAXIMIZABLE |
                                                   KYWC_VIEW_FULLSCREENABLE | KYWC_VIEW_MOVABLE |
                                                   KYWC_VIEW_RESIZABLE);
        view->parent = NULL;
    }

    struct view *child, *tmp;
    wl_list_for_each_safe(child, tmp, &view->children, parent_link) {
        wl_list_remove(&child->parent_link);
        wl_list_init(&child->parent_link);
        child->parent = NULL;
        view_update_capabilities(child, KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_FULLSCREENABLE);
    }
}

void view_unmap(struct view *view)
{
    struct kywc_view *kywc_view = &view->base;
    kywc_view->mapped = false;

    kywc_log(KYWC_DEBUG, "Kywc_view %p unmap", kywc_view);

    if (!kywc_view->minimized && !view_add_slide_effect(view, false) &&
        kywc_view->role == KYWC_VIEW_ROLE_NORMAL) {
        view_add_action_effect(view, EFFECT_ACTION_UNMAP, ACTION_EFFECT_FADE,
                               action_effect_options_adjust, view);
    }
    if (view == view_manager_get_global_authentication()) {
        view_manager_set_global_authentication(NULL);
    } else if (view == view_manager->desktop) {
        view_manager->desktop = NULL;
    }

    wl_signal_emit_mutable(&kywc_view->events.unmap, NULL);

    if (view_manager->mode->impl->view_unmap) {
        view_manager->mode->impl->view_unmap(view);
    }

    struct view_proxy *proxy;
    wl_list_for_each(proxy, &view->view_proxies, view_link) {
        wl_signal_emit_mutable(&proxy->workspace->events.view_leave, view);
    }

    kywc_view->title = kywc_view->app_id = NULL;
    wl_list_remove(&view->outputs_update.link);
    wl_list_init(&view->outputs_update.link);
    scene_node_set_enabled(&view->tree->node, false);

    if (view->pending.configure_timeout) {
        wl_event_source_remove(view->pending.configure_timeout);
        view->pending.configure_timeout = NULL;
    }

    wl_list_remove(&view->layer_link);
    wl_list_init(&view->layer_link);

    view_remove_relationship(view);

    cursor_rebase_all(false);
}

static int view_handle_configure_timeout(void *data)
{
    struct view *view = data;
    kywc_log(KYWC_INFO, "Client (%s) did not respond to configure request", view->base.app_id);

    if (view->impl->configure_timeout) {
        view->impl->configure_timeout(view);
    }

    /* fallback for pending actions */
    if (view_action_change_size(view->pending.configure_action)) {
        struct kywc_box *pending = &view->pending.configure_geometry;
        int x = pending->x, y = pending->y;
        /* fix wobbling when user resize */
        if (view->pending.configure_action & VIEW_ACTION_RESIZE) {
            struct kywc_box *current = &view->base.geometry;
            uint32_t resize_edges = view->current_resize_edges;
            if (resize_edges & KYWC_EDGE_LEFT) {
                x += pending->width - current->width;
            }
            if (resize_edges & KYWC_EDGE_TOP) {
                y += pending->height - current->height;
            }
        }
        view_helper_move(view, x, y);
    }

    view_configured(view, true);

    return 0;
}

void view_configure(struct view *view, uint32_t serial)
{
    view->pending.configure_serial = serial;
    view->pending.configure_action |= view->pending.action;
    view->pending.configure_geometry = view->pending.geometry;

    view->pending.action = VIEW_ACTION_NOP;
    view->pending.geometry = (struct kywc_box){ 0 };
    view->pending.geometry = view->base.geometry;

    if (serial == 0) {
        return;
    }

    if (!view->pending.configure_timeout) {
        view->pending.configure_timeout = wl_event_loop_add_timer(
            view_manager->server->event_loop, view_handle_configure_timeout, view);
    }

    uint32_t timeout = view_manager_get_configure_timeout(view);
    wl_event_source_timer_update(view->pending.configure_timeout, timeout);
}

void view_configured(struct view *view, bool reset)
{
    struct kywc_view *kywc_view = &view->base;

    if (view->pending.configure_timeout && reset) {
        wl_event_source_remove(view->pending.configure_timeout);
        view->pending.configure_timeout = NULL;
    }

    if (view->pending.configure_action == VIEW_ACTION_NOP) {
        return;
    }

    if (view->pending.configure_action & VIEW_ACTION_FULLSCREEN) {
        view_add_scale_effect(view, SCALE_FULLSCREEN);
        wl_signal_emit_mutable(&kywc_view->events.fullscreen, NULL);
    }

    if (view->pending.configure_action & VIEW_ACTION_MAXIMIZE) {
        view_add_scale_effect(view, SCALE_MAXIMIZE);
        wl_signal_emit_mutable(&kywc_view->events.maximize, NULL);
    }

    if (view->pending.configure_action & VIEW_ACTION_TILE) {
        wl_signal_emit_mutable(&kywc_view->events.tile, NULL);
    }

    if ((view->pending.configure_action & VIEW_ACTION_RESIZE) == 0) {
        cursor_rebase_all(true);
    }

    if (view->pending.configure_action &
        (VIEW_ACTION_FULLSCREEN | VIEW_ACTION_TILE | VIEW_ACTION_MAXIMIZE)) {
        view_update_round_corner(view);
    }

    view->pending.configure_serial = 0;
    view->pending.configure_action = VIEW_ACTION_NOP;
    view->pending.configure_geometry = (struct kywc_box){ 0 };
    view->pending.geometry = (struct kywc_box){ 0 };
}

void view_send_ping(struct view *view)
{
    if (view->impl->ping) {
        view->impl->ping(view);
    }
}

void view_proxy_destroy(struct view_proxy *view_proxy)
{
    if (!view_proxy) {
        return;
    }

    if (view_proxy->view->base.mapped) {
        wl_signal_emit_mutable(&view_proxy->workspace->events.view_leave, view_proxy->view);
        wl_signal_emit_mutable(&view_proxy->view->events.workspace_leave, view_proxy->workspace);
    }
    wl_list_remove(&view_proxy->view_link);
    wl_list_remove(&view_proxy->workspace_link);
    scene_node_destroy(&view_proxy->tree->node);
    free(view_proxy);
}

void view_set_current_proxy(struct view *view, struct view_proxy *view_proxy)
{
    if (view->current_proxy == view_proxy) {
        return;
    }

    view->current_proxy = view_proxy;
    if (!view_proxy) {
        assert(wl_list_empty(&view->view_proxies));
    }
    if (view_proxy) {
        struct view_layer *view_layer =
            view_manager_get_layer_by_node(&view_proxy->tree->node, true);
        view_move_to_layer(view, view_layer, true);
    }
    view_update_capabilities(view, KYWC_VIEW_ABOVEABLE | KYWC_VIEW_BELOWABLE);
    wl_signal_emit_mutable(&view->events.workspace, NULL);
}

static void view_proxies_destroy(struct view *view)
{
    struct view_proxy *proxy, *tmp;
    wl_list_for_each_safe(proxy, tmp, &view->view_proxies, view_link) {
        view_proxy_destroy(proxy);
    }
    view_set_current_proxy(view, NULL);
}

void view_destroy(struct view *view)
{
    struct kywc_view *kywc_view = &view->base;
    kywc_log(KYWC_DEBUG, "Kywc_view %p destroy", kywc_view);

    wl_signal_emit_mutable(&kywc_view->events.destroy, NULL);
    wl_list_remove(&view->link);
    wl_list_remove(&view->layer_link);
    wl_list_remove(&view->output_destroy.link);
    wl_list_remove(&view->outputs_update.link);
    wl_list_remove(&view->update_capabilities.link);
    wl_list_remove(&view->minimized_geometry.panel_surface_destroy.link);
    assert(wl_list_empty(&view->events.output.listener_list));
    assert(wl_list_empty(&view->events.parent.listener_list));
    assert(wl_list_empty(&view->events.workspace.listener_list));
    assert(wl_list_empty(&view->events.workspace_enter.listener_list));
    assert(wl_list_empty(&view->events.workspace_leave.listener_list));
    assert(wl_list_empty(&view->events.icon_update.listener_list));
    assert(wl_list_empty(&view->events.position.listener_list));
    assert(wl_list_empty(&view->events.update_capabilities.listener_list));
    assert(wl_list_empty(&view->events.stack.listener_list));
    assert(wl_list_empty(&kywc_view->events.premap.listener_list));
    assert(wl_list_empty(&kywc_view->events.map.listener_list));
    assert(wl_list_empty(&kywc_view->events.unmap.listener_list));
    assert(wl_list_empty(&kywc_view->events.destroy.listener_list));
    assert(wl_list_empty(&kywc_view->events.activate.listener_list));
    assert(wl_list_empty(&kywc_view->events.maximize.listener_list));
    assert(wl_list_empty(&kywc_view->events.minimize.listener_list));
    assert(wl_list_empty(&kywc_view->events.fullscreen.listener_list));
    assert(wl_list_empty(&kywc_view->events.tile.listener_list));
    assert(wl_list_empty(&kywc_view->events.above.listener_list));
    assert(wl_list_empty(&kywc_view->events.below.listener_list));
    assert(wl_list_empty(&kywc_view->events.sticky.listener_list));
    assert(wl_list_empty(&kywc_view->events.skip_taskbar.listener_list));
    assert(wl_list_empty(&kywc_view->events.skip_switcher.listener_list));
    assert(wl_list_empty(&kywc_view->events.demands_attention.listener_list));
    assert(wl_list_empty(&kywc_view->events.modal.listener_list));
    assert(wl_list_empty(&kywc_view->events.capabilities.listener_list));
    assert(wl_list_empty(&kywc_view->events.title.listener_list));
    assert(wl_list_empty(&kywc_view->events.app_id.listener_list));
    assert(wl_list_empty(&kywc_view->events.position.listener_list));
    assert(wl_list_empty(&kywc_view->events.size.listener_list));
    assert(wl_list_empty(&kywc_view->events.decoration.listener_list));
    assert(wl_list_empty(&kywc_view->events.opacity.listener_list));

    // some apps might be destroyed directly without a map
    view_remove_relationship(view);

    scene_node_destroy(&view->tree->node);
    view_proxies_destroy(view);
    free(view->icon_name);
    free((void *)kywc_view->uuid);
    free((void *)kywc_view->exec);

    view->impl->destroy(view);
}

void view_set_title(struct view *view, const char *title)
{
    struct kywc_view *kywc_view = &view->base;

    kywc_view->title = title;
    kywc_log(KYWC_DEBUG, "Kywc_view %p title %s", kywc_view, title);

    wl_signal_emit_mutable(&kywc_view->events.title, NULL);
}

void view_set_app_id(struct view *view, const char *app_id)
{
    struct kywc_view *kywc_view = &view->base;

    kywc_view->app_id = app_id;
    kywc_log(KYWC_DEBUG, "Kywc_view %p app_id %s", kywc_view, app_id);

    if (kywc_view->mapped && !view->icon_name) {
        view_set_icon(view, false);
    }
    wl_signal_emit_mutable(&kywc_view->events.app_id, NULL);
}

void view_set_decoration(struct view *view, enum kywc_ssd ssd)
{
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->ssd == ssd) {
        return;
    }

    kywc_view->ssd = ssd;
    view_update_round_corner(view);
    kywc_log(KYWC_DEBUG, "Kywc_view %p need ssd %d", kywc_view, ssd);

    wl_signal_emit_mutable(&kywc_view->events.decoration, NULL);
}

void view_set_opacity(struct view *view, float opacity)
{
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->opacity == opacity) {
        return;
    }

    kywc_view->opacity = opacity;
    kywc_log(KYWC_DEBUG, "Kywc_view %p opacity %f", kywc_view, opacity);

    wl_signal_emit_mutable(&kywc_view->events.opacity, NULL);
}

void view_set_icon_name(struct view *view, const char *icon_name)
{
    if ((!view->icon_name && !icon_name) ||
        (view->icon_name && icon_name && !strcmp(view->icon_name, icon_name))) {
        return;
    }

    free(view->icon_name);
    view->icon_name = icon_name ? strdup(icon_name) : NULL;

    if (view->base.mapped) {
        view_set_icon(view, false);
    }
}

static void view_handle_panel_surface_destroy(struct wl_listener *listener, void *data)
{
    struct view *view = wl_container_of(listener, view, minimized_geometry.panel_surface_destroy);
    wl_list_remove(&view->minimized_geometry.panel_surface_destroy.link);
    wl_list_init(&view->minimized_geometry.panel_surface_destroy.link);
    view->minimized_geometry.box = (struct kywc_box){ 0 };
    view->minimized_geometry.panel_surface = NULL;
}

void view_set_minimized_geometry(struct view *view, struct wlr_surface *panel_surface,
                                 const struct kywc_box *box)
{
    /* update geometry if panel_surface is the same */
    if (view->minimized_geometry.panel_surface == panel_surface) {
        view->minimized_geometry.box = box ? *box : (struct kywc_box){ 0 };
        return;
    }

    /* remove old listener if exists */
    if (view->minimized_geometry.panel_surface) {
        wl_list_remove(&view->minimized_geometry.panel_surface_destroy.link);
        wl_list_init(&view->minimized_geometry.panel_surface_destroy.link);
    }

    view->minimized_geometry.panel_surface = panel_surface;
    view->minimized_geometry.box = box && panel_surface ? *box : (struct kywc_box){ 0 };

    if (panel_surface) {
        wl_signal_add(&panel_surface->events.destroy,
                      &view->minimized_geometry.panel_surface_destroy);
    }
}

struct view_proxy *view_proxy_by_workspace(struct view *view, struct workspace *workspace)
{
    struct view_proxy *view_proxy;
    wl_list_for_each(view_proxy, &view->view_proxies, view_link) {
        if (view_proxy->workspace == workspace) {
            return view_proxy;
        }
    }
    return NULL;
}

static const char *view_proxy_handle_desc(const struct scene_node *node, void *data)
{
    struct view_proxy *proxy = data;
    return proxy->view->base.app_id;
}

static const struct scene_node_role_interface view_proxy_role_impl = {
    .desc = view_proxy_handle_desc,
};

static struct view_proxy *view_proxy_create(struct view *view, struct workspace *workspace)
{
    struct view_proxy *proxy = calloc(1, sizeof(*proxy));
    if (!proxy) {
        return NULL;
    }

    proxy->view = view;
    proxy->workspace = workspace;

    wl_list_insert(&workspace->view_proxies, &proxy->workspace_link);
    wl_list_insert(&view->view_proxies, &proxy->view_link);

    /* create view_proxy tree from new workspace view_layer */
    enum layer layer =
        view->base.kept_above ? LAYER_ABOVE : (view->base.kept_below ? LAYER_BELOW : LAYER_NORMAL);
    struct view_layer *view_layer = workspace_layer(workspace, layer);
    proxy->tree = scene_tree_create(view_layer->tree);
    scene_node_set_role(&proxy->tree->node, SCENE_ROLE_VIEW_PROXY, &view_proxy_role_impl, proxy);
    if (view->base.mapped) {
        wl_signal_emit_mutable(&workspace->events.view_enter, view);
        wl_signal_emit_mutable(&view->events.workspace_enter, workspace);
    }
    return proxy;
}

struct view *view_find_ancestor(struct view *view)
{
    struct view *ancestor = view;
    while (ancestor->parent) {
        ancestor = ancestor->parent;
    }
    return ancestor;
}

static void view_do_set_workspace(struct view *view, struct workspace *workspace)
{
    struct view_proxy *proxy = view_proxy_by_workspace(view, workspace);
    if (!proxy) {
        proxy = view_proxy_create(view, workspace);
        if (!proxy) {
            return;
        }
    }

    if (proxy->tree != view->tree->node.parent) {
        view_set_current_proxy(view, proxy);
    }

    // destroy all proxies except the new worskpace
    struct view_proxy *view_proxy, *tmp;
    wl_list_for_each_safe(view_proxy, tmp, &view->view_proxies, view_link) {
        if (view_proxy != proxy) {
            view_proxy_destroy(view_proxy);
        }
    }
    if (view->base.sticky) {
        view->base.sticky = false;
        wl_signal_emit_mutable(&view->base.events.sticky, NULL);
    }

    struct view *child;
    wl_list_for_each_reverse(child, &view->children, parent_link) {
        view_do_set_workspace(child, workspace);
    }
}

void view_set_workspace(struct view *view, struct workspace *workspace)
{
    assert(view && workspace);

    struct view *ancestor = view_find_ancestor(view);
    view_do_set_workspace(ancestor, workspace);
}

static void view_do_unset_workspace(struct view *view, struct view_layer *layer)
{
    /* set workspace to null must reparent view tree first */
    view_proxies_destroy(view);
    wl_list_init(&view->view_proxies);

    struct view *child;
    wl_list_for_each_reverse(child, &view->children, parent_link) {
        view_move_to_layer(child, layer, true);
        view_do_unset_workspace(child, layer);
    }
}

void view_unset_workspace(struct view *view, struct view_layer *layer)
{
    assert(layer->layer != LAYER_BELOW && layer->layer != LAYER_NORMAL &&
           layer->layer != LAYER_ABOVE && view);

    struct view *ancestor = view_find_ancestor(view);
    view_move_to_layer(ancestor, layer, true);
    view_do_unset_workspace(ancestor, layer);
}

static struct view_proxy *view_do_add_workspace(struct view *view, struct workspace *workspace)
{
    struct view_proxy *view_proxy = view_proxy_by_workspace(view, workspace);
    if (!view_proxy) {
        view_proxy = view_proxy_create(view, workspace);
        struct workspace *current_workspace = workspace_manager_get_current();
        if (workspace == current_workspace) {
            view_set_current_proxy(view, view_proxy);
        }
    }

    struct view *child;
    wl_list_for_each_reverse(child, &view->children, parent_link) {
        view_do_add_workspace(child, workspace);
    }
    return view_proxy;
}

struct view_proxy *view_add_workspace(struct view *view, struct workspace *workspace)
{
    if (!view || !workspace) {
        return NULL;
    }

    struct view *ancestor = view_find_ancestor(view);
    view_do_add_workspace(ancestor, workspace);
    return view_proxy_by_workspace(view, workspace);
}

static void view_do_remove_workspace(struct view *view, struct workspace *workspace)
{
    struct view_proxy *view_proxy = view_proxy_by_workspace(view, workspace);
    if (!view_proxy) {
        return;
    }

    struct view_proxy *proxy;
    wl_list_for_each_reverse(proxy, &view->view_proxies, view_link) {
        if (proxy != view_proxy) {
            break;
        }
    }
    /* add to all workspace if the view only exists in this workspace */
    if (&proxy->view_link == &view->view_proxies) {
        view_add_all_workspace(view);
        return;
    }
    /* del the proxy if view exists in multi workspaces */
    if (view->current_proxy == view_proxy) {
        view_set_current_proxy(view, proxy);
    }
    view_proxy_destroy(view_proxy);
    if (view->base.sticky) {
        view->base.sticky = false;
        wl_signal_emit_mutable(&view->base.events.sticky, NULL);
    }

    struct view *child;
    wl_list_for_each_reverse(child, &view->children, parent_link) {
        view_do_remove_workspace(child, workspace);
    }
}

void view_remove_workspace(struct view *view, struct workspace *workspace)
{
    if (!view || !workspace) {
        return;
    }

    struct view *ancestor = view_find_ancestor(view);
    view_do_remove_workspace(ancestor, workspace);
}

void view_add_all_workspace(struct view *view)
{
    if (!view || view->base.sticky) {
        return;
    }
    int workspace_num = workspace_manager_get_count();
    struct workspace *workspace = NULL;
    for (int i = 0; i < workspace_num; i++) {
        workspace = workspace_by_position(i);
        view_add_workspace(view, workspace);
    }
    view->base.sticky = true;
    wl_signal_emit_mutable(&view->base.events.sticky, NULL);
}

static void view_follow_parent_layer(struct view *view, struct view *parent)
{
    struct view_layer *layer = view_manager_get_layer_by_node(&parent->tree->node, false);
    if (!layer) {
        return;
    }

    if (view->current_proxy && !parent->current_proxy) {
        view_unset_workspace(view, layer);
        return;
    }

    if (!view->current_proxy && parent->current_proxy) {
        view_add_workspace(view, parent->current_proxy->workspace);
    }
    view_move_to_layer(view, layer, true);
}

void view_add_parent_workspace(struct view *view)
{
    if (!view || !view->parent) {
        return;
    }
    struct view *parent = view->parent;
    struct view_proxy *proxy;
    wl_list_for_each(proxy, &parent->view_proxies, view_link) {
        view_add_workspace(view, proxy->workspace);
    }
}

void view_set_parent(struct view *view, struct view *parent)
{
    if (view->parent == parent) {
        return;
    }

    wl_list_remove(&view->parent_link);
    if (parent) {
        wl_list_insert(&parent->children, &view->parent_link);
        struct view_layer *parent_layer =
            view_manager_get_layer_by_node(&parent->tree->node, false);
        struct view_layer *layer = view_manager_get_layer_by_node(&view->tree->node, false);
        /* do not set layer if parent layer below view layer */
        if (parent_layer && layer && parent_layer->layer > layer->layer) {
            view_follow_parent_layer(view, parent);
        }
    } else {
        wl_list_init(&view->parent_link);
    }

    kywc_log(KYWC_DEBUG, "View %p set parent to %p", view, parent);

    view->parent = parent;
    view_update_descendant_capabilities(parent ? parent : view,
                                        KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_FULLSCREENABLE |
                                            KYWC_VIEW_MOVABLE | KYWC_VIEW_RESIZABLE);
    view_add_parent_workspace(view);

    wl_signal_emit_mutable(&view->events.parent, NULL);
}

void kywc_view_add_new_listener(struct wl_listener *listener)
{
    wl_signal_add(&view_manager->events.new_view, listener);
}

void kywc_view_add_new_mapped_listener(struct wl_listener *listener)
{
    wl_signal_add(&view_manager->events.new_mapped_view, listener);
}

struct kywc_view *kywc_view_by_uuid(const char *uuid)
{
    struct view *view;
    wl_list_for_each(view, &view_manager->views, link) {
        if (strcmp(uuid, view->base.uuid) == 0) {
            return &view->base;
        }
    }
    return NULL;
}

struct view *view_from_kywc_view(struct kywc_view *kywc_view)
{
    struct view *view = wl_container_of(kywc_view, view, base);
    return view;
}

struct view *view_try_from_wlr_surface(struct wlr_surface *wlr_surface)
{
    return wlr_surface->data;
}

void kywc_view_close(struct kywc_view *kywc_view)
{
    struct view *view = view_from_kywc_view(kywc_view);
    if (!KYWC_VIEW_IS_CLOSEABLE(kywc_view)) {
        return;
    }

    if (view->impl->close) {
        view->impl->close(view);
    }
    view_send_ping(view);
}

void view_do_move(struct view *view, int x, int y)
{
    view->pending.action |= VIEW_ACTION_MOVE;
    view->pending.geometry.x = x;
    view->pending.geometry.y = y;

    if (view->base.mapped && view->impl->configure) {
        view->impl->configure(view);
    }
}

void kywc_view_move(struct kywc_view *kywc_view, int x, int y)
{
    if (view_manager->mode->impl->view_request_move) {
        view_manager->mode->impl->view_request_move(view_from_kywc_view(kywc_view), x, y);
    }
}

void view_do_resize(struct view *view, struct kywc_box *geometry)
{
    view->pending.action |= VIEW_ACTION_RESIZE;
    view->pending.geometry = *geometry;

    if (view->base.mapped && view->impl->configure) {
        view->impl->configure(view);
    }
}

void kywc_view_resize(struct kywc_view *kywc_view, struct kywc_box *geometry)
{
    if (view_manager->mode->impl->view_request_resize) {
        view_manager->mode->impl->view_request_resize(view_from_kywc_view(kywc_view), geometry);
    }
}

static void view_set_activated(struct view *view, bool activated);

static void handle_activated_view_minimized(struct wl_listener *listener, void *data)
{
    struct view *view = view_manager->activated.view;
    if (!view->base.minimized) {
        return;
    }

    view_set_activated(view, false);
    view_activate_topmost(false);
}

static void handle_activated_view_unmap(struct wl_listener *listener, void *data)
{
    struct view *view = view_manager->activated.view;
    view_set_activated(view, false);
    view_activate_topmost(false);
}

static void view_reparent_fullscreen(struct view *view, bool active);

static struct view *view_find_fullscreen_ancestor(struct view *view)
{
    if (!view->parent) {
        return view->base.fullscreen ? view : NULL;
    }

    struct view *ancestor = NULL;
    while (view->parent) {
        if (view->parent->base.fullscreen) {
            ancestor = view->parent;
        }
        view = view->parent;
    }
    return ancestor;
}

static void view_set_activated(struct view *view, bool activated)
{
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->activated == activated) {
        return;
    }

    if (activated) {
        /* listen activated view's minimize and unmap signals,
         * so that we can auto activate another view.
         */
        view_manager->activated.view = view;
        wl_list_remove(&view_manager->activated.minimize.link);
        wl_list_remove(&view_manager->activated.unmap.link);
        wl_signal_add(&kywc_view->events.minimize, &view_manager->activated.minimize);
        wl_signal_add(&kywc_view->events.unmap, &view_manager->activated.unmap);
    } else if (view_manager->activated.view == view) {
        wl_list_remove(&view_manager->activated.minimize.link);
        wl_list_remove(&view_manager->activated.unmap.link);
        wl_list_init(&view_manager->activated.minimize.link);
        wl_list_init(&view_manager->activated.unmap.link);
        view_manager->activated.view = NULL;
    }

    bool skip_reparent = !activated && view_manager->activated.view &&
                         (view_has_descendant(view, view_manager->activated.view) ||
                          view_has_ancestor(view, view_manager->activated.view));
    if (!skip_reparent) {
        struct view *ancestor = view_find_fullscreen_ancestor(view);
        if (ancestor) {
            view_reparent_fullscreen(ancestor, activated);
        }
    }

    if (kywc_view->minimized && activated) {
        kywc_view_set_minimized(kywc_view, false);
    }

    kywc_view->activated = activated;
    view->pending.action |= VIEW_ACTION_ACTIVATE;

    if (kywc_view->mapped && view->impl->configure) {
        view->impl->configure(view);
    }

    wl_signal_emit_mutable(&kywc_view->events.activate, NULL);
}

void view_do_activate(struct view *view)
{
    if (view && !KYWC_VIEW_IS_ACTIVATABLE(&view->base)) {
        return;
    }

    struct view *last = view_manager->activated.view;
    if (last != view) {
        if (view_manager->show_activated_only_enabled) {
            view_manager_show_activated_only(false, false);
        }
        if (view) {
            view_set_activated(view, true);
        }
        if (last) {
            view_set_activated(last, false);
        }
        wl_signal_emit_mutable(&view_manager->events.activate_view, view_manager->activated.view);
    } else if (view) {
        struct view *ancestor = view_find_fullscreen_ancestor(view);
        if (ancestor) {
            view_reparent_fullscreen(ancestor, true);
        }
    }

    if (!view) {
        return;
    }

    struct workspace *workspace = view->current_proxy ? view->current_proxy->workspace : NULL;
    if (workspace && workspace != workspace_manager_get_current()) {
        workspace_activate_with_effect(view->current_proxy->workspace);
    }
}

static void view_hide_fullscreen_view_in_empty_workspace(void)
{
    struct workspace *workspace = workspace_manager_get_current();
    struct view *view = view_manager->activated.view;
    if (!view) {
        return;
    }

    struct view *ancestor = view_find_fullscreen_ancestor(view);
    if (ancestor) {
        struct view_proxy *view_proxy = view_proxy_by_workspace(ancestor, workspace);
        if (!view_proxy) {
            view_reparent_fullscreen(ancestor, false);
            view_set_activated(ancestor, false);
        }
    }
}

void view_activate_topmost(bool force)
{
    struct view *view = view_manager_get_global_authentication();
    /* activate desktop at last */

    struct view *current_view = NULL;
    if (view_manager->activated.view &&
        view_manager->activated.view->base.role != KYWC_VIEW_ROLE_DESKTOP) {
        current_view = view_manager->activated.view;
    }
    if (!view && current_view && (!force || !current_view->current_proxy) &&
        current_view->base.mapped && !current_view->base.minimized) {
        /* keep current activated view */
        view = current_view;
    }

    if (!view) {
        struct view_proxy *view_proxy;
        struct workspace *workspace = workspace_manager_get_current();
        /* find topmost enabled(mapped and not minimized) view and activate it */
        wl_list_for_each(view_proxy, &workspace->view_proxies, workspace_link) {
            if (view_proxy->view->base.mapped && !view_proxy->view->base.minimized &&
                KYWC_VIEW_IS_ACTIVATABLE(&view_proxy->view->base)) {
                view = view_proxy->view;
                break;
            }
        }
    }

    if (!view) {
        view = view_manager->desktop;
    }

    if (view) {
        view_do_activate(view);
        view_set_focus(view, input_manager_get_default_seat());
        return;
    }

    view_do_activate(NULL);
    /* clear keyboard focus */
    seat_focus_surface(input_manager_get_default_seat(), NULL);

    /* workaround to hide fullscreen view when no view in workspace */
    view_hide_fullscreen_view_in_empty_workspace();
    wl_signal_emit_mutable(&view_manager->events.activate_view, view_manager->activated.view);
}

void view_raise_to_top(struct view *view, bool find_parent)
{
    if (!view) {
        return;
    }

    if (view->parent && find_parent) {
        view_raise_to_top(view->parent, true);
    }

    view_move_to_layer(view, view->view_layer, true);

    /* raise children if any */
    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        view_raise_to_top(child, false);
    }
}

void kywc_view_activate(struct kywc_view *kywc_view)
{
    struct view *view = kywc_view ? view_from_kywc_view(kywc_view) : NULL;
    if (!view) {
        view_do_activate(NULL);
        return;
    }

    if (view_manager->mode->impl->view_request_activate) {
        view_manager->mode->impl->view_request_activate(view);
    }
}

void view_manager_update_input_panel_layer(struct wlr_surface *surface)
{
    if (!surface) {
        return;
    }

    struct scene_buffer *buffer = scene_buffer_try_from_surface(surface);
    if (!buffer) {
        return;
    }

    struct view_layer *input_layer = view_manager_get_layer(LAYER_INPUT_PANEL, false);
    struct view_layer *sibling_layer = view_manager_get_layer_by_node(&buffer->node, false);
    /**
     * No need to update input_panel if focused surface is already on it.
     * On same layer, later-mapped surfaces appear on top, possibly occluding others.
     */
    if (sibling_layer == input_layer) {
        return;
    }

    if (sibling_layer->layer < LAYER_INPUT_PANEL) {
        sibling_layer = view_manager_get_layer(LAYER_INPUT_PANEL - 1, false);
    }
    scene_node_place_above(&input_layer->tree->node, &sibling_layer->tree->node);
}

void view_set_focus(struct view *view, struct seat *seat)
{
    if (!view || !seat) {
        return;
    }

    struct view *descendant = view_find_descendant_modal(view);
    if (descendant) {
        view = descendant;
    }

    if (!KYWC_VIEW_IS_FOCUSABLE(&view->base)) {
        return;
    }

    seat_focus_surface(seat, view->surface);
    view_send_ping(view);
}

void view_do_tiled(struct view *view, enum kywc_tile tile, struct kywc_output *kywc_output,
                   struct kywc_box *geometry)
{
    struct kywc_view *kywc_view = &view->base;

    /* tiled mode may switch between outputs */
    if (kywc_view->tiled == tile && (!kywc_output || kywc_output == view->output) &&
        kywc_box_equal(&kywc_view->geometry, geometry)) {
        if (kywc_box_not_empty(&view->tile_start_geometry)) {
            view_add_scale_effect(view, SCALE_RESIZE);
        }
        return;
    }

    /* may switch between tiled modes */
    if (kywc_view->tiled == KYWC_TILE_NONE && tile != KYWC_TILE_NONE) {
        if (!kywc_view->maximized && !view->interactive_moving &&
            (!kywc_output || kywc_output == view->output)) {
            view->saved.geometry = view_action_change_size(view->pending.configure_action)
                                       ? view->pending.configure_geometry
                                       : kywc_view->geometry;
        }
    }

    if (kywc_view->maximized) {
        view->pending.action |= VIEW_ACTION_MAXIMIZE;
        kywc_view->maximized = false;
    }

    kywc_view->tiled = tile;
    view->pending.action |= VIEW_ACTION_TILE;
    view->pending.geometry = *geometry;

    view_add_scale_effect(view, SCALE_RESIZE);

    if (kywc_view->mapped && view->impl->configure) {
        view->impl->configure(view);
    }
}

void kywc_view_set_tiled(struct kywc_view *kywc_view, enum kywc_tile tile,
                         struct kywc_output *kywc_output)
{
    if (view_manager->mode->impl->view_request_tiled) {
        view_manager->mode->impl->view_request_tiled(view_from_kywc_view(kywc_view), tile,
                                                     kywc_output);
    }
}

void view_do_minimized(struct view *view, bool minimized)
{
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->minimized == minimized) {
        return;
    }

    kywc_view->minimized = minimized;
    view->pending.action |= VIEW_ACTION_MINIMIZE;

    if (!kywc_view->mapped) {
        return;
    }

    scene_node_set_enabled(&view->tree->node, !minimized);
    if (view->impl->configure) {
        view->impl->configure(view);
    }

    bool used_slide = false;
    if (kywc_view->role == KYWC_VIEW_ROLE_SYSTEMWINDOW) {
        used_slide = view_add_slide_effect(view, !kywc_view->minimized);
    }

    if (!used_slide) {
        /* if view is the activated view, process it in activated.minimize listener */
        if (view_manager->state.minimize_effect_type == MINIMIZE_EFFECT_TYPE_MAGIC_LAMP) {
            if (!view_add_magic_lamp_effect(view)) {
                view_add_scale_effect(view, SCALE_MINIMIZE);
            }
        } else {
            view_add_scale_effect(view, SCALE_MINIMIZE);
        }
    }

    wl_signal_emit_mutable(&kywc_view->events.minimize, NULL);

    if (!kywc_view->minimized) {
        if (view_manager->show_desktop_enabled) {
            view_manager_show_desktop(false, false);
        }
        if (view_manager->show_activated_only_enabled) {
            view_manager_show_activated_only(false, false);
        }
    }

    cursor_rebase_all(false);

    if (!view->minimized_when_show_desktop) {
        struct view *child;
        wl_list_for_each(child, &view->children, parent_link) {
            view_do_minimized(child, minimized);
        }
    }
}

void kywc_view_set_minimized(struct kywc_view *kywc_view, bool minimized)
{
    if (view_manager->mode->impl->view_request_minimized) {
        view_manager->mode->impl->view_request_minimized(view_from_kywc_view(kywc_view), minimized);
    }
}

void kywc_view_toggle_minimized(struct kywc_view *kywc_view)
{
    kywc_view_set_minimized(kywc_view, !kywc_view->minimized);
}

void view_do_maximized(struct view *view, bool maximized, struct kywc_output *kywc_output)
{
    struct kywc_view *kywc_view = &view->base;

    /* tiled to unmaximized after tiled from maximized */
    if (!kywc_view->tiled && kywc_view->maximized == maximized &&
        (!kywc_output || kywc_output == view->output)) {
        return;
    }

    struct kywc_box geo = { 0 };

    if (maximized) {
        view_get_tiled_geometry(view, &geo, kywc_output ? kywc_output : view->output,
                                KYWC_TILE_ALL);
        if (!kywc_view->tiled && !view->interactive_moving &&
            (!kywc_output || kywc_output == view->output)) {
            view->saved.geometry = view_action_change_size(view->pending.configure_action)
                                       ? view->pending.configure_geometry
                                       : kywc_view->geometry;
        }
    } else {
        if (kywc_box_not_empty(&view->restore_geometry)) {
            geo = view->restore_geometry;
        } else {
            geo = view->saved.geometry;
        }
    }

    /* don't restore tiled mode followed other compositors */
    if (kywc_view->tiled) {
        view->pending.action |= VIEW_ACTION_TILE;
        kywc_view->tiled = KYWC_TILE_NONE;
    }

    kywc_view->maximized = maximized;
    view->pending.action |= VIEW_ACTION_MAXIMIZE;
    view->pending.geometry = geo;

    if (kywc_view->mapped && view->impl->configure) {
        view->impl->configure(view);
    }
}

void kywc_view_set_maximized(struct kywc_view *kywc_view, bool maximized,
                             struct kywc_output *kywc_output)
{
    if (view_manager->mode->impl->view_request_maximized) {
        view_manager->mode->impl->view_request_maximized(view_from_kywc_view(kywc_view), maximized,
                                                         kywc_output);
    }
}

void kywc_view_toggle_maximized(struct kywc_view *kywc_view)
{
    kywc_view_set_maximized(kywc_view, !kywc_view->maximized, NULL);
}

static void view_reparent_fullscreen(struct view *view, bool active)
{
    if (active) {
        view_move_to_layer(view, view_manager_get_layer(LAYER_ACTIVE, false), true);
    } else if (view->current_proxy) {
        struct view_layer *view_layer =
            view_manager_get_layer_by_node(&view->current_proxy->tree->node, true);
        view_move_to_layer(view, view_layer, true);
    }

    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        view_reparent_fullscreen(child, active);
    }
}

void view_do_fullscreen(struct view *view, bool fullscreen, struct kywc_output *kywc_output)
{
    struct kywc_view *kywc_view = &view->base;

    if (kywc_view->fullscreen == fullscreen && (!kywc_output || kywc_output == view->output)) {
        return;
    }

    struct kywc_box geo = { 0 };

    if (fullscreen) {
        kywc_output_effective_geometry(kywc_output ? kywc_output : view->output, &geo);
        if (!kywc_view->maximized && !kywc_view->tiled) {
            view->saved.geometry = view_action_change_size(view->pending.configure_action)
                                       ? view->pending.configure_geometry
                                       : kywc_view->geometry;
        }
    } else if (kywc_view->maximized) {
        view_get_tiled_geometry(view, &geo, view->output, KYWC_TILE_ALL);
    } else if (kywc_view->tiled) {
        view_get_tiled_geometry(view, &geo, view->output, kywc_view->tiled);
    } else {
        if (kywc_box_not_empty(&view->restore_geometry)) {
            geo = view->restore_geometry;
        } else {
            geo = view->saved.geometry;
        }
    }

    view_reparent_fullscreen(view, fullscreen);
    kywc_view->fullscreen = fullscreen;
    view_update_capabilities(view, KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_MOVABLE | KYWC_VIEW_RESIZABLE);

    view->pending.action |= VIEW_ACTION_FULLSCREEN;
    view->pending.geometry = geo;

    if (kywc_view->mapped && view->impl->configure) {
        view->impl->configure(view);
    }
}

void kywc_view_set_fullscreen(struct kywc_view *kywc_view, bool fullscreen,
                              struct kywc_output *kywc_output)
{
    if (view_manager->mode->impl->view_request_fullscreen) {
        view_manager->mode->impl->view_request_fullscreen(view_from_kywc_view(kywc_view),
                                                          fullscreen, kywc_output);
    }
}

void kywc_view_toggle_fullscreen(struct kywc_view *kywc_view)
{
    kywc_view_set_fullscreen(kywc_view, !kywc_view->fullscreen, NULL);
}

static void view_do_set_kept_above(struct view *view, bool kept_above)
{
    if (view->parent && !kept_above) {
        view_do_set_kept_above(view->parent, kept_above);
    }

    if (view->base.kept_above != kept_above) {
        view->base.kept_above = kept_above;
        view->base.kept_below = false;

        enum layer layer = kept_above ? LAYER_ABOVE : LAYER_NORMAL;
        if (view->view_layer->layer == LAYER_ACTIVE) {
            /* only need move the proxy if the view is in fullscreen layer */
            struct view_proxy *proxy;
            wl_list_for_each(proxy, &view->view_proxies, view_link) {
                struct view_layer *view_layer = workspace_layer(proxy->workspace, layer);
                scene_node_reparent(&proxy->tree->node, view_layer->tree);
            }
        } else {
            view_move_to_layer(view, view_manager_get_layer(layer, true), true);
        }
        wl_signal_emit_mutable(&view->base.events.above, NULL);
    }

    if (kept_above) {
        struct view *child;
        wl_list_for_each_reverse(child, &view->children, parent_link) {
            view_do_set_kept_above(child, kept_above);
        }
    }
}

void kywc_view_set_kept_above(struct kywc_view *kywc_view, bool kept_above)
{
    struct view *view = view_from_kywc_view(kywc_view);
    if (!KYWC_VIEW_IS_ABOVEABLE(kywc_view)) {
        return;
    }

    view_do_set_kept_above(view, kept_above);
}

void kywc_view_toggle_kept_above(struct kywc_view *kywc_view)
{
    kywc_view_set_kept_above(kywc_view, !kywc_view->kept_above);
}

static void view_do_set_kept_below(struct view *view, bool kept_below)
{
    /* ancestor may already keep below, skip and go on */
    if (view->base.kept_below != kept_below) {
        view->base.kept_below = kept_below;
        view->base.kept_above = false;

        enum layer layer = kept_below ? LAYER_BELOW : LAYER_NORMAL;
        if (view->view_layer->layer == LAYER_ACTIVE) {
            /* only need move the proxy if the view is in fullscreen layer */
            struct view_proxy *proxy;
            wl_list_for_each(proxy, &view->view_proxies, view_link) {
                struct view_layer *view_layer = workspace_layer(proxy->workspace, layer);
                scene_node_reparent(&proxy->tree->node, view_layer->tree);
                if (kept_below) {
                    scene_node_lower_to_bottom(&proxy->tree->node);
                }
            }
        } else {
            view_move_to_layer(view, view_manager_get_layer(layer, true), !kept_below);
        }
        wl_signal_emit_mutable(&view->base.events.below, NULL);
    }

    if (!kept_below) {
        struct view *child;
        wl_list_for_each_reverse(child, &view->children, parent_link) {
            view_do_set_kept_below(child, kept_below);
        }
    } else if (view->parent) {
        view_do_set_kept_below(view->parent, kept_below);
    }
}

void kywc_view_set_kept_below(struct kywc_view *kywc_view, bool kept_below)
{
    struct view *view = view_from_kywc_view(kywc_view);
    if (!KYWC_VIEW_IS_BELOWABLE(kywc_view)) {
        return;
    }

    view_do_set_kept_below(view, kept_below);
}

void kywc_view_toggle_kept_below(struct kywc_view *kywc_view)
{
    kywc_view_set_kept_below(kywc_view, !kywc_view->kept_below);
}

void kywc_view_set_skip_taskbar(struct kywc_view *kywc_view, bool skip_taskbar)
{
    if (kywc_view->skip_taskbar == skip_taskbar) {
        return;
    }
    kywc_view->skip_taskbar = skip_taskbar;
    wl_signal_emit_mutable(&kywc_view->events.skip_taskbar, NULL);
}

void kywc_view_set_skip_switcher(struct kywc_view *kywc_view, bool skip_switcher)
{
    if (kywc_view->skip_switcher == skip_switcher) {
        return;
    }
    kywc_view->skip_switcher = skip_switcher;
    wl_signal_emit_mutable(&kywc_view->events.skip_switcher, NULL);
}

void kywc_view_set_demands_attention(struct kywc_view *kywc_view, bool demands_attention)
{
    if (kywc_view->demands_attention == demands_attention) {
        return;
    }
    kywc_view->demands_attention = demands_attention;
    wl_signal_emit_mutable(&kywc_view->events.demands_attention, NULL);
}

void kywc_view_set_modal(struct kywc_view *kywc_view, bool modal)
{
    if (kywc_view->modal == modal) {
        return;
    }

    kywc_view->modal = modal;

    struct view *ancestor = view_find_ancestor(view_from_kywc_view(kywc_view));
    view_update_descendant_capabilities(
        ancestor, KYWC_VIEW_MINIMIZABLE | KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_CLOSEABLE |
                      KYWC_VIEW_FULLSCREENABLE | KYWC_VIEW_MOVABLE | KYWC_VIEW_RESIZABLE |
                      KYWC_VIEW_MINIMIZE_BUTTON | KYWC_VIEW_MAXIMIZE_BUTTON);

    wl_signal_emit_mutable(&kywc_view->events.modal, NULL);
}

void kywc_view_set_sticky(struct kywc_view *kywc_view, bool sticky)
{
    if (kywc_view->sticky == sticky) {
        return;
    }

    struct view *view = view_from_kywc_view(kywc_view);
    if (sticky) {
        view_add_all_workspace(view);
    } else {
        view_set_workspace(view, workspace_manager_get_current());
    }
}

void view_helper_move(struct view *view, int x, int y)
{
    struct kywc_box *geo = &view->base.geometry;

    if (geo->x != x || geo->y != y) {
        geo->x = x, geo->y = y;
        scene_node_set_position(&view->tree->node, x, y);
        wl_signal_emit_mutable(&view->base.events.position, NULL);
    }

    if (!view->interactive_moving && view->current_resize_edges == KYWC_EDGE_NONE) {
        wl_signal_emit_mutable(&view->events.position, NULL);
    }
}

static bool view_has_modal_child(struct view *view)
{
    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        if (child->base.mapped && (child->base.modal || view_has_modal_child(child))) {
            return true;
        }
    }
    return false;
}

bool view_has_modal_property(struct view *view)
{
    return view->base.modal || view_has_modal_child(view);
}

struct view *view_find_descendant_modal(struct view *view)
{
    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        if (!child->base.mapped || !child->base.modal) {
            continue;
        }
        return view_find_descendant_modal(child);
    }

    return view->base.modal ? view : NULL;
}

static void view_handle_update_capabilities(struct wl_listener *listener, void *data)
{
    struct view *view = wl_container_of(listener, view, update_capabilities);
    struct kywc_view *kywc_view = &view->base;
    struct view_update_capabilities_event *event = data;

    if (event->mask & KYWC_VIEW_MINIMIZABLE) {
        if (!view->minimized_when_show_desktop && view_has_modal_property(view)) {
            event->state &= ~KYWC_VIEW_MINIMIZABLE;
        }
    }

    if (event->mask & KYWC_VIEW_MAXIMIZABLE) {
        if (kywc_view->fullscreen || view_has_modal_child(view) ||
            (KYWC_VIEW_HAS_FIXED_SIZE(kywc_view) && !kywc_view->maximized)) {
            event->state &= ~KYWC_VIEW_MAXIMIZABLE;
        }
    }

    if (event->mask & KYWC_VIEW_CLOSEABLE) {
        if (view_has_modal_child(view)) {
            event->state &= ~KYWC_VIEW_CLOSEABLE;
        }
    }

    if (event->mask & KYWC_VIEW_FULLSCREENABLE) {
        if (view_has_modal_child(view) || (KYWC_VIEW_HAS_FIXED_SIZE(kywc_view) && view->parent)) {
            event->state &= ~KYWC_VIEW_FULLSCREENABLE;
        }
    }

    if (event->mask & KYWC_VIEW_MOVABLE) {
        if (kywc_view->fullscreen || view_has_modal_child(view)) {
            event->state &= ~KYWC_VIEW_MOVABLE;
        }
    }

    if (event->mask & KYWC_VIEW_RESIZABLE) {
        if (kywc_view->fullscreen || view_has_modal_child(view) ||
            KYWC_VIEW_HAS_FIXED_SIZE(kywc_view)) {
            event->state &= ~KYWC_VIEW_RESIZABLE;
        }
    }

    if (event->mask & KYWC_VIEW_ABOVEABLE) {
        if (!view->current_proxy) {
            event->state &= ~KYWC_VIEW_ABOVEABLE;
        }
    }
    if (event->mask & KYWC_VIEW_BELOWABLE) {
        if (!view->current_proxy) {
            event->state &= ~KYWC_VIEW_BELOWABLE;
        }
    }

    if (event->mask & KYWC_VIEW_MINIMIZE_BUTTON) {
        if (kywc_view->modal) {
            event->state &= ~KYWC_VIEW_MINIMIZE_BUTTON;
        }
    }

    if (event->mask & KYWC_VIEW_MAXIMIZE_BUTTON) {
        if (kywc_view->modal || KYWC_VIEW_HAS_FIXED_SIZE(kywc_view)) {
            event->state &= ~KYWC_VIEW_MAXIMIZE_BUTTON;
        }
    }
}

struct view_layer *view_manager_get_layer_by_role(enum kywc_view_role role)
{
    switch (role) {
    default:
        return NULL;
    case KYWC_VIEW_ROLE_NORMAL:
        return view_manager_get_layer(LAYER_NORMAL, true);
    case KYWC_VIEW_ROLE_DESKTOP:
        return view_manager_get_layer(LAYER_DESKTOP, false);
    case KYWC_VIEW_ROLE_PANEL:
    case KYWC_VIEW_ROLE_APPLETPOPUP:
        return view_manager_get_layer(LAYER_DOCK, false);
    case KYWC_VIEW_ROLE_ONSCREENDISPLAY:
        return view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    case KYWC_VIEW_ROLE_NOTIFICATION:
        return view_manager_get_layer(LAYER_NOTIFICATION, false);
    case KYWC_VIEW_ROLE_TOOLTIP:
        return view_manager_get_layer(LAYER_POPUP, false);
    case KYWC_VIEW_ROLE_CRITICALNOTIFICATION:
        return view_manager_get_layer(LAYER_CRITICAL_NOTIFICATION, false);
    case KYWC_VIEW_ROLE_SYSTEMWINDOW:
        return view_manager_get_layer(LAYER_SYSTEM_WINDOW, false);
    case KYWC_VIEW_ROLE_SWITCHER:
        return view_manager_get_layer(LAYER_SWITCHER, false);
    case KYWC_VIEW_ROLE_INPUTPANEL:
        return view_manager_get_layer(LAYER_INPUT_PANEL, false);
    case KYWC_VIEW_ROLE_LOGOUT:
        return view_manager_get_layer(LAYER_LOGOUT, false);
    case KYWC_VIEW_ROLE_SCREENLOCKNOTIFICATION:
        return view_manager_get_layer(LAYER_SCREEN_LOCK_NOTIFICATION, false);
    case KYWC_VIEW_ROLE_WATERMARK:
        return view_manager_get_layer(LAYER_WATERMARK, false);
    case KYWC_VIEW_ROLE_SCREENLOCK:
        return view_manager_get_layer(LAYER_SCREEN_LOCK, false);
    case KYWC_VIEW_ROLE_AUTHENTICATION:
        return view_manager_get_layer(LAYER_CRITICAL_NOTIFICATION, false);
    }
}

void view_set_role(struct view *view, enum kywc_view_role role)
{
    struct kywc_view *kywc_view = &view->base;
    if (kywc_view->role == role) {
        return;
    }
    kywc_view->role = role;

    if (kywc_view->role == KYWC_VIEW_ROLE_AUTHENTICATION) {
        /* cleared when view is unmapoed */
        global_authentication_create(view);
    }

    if (kywc_view->role == KYWC_VIEW_ROLE_DESKTOP) {
        view_manager->desktop = view;
    }

    if (kywc_view->role == KYWC_VIEW_ROLE_NORMAL) {
        view_set_workspace(view, workspace_manager_get_current());
    } else {
        struct view_layer *layer = view_manager_get_layer_by_role(kywc_view->role);
        view_unset_workspace(view, layer);
    }

    kywc_view->capabilities = 0;
    if (kywc_view->role == KYWC_VIEW_ROLE_NORMAL) {
        kywc_view->capabilities |= KYWC_VIEW_CAPABILITIES_ALL;
    } else { // not normal
        if (kywc_view->role != KYWC_VIEW_ROLE_DESKTOP && kywc_view->role != KYWC_VIEW_ROLE_PANEL) {
            kywc_view->capabilities |= KYWC_VIEW_CLOSEABLE;
        }
        if (kywc_view->role == KYWC_VIEW_ROLE_AUTHENTICATION) {
            kywc_view->capabilities |= KYWC_VIEW_MOVABLE;
        }
        if (kywc_view->role == KYWC_VIEW_ROLE_PANEL) {
            kywc_view->capabilities |= KYWC_VIEW_RESIZABLE;
        }
        if (kywc_view->role != KYWC_VIEW_ROLE_PANEL && kywc_view->role != KYWC_VIEW_ROLE_TOOLTIP &&
            kywc_view->role != KYWC_VIEW_ROLE_WATERMARK &&
            kywc_view->role != KYWC_VIEW_ROLE_SWITCHER &&
            kywc_view->role != KYWC_VIEW_ROLE_NOTIFICATION) {
            kywc_view->capabilities |= KYWC_VIEW_ACTIVATABLE;
        }
        if (kywc_view->role == KYWC_VIEW_ROLE_DESKTOP ||
            kywc_view->role == KYWC_VIEW_ROLE_SYSTEMWINDOW ||
            kywc_view->role == KYWC_VIEW_ROLE_SCREENLOCK ||
            kywc_view->role == KYWC_VIEW_ROLE_APPLETPOPUP ||
            kywc_view->role == KYWC_VIEW_ROLE_ONSCREENDISPLAY ||
            kywc_view->role == KYWC_VIEW_ROLE_AUTHENTICATION) {
            kywc_view->capabilities |= KYWC_VIEW_FOCUSABLE;
        }
    }

    // panel need to be unconstrained
    kywc_view->unconstrained = kywc_view->role == KYWC_VIEW_ROLE_PANEL;

    kywc_view->has_round_corner = kywc_view->role == KYWC_VIEW_ROLE_NORMAL ||
                                  kywc_view->role == KYWC_VIEW_ROLE_SYSTEMWINDOW ||
                                  kywc_view->role == KYWC_VIEW_ROLE_APPLETPOPUP ||
                                  kywc_view->role == KYWC_VIEW_ROLE_AUTHENTICATION;

    view_update_round_corner(view);
}

void view_update_size(struct view *view, int width, int height, int min_width, int min_height,
                      int max_width, int max_height)
{
    struct kywc_view *kywc_view = &view->base;

    if (kywc_view->min_width != min_width || kywc_view->min_height != min_height) {
        kywc_view->min_width = min_width;
        kywc_view->min_height = min_height;
        kywc_log(KYWC_DEBUG, "View %p minimal size to %d x %d", view, min_width, min_height);
        view_update_capabilities(view, KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_FULLSCREENABLE |
                                           KYWC_VIEW_RESIZABLE | KYWC_VIEW_MAXIMIZE_BUTTON);
    }

    if (kywc_view->max_width != max_width || kywc_view->max_height != max_height) {
        kywc_view->max_width = max_width;
        kywc_view->max_height = max_height;
        kywc_log(KYWC_DEBUG, "View %p maximal size to %d x %d", view, max_width, max_height);
        view_update_capabilities(view, KYWC_VIEW_MAXIMIZABLE | KYWC_VIEW_FULLSCREENABLE |
                                           KYWC_VIEW_RESIZABLE | KYWC_VIEW_MAXIMIZE_BUTTON);
    }

    if (kywc_view->geometry.width != width || kywc_view->geometry.height != height) {
        kywc_view->geometry.width = width;
        kywc_view->geometry.height = height;

        kywc_log(KYWC_DEBUG, "View %p size to %d x %d", view, width, height);
        wl_signal_emit_mutable(&view->base.events.size, NULL);
    }
}

void view_update_descendant_capabilities(struct view *view, uint32_t mask)
{
    view_update_capabilities(view, mask);

    struct view *child;
    wl_list_for_each(child, &view->children, parent_link) {
        view_update_descendant_capabilities(child, mask);
    }
}

void view_update_capabilities(struct view *view, uint32_t mask)
{
    struct kywc_view *kywc_view = &view->base;
    struct view_update_capabilities_event event = {
        .mask = mask,
        .state = KYWC_VIEW_CAPABILITIES_ALL & mask,
    };
    wl_signal_emit_mutable(&view->events.update_capabilities, &event);

    uint32_t cleared = kywc_view->capabilities & ~mask;
    uint32_t new_state = event.state & mask;
    uint32_t capabilities = cleared | new_state;
    if (kywc_view->capabilities == capabilities) {
        return;
    }

    mask = kywc_view->capabilities ^ capabilities;
    kywc_view->capabilities = capabilities;
    struct kywc_view_capabilities_event ev = {
        .mask = mask,
    };
    wl_signal_emit_mutable(&kywc_view->events.capabilities, &ev);
}

void view_add_update_capabilities_listener(struct view *view, struct wl_listener *listener)
{
    wl_signal_add(&view->events.update_capabilities, listener);
}

void view_close_popups(struct view *view)
{
    if (view->impl->close_popups) {
        view->impl->close_popups(view);
    }
}

void view_show_window_menu(struct view *view, struct seat *seat, int x, int y)
{
    if (view_manager->mode->impl->view_request_show_menu) {
        view_manager->mode->impl->view_request_show_menu(view, seat, x, y);
    }
}

bool view_show_tile_flyout(struct view *view, struct seat *seat, struct kywc_box *box)
{
    if (view_manager->mode->impl->view_request_show_tile_flyout) {
        view_manager->mode->impl->view_request_show_tile_flyout(view, seat, box);
        return true;
    }
    return false;
}

bool view_show_tile_linkage_bar(struct view *view, uint32_t edges)
{
    if (view_manager->mode->impl->view_request_show_tile_linkage_bar) {
        view_manager->mode->impl->view_request_show_tile_linkage_bar(view, edges);
        return true;
    }
    return false;
}

void highlight_view(struct view *view, bool enable)
{
    view_manager->highlight = enable ? view : NULL;

    struct workspace *workspace = workspace_manager_get_current();
    struct view_proxy *view_proxy;
    wl_list_for_each_reverse(view_proxy, &workspace->view_proxies, workspace_link) {
        if (!view_proxy->view->base.mapped || view_proxy->view->base.minimized) {
            continue;
        }

        scene_node_set_enabled(&view_proxy->view->tree->node, !enable);
    }

    if (view) {
        // Unset highlight window need to restore status when highlight window is minimized
        scene_node_set_enabled(&view->tree->node, enable ? enable : !view->base.minimized);
    }

    wl_signal_emit_mutable(&view_manager->events.highlight, view);
}

void view_manager_add_highlight_view_listener(struct wl_listener *listener)
{
    wl_signal_add(&view_manager->events.highlight, listener);
}

struct view *view_manager_get_highlight(void)
{
    return view_manager->highlight;
}

void view_show_hang_window(struct view *view)
{
    // TODO: implement show_hang_window
    kywc_log(KYWC_WARN, "%s view is not responding", view->base.app_id);
    if (view_manager->impl.show_hang_window) {
        view_manager->impl.show_hang_window(view);
    }
}

void view_manager_show_desktop(bool enabled, bool apply)
{
    if (view_manager->show_desktop_enabled == enabled) {
        return;
    }

    view_manager->show_desktop_enabled = enabled;

    /* minimize all view in current workspace */
    struct workspace *workspace = workspace_manager_get_current();
    struct view_proxy *view_proxy;
    wl_list_for_each_reverse(view_proxy, &workspace->view_proxies, workspace_link) {
        struct view *view = view_proxy->view;
        /* skip views not mapped */
        if (!view->base.mapped) {
            continue;
        }
        /* skip restoring views not minimized by show desktop */
        if (!enabled && !view->minimized_when_show_desktop) {
            continue;
        }

        /* true only the view is not minimized when going show desktop */
        if (enabled) {
            view->minimized_when_show_desktop = !view->base.minimized;
            view_update_capabilities(view, KYWC_VIEW_MINIMIZABLE);
        }
        /* don't restoring views if the state is breaked */
        if (apply || view_has_modal_property(view)) {
            kywc_view_set_minimized(&view->base, enabled);
        }
        if (!enabled) {
            view->minimized_when_show_desktop = false;
            view_update_capabilities(view, KYWC_VIEW_MINIMIZABLE);
        }
    }

    if (apply && !enabled) {
        view_activate_topmost(false);
    }

    /* set focus to desktop after showing desktop */
    if (apply && enabled && view_manager->desktop) {
        view_set_focus(view_manager->desktop, input_manager_get_default_seat());
    }

    wl_signal_emit_mutable(&view_manager->events.show_desktop, NULL);
}

void view_manager_add_show_desktop_listener(struct wl_listener *listener)
{
    wl_signal_add(&view_manager->events.show_desktop, listener);
}

bool view_manager_get_show_desktop(void)
{
    return view_manager->show_desktop_enabled;
}

bool view_manager_get_show_switcher(void)
{
    return view_manager->switcher_shown;
}

void view_manager_show_activated_only(bool enabled, bool apply)
{
    if (view_manager->show_activated_only_enabled == enabled) {
        return;
    }
    view_manager->show_activated_only_enabled = enabled;

    struct view *current_view = view_manager_get_activated();
    if (!current_view) {
        return;
    }

    /* minimize all view in current workspace */
    struct workspace *workspace = workspace_manager_get_current();
    struct view_proxy *view_proxy;
    wl_list_for_each_reverse(view_proxy, &workspace->view_proxies, workspace_link) {
        struct view *view = view_proxy->view;
        if (!view->base.mapped || view == current_view || view_has_modal_property(view)) {
            continue;
        }

        /* skip restoring views not minimized by show only current view */
        if (!enabled && !view->minimized_when_show_active_only) {
            continue;
        }

        if (enabled) {
            view->minimized_when_show_active_only = !view->base.minimized;
        }
        /* don't restoring views if the state is breaked */
        if (apply) {
            if (view->base.minimized == enabled || !KYWC_VIEW_IS_MINIMIZABLE(&view->base)) {
                continue;
            }

            scene_node_set_enabled(&view->tree->node, !enabled);
            view->base.minimized = enabled;
            view->pending.action |= VIEW_ACTION_MINIMIZE;

            if (view->impl->configure) {
                view->impl->configure(view);
            }
            wl_signal_emit_mutable(&view->base.events.minimize, NULL);
        }

        if (!enabled) {
            view->minimized_when_show_active_only = false;
        }
    }
}

bool view_manager_get_show_activte_only(void)
{
    return view_manager->show_activated_only_enabled;
}

uint32_t view_manager_get_adsorption(void)
{
    return view_manager->state.view_adsorption;
}

uint32_t view_manager_get_resize_filter(struct view *view)
{
    if (xwayland_check_view(view)) {
        return view_manager->state.resize_filter[1];
    } else {
        return view_manager->state.resize_filter[0];
    }
}

uint32_t view_manager_get_configure_timeout(struct view *view)
{
    if (!view) {
        return MAX(view_manager->state.configure_timeout[0],
                   view_manager->state.configure_timeout[1]);
    } else if (xwayland_check_view(view)) {
        return view_manager->state.configure_timeout[1];
    } else {
        return view_manager->state.configure_timeout[0];
    }
}

void view_manager_set_global_authentication(struct view *view)
{
    view_manager->global_authentication_view = view;
}

struct view *view_manager_get_global_authentication(void)
{
    return view_manager->global_authentication_view;
}

static void handle_server_ready(struct wl_listener *listener, void *data)
{
    theme_manager_add_update_listener(&view_manager->theme_update, false);
    theme_manager_add_icon_update_listener(&view_manager->theme_icon_update);

    window_menu_manager_create(view_manager);
    maximize_switcher_create(view_manager);
    tile_flyout_manager_create(view_manager);

    stack_mode_register(view_manager);
    tablet_mode_register(view_manager);

    if (!view_manager->mode) {
        view_manager->mode = view_manager_mode_from_name("stack_mode");
    }
    if (view_manager->mode->impl->view_mode_enter) {
        view_manager->mode->impl->view_mode_enter();
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    assert(wl_list_empty(&view_manager->events.new_view.listener_list));
    assert(wl_list_empty(&view_manager->events.new_mapped_view.listener_list));
    assert(wl_list_empty(&view_manager->events.show_desktop.listener_list));
    assert(wl_list_empty(&view_manager->events.activate_view.listener_list));
    assert(wl_list_empty(&view_manager->events.highlight.listener_list));

    wl_list_remove(&view_manager->server_destroy.link);
    wl_list_remove(&view_manager->server_ready.link);
    wl_list_remove(&view_manager->server_terminate.link);
    wl_list_remove(&view_manager->theme_update.link);
    wl_list_remove(&view_manager->theme_icon_update.link);

    struct view_mode *mode, *tmp;
    wl_list_for_each_safe(mode, tmp, &view_manager->view_modes, link) {
        if (mode->impl->mode_destroy) {
            mode->impl->mode_destroy();
        }
        view_manager_mode_unregister(mode);
    }

    for (int layer = LAYER_FIRST; layer < LAYER_NUMBER; layer++) {
        scene_node_destroy(&view_manager->layers[layer].tree->node);
    }

    free(view_manager);
    view_manager = NULL;
}

static void handle_server_terminate(struct wl_listener *listener, void *data)
{
    view_manager->state.num_workspaces = workspace_manager_get_count();
    view_write_config(view_manager);
}

static void handle_theme_update(struct wl_listener *listener, void *data)
{
    struct theme_update_event *update_event = data;
    if (!(update_event->update_mask & THEME_UPDATE_MASK_CORNER_RADIUS)) {
        return;
    }

    struct view *view;
    wl_list_for_each(view, &view_manager->views, link) {
        view_update_round_corner(view);
    }
}

static void handle_theme_icon_update(struct wl_listener *listener, void *data)
{
    struct view *view;
    wl_list_for_each(view, &view_manager->views, link) {
        if (view->base.mapped) {
            view_set_icon(view, false);
        }
    }
}

void view_manager_set_switcher_shown(bool shown)
{
    view_manager->switcher_shown = shown;
}

void view_click(struct seat *seat, struct view *view, uint32_t button, bool pressed,
                enum click_state state)
{
    if (view_manager->mode->impl->view_click) {
        view_manager->mode->impl->view_click(seat, view, button, pressed, state);
    }
}

void view_hover(struct seat *seat, struct view *view)
{
    if (view_manager->mode->impl->view_hover) {
        view_manager->mode->impl->view_hover(seat, view);
    }
}

struct view_mode *view_manager_mode_from_name(const char *name)
{
    if (!name || !*name) {
        return NULL;
    }

    struct view_mode *mode;
    wl_list_for_each(mode, &view_manager->view_modes, link) {
        if (mode->impl->name && strcmp(mode->impl->name, name) == 0) {
            return mode;
        }
    }
    return NULL;
}

bool view_manager_set_view_mode(const char *name)
{
    struct view_mode *view_mode = view_manager_mode_from_name(name);
    if (!view_mode) {
        return false;
    }

    if (view_manager->mode == view_mode) {
        return true;
    }

    if (!view_manager->server->ready) {
        view_manager->mode = view_mode;
        return true;
    }

    if (view_manager->mode && view_manager->mode->impl->view_mode_leave) {
        view_manager->mode->impl->view_mode_leave();
    }
    view_manager->mode = view_mode;
    if (view_manager->mode->impl->view_mode_enter) {
        view_manager->mode->impl->view_mode_enter();
    }
    return true;
}

void view_manager_set_effect_options_impl(action_effect_options_adjust_func_t impl)
{
    view_manager->impl.action_effect_options_adjust = impl;
}

void view_manager_adjust_effect_options(enum effect_action action,
                                        struct action_effect_options *options, void *user_data)
{
    if (!view_manager->impl.action_effect_options_adjust) {
        return;
    }

    view_manager->impl.action_effect_options_adjust(ACTION_EFFECT_OPTIONS_SURFACE, action, options,
                                                    user_data);
}

bool view_has_descendant(struct view *view, struct view *descendant)
{
    if (!descendant) {
        return !wl_list_empty(&view->children);
    }

    struct view *tmp;
    wl_list_for_each(tmp, &view->children, parent_link) {
        if (tmp == descendant) {
            return true;
        }
        if (view_has_descendant(tmp, descendant)) {
            return true;
        }
    }

    return false;
}

bool view_has_ancestor(struct view *view, struct view *ancestor)
{
    if (!view->parent) {
        return false;
    }

    if (!ancestor) {
        return !!view->parent;
    }

    if (view->parent == ancestor) {
        return true;
    }

    return view_has_ancestor(view->parent, ancestor);
}

void view_move_to_center(struct view *view)
{
    struct output *output = output_from_kywc_output(view->output);
    int x_center = output->geometry.x + output->geometry.width / 2;
    int y_center = output->geometry.y + output->geometry.height / 2;
    int x = x_center - view->base.geometry.width / 2;
    int y = y_center - view->base.geometry.height / 2;

    view_do_move(view, x, y);
}

bool view_validate_move_or_resize_request(struct view *view, struct seat *seat)
{
    bool left_button_pressed =
        LEFT_BUTTON_PRESSED(seat->cursor->last_click_button, seat->cursor->last_click_pressed);
    bool pointer_touch_id = seat->cursor->pointer_touch_id >= 0;
    bool tablet_tool_tip_down = seat->cursor->tablet_tool_tip_down;

    /**
     * Check for touch or tablet, then check for mouse.
     *
     * The touch device first clicks the SSD area, records the mouse state,
     * and triggers a grab. After grabbing, it does not update. At this point,
     * if using a touch device to click the surface, the mouse state remains as before.
     */
    struct scene_node *node = NULL;
    if (pointer_touch_id || tablet_tool_tip_down) {
        return true;
    } else if (left_button_pressed) {
        node = seat->cursor->focus.node;
    }

    return node && wlr_surface_try_from_node(node) == view->surface;
}

struct view_mode *view_manager_mode_register(const struct view_mode_interface *impl)
{
    struct view_mode *mode = malloc(sizeof(*mode));
    if (!mode) {
        return NULL;
    }

    wl_list_insert(&view_manager->view_modes, &mode->link);
    mode->impl = impl;
    return mode;
}

void view_manager_mode_unregister(struct view_mode *mode)
{
    wl_list_remove(&mode->link);
    free(mode);
}

void view_manager_for_each_view(view_iterator_func_t iterator, bool mapped, void *data)
{
    struct view *view;
    wl_list_for_each(view, &view_manager->views, link) {
        if (view->base.mapped == mapped) {
            if (iterator(&view->base, data)) {
                break;
            }
        }
    }
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&view_manager->display_destroy.link);
    wl_list_remove(&view_manager->new_xdg_toplevel.link);
}

static void xdg_foreign_create(struct server *server)
{
    struct wlr_xdg_foreign_registry *foreign_registry =
        wlr_xdg_foreign_registry_create(server->display);
    if(!foreign_registry) {
        kywc_log(KYWC_ERROR, "Failed to create xdg foreign registry");
        return;
    }
    wlr_xdg_foreign_v1_create(server->display, foreign_registry);
    wlr_xdg_foreign_v2_create(server->display, foreign_registry);
    view_manager->foreign_registry = foreign_registry;
}

static const char *view_layer_handle_desc(const struct scene_node *node, void *data)
{
    static char *layers[] = {
        "desktop",
        "watermark",
        "below",
        "normal",
        "dock",
        "above",
        "notification",
        "active",
        "system_window",
        "switcher",
        "popup",
        "input_panel",
        "critical_notification",
        "logout",
        "unmanaged",
        "screen_lock",
        "screen_lock_notification",
        "on_screen_display",
    };

    struct view_layer *view_layer = data;
    return layers[view_layer - view_manager->layers];
}

static const struct scene_node_role_interface role_impl = {
    .desc = view_layer_handle_desc,
};

struct view_manager *view_manager_create(struct server *server)
{
    view_manager = calloc(1, sizeof(*view_manager));
    if (!view_manager) {
        return NULL;
    }

    view_manager->server = server;
    wl_list_init(&view_manager->views);
    wl_signal_init(&view_manager->events.new_view);
    wl_signal_init(&view_manager->events.new_mapped_view);
    wl_signal_init(&view_manager->events.show_desktop);
    wl_signal_init(&view_manager->events.activate_view);
    wl_signal_init(&view_manager->events.highlight);

    view_manager->theme_update.notify = handle_theme_update;
    wl_list_init(&view_manager->theme_update.link);
    view_manager->theme_icon_update.notify = handle_theme_icon_update;
    wl_list_init(&view_manager->theme_icon_update.link);

    view_manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &view_manager->display_destroy);
    view_manager->server_terminate.notify = handle_server_terminate;
    wl_signal_add(&server->events.terminate, &view_manager->server_terminate);
    view_manager->server_ready.notify = handle_server_ready;
    wl_signal_add(&server->events.ready, &view_manager->server_ready);
    view_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &view_manager->server_destroy);

    view_manager->activated.minimize.notify = handle_activated_view_minimized;
    wl_list_init(&view_manager->activated.minimize.link);
    view_manager->activated.unmap.notify = handle_activated_view_unmap;
    wl_list_init(&view_manager->activated.unmap.link);

    /* create all layers */
    for (int layer = LAYER_FIRST; layer < LAYER_NUMBER; layer++) {
        struct view_layer *view_layer = &view_manager->layers[layer];
        view_layer->layer = layer;
        view_layer->tree = scene_tree_create(&server->scene->tree);
        scene_node_set_role(&view_layer->tree->node, SCENE_ROLE_LAYER, &role_impl, view_layer);
        wl_list_init(&view_layer->views);
    }

    wl_list_init(&view_manager->view_modes);
    view_manager_config_init(view_manager);

    view_manager->state.num_workspaces = 4;
    view_manager->state.view_adsorption = VIEW_ADSORPTION_ALL;
    view_manager->state.resize_filter[0] = 10;
    view_manager->state.resize_filter[1] = 40;
    view_manager->state.configure_timeout[0] = 200;
    view_manager->state.configure_timeout[1] = 400;
    view_read_config(view_manager);

    workspace_manager_create(view_manager);
    decoration_manager_create(view_manager);
    server_decoration_manager_create(view_manager);
    positioner_manager_create(view_manager);
    tile_manager_create(view_manager);
    window_actions_create(view_manager);
    view_manager_actions_create(view_manager);

    wl_list_init(&view_manager->new_xdg_toplevel.link);
    xdg_shell_init(view_manager);
    ukui_startup_management_create(view_manager);

    modal_manager_create(server);
    wlr_layer_shell_manager_create(server);
    wlr_foreign_toplevel_manager_create(server);
    ky_toplevel_manager_create(server);
    ext_foreign_toplevel_create(server);
    xdg_toplevel_icon_manager_create(server);
    kde_plasma_shell_create(server);
    kde_plasma_window_management_create(server);
    kde_blur_manager_create(server);
    kde_slide_manager_create(server);
    xdg_dialog_create(server);
    xdg_activation_create(server);
    ukui_shell_create(server);
    ukui_window_management_create(server);
    ukui_blur_manager_create(server);
    xdg_foreign_create(server);

    return view_manager;
}
