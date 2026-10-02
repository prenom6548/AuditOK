// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "effect/action.h"
#include "effect/effect.h"
#include "input/cursor.h"
#include "input/event.h"
#include "output.h"
#include "scene/decoration.h"
#include "scene/xdg_shell.h"
#include "theme.h"
#include "util/color.h"
#include "util/macros.h"
#include "view_p.h"

#define XDG_POPUP_NO_DECORATION (0)

struct xdg_popup {
    struct wlr_xdg_popup *wlr_xdg_popup;

    struct scene_tree *popup_tree;
    /* may a view/layer tree or popup tree */
    struct scene_tree *parent_tree;
    /* shell tree xdg-shell or layer-shell */
    struct scene_tree *shell_tree;
    /* for shadow and blur */
    struct scene_decoration *deco;

    struct wl_listener destroy;
    struct wl_listener new_popup;
    struct wl_listener commit;
    struct wl_listener reposition;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener theme_update;

    struct view *parent_view;
    struct wl_listener parent_position;
    struct wl_listener parent_unmap;

    uint32_t decoration_flags;
    bool topmost_popup;
    bool topmost_force_enter;
    bool use_usable_area;
};

enum xdg_popup_decoration_flags { ROUND_CORNER = 0, BORDER, SHADOW };

static void handle_xdg_popup_destroy(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, destroy);

    wl_list_remove(&popup->destroy.link);
    wl_list_remove(&popup->new_popup.link);
    wl_list_remove(&popup->commit.link);
    wl_list_remove(&popup->reposition.link);
    wl_list_remove(&popup->map.link);
    wl_list_remove(&popup->unmap.link);
    wl_list_remove(&popup->theme_update.link);
    wl_list_remove(&popup->parent_position.link);
    wl_list_remove(&popup->parent_unmap.link);

    /* only need to destroy the topmost popup parent tree,
     * popup tree will be destroyed by xdg_surface destroy in scene
     */
    if (popup->topmost_popup) {
        scene_node_destroy(&popup->parent_tree->node);
    }

    free(popup);
}

static struct xdg_popup *_xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup,
                                           struct scene_tree *parent, struct scene_tree *shell,
                                           bool use_usable_area);

static void popup_handle_new_xdg_popup(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, new_popup);
    struct wlr_xdg_popup *wlr_popup = data;

    _xdg_popup_create(wlr_popup, popup->popup_tree, popup->shell_tree, popup->use_usable_area);
}

static void popup_unconstrain(struct xdg_popup *popup)
{
    int lx = 0, ly = 0;
    scene_node_coords(&popup->shell_tree->node, &lx, &ly);

    /**
     * -1 avoids landing exactly on the output edge when the parent window is maximized.
     *  Otherwise kywc_output_at_point() may pick the wrong output and the popup gets
     *  constrained to the wrong usable_area.
     */
    struct wlr_box *popup_box = &popup->wlr_xdg_popup->scheduled.geometry;
    struct kywc_output *kywc_output =
        kywc_output_at_point(lx + MAX(popup_box->x - 1, 0), ly + MAX(popup_box->y - 1, 0));
    struct output *output = kywc_output ? output_from_kywc_output(kywc_output)
                                        : input_current_output(input_manager_get_default_seat());

    struct kywc_box *output_box = popup->use_usable_area ? &output->usable_area : &output->geometry;

    struct wlr_box toplevel_space_box = {
        .x = output_box->x - lx,
        .y = output_box->y - ly,
        .width = output_box->width,
        .height = output_box->height,
    };

    wlr_xdg_popup_unconstrain_from_box(popup->wlr_xdg_popup, &toplevel_space_box);
}

static void handle_xdg_popup_commit(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, commit);
    struct wlr_xdg_surface *xdg_surface = popup->wlr_xdg_popup->base;

    if (xdg_surface->initial_commit) {
        popup_unconstrain(popup);
    }

    if (popup->deco) {
        struct wlr_box *geo = &xdg_surface->current.geometry;
        int width = geo->width, height = geo->height;
        if (width == 0 || height == 0) {
            width = xdg_surface->surface->current.width;
            height = xdg_surface->surface->current.height;
        }
        scene_decoration_set_surface_size(popup->deco, width, height);
    }
}

static void handle_xdg_popup_reposition(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, reposition);
    popup_unconstrain(popup);
}

