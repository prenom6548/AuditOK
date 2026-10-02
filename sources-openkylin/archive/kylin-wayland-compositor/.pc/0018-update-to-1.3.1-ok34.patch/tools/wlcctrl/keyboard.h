// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <xkbcommon/xkbcommon.h>

#include "virtual-keyboard-unstable-v1-client-protocol.h"

struct keymap_entry;

enum {
    MOD_PRESS = 1,
    MOD_RELEASE = 2,
    MOD_BUTTON = 3,
};

typedef struct _keyboard {
    struct wl_display *display;
    struct zwp_virtual_keyboard_v1 *virtual_keyboard;

    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    bool shift;

    size_t keymap_len;
    struct keymap_entry *keymap_entry;
    unsigned int *key_codes;
    size_t key_codes_len;
} keyboard;

int keyboard_init(keyboard *keyboard, const struct xkb_rule_names *rule_names);

void keyboard_destroy(keyboard *keyboard);

void keyboard_set(keyboard *keyboard, char *text);

void keyboard_press(keyboard *keyboard, char *text, bool press);

int key_string_action(keyboard *keyboard, char *str);

#endif
