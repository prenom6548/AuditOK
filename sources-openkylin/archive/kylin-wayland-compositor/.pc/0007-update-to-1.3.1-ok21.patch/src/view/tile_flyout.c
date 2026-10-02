// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_seat.h>

#include "effect/fade.h"
#include "input/cursor.h"
#include "output.h"
#include "scene/decoration.h"
#include "scene/linear_gradient.h"
#include "theme.h"
#include "util/color.h"
#include "util/macros.h"
#include "view_p.h"

#define FLYOUT_WIDTH (204)
#define FLYOUT_HEIGHT (80)
#define FLYOUT_GAP (8)
#define FLYOUT_LAYOUT_GAP (4)
#define FLYOUT_ITEM_GAP (2)
#define FLYOUT_BORDER (1)
#define FLYOUT_OFFSET (2)

enum tile_layout_mode {
    TILE_LAYOUT_MODE_HALF = 0,
    TILE_LAYOUT_MODE_QUARTER,
    TILE_LAYOUT_MODE_COUNT,
};

struct tile_flyout_item {
    struct wl_list link;
    struct tile_flyout_layout *layout;

    struct ky_scene_linear_gradient *gradient;
    enum kywc_tile tile_type;
};

struct tile_flyout_layout {
    struct wl_list link;
    struct tile_flyout *flyout;

    struct ky_scene_tree *tree;
    struct ky_scene_decoration *deco;
    struct wl_list items;

    enum tile_layout_mode mode;
};

struct tile_flyout {
    struct wl_list link;

    struct seat *seat;
    struct wl_listener seat_destroy;

    struct view *view;
    struct wl_listener view_unmap;

    struct ky_scene_tree *tree;
    struct ky_scene_decoration *deco;

    struct wl_list layouts;

    struct seat_pointer_grab pointer_grab;
    struct kywc_box box;

    struct wl_event_source *timer;
    bool timer_enabled, entered;
};

static struct tile_flyout_manager {
    struct wl_list flyouts;
    struct wl_listener theme_update;
    struct wl_listener server_destroy;
} *manager = NULL;

static void tile_flyout_item_set_gradient(struct tile_flyout_item *item,
                                          struct theme_gradient *gradient)
{
    struct ky_scene_linear_gradient *linear_gradient = item->gradient;
    ky_scene_linear_gradient_set_linear(linear_gradient, gradient->angle);

    float color[4];
    color_float_pa(color, gradient->background);
    ky_scene_linear_gradient_set_background_color(linear_gradient, color);
    color_float_pa(color, gradient->start);
    ky_scene_linear_gradient_set_color_stop_0(linear_gradient, 0, color);
    color_float_pa(color, gradient->stop);
    ky_scene_linear_gradient_set_color_stop_1(linear_gradient, 1, color);
}

static void tile_flyout_update(struct tile_flyout *flyout)
{
    struct theme *theme = theme_manager_get_theme();
    float null_color[4] = { 0, 0, 0, 0 };

    struct theme_shadow_layer *shadow = &theme->menu_shadow_color.layers[0];
    float shadow_color[4];
    color_float_pa(shadow_color, shadow->color);
    ky_scene_decoration_set_shadow(flyout->deco, shadow->off_x, shadow->off_y, shadow->blur,
                                   shadow_color);

    float border_color[4];
    color_float_pa(border_color, theme->active_border_color);
    ky_scene_decoration_set_margin_color(flyout->deco, null_color, border_color);

    float bg_color[4];
    color_float_pax(bg_color, theme->active_bg_color, theme->opacity / 100.0);
    ky_scene_decoration_set_surface_color(flyout->deco, bg_color);

    int r = theme->menu_radius;
    ky_scene_decoration_set_round_corner_radius(flyout->deco, (int[4]){ r, r, r, r });
    ky_scene_decoration_set_surface_blurred(flyout->deco, theme->opacity != 100);

    r = theme->normal_radius;
    struct tile_flyout_layout *layout;
    wl_list_for_each(layout, &flyout->layouts, link) {
        ky_scene_decoration_set_margin_color(layout->deco, null_color, border_color);
        ky_scene_decoration_set_round_corner_radius(layout->deco, (int[4]){ r, r, r, r });

        struct tile_flyout_item *item;
        wl_list_for_each(item, &layout->items, link) {
            tile_flyout_item_set_gradient(item, &theme->normal_state_color);
        }
    }
}

