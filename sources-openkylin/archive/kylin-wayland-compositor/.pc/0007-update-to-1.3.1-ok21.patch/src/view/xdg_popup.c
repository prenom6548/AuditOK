// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "effect/fade.h"
#include "input/event.h"
#include "output.h"
#include "scene/decoration.h"
#include "scene/xdg_shell.h"
#include "theme.h"
#include "util/color.h"
#include "view_p.h"

#define XDG_POPUP_NO_DECORATION (0)

struct xdg_popup {
    struct wlr_xdg_popup *wlr_xdg_popup;

    struct ky_scene_tree *popup_tree;
    /* may a view/layer tree or popup tree */
    struct ky_scene_tree *parent_tree;
    /* shell tree xdg-shell or layer-shell */
    struct ky_scene_tree *shell_tree;
    /* for shadow and blur */
    struct ky_scene_decoration *deco;

    struct wl_listener destroy;
    struct wl_listener new_popup;
    struct wl_listener commit;
    struct wl_listener reposition;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener theme_update;

    uint32_t decoration_flags;
    bool topmost_popup;
    bool topmost_clear_focus;
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

    /* only need to destroy the topmost popup parent tree,
     * popup tree will be destroyed by xdg_surface destroy in scene
     */
    if (popup->topmost_popup) {
        ky_scene_node_destroy(&popup->parent_tree->node);
    }

    if (popup->topmost_clear_focus) {
        view_activate_topmost();
    }

    free(popup);
}

static struct xdg_popup *_xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup,
                                           struct ky_scene_tree *parent,
                                           struct ky_scene_tree *shell, bool use_usable_area);

static void popup_handle_new_xdg_popup(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, new_popup);
    struct wlr_xdg_popup *wlr_popup = data;

    _xdg_popup_create(wlr_popup, popup->popup_tree, popup->shell_tree, popup->use_usable_area);
}

static void popup_unconstrain(struct xdg_popup *popup)
{
    /* TODO: popup unconstrain output, add input_manager_get_last_seat */
    struct output *output = input_current_output(input_manager_get_default_seat());
    struct kywc_box *output_box = popup->use_usable_area ? &output->usable_area : &output->geometry;

    int lx = 0, ly = 0;
    ky_scene_node_coords(&popup->shell_tree->node, &lx, &ly);

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
        ky_scene_decoration_set_surface_size(popup->deco, width, height);
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
        struct ky_scene_buffer *buffer =
            ky_scene_buffer_try_from_surface(xdg_popup->wlr_xdg_popup->base->surface);
        ky_scene_node_set_radius(&buffer->node, (int[4]){ r, r, r, r });
        ky_scene_decoration_set_round_corner_radius(xdg_popup->deco, (int[4]){ r, r, r, r });
    }

    if ((flags >> BORDER) & 0x1) {
        float border_color[4];
        color_float_pa(border_color, theme->active_border_color);
        ky_scene_decoration_set_margin(xdg_popup->deco, 0, theme->border_width);
        ky_scene_decoration_set_margin_color(xdg_popup->deco, (float[4]){ 0, 0, 0, 0 },
                                             border_color);
    }

    if ((flags >> SHADOW) & 0x1) {
        struct theme_shadow_layer *shadow = &theme->menu_shadow_color.layers[0];
        float shadow_color[4];
        color_float_pa(shadow_color, shadow->color);
        ky_scene_decoration_set_shadow(xdg_popup->deco, shadow->off_x, shadow->off_y, shadow->blur,
                                       shadow_color);
    }

    int offset = (flags >> BORDER) & 0x1 ? -theme->border_width : 0;
    ky_scene_node_set_position(ky_scene_node_from_decoration(xdg_popup->deco), offset, offset);
}

static void create_popup_decoration(struct xdg_popup *xdg_popup)
{
    struct wlr_xdg_surface *xdg_surface = xdg_popup->wlr_xdg_popup->base;
    uint32_t flags = ukui_shell_get_surface_decoration_flags(xdg_surface->surface);
    if (flags == XDG_POPUP_NO_DECORATION) {
        return;
    }

    xdg_popup->deco = ky_scene_decoration_create(xdg_popup->popup_tree);
    theme_manager_add_update_listener(&xdg_popup->theme_update, false);

    struct ky_scene_node *node = ky_scene_node_from_decoration(xdg_popup->deco);
    ky_scene_node_lower_to_bottom(node);
    ky_scene_node_set_input_bypassed(node, true);

    struct wlr_box *geo = &xdg_surface->current.geometry;
    int width = geo->width, height = geo->height;
    if (width == 0 || height == 0) {
        width = xdg_surface->surface->current.width;
        height = xdg_surface->surface->current.height;
    }
    ky_scene_decoration_set_surface_size(xdg_popup->deco, width, height);
    ky_scene_decoration_set_mask(xdg_popup->deco, DECORATION_MASK_ALL);

    xdg_popup->decoration_flags = flags;
    xdg_popup_update_decoration(xdg_popup, flags);
}

