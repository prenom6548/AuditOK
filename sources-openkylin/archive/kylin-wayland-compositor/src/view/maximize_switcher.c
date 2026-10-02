// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_output.h>

#include <kywc/binding.h>

#include "input/cursor.h"
#include "input/seat.h"
#include "output.h"
#include "painter.h"
#include "scene/decoration.h"
#include "theme.h"
#include "util/color.h"
#include "util/macros.h"
#include "view_p.h"
#include "widget/scaled_buffer.h"
#include "widget/widget.h"

#define SELECT_WIDTH_GAP (5)
#define ICON_OPACITY (0.4)
#define ITEM_HEIGHT (48)
#define MAX_DISPLAY_VIEW (25)
#define MIN_DISPLAY_VIEW (4)
#define MAX_WIDTH_RATIO (0.8)
#define MAX_HEIGHT_RATIO (0.8)
#define MIN_WIDTH_RATIO (0.2)
#define INSTANCE_BORDER (1)

struct item_view {
    struct wl_list link;

    struct view *view;
    struct widget *title_text;
    struct scene_tree *tree;
    struct scene_node *icon_node;
    struct scene_node *text_node;

    struct switcher_instance *instance;

    int text_width, text_height;
    char *text;

    struct wl_listener view_unmap;
};

struct switcher_instance {
    struct wl_list items;

    struct scene_tree *tree;
    struct scene_decoration *background;
    struct scene_decoration *select;

    struct item_view *active;

    struct seat *seat;
    struct seat_pointer_grab pointer_grab;
    struct seat_keyboard_grab keyboard_grab;
    struct seat_touch_grab touch_grab;

    int pending, current;
    int width, height;
    int max_width, max_height;
    /* restrict page display items when the height exceeds the maximum height of the background. */
    int item_limit, first_item_index, total_views;

    float icon_opacity;
    float scale;

    struct maximize_switcher *switcher;
    struct wl_listener configured_output;
};

static struct maximize_switcher {
    struct switcher_instance *instance;
    struct wl_listener server_destroy;
} *switcher = NULL;

static struct shortcut {
    char *keybind;
    char *desc;
    enum function_id func_id;
} shortcuts[] = {
    { "Alt+m", "maximized_window_traverse", FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS },
};

static void switcher_instance_destroy(struct switcher_instance *instance)
{
    instance->switcher->instance = NULL;

    struct item_view *item_view, *tmp;
    wl_list_for_each_safe(item_view, tmp, &instance->items, link) {
        wl_list_remove(&item_view->link);
        wl_list_remove(&item_view->view_unmap.link);
        scene_node_destroy(&item_view->tree->node);
        free(item_view->text);
        free(item_view);
    }

    wl_list_remove(&instance->configured_output.link);
    scene_node_set_enabled(&instance->tree->node, false);
    scene_node_destroy(&instance->tree->node);

    seat_end_pointer_grab(instance->seat, &instance->pointer_grab);
    seat_end_keyboard_grab(instance->seat, &instance->keyboard_grab);
    seat_end_touch_grab(instance->seat, &instance->touch_grab);

    free(instance);
}

static void set_icon_buffer(struct item_view *item_view, float opacity, float scale)
{
    struct wlr_buffer *buf = view_get_icon_buffer(item_view->view, scale);
    if (!buf) {
        return;
    }

    struct scene_buffer *buffer = scene_buffer_from_node(item_view->icon_node);
    if (buffer->buffer != buf) {
        scene_buffer_set_buffer(buffer, buf);
    }

    scene_buffer_set_opacity(buffer, opacity);
    struct theme *theme = theme_manager_get_theme();
    scene_buffer_set_dest_size(buffer, theme->icon_size, theme->icon_size);
}

static void set_select_item_view(struct switcher_instance *instance, struct item_view *current,
                                 struct item_view *new)
{
    if (current == new) {
        return;
    }
    if (current) {
        set_icon_buffer(current, instance->icon_opacity, instance->scale);
    }
    set_icon_buffer(new, 1, instance->scale);