static void tile_flyout_set_timer_enabled(struct tile_flyout *flyout, bool enabled)
{
    if (flyout->timer_enabled == enabled) {
        return;
    }

    flyout->timer_enabled = enabled;
    wl_event_source_timer_update(flyout->timer, enabled ? 250 : 0);
}

static void tile_flyout_set_enabled(struct tile_flyout *flyout, bool enabled, bool force)
{
    if (flyout->tree->node.enabled == enabled) {
        return;
    }

    if (!enabled) {
        if (!force) {
            tile_flyout_set_timer_enabled(flyout, true);
            return;
        }

        tile_flyout_set_timer_enabled(flyout, false);
        seat_end_pointer_grab(flyout->seat, &flyout->pointer_grab);

        wl_list_remove(&flyout->view_unmap.link);
        wl_list_init(&flyout->view_unmap.link);

        struct cursor *cursor = flyout->seat->cursor;
        bool need_rebase = flyout->view == NULL;
        if (!need_rebase) {
            struct kywc_box *geo = &flyout->view->base.geometry;
            struct wlr_box box = { geo->x + flyout->box.x, geo->y + flyout->box.y,
                                   flyout->box.width, flyout->box.height };
            need_rebase = !wlr_box_contains_point(&box, cursor->lx, cursor->ly);
        }
        if (need_rebase) {
            cursor_rebase(flyout->seat->cursor);
        }

        flyout->view = NULL;
        flyout->entered = false;

        popup_add_fade_effect(&flyout->tree->node, FADE_OUT, true, false, 1.0);
        ky_scene_node_set_enabled(&flyout->tree->node, false);
        return;
    }

    if (!flyout->view) {
        return;
    }

    wl_list_remove(&flyout->view_unmap.link);
    wl_signal_add(&flyout->view->base.events.unmap, &flyout->view_unmap);
    seat_start_pointer_grab(flyout->seat, &flyout->pointer_grab);
    tile_flyout_set_timer_enabled(flyout, false);

    struct kywc_box *geo = &flyout->view->base.geometry;
    int lx = geo->x + flyout->box.x - (FLYOUT_WIDTH - flyout->box.width) / 2;
    int ly = geo->y + flyout->box.y + flyout->box.height - FLYOUT_OFFSET;

    struct kywc_output *output = kywc_output_at_point(lx, ly);
    geo = &output_from_kywc_output(output)->geometry;
    int min_x = geo->x, max_x = geo->x + geo->width - FLYOUT_WIDTH;
    lx = CLAMP(lx, min_x, max_x);

    ky_scene_node_set_position(&flyout->tree->node, lx, ly);
    ky_scene_node_raise_to_top(&flyout->tree->node);
    ky_scene_node_set_enabled(&flyout->tree->node, true);
    popup_add_fade_effect(&flyout->tree->node, FADE_IN, true, false, output->state.scale);
}

static bool tile_flyout_item_hover(struct seat *seat, struct ky_scene_node *node, double x,
                                   double y, uint32_t time, bool first, bool hold, void *data)
{
    if (!first) {
        return false;
    }

    struct tile_flyout_item *item = data;
    struct theme *theme = theme_manager_get_theme();
    tile_flyout_item_set_gradient(item, &theme->hover_state_color);

    return false;
}

static void tile_flyout_item_leave(struct seat *seat, struct ky_scene_node *node, bool last,
                                   void *data)
{
    struct tile_flyout_item *item = data;
    struct theme *theme = theme_manager_get_theme();
    tile_flyout_item_set_gradient(item, &theme->normal_state_color);
}

static void tile_flyout_item_click(struct seat *seat, struct ky_scene_node *node, uint32_t button,
                                   bool pressed, uint32_t time, enum click_state state, void *data)
{
    struct tile_flyout_item *item = data;
    struct theme *theme = theme_manager_get_theme();

    if (pressed) {
        tile_flyout_item_set_gradient(item, &theme->click_state_color);
        return;
    }

    struct view *view = item->layout->flyout->view;
    struct output *output = output_from_kywc_output(view->output);
    view_begin_tile_preview(view, seat, output, item->tile_type);

    tile_flyout_item_set_gradient(item, &theme->normal_state_color);
}