static void xdg_popup_update_decoration(struct xdg_popup *xdg_popup, uint32_t flags)
{
    struct theme *theme = theme_manager_get_theme();

    if ((flags >> ROUND_CORNER) & 0x1) {
        int r = theme->menu_radius;
        struct scene_buffer *buffer =
            scene_buffer_try_from_surface(xdg_popup->wlr_xdg_popup->base->surface);
        scene_node_set_radius(&buffer->node, (int[4]){ r, r, r, r });
        scene_decoration_set_round_corner_radius(xdg_popup->deco, (int[4]){ r, r, r, r });
    }

    if ((flags >> BORDER) & 0x1) {
        float border_color[4];
        color_float_pa(border_color, theme->active_border_color);
        scene_decoration_set_margin(xdg_popup->deco, 0, theme->border_width);
        scene_decoration_set_margin_color(xdg_popup->deco, (float[4]){ 0, 0, 0, 0 }, border_color);
    }

    if ((flags >> SHADOW) & 0x1) {
        scene_decoration_set_shadow_count(xdg_popup->deco, theme->menu_shadow_color.num_layers);
        for (int i = 0; i < theme->menu_shadow_color.num_layers; i++) {
            struct theme_shadow_layer *shadow = &theme->menu_shadow_color.layers[i];
            float shadow_color[4];
            color_float_pa(shadow_color, shadow->color);
            scene_decoration_set_shadow(xdg_popup->deco, i, shadow->off_x, shadow->off_y,
                                        shadow->spread, shadow->blur, shadow_color);
        }
    }
}

static void create_popup_decoration(struct xdg_popup *xdg_popup)
{
    struct wlr_xdg_surface *xdg_surface = xdg_popup->wlr_xdg_popup->base;
    uint32_t flags = ukui_shell_get_surface_decoration_flags(xdg_surface->surface);
    if (flags == XDG_POPUP_NO_DECORATION) {
        return;
    }

    xdg_popup->deco = scene_decoration_create_with_border_type(xdg_popup->popup_tree,
                                                               DECORATION_BORDER_TYPE_INNER);
    theme_manager_add_update_listener(&xdg_popup->theme_update, false);

    struct scene_node *node = scene_node_from_decoration(xdg_popup->deco);
    scene_node_raise_to_top(node);
    scene_node_set_input_bypassed(node, true);

    struct wlr_box *geo = &xdg_surface->current.geometry;
    int width = geo->width, height = geo->height;
    if (width == 0 || height == 0) {
        width = xdg_surface->surface->current.width;
        height = xdg_surface->surface->current.height;
    }
    scene_decoration_set_surface_size(xdg_popup->deco, width, height);
    scene_decoration_set_mask(xdg_popup->deco, DECORATION_MASK_ALL);

    xdg_popup->decoration_flags = flags;
    xdg_popup_update_decoration(xdg_popup, flags);
}

static void action_effect_options_adjust(enum action_effect_options_step step,
                                         enum effect_action action,
                                         struct action_effect_options *options, void *user_data)
{
    struct xdg_popup *popup = user_data;

    switch (step) {
    case ACTION_EFFECT_OPTIONS_ADD:
        /* FADE_TYPE_CENTER */
        options->style = 0;
        options->alpha = 0.0;
        options->width_scale = 1.0;
        options->height_scale = 1.0;
        bool topmost = popup->topmost_popup;
        bool seat = popup->wlr_xdg_popup->seat;
        if (action == EFFECT_ACTION_MAP) {
            options->duration = 220;
            if (topmost && !seat) {
                /* tooltips */
                options->width_scale = 0.85;
                options->height_scale = 0.85;
                options->duration = 200;
            } else if (!topmost) {
                /* sub menu */
                options->y_offset = -4;
            }
        } else {
            options->duration = 180;
            if (topmost && !seat) {
                options->width_scale = 0.85;
                options->height_scale = 0.85;
                options->duration = 150;
            } else if (!topmost) {
                options->y_offset = 4;
            }
        }

        struct animation *animation;
        if (topmost && seat) {
            animation = animation_manager_get(ANIMATION_TYPE_30_2_8_100);
            options->animations.alpha = animation;
            options->animations.geometry = animation;
        } else {
            animation = animation_manager_get(ANIMATION_TYPE_EASE);
            options->animations.alpha = animation;
            options->animations.geometry = animation;
        }

        options->surface = popup->wlr_xdg_popup->base->surface;
        options->scale = options->surface ? scene_surface_get_scale(options->surface) : 1.0f;
        break;
    case ACTION_EFFECT_OPTIONS_SURFACE:
    case ACTION_EFFECT_OPTIONS_CONFIRM:
        break;
    }
}