    int new_x, new_y;
    int instance_x, instance_y;
    scene_node_coords(&new->tree->node, &new_x, &new_y);
    scene_node_coords(&instance->tree->node, &instance_x, &instance_y);
    scene_node_set_position(scene_node_from_decoration(instance->select), new_x - instance_x,
                            new_y - instance_y);
}

static void show_item_views(struct switcher_instance *instance)
{
    int pending = instance->pending;
    int first_index = instance->first_item_index;

    if (pending == instance->total_views) {
        pending = 0;
    } else if (pending < 0) {
        pending = instance->total_views - 1;
    }

    if (pending < first_index) {
        first_index = pending;
    } else if (pending >= first_index + instance->item_limit) {
        first_index = pending - instance->item_limit + 1;
    }

    if (first_index < 0) {
        first_index = 0;
    }
    if (first_index + instance->item_limit > instance->total_views) {
        first_index = instance->total_views - instance->item_limit;
        if (first_index < 0) {
            first_index = 0;
        }
    }

    instance->first_item_index = first_index;

    struct theme *theme = theme_manager_get_theme();
    int icon_width = theme->button_width;
    int select_width_gap = SELECT_WIDTH_GAP;

    int icon_x = theme->layout_is_right_to_left ? instance->width - select_width_gap - icon_width
                                                : select_width_gap;
    icon_x += (icon_width - theme->icon_size) / 2;
    int icon_y = (ITEM_HEIGHT - theme->icon_size) / 2;

    int text_offset_x = theme->layout_is_right_to_left ? 0 : icon_width + select_width_gap;

    int offset_y = instance->total_views < MIN_DISPLAY_VIEW
                       ? (instance->height - instance->total_views * ITEM_HEIGHT) / 2
                       : 0;

    int i = 0;
    struct item_view *item_view, *current_item = NULL, *pending_item = NULL;
    wl_list_for_each_reverse(item_view, &instance->items, link) {
        if (i == instance->current) {
            current_item = item_view;
        }
        if (i == pending) {
            pending_item = item_view;
        }
        /* skipped view */
        int start_i = i - first_index;
        if (start_i < 0 || start_i >= instance->item_limit) {
            scene_node_set_enabled(&item_view->tree->node, false);
            i++;
            continue;
        }

        scene_node_set_enabled(&item_view->tree->node, true);
        scene_node_set_position(&item_view->tree->node, 0, offset_y + start_i * ITEM_HEIGHT);
        /* set icon position */
        scene_node_set_position(item_view->icon_node, icon_x, icon_y);
        /* set text position */
        int text_x =
            text_offset_x +
            (instance->width - icon_width - item_view->text_width - select_width_gap * 2) / 2;
        int text_y = (ITEM_HEIGHT - item_view->text_height) / 2;
        scene_node_set_position(item_view->text_node, text_x, text_y);

        i++;
    }

    /* select */
    set_select_item_view(instance, current_item, pending_item);

    instance->pending = instance->current = pending;
    instance->active = pending_item;
}

static bool item_hover(struct seat *seat, struct scene_node *node, double x, double y,
                       uint32_t time, bool first, bool hold, void *data)
{
    return false;
}

static void item_leave(struct seat *seat, struct scene_node *node, bool last, void *data) {}

static void item_click(struct seat *seat, struct scene_node *node, uint32_t button, bool pressed,
                       uint32_t time, enum click_state state, void *data)
{
    /* do actions when released */
    if (pressed || button != BTN_LEFT) {
        return;
    }

    struct item_view *item = data;
    struct switcher_instance *instance = item->instance;
    int i = 0;

    struct item_view *item_view;
    wl_list_for_each_reverse(item_view, &instance->items, link) {
        if (item == item_view) {
            instance->pending = i;
            break;
        }

        i++;
    }

    show_item_views(instance);
}

static const struct input_event_node_impl item_impl = {
    .hover = item_hover,
    .leave = item_leave,
    .click = item_click,
};

static void pointer_grab_cancel(struct seat_pointer_grab *pointer_grab)
{
    switcher_instance_destroy(pointer_grab->data);
}