static struct ky_scene_node *get_tile_flyout_item_root(void *data)
{
    struct tile_flyout_item *item = data;
    return &item->layout->flyout->tree->node;
}

static const struct input_event_node_impl tile_flyout_item_impl = {
    .hover = tile_flyout_item_hover,
    .leave = tile_flyout_item_leave,
    .click = tile_flyout_item_click,
};

static struct tile_flyout_item *tile_flyout_item_create(struct tile_flyout_layout *layout,
                                                        int width, int height,
                                                        enum kywc_tile tile_type)
{
    struct tile_flyout_item *item = calloc(1, sizeof(*item));
    if (!item) {
        return NULL;
    }

    item->tile_type = tile_type;
    item->layout = layout;
    wl_list_insert(&layout->items, &item->link);

    /* color is set in tile_flyout_update */
    item->gradient =
        ky_scene_linear_gradient_create(layout->tree, width, height, (float[4]){ 0, 0, 0, 0 });
    struct ky_scene_node *node = ky_scene_node_from_linear_gradient(item->gradient);
    ky_scene_node_set_radius(node, (int[4]){ 4, 4, 4, 4 });
    input_event_node_create(node, &tile_flyout_item_impl, get_tile_flyout_item_root, NULL, item);

    return item;
}

static struct tile_flyout_layout *tile_flyout_layout_create(struct tile_flyout *flyout, int width,
                                                            int height, enum tile_layout_mode mode)
{
    struct tile_flyout_layout *layout = calloc(1, sizeof(*layout));
    if (!layout) {
        return NULL;
    }

    layout->mode = mode;
    wl_list_init(&layout->items);
    layout->flyout = flyout;
    wl_list_insert(&flyout->layouts, &layout->link);

    layout->tree = ky_scene_tree_create(flyout->tree);
    layout->deco = ky_scene_decoration_create(layout->tree);
    struct ky_scene_node *deco_node = ky_scene_node_from_decoration(layout->deco);
    ky_scene_node_set_input_bypassed(deco_node, true);

    ky_scene_decoration_set_mask(layout->deco, DECORATION_MASK_ALL);
    ky_scene_decoration_set_margin(layout->deco, 0, FLYOUT_BORDER);
    ky_scene_decoration_set_surface_size(layout->deco, width, height);

    int item_count = 0;
    int item_width = (width - 2 * FLYOUT_LAYOUT_GAP - FLYOUT_ITEM_GAP) / 2;
    int item_height = height - 2 * FLYOUT_LAYOUT_GAP;

    if (mode == TILE_LAYOUT_MODE_HALF) {
        item_count = 2;
    } else if (mode == TILE_LAYOUT_MODE_QUARTER) {
        item_count = 4;
        item_height = (item_height - FLYOUT_ITEM_GAP) / 2;
    }

    for (int i = 0; i < item_count; i++) {
        if (mode == TILE_LAYOUT_MODE_HALF) {
            struct tile_flyout_item *item =
                tile_flyout_item_create(layout, item_width, item_height, i + KYWC_TILE_LEFT);
            int x = FLYOUT_LAYOUT_GAP + i * (item_width + FLYOUT_ITEM_GAP) + FLYOUT_BORDER;
            int y = FLYOUT_LAYOUT_GAP + FLYOUT_BORDER;
            ky_scene_node_set_position(ky_scene_node_from_linear_gradient(item->gradient), x, y);
        } else if (mode == TILE_LAYOUT_MODE_QUARTER) {
            struct tile_flyout_item *item =
                tile_flyout_item_create(layout, item_width, item_height, i + KYWC_TILE_TOP_LEFT);
            int x = FLYOUT_LAYOUT_GAP + i / 2 * (item_width + FLYOUT_ITEM_GAP) + FLYOUT_BORDER;
            int y = FLYOUT_LAYOUT_GAP + i % 2 * (item_height + FLYOUT_ITEM_GAP) + FLYOUT_BORDER;
            ky_scene_node_set_position(ky_scene_node_from_linear_gradient(item->gradient), x, y);
        }
    }

    return layout;
}

static void pointer_grab_cancel(struct seat_pointer_grab *pointer_grab)
{
    struct tile_flyout *flyout = pointer_grab->data;
    tile_flyout_set_enabled(flyout, false, true);
}

