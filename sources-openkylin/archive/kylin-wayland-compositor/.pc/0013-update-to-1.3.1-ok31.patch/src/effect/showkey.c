// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>
#include <stdlib.h>

#include <cairo.h>
#include <wlr/types/wlr_seat.h>

#include <kywc/log.h>
#include <kywc/output.h>

#include "effect_p.h"
#include "painter.h"
#include "scene/scene.h"
#include "theme.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "view/view.h"
#include "widget/scaled_buffer.h"

#define KEYBOARD_WIDTH (1500)
#define KEYBOARD_HEIGHT (500)
#define KEY_GAP (20)
#define BUTTON_ALPHA (0.85)

#define KEY_BUTTON_WIDTH (60)
#define KEY_BUTTON_HEIGHT (40)

#define ROW_DISPLAY_TIME (150)
#define NO_KEY_WAITING_TIME (1500)

#define MIN_BACKGROUND_WIDTH (50)
#define MIN_BACKGROUND_HEIGHT (40)

#define DEFAULT_FONT_SIZE (18)
#define DEFAULT_FONT_FACE "Sans"

static float background_color[4] = { 0, 0, 0, 0 };

struct key_unit {
    const struct keycode_map *key;
    uint64_t last_press_time;
    int row_index;
    int pos_x, pos_y;
};

struct showkey_seat {
    struct wl_list link;
    struct showkey *showkey;

    struct seat *seat;
    struct wl_listener seat_destroy;
    struct wl_listener keyboard_key;
};

struct showkey {
    struct showkey_effect *effect;

    struct scene_tree *tree;
    struct scene_rect *rect;
    struct kywc_box rect_box;
    struct key_unit key_unit;

    struct wl_list seats;
    struct wl_listener new_seat;
    struct wl_event_source *timer;
};

struct showkey_effect {
    struct effect *effect;
    struct wl_listener enable;
    struct wl_listener disable;
    struct wl_listener destroy;

    struct showkey *showkey;
    struct wl_list scaled_buffers;
    struct wl_listener theme_update;

    struct server *server;
    struct dbus_object *dbus;
    struct wl_listener display_destroy;
};