static bool pointer_grab_button(struct seat_pointer_grab *pointer_grab, uint32_t time,
                                uint32_t button, bool pressed)
{
    struct switcher_instance *instance = pointer_grab->data;
    struct seat *seat = pointer_grab->seat;

    /* check current hover node in the window menu tree */
    struct input_event_node *inode = input_event_node_from_node(seat->cursor->hover.node);
    struct scene_node *node = input_event_node_root(inode);
    if (node == &instance->tree->node) {
        inode->impl->click(seat, seat->cursor->hover.node, button, pressed, time, CLICK_STATE_NONE,
                           inode->data);
    } else if (pressed) {
        switcher_instance_destroy(pointer_grab->data);
    }

    return true;
}

static bool pointer_grab_motion(struct seat_pointer_grab *pointer_grab, uint32_t time, double lx,
                                double ly)
{
    return false;
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

static bool touch_grab_touch(struct seat_touch_grab *touch_grab, uint32_t time, bool down,
                             uint32_t serial)
{
    seat_feed_pointer_motion(touch_grab->seat, touch_grab->seat->cursor->lx,
                             touch_grab->seat->cursor->ly, true);
    struct switcher_instance *instance = touch_grab->data;
    return pointer_grab_button(&instance->pointer_grab, time, BTN_LEFT, down);
}

static bool touch_grab_motion(struct seat_touch_grab *touch_grab, uint32_t time, double lx,
                              double ly, uint32_t serial)
{
    return false;
}

static void touch_grab_cancel(struct seat_touch_grab *touch_grab)
{
    switcher_instance_destroy(touch_grab->data);
}

static const struct seat_touch_grab_interface touch_grab_impl = {
    .touch = touch_grab_touch,
    .motion = touch_grab_motion,
    .cancel = touch_grab_cancel,
};

static bool keyboard_grab_key(struct seat_keyboard_grab *keyboard_grab, struct keyboard *keyboard,
                              uint32_t time, uint32_t key, bool pressed, uint32_t modifiers)
{
    struct switcher_instance *instance = keyboard_grab->data;

    if (key == KEY_ESC) {
        switcher_instance_destroy(instance);
        return true;
    }

    if (!pressed) {
        if (key != KEY_LEFTALT && key != KEY_RIGHTALT) {
            return true;
        }

        if (instance->active->view) {
            kywc_view_activate(&instance->active->view->base);
            view_set_focus(instance->active->view, keyboard_grab->seat);
        }

        switcher_instance_destroy(instance);
        return true;
    }

    if (key == KEY_UP) {
        instance->pending--;
    } else if (key == KEY_M || key == KEY_DOWN) {
        instance->pending++;
    }

    show_item_views(instance);

    return true;
}

static void keyboard_grab_cancel(struct seat_keyboard_grab *keyboard_grab)
{
    switcher_instance_destroy(keyboard_grab->data);
}

static const struct seat_keyboard_grab_interface keyboard_grab_impl = {
    .key = keyboard_grab_key,
    .cancel = keyboard_grab_cancel,
};

static void update_title_text(struct item_view *item_view)
{
    struct theme *theme = theme_manager_get_theme();
    int select_width_gap = SELECT_WIDTH_GAP * 2;
    int max_width = item_view->instance->max_width - theme->button_width - select_width_gap;
    widget_set_text(item_view->title_text, item_view->text, JUSTIFY_CENTER,
                    item_view->view->base.minimized ? TEXT_ATTR_SLANT : TEXT_ATTR_NONE);

    widget_set_font(item_view->title_text, theme->font_name, theme->font_size);
    widget_set_front_color(item_view->title_text, theme->active_text_color);

    widget_set_max_size(item_view->title_text, max_width, ITEM_HEIGHT);
    widget_set_auto_resize(item_view->title_text, AUTO_RESIZE_ONLY);

    widget_set_enabled(item_view->title_text, true);
    widget_update(item_view->title_text, true);

    int text_width, text_height;
    widget_get_size(item_view->title_text, &text_width, &text_height);

    if (text_width > max_width) {
        item_view->instance->width = max_width + theme->button_width + select_width_gap;
    } else if (text_width > item_view->instance->width - theme->button_width - select_width_gap) {
        item_view->instance->width = text_width + theme->button_width + select_width_gap;
    }

    item_view->text_width = text_width;
    item_view->text_height = text_height;
}

static void update_buffer(struct scene_buffer *buffer, float scale, void *data)
{
    struct item_view *item_view = data;
    /* update scene_buffer with new buffer */
    set_icon_buffer(item_view, item_view->instance->icon_opacity, item_view->instance->scale);
}

static void destroy_buffer(struct scene_buffer *buffer, void *data)
{
    /* buffers are destroyed in theme */
}

static struct scene_node *item_get_root(void *data)
{
    struct item_view *item = data;
    return &item->instance->tree->node;
}

static void handle_configured_output(struct wl_listener *listener, void *data)
{
    struct switcher_instance *instance = wl_container_of(listener, instance, configured_output);
    switcher_instance_destroy(instance);
}

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct item_view *item_view = wl_container_of(listener, item_view, view_unmap);
    switcher_instance_destroy(item_view->instance);
}