static bool pointer_grab_button(struct seat_pointer_grab *pointer_grab, uint32_t time,
                                uint32_t button, bool pressed)
{
    struct tile_flyout *flyout = pointer_grab->data;
    struct seat *seat = pointer_grab->seat;

    // support click in flyout->box
    struct input_event_node *inode = input_event_node_from_node(seat->cursor->hover.node);
    struct ky_scene_node *node = input_event_node_root(inode);
    if (node == &flyout->tree->node) {
        inode->impl->click(seat, seat->cursor->hover.node, button, pressed, time, CLICK_STATE_NONE,
                           inode->data);
    } else if (pressed) {
        tile_flyout_set_enabled(flyout, false, true);
    }
    return true;
}

static bool pointer_grab_motion(struct seat_pointer_grab *pointer_grab, uint32_t time, double lx,
                                double ly)
{
    struct tile_flyout *flyout = pointer_grab->data;
    struct seat *seat = pointer_grab->seat;

    struct kywc_box *geo = &flyout->view->base.geometry;
    struct wlr_box box = { geo->x + flyout->box.x, geo->y + flyout->box.y, flyout->box.width,
                           flyout->box.height };

    struct ky_scene_node *hover = ky_scene_node_at(&seat->scene->tree.node, lx, ly, NULL, NULL);
    struct input_event_node *inode = input_event_node_from_node(hover);
    struct ky_scene_node *node = input_event_node_root(inode);

    bool hoverd = node == &flyout->tree->node;
    if (!flyout->entered && hoverd) {
        flyout->entered = true;
    }

    if (hoverd || (!flyout->entered && wlr_box_contains_point(&box, lx, ly))) {
        // leak to cursor motion to generate leave event
        tile_flyout_set_timer_enabled(flyout, false);
        return false;
    }

    tile_flyout_set_enabled(flyout, false, false);
    return true;
}

static bool pointer_grab_axis(struct seat_pointer_grab *pointer_grab, uint32_t time, bool vertical,
                              double value)
{
    return true;
}

static const struct seat_pointer_grab_interface pointer_grab_impl = {
    .motion = pointer_grab_motion,
    .button = pointer_grab_button,
    .axis = pointer_grab_axis,
    .cancel = pointer_grab_cancel,
};

static void flyout_handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct tile_flyout *flyout = wl_container_of(listener, flyout, view_unmap);
    flyout->view = NULL;
    tile_flyout_set_enabled(flyout, false, true);
}

static void flyout_handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct tile_flyout *flyout = wl_container_of(listener, flyout, seat_destroy);
    wl_list_remove(&flyout->seat_destroy.link);
    wl_list_remove(&flyout->view_unmap.link);
    wl_list_remove(&flyout->link);

    struct tile_flyout_layout *layout, *layout_tmp;
    wl_list_for_each_safe(layout, layout_tmp, &flyout->layouts, link) {
        struct tile_flyout_item *item, *item_tmp;
        wl_list_for_each_safe(item, item_tmp, &layout->items, link) {
            ky_scene_node_destroy(ky_scene_node_from_linear_gradient(item->gradient));
            free(item);
        }
        ky_scene_node_destroy(&layout->tree->node);
        free(layout);
    }
    ky_scene_node_destroy(&flyout->tree->node);

    seat_end_pointer_grab(flyout->seat, &flyout->pointer_grab);
    wl_event_source_remove(flyout->timer);
    free(flyout);
}

static bool tile_flyout_hover(struct seat *seat, struct ky_scene_node *node, double x, double y,
                              uint32_t time, bool first, bool hold, void *data)
{
    return false;
}

static void tile_flyout_leave(struct seat *seat, struct ky_scene_node *node, bool last, void *data)
{
    // do nothing
}

static void tile_flyout_click(struct seat *seat, struct ky_scene_node *node, uint32_t button,
                              bool pressed, uint32_t time, enum click_state state, void *data)
{
    // do nothing
}

static struct ky_scene_node *get_tile_flyout_root(void *data)
{
    struct tile_flyout *flyout = data;
    return &flyout->tree->node;
}

static const struct input_event_node_impl tile_flyout_impl = {
    .hover = tile_flyout_hover,
    .leave = tile_flyout_leave,
    .click = tile_flyout_click,
};

static int handle_timeout(void *data)
{
    struct tile_flyout *flyout = data;
    tile_flyout_set_enabled(flyout, false, true);
    return 0;
}

