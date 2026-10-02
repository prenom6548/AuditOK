// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <linux/input-event-codes.h>
#include <stdio.h>
#include <wayland-client.h>

#include "util/macros.h"

#include "commands.h"
#include "cursor.h"
#include "keyboard.h"
#include "output.h"

static cursor *get_cursor(struct context *ctx)
{
    if (!ctx->manager->cursor) {
        fprintf(stderr, "Pointer is not available.\n");
        return NULL;
    }
    return ctx->manager->cursor;
}

static keyboard *get_keyboard(struct context *ctx)
{
    if (!ctx->manager->keyboard) {
        fprintf(stderr, "Keyboard is not available.\n");
        return NULL;
    }
    return ctx->manager->keyboard;
}

/* pointer commands */

int cmd_mousemove(struct context *ctx, const char *param)
{
    cursor *cursor = get_cursor(ctx);
    if (!cursor) {
        return -1;
    }
    if (!cursor->output) {
        fprintf(stderr, "No output is available for pointer motion.\n");
        return -1;
    }

    long xy[2];
    if (!parse_ints(param, xy, 2) || xy[0] < 1 || xy[1] < 1) {
        fprintf(stderr,
                "The --mousemove option expects two positive integers separated with a comma.\n");
        return -1;
    }

    uint32_t pos_x, pos_y;
    output_transform_coord(cursor->output, xy[0] - 1, xy[1] - 1, &pos_x, &pos_y);
    cursor_set_motion_absolute(cursor, pos_x, pos_y);
    output_transform_coord(cursor->output, xy[0], xy[1], &pos_x, &pos_y);
    cursor_set_motion_absolute(cursor, pos_x, pos_y);
    return 0;
}

static const uint32_t mouse_buttons[] = {
    0, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_EXTRA, BTN_SIDE, BTN_FORWARD, BTN_BACK,
};

static bool parse_mouse_button(const char *param, uint32_t *button)
{
    uint32_t index;
    if (!parse_u32(param, &index) || index == 0 || index >= ARRAY_SIZE(mouse_buttons)) {
        fprintf(stderr, "The button code must be an integer between 1 and %zu.\n",
                ARRAY_SIZE(mouse_buttons) - 1);
        return false;
    }
    *button = mouse_buttons[index];
    return true;
}

int cmd_mousebutton(struct context *ctx, const char *param)
{
    cursor *cursor = get_cursor(ctx);
    uint32_t button;
    if (!cursor || !parse_mouse_button(param, &button)) {
        return -1;
    }

    cursor_button_click(cursor, button);
    return 0;
}

int cmd_mousepress(struct context *ctx, const char *param)
{
    cursor *cursor = get_cursor(ctx);
    uint32_t button;
    if (!cursor || !parse_mouse_button(param, &button)) {
        return -1;
    }

    cursor_button_press(cursor, button, true);
    return 0;
}

int cmd_mouserelease(struct context *ctx, const char *param)
{
    cursor *cursor = get_cursor(ctx);
    uint32_t button;
    if (!cursor || !parse_mouse_button(param, &button)) {
        return -1;
    }

    cursor_button_press(cursor, button, false);
    return 0;
}

int cmd_scroll(struct context *ctx, const char *param)
{
    if (!ctx->x && !ctx->y) {
        fprintf(stderr, "The --scroll option expects -x or -y parameters.\n");
        return -1;
    }

    int32_t x = 0, y = 0;
    if ((ctx->x && !parse_i32(ctx->x, &x)) || (ctx->y && !parse_i32(ctx->y, &y))) {
        fprintf(stderr, "The --scroll option expects integer -x and -y parameters.\n");
        return -1;
    }

    cursor *cursor = get_cursor(ctx);
    if (!cursor) {
        return -1;
    }

    cursor_axis_set(cursor, x, y);
    return 0;
}

static void cursor_handle_position(kywc_cursor *cursor, uint32_t x, uint32_t y)
{
    printf("current cursor position : %u, %u\n", x, y);
}

static const struct kywc_cursor_interface cursor_impl = {
    .position = cursor_handle_position,
};

int cmd_getmouselocation(struct context *ctx, const char *param)
{
    if (!ctx->manager->seat) {
        fprintf(stderr, "Seat is not available.\n");
        return -1;
    }

    if (!kywc_cursor_create(ctx->kywc, ctx->manager->seat->wl_seat, &cursor_impl, NULL)) {
        fprintf(stderr, "Failed to create the cursor.\n");
        return -1;
    }
    wl_display_roundtrip(ctx->manager->wl_display);
    return 0;
}

/* keyboard commands */

int cmd_keystring(struct context *ctx, const char *param)
{
    keyboard *keyboard = get_keyboard(ctx);
    if (!keyboard) {
        return -1;
    }

    if (key_string_action(keyboard, param) != 0) {
        fprintf(stderr, "Failed to type the input string.\n");
        return -1;
    }
    return 0;
}

int cmd_key(struct context *ctx, const char *param)
{
    keyboard *keyboard = get_keyboard(ctx);
    if (!keyboard) {
        return -1;
    }

    return keyboard_set(keyboard, param);
}

int cmd_keydown(struct context *ctx, const char *param)
{
    keyboard *keyboard = get_keyboard(ctx);
    if (!keyboard) {
        return -1;
    }

    return keyboard_press(keyboard, param, true);
}

int cmd_keyup(struct context *ctx, const char *param)
{
    keyboard *keyboard = get_keyboard(ctx);
    if (!keyboard) {
        return -1;
    }

    return keyboard_press(keyboard, param, false);
}