static struct keycode_map {
    const char *key;
    uint32_t code;
    int index;
    int width;
    int height;
} keycode_maps[] = {
    { .key = "Esc", .code = KEY_ESC, .width = 50, .height = 40 },
    { .key = "1", .code = KEY_1, .width = 50, .height = 40 },
    { .key = "2", .code = KEY_2, .width = 50, .height = 40 },
    { .key = "3", .code = KEY_3, .width = 50, .height = 40 },
    { .key = "4", .code = KEY_4, .width = 50, .height = 40 },
    { .key = "5", .code = KEY_5, .width = 50, .height = 40 },
    { .key = "6", .code = KEY_6, .width = 50, .height = 40 },
    { .key = "7", .code = KEY_7, .width = 50, .height = 40 },
    { .key = "8", .code = KEY_8, .width = 50, .height = 40 },
    { .key = "9", .code = KEY_9, .width = 50, .height = 40 },
    { .key = "0", .code = KEY_0, .width = 50, .height = 40 },
    { .key = "-", .code = KEY_MINUS, .width = 50, .height = 40 },
    { .key = "=", .code = KEY_EQUAL, .width = 50, .height = 40 },
    { .key = "Back", .code = KEY_BACKSPACE, .width = 50, .height = 40 },
    { .key = "Tab", .code = KEY_TAB, .width = 50, .height = 40 },
    { .key = "Q", .code = KEY_Q, .width = 50, .height = 40 },
    { .key = "W", .code = KEY_W, .width = 50, .height = 40 },
    { .key = "E", .code = KEY_E, .width = 50, .height = 40 },
    { .key = "R", .code = KEY_R, .width = 50, .height = 40 },
    { .key = "T", .code = KEY_T, .width = 50, .height = 40 },
    { .key = "Y", .code = KEY_Y, .width = 50, .height = 40 },
    { .key = "U", .code = KEY_U, .width = 50, .height = 40 },
    { .key = "I", .code = KEY_I, .width = 50, .height = 40 },
    { .key = "O", .code = KEY_O, .width = 50, .height = 40 },
    { .key = "P", .code = KEY_P, .width = 50, .height = 40 },
    { .key = "{", .code = KEY_LEFTBRACE, .width = 50, .height = 40 },
    { .key = "}", .code = KEY_RIGHTBRACE, .width = 50, .height = 40 },
    { .key = "Enter", .code = KEY_ENTER, .width = 50, .height = 40 },
    { .key = "Ctrl", .code = KEY_LEFTCTRL, .width = 50, .height = 40 },
    { .key = "A", .code = KEY_A, .width = 50, .height = 40 },
    { .key = "S", .code = KEY_S, .width = 50, .height = 40 },
    { .key = "D", .code = KEY_D, .width = 50, .height = 40 },
    { .key = "F", .code = KEY_F, .width = 50, .height = 40 },
    { .key = "G", .code = KEY_G, .width = 50, .height = 40 },
    { .key = "H", .code = KEY_H, .width = 50, .height = 40 },
    { .key = "J", .code = KEY_J, .width = 50, .height = 40 },
    { .key = "K", .code = KEY_K, .width = 50, .height = 40 },
    { .key = "L", .code = KEY_L, .width = 50, .height = 40 },
    { .key = ";", .code = KEY_SEMICOLON, .width = 50, .height = 40 },
    { .key = "'", .code = KEY_APOSTROPHE, .width = 50, .height = 40 },
    { .key = "`", .code = KEY_GRAVE, .width = 50, .height = 40 },
    { .key = "Shift", .code = KEY_LEFTSHIFT, .width = 50, .height = 40 },
    { .key = "\\", .code = KEY_BACKSLASH, .width = 50, .height = 40 },
    { .key = "Z", .code = KEY_Z, .width = 50, .height = 40 },
    { .key = "X", .code = KEY_X, .width = 50, .height = 40 },
    { .key = "C", .code = KEY_C, .width = 50, .height = 40 },
    { .key = "V", .code = KEY_V, .width = 50, .height = 40 },
    { .key = "B", .code = KEY_B, .width = 50, .height = 40 },
    { .key = "N", .code = KEY_N, .width = 50, .height = 40 },
    { .key = "M", .code = KEY_M, .width = 50, .height = 40 },
    { .key = ",", .code = KEY_COMMA, .width = 50, .height = 40 },
    { .key = ".", .code = KEY_DOT, .width = 50, .height = 40 },
    { .key = "/", .code = KEY_SLASH, .width = 50, .height = 40 },
    { .key = "Shift", .code = KEY_RIGHTSHIFT, .width = 50, .height = 40 },
    { .key = "*", .code = KEY_KPASTERISK, .width = 50, .height = 40 },
    { .key = "Alt", .code = KEY_LEFTALT, .width = 50, .height = 40 },
    { .key = "Space", .code = KEY_SPACE, .width = 50, .height = 40 },
    { .key = "Caps", .code = KEY_CAPSLOCK, .width = 50, .height = 40 },
    { .key = "F1", .code = KEY_F1, .width = 50, .height = 40 },
    { .key = "F2", .code = KEY_F2, .width = 50, .height = 40 },
    { .key = "F3", .code = KEY_F3, .width = 50, .height = 40 },
    { .key = "F4", .code = KEY_F4, .width = 50, .height = 40 },
    { .key = "F5", .code = KEY_F5, .width = 50, .height = 40 },
    { .key = "F6", .code = KEY_F6, .width = 50, .height = 40 },
    { .key = "F7", .code = KEY_F7, .width = 50, .height = 40 },
    { .key = "F8", .code = KEY_F8, .width = 50, .height = 40 },
    { .key = "F9", .code = KEY_F9, .width = 50, .height = 40 },
    { .key = "F10", .code = KEY_F10, .width = 50, .height = 40 },
    { .key = "Num", .code = KEY_NUMLOCK, .width = 50, .height = 40 },
    { .key = "SL", .code = KEY_SCROLLLOCK, .width = 50, .height = 40 },
    { .key = "7", .code = KEY_KP7, .width = 50, .height = 40 },
    { .key = "8", .code = KEY_KP8, .width = 50, .height = 40 },
    { .key = "9", .code = KEY_KP9, .width = 50, .height = 40 },
    { .key = "-", .code = KEY_KPMINUS, .width = 50, .height = 40 },
    { .key = "4", .code = KEY_KP4, .width = 50, .height = 40 },
    { .key = "5", .code = KEY_KP5, .width = 50, .height = 40 },
    { .key = "6", .code = KEY_KP6, .width = 50, .height = 40 },
    { .key = "+", .code = KEY_KPPLUS, .width = 50, .height = 40 },
    { .key = "1", .code = KEY_KP1, .width = 50, .height = 40 },
    { .key = "2", .code = KEY_KP2, .width = 50, .height = 40 },
    { .key = "3", .code = KEY_KP3, .width = 50, .height = 40 },
    { .key = "0", .code = KEY_KP0, .width = 50, .height = 40 },
    { .key = ".", .code = KEY_KPDOT, .width = 50, .height = 40 },
    { .key = "F11", .code = KEY_F11, .width = 50, .height = 40 },
    { .key = "F12", .code = KEY_F12, .width = 50, .height = 40 },
    { .key = "Enter", .code = KEY_KPENTER, .width = 50, .height = 40 },
    { .key = "Ctrl", .code = KEY_RIGHTCTRL, .width = 50, .height = 40 },
    { .key = "/", .code = KEY_KPSLASH, .width = 50, .height = 40 },
    { .key = "PS", .code = KEY_SYSRQ, .width = 50, .height = 40 },
    { .key = "Alt", .code = KEY_RIGHTALT, .width = 50, .height = 40 },
    { .key = "HM", .code = KEY_HOME, .width = 50, .height = 40 },
    { .key = "↑", .code = KEY_UP, .width = 50, .height = 40 },
    { .key = "PU", .code = KEY_PAGEUP, .width = 50, .height = 40 },
    { .key = "←", .code = KEY_LEFT, .width = 50, .height = 40 },
    { .key = "→", .code = KEY_RIGHT, .width = 50, .height = 40 },
    { .key = "End", .code = KEY_END, .width = 50, .height = 40 },
    { .key = "↓", .code = KEY_DOWN, .width = 50, .height = 40 },
    { .key = "PD", .code = KEY_PAGEDOWN, .width = 50, .height = 40 },
    { .key = "Ins", .code = KEY_INSERT, .width = 50, .height = 40 },
    { .key = "Del", .code = KEY_DELETE, .width = 50, .height = 40 },
    { .key = "PB", .code = KEY_PAUSE, .width = 50, .height = 40 },
    { .key = "Win", .code = KEY_LEFTMETA, .width = 50, .height = 40 },
    { .key = "Win", .code = KEY_RIGHTMETA, .width = 50, .height = 40 },
    { .key = "Menu", .code = KEY_COMPOSE, .width = 50, .height = 40 },
};