static void handle_xdg_popup_map(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, map);
    struct wlr_seat *seat = popup->wlr_xdg_popup->seat;
    /* clear prev focused surface, skip the surface when it's the parent of popup */
    popup->topmost_clear_focus =
        popup->topmost_popup && seat && seat->keyboard_state.focused_surface &&
        seat->keyboard_state.focused_surface != popup->wlr_xdg_popup->parent;
    if (popup->topmost_clear_focus) {
        wlr_seat_keyboard_clear_focus(seat);
    }

    create_popup_decoration(popup);

    struct ky_scene_node *node = &popup->popup_tree->node;
    ky_scene_node_set_enabled(node, true);
    popup_add_fade_effect(node, FADE_IN, popup->topmost_popup, seat,
                          popup->wlr_xdg_popup->base->surface->current.scale);
}

static void handle_xdg_popup_unmap(struct wl_listener *listener, void *data)
{
    struct xdg_popup *popup = wl_container_of(listener, popup, unmap);
    struct wlr_seat *seat = popup->wlr_xdg_popup->seat;
    struct ky_scene_node *node = &popup->popup_tree->node;
    popup_add_fade_effect(node, FADE_OUT, popup->topmost_popup, seat,
                          popup->wlr_xdg_popup->base->surface->current.scale);
    ky_scene_node_set_enabled(node, false);
    if (popup->deco) {
        ky_scene_node_destroy(ky_scene_node_from_decoration(popup->deco));
        wl_list_remove(&popup->theme_update.link);
        wl_list_init(&popup->theme_update.link);
        popup->deco = NULL;
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

static struct xdg_popup *_xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup,
                                           struct ky_scene_tree *parent,
                                           struct ky_scene_tree *shell, bool use_usable_area)
{
    struct xdg_popup *popup = calloc(1, sizeof(struct xdg_popup));
    if (!popup) {
        return NULL;
    }

    popup->wlr_xdg_popup = wlr_xdg_popup;
    popup->parent_tree = parent;
    popup->shell_tree = shell;
    popup->use_usable_area = use_usable_area;

    /* add popup surface to parent tree, popup map and unmap is handled in scene */
    popup->popup_tree = ky_scene_xdg_surface_create(parent, wlr_xdg_popup->base);
    ky_scene_node_set_enabled(&popup->popup_tree->node, wlr_xdg_popup->base->surface->mapped);

    popup->destroy.notify = handle_xdg_popup_destroy;
    wl_signal_add(&wlr_xdg_popup->base->events.destroy, &popup->destroy);
    popup->new_popup.notify = popup_handle_new_xdg_popup;
    wl_signal_add(&wlr_xdg_popup->base->events.new_popup, &popup->new_popup);
    popup->commit.notify = handle_xdg_popup_commit;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.commit, &popup->commit);
    popup->reposition.notify = handle_xdg_popup_reposition;
    wl_signal_add(&wlr_xdg_popup->events.reposition, &popup->reposition);
    popup->map.notify = handle_xdg_popup_map;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.map, &popup->map);
    popup->unmap.notify = handle_xdg_popup_unmap;
    wl_signal_add(&wlr_xdg_popup->base->surface->events.unmap, &popup->unmap);
    popup->theme_update.notify = xdg_popup_handle_theme_update;
    wl_list_init(&popup->theme_update.link);

    return popup;
}

static bool xdg_popup_hover(struct seat *seat, struct ky_scene_node *node, double x, double y,
                            uint32_t time, bool first, bool hold, void *data)
{
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    if (first) {
        kywc_log(KYWC_DEBUG, "first hover surface %p (%f %f)", surface, x, y);
    }

    seat_notify_motion(seat, surface, time, x, y, first);
    return false;
}

static void xdg_popup_click(struct seat *seat, struct ky_scene_node *node, uint32_t button,
                            bool pressed, uint32_t time, enum click_state state, void *data)
{
    seat_notify_button(seat, time, button, pressed);
}

static void xdg_popup_leave(struct seat *seat, struct ky_scene_node *node, bool last, void *data)
{
    /* so surface will call set_cursor when enter again */
    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    seat_notify_leave(seat, surface);
}

static struct ky_scene_node *xdg_popup_get_root(void *data)
{
    struct xdg_popup *popup = data;
    return &popup->parent_tree->node;
}

static const struct input_event_node_impl xdg_popup_event_node_impl = {
    .hover = xdg_popup_hover,
    .click = xdg_popup_click,
    .leave = xdg_popup_leave,
};

void xdg_popup_create(struct wlr_xdg_popup *wlr_xdg_popup, struct ky_scene_tree *shell,
                      struct view_layer *layer, bool use_usable_area)
{
    struct ky_scene_tree *parent = ky_scene_tree_create(layer->tree);

    /* get shell layout coord, and set it to parent tree */
    int lx, ly;
    ky_scene_node_coords(&shell->node, &lx, &ly);
    ky_scene_node_set_position(&parent->node, lx, ly);

    struct xdg_popup *popup = _xdg_popup_create(wlr_xdg_popup, parent, shell, use_usable_area);
    popup->topmost_popup = true;

    input_event_node_create(&parent->node, &xdg_popup_event_node_impl, xdg_popup_get_root, NULL,
                            popup);
}