static void paint_item_view(struct switcher_instance *instance)
{
    struct item_view *item_view;
    wl_list_for_each_reverse(item_view, &instance->items, link) {
        item_view->tree = scene_tree_create(instance->tree);
        input_event_node_create(&item_view->tree->node, &item_impl, item_get_root, NULL, item_view);
        /* create title */
        item_view->title_text = widget_create(item_view->tree);
        item_view->text_node = scene_node_from_widget(item_view->title_text);
        /* create icon */
        struct scene_buffer *buf = scaled_buffer_create(item_view->tree, instance->scale,
                                                        update_buffer, destroy_buffer, item_view);
        item_view->icon_node = &buf->node;
        /* update */
        set_icon_buffer(item_view, instance->icon_opacity, instance->scale);
        update_title_text(item_view);
    }
}

static void update_item_views(struct switcher_instance *instance)
{
    struct workspace *workspace = workspace_manager_get_current();
    instance->total_views = 0;

    struct view_proxy *view_proxy;
    wl_list_for_each(view_proxy, &workspace->view_proxies, workspace_link) {
        struct view *view = view_proxy->view;
        if (!view->base.mapped || (!view->base.maximized && !view->base.fullscreen)) {
            continue;
        }

        struct item_view *item_view = calloc(1, sizeof(*item_view));
        if (!item_view) {
            kywc_log(KYWC_ERROR, "falied to create item_view for %s", view->base.app_id);
            continue;
        }

        wl_list_insert(&instance->items, &item_view->link);

        item_view->view = view;
        item_view->instance = instance;

        if (view->base.minimized) {
            int len = strlen(view->base.title) + 3;
            item_view->text = malloc(len * sizeof(char));
            snprintf(item_view->text, len, "(%s)", view->base.title);
        } else {
            item_view->text = strdup(view->base.title);
        }

        item_view->view_unmap.notify = handle_view_unmap;
        wl_signal_add(&view->base.events.unmap, &item_view->view_unmap);

        instance->total_views++;
    }
}

static void scene_decoration_init(struct scene_decoration *dec, float *color, int width, int height,
                                  bool is_opacity, bool blur)
{
    struct theme *theme = theme_manager_get_theme();
    scene_decoration_set_margin(dec, 0, INSTANCE_BORDER);
    scene_decoration_set_mask(dec, DECORATION_MASK_ALL);
    scene_node_set_position(scene_node_from_decoration(dec), -INSTANCE_BORDER, -INSTANCE_BORDER);
    float border_color[4];
    color_float_pa(border_color, theme->active_border_color);
    scene_decoration_set_margin_color(dec, (float[4]){ 0, 0, 0, 0 }, border_color);

    if (blur) {
        scene_decoration_set_surface_blurred(dec, theme->opacity != 100);
    }
    float bg_color[4];
    if (is_opacity) {
        color_float_pax(bg_color, color, theme->opacity / 100.0);
    } else {
        color_float_pa(bg_color, color);
    }

    scene_decoration_set_surface_color(dec, bg_color);

    int r = theme->menu_radius;
    scene_decoration_set_round_corner_radius(dec, (int[4]){ r, r, r, r });
    scene_decoration_set_surface_size(dec, width, height);
}