struct keyboard_buffer {
    struct wl_list link;
    struct wlr_buffer *buffer;
    struct wlr_fbox src_boxs[ARRAY_SIZE(keycode_maps)];
    float scale;
};

static int compare_keycode_map(const void *p1, const void *p2)
{
    uint32_t code1 = ((struct keycode_map *)p1)->code;
    uint32_t code2 = ((struct keycode_map *)p2)->code;

    return (code1 > code2) ? 1 : ((code1 == code2) ? 0 : -1);
}

static bool find_keycode_map(uint32_t code, struct keycode_map **result)
{
    struct keycode_map key = { .code = code };
    /* binary search for the keycode map */
    *result = bsearch(&key, keycode_maps, ARRAY_SIZE(keycode_maps), sizeof(struct keycode_map),
                      compare_keycode_map);
    return (*result != NULL);
}

static void showkey_destroy_display_tree(struct showkey *showkey)
{
    if (!showkey->tree) {
        return;
    }

    showkey->key_unit.pos_x = 0;
    showkey->key_unit.pos_y = 0;
    showkey->key_unit.last_press_time = 0;
    showkey->key_unit.row_index = 0;
    showkey->key_unit.key = NULL;
    scene_node_destroy(&showkey->tree->node);
    showkey->tree = NULL;
}