static void handle_xdg_popup_map(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, map);
    struct wlr_seat *seat = popup->wlr_xdg_popup->seat;
    /* force enter the popup surface if has grab */
    popup->topmost_force_enter = popup->topmost_popup && seat;
    if (popup->topmost_force_enter) {
        struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
        if (keyboard) {
            wlr_seat_keyboard_enter(seat, popup->wlr_xdg_popup->base->surface, keyboard->keycodes,
                                    keyboard->num_keycodes, &keyboard->modifiers);
        } else {
            wlr_seat_keyboard_enter(seat, popup->wlr_xdg_popup->base->surface, NULL, 0, NULL);
        }
    }

    create_popup_decoration(popup);

    struct scene_node *node = &popup->popup_tree->node;
    scene_node_set_enabled(node, true);

    node_add_action_effect(node, EFFECT_ACTION_MAP, ACTION_EFFECT_FADE,
                           action_effect_options_adjust, popup);

    cursor_rebase_all(false);
}

static void handle_xdg_popup_unmap(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, unmap);
    struct scene_node *node = &popup->popup_tree->node;

    node_add_action_effect(node, EFFECT_ACTION_UNMAP, ACTION_EFFECT_FADE,
                           action_effect_options_adjust, popup);
    scene_node_set_enabled(node, false);

    if (popup->deco) {
        scene_node_destroy(scene_node_from_decoration(popup->deco));
        wl_list_remove(&popup->theme_update.link);
        wl_list_init(&popup->theme_update.link);
        popup->deco = NULL;
    }

    if (popup->topmost_force_enter) {
        struct wlr_seat *wlr_seat = popup->wlr_xdg_popup->seat;
        wlr_seat_keyboard_end_grab(wlr_seat);
        struct view *parent = popup->parent_view;
        struct view *activated_view = view_manager_get_activated();
        struct wlr_surface *focused_surface = wlr_seat->keyboard_state.focused_surface;

        if (activated_view && activated_view != parent) {
            if (activated_view->surface != focused_surface) {
                view_set_focus(activated_view, seat_from_wlr_seat(wlr_seat));
            }
            return;
        }

        if (parent && KYWC_VIEW_IS_ACTIVATABLE(&parent->base) &&
            KYWC_VIEW_IS_FOCUSABLE(&parent->base)) {
            view_do_activate(parent);
            view_set_focus(parent, seat_from_wlr_seat(wlr_seat));
        } else {
            view_activate_topmost(false);
        }
    }
}

static void xdg_popup_handle_theme_update(struct wl_listener *listener, void *data)
{
    struct xdg_popup *xdg_popup = wl_container_of(listener, xdg_popup, theme_update);

    struct theme_update_event *update_event = data;
    uint32_t allowed_mask = THEME_UPDATE_MASK_CORNER_RADIUS | THEME_UPDATE_MASK_OPACITY |
                            THEME_UPDATE_MASK_BORDER_COLOR | THEME_UPDATE_MASK_SHADOW_COLOR;
    if (update_event->update_mask & allowed_mask) {
        /* force update all items */
        xdg_popup_update_decoration(xdg_popup, xdg_popup->decoration_flags);
    }
}

static void xdg_popup_handle_parent_position(struct wl_listener *listener, void *data)
{
    struct xdg_popup *xdg_popup = wl_container_of(listener, xdg_popup, parent_position);

    int lx = xdg_popup->parent_view->base.geometry.x;
    int ly = xdg_popup->parent_view->base.geometry.y;
    scene_node_set_position(&xdg_popup->parent_tree->node, lx, ly);
}

static void xdg_popup_handle_parent_unmap(struct wl_listener *listener, void *data)
{
    struct xdg_popup *xdg_popup = wl_container_of(listener, xdg_popup, parent_unmap);
    wl_list_remove(&xdg_popup->parent_position.link);
    wl_list_init(&xdg_popup->parent_position.link);
    wl_list_remove(&xdg_popup->parent_unmap.link);
    wl_list_init(&xdg_popup->parent_unmap.link);
    xdg_popup->parent_view = NULL;
}