static struct tile_flyout *tile_flyout_create(struct seat *seat)
{
    struct tile_flyout *flyout = calloc(1, sizeof(*flyout));
    if (!flyout) {
        return NULL;
    }

    flyout->seat = seat;
    flyout->seat_destroy.notify = flyout_handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &flyout->seat_destroy);
    flyout->view_unmap.notify = flyout_handle_view_unmap;
    wl_list_init(&flyout->view_unmap.link);

    wl_list_init(&flyout->layouts);
    wl_list_insert(&manager->flyouts, &flyout->link);

    struct wl_event_loop *loop = wl_display_get_event_loop(seat->wlr_seat->display);
    flyout->timer = wl_event_loop_add_timer(loop, handle_timeout, flyout);

    flyout->pointer_grab.seat = seat;
    flyout->pointer_grab.data = flyout;
    flyout->pointer_grab.interface = &pointer_grab_impl;

    struct view_layer *layer = view_manager_get_layer(LAYER_POPUP, false);
    flyout->tree = ky_scene_tree_create(layer->tree);
    ky_scene_node_set_enabled(&flyout->tree->node, false);

    flyout->deco = ky_scene_decoration_create(flyout->tree);
    ky_scene_decoration_set_mask(flyout->deco, DECORATION_MASK_ALL);
    ky_scene_decoration_set_margin(flyout->deco, 0, FLYOUT_BORDER);
    ky_scene_decoration_set_surface_size(flyout->deco, FLYOUT_WIDTH, FLYOUT_HEIGHT);

    struct ky_scene_node *deco_node = ky_scene_node_from_decoration(flyout->deco);
    input_event_node_create(deco_node, &tile_flyout_impl, &get_tile_flyout_root, NULL, flyout);

    int layout_width = (FLYOUT_WIDTH - 3 * FLYOUT_GAP) / TILE_LAYOUT_MODE_COUNT;
    int layout_height = FLYOUT_HEIGHT - 2 * FLYOUT_GAP;

    for (int i = 0; i < TILE_LAYOUT_MODE_COUNT; i++) {
        struct tile_flyout_layout *layout = tile_flyout_layout_create(
            flyout, layout_width, layout_height, i + TILE_LAYOUT_MODE_HALF);
        int x = FLYOUT_GAP + i * (layout_width + FLYOUT_GAP);
        int y = FLYOUT_GAP;
        ky_scene_node_set_position(&layout->tree->node, x, y);
    }

    tile_flyout_update(flyout);

    return flyout;
}

static struct tile_flyout *tile_flyout_get_or_create(struct seat *seat)
{
    struct tile_flyout *flyout;
    wl_list_for_each(flyout, &manager->flyouts, link) {
        if (flyout->seat == seat) {
            return flyout;
        }
    }
    return tile_flyout_create(seat);
}

void tile_flyout_show(struct view *view, struct seat *seat, struct kywc_box *box)
{
    if (!manager) {
        return;
    }

    struct tile_flyout *flyout = tile_flyout_get_or_create(seat);
    if (!flyout) {
        return;
    }

    /* disable current flyout if needed */
    tile_flyout_set_enabled(flyout, false, true);
    flyout->view = view;
    flyout->box = *box;
    tile_flyout_set_enabled(flyout, true, true);
}

static void handle_theme_update(struct wl_listener *listener, void *data)
{
    struct theme_update_event *update_event = data;
    uint32_t allowed_mask = THEME_UPDATE_MASK_STATE_COLOR | THEME_UPDATE_MASK_OPACITY |
                            THEME_UPDATE_MASK_BORDER_COLOR | THEME_UPDATE_MASK_BACKGROUND_COLOR |
                            THEME_UPDATE_MASK_CORNER_RADIUS | THEME_UPDATE_MASK_SHADOW_COLOR;
    if (!(update_event->update_mask & allowed_mask)) {
        return;
    }

    struct tile_flyout *flyout;
    wl_list_for_each(flyout, &manager->flyouts, link) {
        tile_flyout_update(flyout);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->theme_update.link);
    free(manager);
}

bool tile_flyout_manager_create(struct view_manager *view_manager)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->flyouts);

    manager->theme_update.notify = handle_theme_update;
    theme_manager_add_update_listener(&manager->theme_update, false);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &manager->server_destroy);

    return true;
}