static void showkey_create_display_tree(struct showkey *showkey)
{
    struct view_layer *layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    showkey->tree = scene_tree_create(layer->tree);
    scene_node_set_position(&showkey->tree->node, showkey->rect_box.x, showkey->rect_box.y);
    scene_node_set_input_bypassed(&showkey->tree->node, true);

    showkey->rect = scene_rect_create(showkey->tree, showkey->rect_box.width,
                                      showkey->rect_box.height, background_color);
}

static bool set_keyboard_timer(struct showkey *showkey, struct seat_keyboard_key_event *event)
{
    if (!event->pressed) {
        if (wl_event_source_timer_update(showkey->timer,
                                         effect_manager_scale_time(NO_KEY_WAITING_TIME)) < 0) {
            kywc_log(KYWC_DEBUG, "Failed to set key timer");
        }
        return false;
    }

    wl_event_source_timer_update(showkey->timer, 0);
    return true;
}

static void get_keyboard_buffer_color(double **background, double **text, double **button)
{
    static double button_color[4];
    static double text_color[4];
    static double light_background[] = { 1.0, 1.0, 1.0, 0 };
    static double dark_background[] = { 0.0, 0.0, 0.0, 0 };
    struct theme *theme = theme_manager_get_theme();
    text_color[0] = theme->active_text_color[0];
    text_color[1] = theme->active_text_color[1];
    text_color[2] = theme->active_text_color[2];
    text_color[3] = theme->active_text_color[3];
    *text = text_color;

    if (theme->type == THEME_TYPE_LIGHT || theme->type == THEME_TYPE_UNDEFINED) {
        button_color[0] = light_background[0];
        button_color[1] = light_background[1];
        button_color[2] = light_background[2];
        button_color[3] = BUTTON_ALPHA;
        *background = light_background;
    } else {
        button_color[0] = dark_background[0];
        button_color[1] = dark_background[1];
        button_color[2] = dark_background[2];
        button_color[3] = BUTTON_ALPHA;
        *background = dark_background;
    }
    *button = button_color;
}

