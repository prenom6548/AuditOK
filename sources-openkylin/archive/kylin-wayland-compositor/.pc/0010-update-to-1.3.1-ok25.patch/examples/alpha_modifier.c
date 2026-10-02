// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <cairo/cairo.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>
#include <wlu.h>

#include "alpha-modifier-v1-client-protocol.h"

struct wp_alpha_modifier_v1 *alpha_modifier = NULL;
struct wp_alpha_modifier_surface_v1 *alpha_modifier_surface = NULL;

static void draw_frame(struct wlu_window *window, void *data)
{
    int32_t width = wlu_window_get_width(window);
    int32_t height = wlu_window_get_height(window);
    size_t swapchain_index = wlu_window_get_swapchain_index(window);
    void *shm_data = wlu_window_get_shm_data(window, swapchain_index);

    const cairo_format_t format = CAIRO_FORMAT_ARGB32;
    int stride = cairo_format_stride_for_width(format, width);
    cairo_surface_t *surface =
        cairo_image_surface_create_for_data(shm_data, format, width, height, stride);
    cairo_t *cr = cairo_create(surface);

    // 半透明背景
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.5, 1.0, 0.5);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // 不透明方块
    cairo_set_source_rgba(cr, 0.0, 0.5, 1.0, 1.0);
    cairo_rectangle(cr, 50, 50, 100, 100);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "Noto Sans Mono CJK SC", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40);

    const char *text = "-键减少Alpha  =键增加Alpha";
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, width / 2.0 - extents.width / 2.0, height / 2.0);
    cairo_show_text(cr, text);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

static void handle_global(struct wlu_display *display, struct wl_registry *registry, uint32_t name,
                          const char *interface, uint32_t version)
{
    if (strcmp(interface, wp_alpha_modifier_v1_interface.name) == 0) {
        alpha_modifier = wl_registry_bind(registry, name, &wp_alpha_modifier_v1_interface, 1);
        printf("bind alpha_modifier. %p\n", alpha_modifier);
    }
}

static void handle_window_create(struct wlu_window *window)
{
    struct wl_surface *wl_surface = wlu_window_get_wl_surface(window);
    alpha_modifier_surface = wp_alpha_modifier_v1_get_surface(alpha_modifier, wl_surface);
    printf("get alpha_modifier_surface. %p\n", alpha_modifier_surface);
}

static void handle_keyboard(struct wlu_window *window, uint32_t key, uint32_t state)
{
    if (state != WL_KEYBOARD_KEY_STATE_PRESSED || !alpha_modifier_surface) {
        return;
    }

    static double factor = 1.0;
    bool modified = false;
    if (key == KEY_EQUAL) {
        factor += 0.1;
        if (factor > 1.0) {
            factor = 0.0;
        }
        modified = true;
    } else if (key == KEY_MINUS) {
        factor -= 0.1;
        if (factor < 0.0) {
            factor = 1.0;
        }
        modified = true;
    }

    if (modified) {
        wp_alpha_modifier_surface_v1_set_multiplier(alpha_modifier_surface, factor * UINT32_MAX);
        wlu_window_request_draw(window, draw_frame, NULL);
        printf("alpha_modifier_surface_set_multiplie. %f\n", factor);
    }
}

static const wlu_display_event_callbacks_t display_event_callbacks = {
    .registry_global = handle_global,
};

static const wlu_window_event_callbacks_t window_event_callbacks = {
    .create = handle_window_create,
    .keyboard = handle_keyboard,
};

int main(int argc, char **argv)
{
    struct wlu_display *display =
        wlu_display_create(&display_event_callbacks, WLU_DISPLAY_CREATE_NONE);
    if (!display) {
        return 1;
    }

    struct wlu_window_create_info window_create_info = {
        .display = display,
        .event_callbacks = &window_event_callbacks,
        .app_id = "os.kylin.wlcom.alpha_modifier",
        .width = 640,
        .height = 480,
        .format = WLU_FORMAT_ARGB8888,
        .title = "Alpha Modifier Protocol Test",
        .flags = WLU_WINDOW_CREATE_DECORATION | WLU_WINDOW_CREATE_TRANSPARENT,
    };
    struct wlu_window *window = wlu_window_create(&window_create_info);
    if (!window) {
        wlu_display_destroy(display);
        return 1;
    }
    wlu_window_request_draw(window, draw_frame, NULL);

    wlu_display_run(display);

    if (alpha_modifier_surface) {
        wp_alpha_modifier_surface_v1_destroy(alpha_modifier_surface);
    }
    if (alpha_modifier) {
        wp_alpha_modifier_v1_destroy(alpha_modifier);
    }

    wlu_display_destroy(display);
    return 0;
}