static void switcher_instance_create(struct maximize_switcher *switcher)
{
    struct switcher_instance *instance = calloc(1, sizeof(*instance));
    if (!instance) {
        return;
    }

    wl_list_init(&instance->items);

    update_item_views(instance);
    if (instance->total_views == 0) {
        free(instance);
        return;
    }

    switcher->instance = instance;
    instance->switcher = switcher;

    instance->seat = input_manager_get_default_seat();
    instance->pointer_grab =
        (struct seat_pointer_grab){ &pointer_grab_impl, instance->seat, instance };
    seat_start_pointer_grab(instance->seat, &instance->pointer_grab);
    instance->keyboard_grab =
        (struct seat_keyboard_grab){ &keyboard_grab_impl, instance->seat, instance };
    seat_start_keyboard_grab(instance->seat, &instance->keyboard_grab);
    instance->touch_grab =
        (struct seat_touch_grab){ &touch_grab_impl, instance->seat, instance, 0 };
    seat_start_touch_grab(instance->seat, &instance->touch_grab);

    struct output *output = input_current_output(instance->seat);
    instance->scale = output->base.state.scale;
    instance->configured_output.notify = handle_configured_output;
    output_manager_add_configured_listener(&instance->configured_output);

    struct kywc_box *usable_area = &output->usable_area;
    instance->max_width = output->usable_area.width * MAX_WIDTH_RATIO;
    instance->max_height = output->usable_area.height * MAX_HEIGHT_RATIO;
    instance->width = output->usable_area.width * MIN_WIDTH_RATIO;

    instance->pending = 1;
    instance->current = -1;
    instance->icon_opacity = ICON_OPACITY;

    struct view_layer *layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    instance->tree = scene_tree_create(layer->tree);
    scene_node_set_enabled(&instance->tree->node, false);
    instance->background = scene_decoration_create(instance->tree);
    instance->select = scene_decoration_create(instance->tree);

    paint_item_view(instance);

    if (instance->total_views >= MAX_DISPLAY_VIEW) {
        instance->height = ITEM_HEIGHT * MAX_DISPLAY_VIEW;
    } else if (instance->total_views <= MIN_DISPLAY_VIEW) {
        instance->height = ITEM_HEIGHT * MIN_DISPLAY_VIEW;
    } else {
        instance->height = ITEM_HEIGHT * instance->total_views;
    }

    if (instance->height > instance->max_height) {
        instance->item_limit = instance->max_height / ITEM_HEIGHT;
        instance->height = instance->item_limit * ITEM_HEIGHT;
    } else {
        instance->item_limit = MAX_DISPLAY_VIEW;
    }

    int x = usable_area->x + usable_area->width / 2 - instance->width / 2;
    int y = usable_area->y + usable_area->height / 2 - instance->height / 2;
    scene_node_set_position(&instance->tree->node, x, y);
    scene_node_set_enabled(&instance->tree->node, true);

    struct theme *theme = theme_manager_get_theme();
    scene_decoration_init(instance->background, theme->active_bg_color, instance->width,
                          instance->height, true, true);
    scene_decoration_init(instance->select, theme->accent_color,
                          instance->width - 2 * INSTANCE_BORDER, ITEM_HEIGHT - 2 * INSTANCE_BORDER,
                          false, false);

    show_item_views(instance);
}

void maximize_switcher_show(void)
{
    if (!switcher->instance) {
        switcher_instance_create(switcher);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct maximize_switcher *switcher = wl_container_of(listener, switcher, server_destroy);
    wl_list_remove(&switcher->server_destroy.link);
    free(switcher);
}

bool maximize_switcher_create(struct view_manager *view_manager)
{
    switcher = calloc(1, sizeof(*switcher));
    if (!switcher) {
        return false;
    }

    switcher->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &switcher->server_destroy);

    for (size_t i = 0; i < ARRAY_SIZE(shortcuts); i++) {
        struct shortcut *shortcut = &shortcuts[i];
        struct key_binding *binding =
            kywc_key_binding_create(shortcut->keybind, shortcut->desc, shortcut->func_id, 0);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->func_id),
                                       function_manager_handle_key_binding, NULL)) {
            kywc_key_binding_destroy(binding);
        }
    }

    return true;
}