static void draw_keyboard_key(cairo_t *cairo_context, int pos_x, int pos_y, int width, int height,
                              const char *label, double *text_color, double *button_color,
                              float scale)
{
    cairo_set_source_rgba(cairo_context, button_color[0], button_color[1], button_color[2],
                          button_color[3]);
    cairo_rectangle(cairo_context, pos_x, pos_y, width, height);
    cairo_fill(cairo_context);

    // draw button labels
    if (strcmp(label, "") != 0) {
        cairo_select_font_face(cairo_context, DEFAULT_FONT_FACE, CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
        double font_size = round(DEFAULT_FONT_SIZE * scale);
        cairo_set_font_size(cairo_context, font_size);
        cairo_text_extents_t extents;
        cairo_text_extents(cairo_context, label, &extents);
        double text_x = round(pos_x + (width - extents.width) / 2 - extents.x_bearing);
        double text_y = round(pos_y + (height - extents.height) / 2 - extents.y_bearing);
        cairo_move_to(cairo_context, text_x, text_y);
        cairo_set_source_rgba(cairo_context, text_color[0], text_color[1], text_color[2],
                              text_color[3]);
        cairo_show_text(cairo_context, label);
    }
}

static struct keyboard_buffer *draw_keyboard_buffer(struct showkey_effect *effect, float scale)
{
    double *background_color, *text_color, *button_color;
    get_keyboard_buffer_color(&background_color, &text_color, &button_color);
    // wlr_buffer
    struct keyboard_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    struct wlr_buffer *keyboard_buffer =
        painter_create_buffer(KEYBOARD_WIDTH, KEYBOARD_HEIGHT, scale);
    if (!keyboard_buffer) {
        free(buffer);
        return NULL;
    }

    buffer->scale = scale;
    buffer->buffer = keyboard_buffer;
    wl_list_insert(&effect->scaled_buffers, &buffer->link);

    void *data = NULL;
    uint32_t drm_format;
    size_t stride;
    if (!wlr_buffer_begin_data_ptr_access(keyboard_buffer, WLR_BUFFER_DATA_PTR_ACCESS_WRITE, &data,
                                          &drm_format, &stride)) {
        wlr_buffer_drop(keyboard_buffer);
        free(buffer);
        return NULL;
    }

    memset(data, 0x0, keyboard_buffer->height * stride);

    int scaled_width = ceil(KEYBOARD_WIDTH * scale);
    int scaled_height = ceil(KEYBOARD_HEIGHT * scale);
    // cairo surface
    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        data, CAIRO_FORMAT_ARGB32, scaled_width, scaled_height, stride);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        wlr_buffer_end_data_ptr_access(keyboard_buffer);
        wlr_buffer_drop(keyboard_buffer);
        free(buffer);
        return NULL;
    }

    // cairo context
    cairo_t *cairo_context = cairo_create(surface);
    if (cairo_status(cairo_context) != CAIRO_STATUS_SUCCESS) {
        wlr_buffer_end_data_ptr_access(keyboard_buffer);
        cairo_surface_destroy(surface);
        wlr_buffer_drop(keyboard_buffer);
        free(buffer);
        return NULL;
    }

    // set background color
    cairo_set_source_rgba(cairo_context, background_color[0], background_color[1],
                          background_color[2], background_color[3]);
    cairo_paint(cairo_context);

    // button size
    int key_width = ceil(KEY_BUTTON_WIDTH * scale);
    int key_height = ceil(KEY_BUTTON_HEIGHT * scale);
    int padding_x = 10;
    int padding_y = 10;

    // draw key
    int row = 0, col = 0;
    for (size_t i = 0; i < ARRAY_SIZE(keycode_maps); i++) {
        struct keycode_map *map = &keycode_maps[i];

        // line break when exceeding width
        if (col > 0 && padding_x + col * (key_width + padding_x) + key_width > scaled_width) {
            col = 0;
            row++;
        }

        int pos_x = ceil(padding_x + col * (key_width + padding_x));
        int pos_y = ceil(padding_y + row * (key_height + padding_y));

        map->index = i;
        buffer->src_boxs[i] = (struct wlr_fbox){ pos_x, pos_y, key_width, key_height };

        draw_keyboard_key(cairo_context, pos_x, pos_y, key_width, key_height, map->key, text_color,
                          button_color, scale);
        col++;
    }

    cairo_destroy(cairo_context);
    cairo_surface_destroy(surface);

    wlr_buffer_end_data_ptr_access(keyboard_buffer);

    return buffer;
}

static struct keyboard_buffer *keyboard_buffer_get_or_create(struct showkey_effect *effect,
                                                             float scale)
{
    /* find scale buffer */
    struct keyboard_buffer *buffer;
    wl_list_for_each(buffer, &effect->scaled_buffers, link) {
        if (buffer->scale == scale) {
            return buffer;
        }
    }

    return draw_keyboard_buffer(effect, scale);
}