static struct xdg_popup *_xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup,
                                           struct scene_tree *parent, struct scene_tree *shell,
                                           bool use_usable_area)
{
    struct xdg_popup *popup = calloc(1, sizeof(*popup));
    if (!popup) {
        return NULL;
    }

    popup->wlr_xdg_popup = wlr_xdg_popup;
    popup->parent_tree = parent;
    popup->shell_tree = shell;
    popup->use_usable_area = use_usable_area;

    /* add popup surface to parent tree, popup map and unmap is handled in scene */
    popup->popup_tree = scene_xdg_surface_create(parent, wlr_xdg_popup->base);
    scene_node_set_role(&popup->popup_tree->node, SCENE_ROLE_XDG_POPUP, NULL, wlr_xdg_popup);
    scene_node_set_enabled(&popup->popup_tree->node, wlr_xdg_popup->base->surface->mapped);

    popup->destroy.notify = handle_xdg_popup_destroy;
    wl_signal_add(&wlr_xdg_popup->events.destroy, &popup->destroy);
    popup->reposition.notify = handle_xdg_popup_reposition;
    wl_signal_add(&wlr_xdg_popup->events.reposition, &popup->reposition);
    popup->new_popup.notify = popup_handle_new_xdg_popup;
    wl_signal_add(&wlr_xdg_popup->base->events.new_popup, &popup->new_popup);
    popup->commit.notify = handle_xdg_popup_commit;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.commit, &popup->commit);
    popup->map.notify = handle_xdg_popup_map;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.map, &popup->map);
    popup->unmap.notify = handle_xdg_popup_unmap;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.unmap, &popup->unmap);
    popup->theme_update.notify = xdg_popup_handle_theme_update;
    wl_list_init(&popup->theme_update.link);
    popup->parent_position.notify = xdg_popup_handle_parent_position;
    wl_list_init(&popup->parent_position.link);
    popup->parent_unmap.notify = xdg_popup_handle_parent_unmap;
    wl_list_init(&popup->parent_unmap.link);

    return popup;
}

static bool xdg_popup_hover(struct seat *seat, struct scene_node *node, double x, double y,
                            uint32_t time, bool first, bool hold, void *data)
{
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    if (first) {
        kywc_log(KYWC_DEBUG, "First hover surface %p (%f %f)", surface, x, y);
    }

    seat_notify_motion(seat, surface, time, x, y, first);
    return false;
}

static void xdg_popup_click(struct seat *seat, struct scene_node *node, uint32_t button,
                            bool pressed, uint32_t time, enum click_state state, void *data)
{
    seat_notify_button(seat, time, button, pressed);
}

static void xdg_popup_leave(struct seat *seat, struct scene_node *node, bool last, void *data)
{
    /* so surface will call set_cursor when enter again */
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    seat_notify_leave(seat, surface);
}

static struct scene_node *xdg_popup_get_root(void *data)
{
    struct xdg_popup *popup = data;
    return &popup->parent_tree->node;
}

static const struct input_event_node_impl xdg_popup_event_node_impl = {
    .hover = xdg_popup_hover,
    .click = xdg_popup_click,
    .leave = xdg_popup_leave,
};

void xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup, struct scene_tree *shell,
                      struct view_layer *layer, bool use_usable_area)
{
    struct scene_tree *parent = scene_tree_create(layer->tree);

    /* get shell layout coord, and set it to parent tree */
    int lx, ly;
    scene_node_coords(&shell->node, &lx, &ly);
    scene_node_set_position(&parent->node, lx, ly);

    struct xdg_popup *popup = _xdg_popup_create(wlr_xdg_popup, parent, shell, use_usable_area);
    popup->topmost_popup = true;

    struct view *parent_view = view_try_from_wlr_surface(wlr_xdg_popup->parent);
    popup->parent_view = parent_view;
    if (parent_view) {
        wl_signal_add(&parent_view->base.events.position, &popup->parent_position);
        wl_signal_add(&parent_view->base.events.destroy, &popup->parent_unmap);
    }

    input_event_node_create(&parent->node, &xdg_popup_event_node_impl, xdg_popup_get_root, NULL,
                            popup);
}