static void showkey_update_key_position(struct showkey *showkey,
                                        struct seat_keyboard_key_event *event)
{
    uint32_t new_row_time = effect_manager_scale_time(ROW_DISPLAY_TIME);
    struct key_unit *key_unit = &showkey->key_unit;
    int key_width = key_unit->key->width;
    int key_height = key_unit->key->height;
    bool need_new_row = (key_unit->last_press_time == 0) ||
                        (event->time_msec - key_unit->last_press_time > new_row_time) ||
                        (key_unit->pos_x + key_width + KEY_GAP > showkey->rect_box.width);
    if (need_new_row) {
        key_unit->pos_x = 0;
        key_unit->row_index++;
        key_unit->pos_y = showkey->rect_box.height - key_unit->row_index * (key_height + KEY_GAP);
        if (key_unit->pos_y < 0) {
            const struct keycode_map *saved_key = showkey->key_unit.key;
            showkey_destroy_display_tree(showkey);
            showkey->key_unit.key = saved_key;
            showkey->key_unit.row_index = 1;
            showkey->key_unit.pos_y = showkey->rect_box.height - (key_height + KEY_GAP);
            showkey->key_unit.last_press_time = event->time_msec;
            return;
        }
    } else {
        key_unit->pos_x += key_width + KEY_GAP;
    }
    key_unit->last_press_time = event->time_msec;
}

static void showkey_destroy_key_buffer(struct scene_buffer *buffer, void *data)
{
    /* buffers are destroyed in tree */
}

static void showkey_set_key_buffer(struct showkey *showkey, struct scene_buffer *key_buffer,
                                   float scale)
{
    struct showkey_effect *effect = showkey->effect;
    struct keyboard_buffer *buffer = keyboard_buffer_get_or_create(effect, scale);
    struct wlr_buffer *wlr_buffer = buffer->buffer;
    if (key_buffer->buffer != wlr_buffer) {
        scene_buffer_set_buffer(key_buffer, wlr_buffer);
    }

    if (key_buffer->buffer != wlr_buffer) {
        return;
    }

    const struct keycode_map *keycode_map = showkey->key_unit.key;
    if (!keycode_map) {
        return;
    }

    int index = keycode_map->index;
    scene_buffer_set_source_box(key_buffer, &buffer->src_boxs[index]);
    scene_buffer_set_dest_size(key_buffer, keycode_map->width, keycode_map->height);
    scene_node_set_position(&key_buffer->node, showkey->key_unit.pos_x, showkey->key_unit.pos_y);

    struct theme *theme = theme_manager_get_theme();
    scene_node_set_radius(&key_buffer->node, (int[4]){ theme->menu_radius, theme->menu_radius,
                                                       theme->menu_radius, theme->menu_radius });
}

static void showkey_update_key_buffer(struct scene_buffer *buffer, float scale, void *data)
{
    struct showkey *showkey = data;
    showkey_set_key_buffer(showkey, buffer, scale);
}

static void showkey_set_key_display(struct showkey *showkey, struct seat_keyboard_key_event *event,
                                    const struct keycode_map *keycode_map)
{
    struct kywc_output *kywc_output = kywc_output_get_primary();
    float scale = kywc_output->state.scale;
    struct scene_buffer *key_buffer = scaled_buffer_create(
        showkey->tree, scale, showkey_update_key_buffer, showkey_destroy_key_buffer, showkey);
    showkey_set_key_buffer(showkey, key_buffer, scale);
}

static void showkey_seat_destroy(struct showkey_seat *showkey_seat)
{
    wl_list_remove(&showkey_seat->seat_destroy.link);
    wl_list_remove(&showkey_seat->keyboard_key.link);
    wl_list_remove(&showkey_seat->link);
    free(showkey_seat);
}

static void showkey_destroy(struct showkey *showkey)
{
    wl_list_remove(&showkey->new_seat.link);
    showkey_destroy_display_tree(showkey);

    struct showkey_seat *showkey_seat, *tmp;
    wl_list_for_each_safe(showkey_seat, tmp, &showkey->seats, link) {
        showkey_seat_destroy(showkey_seat);
    }

    if (showkey->timer) {
        wl_event_source_remove(showkey->timer);
    }

    free(showkey);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct showkey_seat *showkey_seat = wl_container_of(listener, showkey_seat, seat_destroy);
    showkey_seat_destroy(showkey_seat);
}

static int handle_showkey_waiting_time(void *data)
{
    struct showkey *showkey = data;
    showkey_destroy_display_tree(showkey);
    return true;
}

static void handle_theme_update(struct wl_listener *listener, void *data)
{
    struct showkey_effect *effect = wl_container_of(listener, effect, theme_update);
    /* destroy all buffers */
    struct keyboard_buffer *buffer, *tmp;
    wl_list_for_each_safe(buffer, tmp, &effect->scaled_buffers, link) {
        wlr_buffer_drop(buffer->buffer);
        wl_list_remove(&buffer->link);
        free(buffer);
    }
}

static void handle_keyboard_key(struct wl_listener *listener, void *data)
{
    struct showkey_seat *showkey_seat = wl_container_of(listener, showkey_seat, keyboard_key);
    struct seat_keyboard_key_event *event = data;
    if (event->device && event->device->prop.is_virtual) {
        return;
    }

    struct showkey *showkey = showkey_seat->showkey;
    if (!set_keyboard_timer(showkey, event)) {
        return;
    }

    struct keycode_map *keycode_map = NULL;
    if (!find_keycode_map(event->keycode, &keycode_map)) {
        kywc_log(KYWC_DEBUG, "Not find keycode in keycode_map");
        return;
    }

    showkey->key_unit.key = keycode_map;
    showkey_update_key_position(showkey, event);

    if (!showkey->tree) {
        showkey_create_display_tree(showkey);
    }

    showkey_set_key_display(showkey, event, keycode_map);
}

static void showkey_seat_create(struct showkey *showkey, struct seat *seat)
{
    struct showkey_seat *showkey_seat = calloc(1, sizeof(*showkey_seat));
    if (!showkey_seat) {
        return;
    }

    showkey_seat->seat = seat;
    showkey_seat->showkey = showkey;
    wl_list_insert(&showkey->seats, &showkey_seat->link);
    showkey_seat->seat_destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &showkey_seat->seat_destroy);
    showkey_seat->keyboard_key.notify = handle_keyboard_key;
    wl_signal_add(&showkey_seat->seat->events.keyboard_key, &showkey_seat->keyboard_key);
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct showkey *showkey = wl_container_of(listener, showkey, new_seat);
    struct seat *seat = data;
    showkey_seat_create(showkey, seat);
}

static bool handle_iterator_seat(struct seat *seat, int index, void *data)
{
    struct showkey *showkey = data;
    showkey_seat_create(showkey, seat);
    return false;
}

static struct showkey *showkey_create(struct showkey_effect *effect)
{
    struct showkey *showkey = calloc(1, sizeof(*showkey));
    if (!showkey) {
        return NULL;
    }

    showkey->effect = effect;
    wl_list_init(&showkey->seats);
    wl_list_init(&showkey->new_seat.link);
    input_manager_for_each_seat(handle_iterator_seat, showkey);
    seat_add_new_listener(&showkey->new_seat);
    showkey->new_seat.notify = handle_new_seat;

    struct wl_event_loop *loop = wl_display_get_event_loop(effect->server->display);
    showkey->timer = wl_event_loop_add_timer(loop, handle_showkey_waiting_time, showkey);

    return showkey;
}

static int create_showkey(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct showkey_effect *effect = userdata;
    uint32_t x, y, width, height;
    CK(sd_bus_message_read(m, "uuuu", &x, &y, &width, &height));
    if (width < MIN_BACKGROUND_WIDTH || height < MIN_BACKGROUND_HEIGHT) {
        const sd_bus_error error = SD_BUS_ERROR_MAKE_CONST(
            "com.kylin.Wlcom.Showkey.Error.Cancelled", "Invalid width or height");
        return sd_bus_reply_method_error(m, &error);
    }

    if (!effect->showkey) {
        effect->showkey = showkey_create(effect);
        if (!effect->showkey) {
            return sd_bus_reply_method_return(m, "b", false);
        }
    }

    effect->showkey->rect_box = (struct kywc_box){ x, y, width, height };

    if (!effect->showkey->tree) {
        showkey_create_display_tree(effect->showkey);
        if (wl_event_source_timer_update(effect->showkey->timer,
                                         effect_manager_scale_time(NO_KEY_WAITING_TIME)) < 0) {
            return sd_bus_reply_method_return(m, "b", false);
        }

        return sd_bus_reply_method_return(m, "b", true);
    }

    scene_node_set_position(&effect->showkey->tree->node, effect->showkey->rect_box.x,
                            effect->showkey->rect_box.y);
    scene_rect_set_size(effect->showkey->rect, width, height);
    return sd_bus_reply_method_return(m, "b", true);
}

static int destroy_showkey(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct showkey_effect *effect = userdata;
    if (effect->showkey) {
        showkey_destroy(effect->showkey);
        effect->showkey = NULL;
    }
    return sd_bus_reply_method_return(m, NULL);
}

static const sd_bus_vtable showkey_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("createShowkey", "uuuu", "b", create_showkey, 0),
    SD_BUS_METHOD("destroyShowkey", "", "", destroy_showkey, 0),
    SD_BUS_VTABLE_END,
};

static void handle_effect_disable(struct wl_listener *listener, void *data)
{
    struct showkey_effect *effect = wl_container_of(listener, effect, disable);

    dbus_unregister_object(effect->dbus);
    wl_list_remove(&effect->theme_update.link);

    if (effect->showkey) {
        showkey_destroy(effect->showkey);
        effect->showkey = NULL;
    }

    /* destroy all buffers */
    struct keyboard_buffer *buffer, *_tmp;
    wl_list_for_each_safe(buffer, _tmp, &effect->scaled_buffers, link) {
        wlr_buffer_drop(buffer->buffer);
        wl_list_remove(&buffer->link);
        free(buffer);
    }
}

static void handle_effect_enable(struct wl_listener *listener, void *data)
{
    struct showkey_effect *effect = wl_container_of(listener, effect, enable);
    effect->dbus = dbus_register_object(NULL, "/com/kylin/Wlcom/Showkey", "com.kylin.Wlcom.Showkey",
                                        showkey_vtable, effect);

    effect->theme_update.notify = handle_theme_update;
    theme_manager_add_update_listener(&effect->theme_update, false);
}

static void handle_effect_destroy(struct wl_listener *listener, void *data)
{
    struct showkey_effect *effect = wl_container_of(listener, effect, destroy);

    wl_list_remove(&effect->destroy.link);
    wl_list_remove(&effect->enable.link);
    wl_list_remove(&effect->disable.link);

    free(effect);
}

static bool handle_effect_configure(struct effect *effect, const struct effect_option *option)
{
    if (effect_option_is_enabled_option(option)) {
        return true;
    }

    return false;
}

static const struct effect_interface showkey_effect_impl = {
    .configure = handle_effect_configure,
};

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct showkey_effect *effect = wl_container_of(listener, effect, display_destroy);
    wl_list_remove(&effect->display_destroy.link);
    if (effect->showkey && effect->showkey->timer) {
        wl_event_source_remove(effect->showkey->timer);
        effect->showkey->timer = NULL;
    }
}

bool showkey_effect_create(struct effect_manager *manager)
{
    struct showkey_effect *effect = calloc(1, sizeof(*effect));
    if (!effect) {
        return false;
    }

    effect->effect = effect_create("showkey", 106, true, &showkey_effect_impl, effect);
    if (!effect->effect) {
        free(effect);
        return false;
    }

    effect->server = manager->server;
    wl_list_init(&effect->scaled_buffers);
    effect->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(effect->server->display, &effect->display_destroy);

    /* sort keycode_maps array in ascending order */
    qsort(keycode_maps, ARRAY_SIZE(keycode_maps), sizeof(struct keycode_map), compare_keycode_map);

    effect->enable.notify = handle_effect_enable;
    wl_signal_add(&effect->effect->events.enable, &effect->enable);
    effect->disable.notify = handle_effect_disable;
    wl_signal_add(&effect->effect->events.disable, &effect->disable);
    effect->destroy.notify = handle_effect_destroy;
    wl_signal_add(&effect->effect->events.destroy, &effect->destroy);

    if (effect->effect->enabled) {
        handle_effect_enable(&effect->enable, NULL);
    }

    return true;
}
