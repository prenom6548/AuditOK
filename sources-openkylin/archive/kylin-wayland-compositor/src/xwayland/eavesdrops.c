// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <linux/input-event-codes.h>

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

#include "input/keyboard.h"
#include "input/seat.h"
#include "util/time.h"
#include "xwayland_p.h"

static struct eavesdrops_manager {
    struct xwm *xwm;

    struct seat *seat;
    struct wl_listener keyboard_key;
    struct wl_listener keyboard_modifiers;

    /**
     * keycodes that have been forwarded to Xwayland and are still pressed.
     * Their release events must always be forwarded to keep Xwayland's
     * keyboard state consistent.
     *
     * Bitmap of pressed keys, indexed by keycode. Linux keycodes range
     * from 8 to 255, so 256 bits (4 x 64-bit words) are sufficient.
     */
    uint64_t pressed_keys[4];
} *manager;

static bool key_is_modifier(unsigned int code)
{
    switch (code) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:

    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:

    case KEY_LEFTALT:
    case KEY_RIGHTALT:

    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:

    case KEY_CAPSLOCK:
    case KEY_NUMLOCK:
    case KEY_SCROLLLOCK:
        return true;

    default:
        return false;
    }
}

/**
 * Bitmap operations for pressed_keys, following the same pattern as
 * X Server's BitIsOn/SetBit/ClearBit macros in inputstr.h.
 */
#define KEY_BITMAP_BYTES (sizeof(manager->pressed_keys))
#define KEY_BITMAP_BITS (KEY_BITMAP_BYTES * 8)

#define key_bitmap_is_on(bitmap, bit)                                                              \
    (!!(((const uint64_t *)(bitmap))[(bit) >> 6] & (1ULL << ((bit) & 63))))
#define key_bitmap_set(bitmap, bit) (((uint64_t *)(bitmap))[(bit) >> 6] |= (1ULL << ((bit) & 63)))
#define key_bitmap_clear(bitmap, bit)                                                              \
    (((uint64_t *)(bitmap))[(bit) >> 6] &= ~(1ULL << ((bit) & 63)))

static bool eavesdrops_pressed_key_contains(uint32_t keycode)
{
    if (keycode >= KEY_BITMAP_BITS) {
        return false;
    }
    return key_bitmap_is_on(manager->pressed_keys, keycode);
}

static void eavesdrops_pressed_key_add(uint32_t keycode)
{
    if (keycode >= KEY_BITMAP_BITS) {
        return;
    }
    key_bitmap_set(manager->pressed_keys, keycode);
}

static void eavesdrops_pressed_key_remove(uint32_t keycode)
{
    if (keycode >= KEY_BITMAP_BITS) {
        return;
    }
    key_bitmap_clear(manager->pressed_keys, keycode);
}

static void eavesdrops_release_pressed_keys(void)
{
    struct xwayland_manager *xwayland = manager->xwm->xwayland;
    if (!xwayland->xwayland_server || !xwayland->xwayland_server->client) {
        return;
    }

    struct wlr_seat *wlr_seat = manager->seat->wlr_seat;
    struct wlr_seat_client *seat_client =
        wlr_seat_client_for_wl_client(wlr_seat, xwayland->xwayland_server->client);
    if (!seat_client) {
        return;
    }

    uint32_t time_msec = current_time_msec();
    uint32_t serial = wlr_seat_client_next_serial(seat_client);

    for (uint32_t word = 0; word < KEY_BITMAP_BYTES / sizeof(uint64_t); word++) {
        if (manager->pressed_keys[word] == 0) {
            continue;
        }

        for (uint32_t bit = 0; bit < 64; bit++) {
            uint32_t keycode = word * 64 + bit;
            if (!eavesdrops_pressed_key_contains(keycode)) {
                continue;
            }

            struct wl_resource *resource;
            wl_resource_for_each(resource, &seat_client->keyboards) {
                wl_keyboard_send_key(resource, serial, time_msec, keycode, false);
            }

            eavesdrops_pressed_key_remove(keycode);
        }
    }
}

static void handle_keyboard_key(struct wl_listener *listener, void *data)
{
    struct xwayland_manager *xwayland = manager->xwm->xwayland;
    if (!xwayland->xwayland_server || !xwayland->xwayland_server->client) {
        return;
    }

    if (xwayland->state.eavesdrops_key_mode == NONE_KEYS) {
        return;
    }

    struct wlr_seat *wlr_seat = manager->seat->wlr_seat;
    if (wlr_seat->keyboard_state.focused_client &&
        xwayland_check_client(wlr_seat->keyboard_state.focused_client->client)) {
        return;
    }

    struct seat *seat = manager->seat;
    struct seat_keyboard_key_event *event = data;

    /**
     * If there's an active keystroke, keep sending events regardless what the
     * key filter says. This ensures the release event of a key whose press was
     * forwarded to Xwayland is always delivered, keeping Xwayland's keyboard
     * state consistent.
     */
    if (!eavesdrops_pressed_key_contains(event->keycode)) {
        if (event->consumed && !key_is_modifier(event->keycode)) {
            return;
        }

        uint32_t modifiers = wlr_keyboard_get_modifiers(seat->keyboard->wlr_keyboard);
        if (xwayland->state.eavesdrops_key_mode == ALL_KEYS_WITH_MODIFIER) {
            /**
             * Only Ctrl/Alt/Meta (and non-character keys) trigger forwarding.
             * Shift is deliberately excluded so that e.g. Shift+A is not
             * forwarded, matching KWin's AllKeysWithModifier behavior.
             */
            const uint32_t modifier_keys = WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT | WLR_MODIFIER_LOGO;
            if ((modifiers & modifier_keys) == 0 && !key_is_modifier(event->keycode)) {
                return;
            }
        }
    }

    struct wlr_seat_client *seat_client =
        wlr_seat_client_for_wl_client(wlr_seat, xwayland->xwayland_server->client);
    if (!seat_client) {
        return;
    }

    if (event->pressed) {
        eavesdrops_pressed_key_add(event->keycode);
    } else {
        eavesdrops_pressed_key_remove(event->keycode);
    }

    uint32_t serial = wlr_seat_client_next_serial(seat_client);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &seat_client->keyboards) {
        wl_keyboard_send_key(resource, serial, event->time_msec, event->keycode, event->pressed);
    }
}

static void handle_keyboard_modifiers(struct wl_listener *listener, void *data)
{
    struct xwayland_manager *xwayland = manager->xwm->xwayland;
    if (!xwayland->xwayland_server || !xwayland->xwayland_server->client) {
        return;
    }

    if (xwayland->state.eavesdrops_key_mode == NONE_KEYS) {
        return;
    }

    struct wlr_seat *wlr_seat = manager->seat->wlr_seat;
    if (wlr_seat->keyboard_state.focused_client &&
        xwayland_check_client(wlr_seat->keyboard_state.focused_client->client)) {
        return;
    }

    struct wlr_seat_client *seat_client =
        wlr_seat_client_for_wl_client(wlr_seat, xwayland->xwayland_server->client);
    if (!seat_client) {
        return;
    }

    struct wlr_keyboard_modifiers *modifiers = data;
    uint32_t serial = wlr_seat_client_next_serial(seat_client);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &seat_client->keyboards) {
        if (!modifiers) {
            wl_keyboard_send_modifiers(resource, serial, 0, 0, 0, 0);
        } else {
            wl_keyboard_send_modifiers(resource, serial, modifiers->depressed, modifiers->latched,
                                       modifiers->locked, modifiers->group);
        }
    }
}

void eavesdrops_manager_destroy(void)
{
    if (!manager) {
        return;
    }

    wl_list_remove(&manager->keyboard_key.link);
    wl_list_remove(&manager->keyboard_modifiers.link);
    free(manager);
    manager = NULL;
}

bool eavesdrops_manager_create(struct xwm *xwm)
{
    if (manager) {
        return false;
    }

    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->xwm = xwm;
    manager->keyboard_key.notify = handle_keyboard_key;
    wl_list_init(&manager->keyboard_key.link);
    manager->keyboard_modifiers.notify = handle_keyboard_modifiers;
    wl_list_init(&manager->keyboard_modifiers.link);

    return true;
}

void eavesdrops_manager_set_seat(struct seat *seat)
{
    if (!manager) {
        return;
    }

    if (manager->seat != NULL) {
        eavesdrops_release_pressed_keys();

        wl_list_remove(&manager->keyboard_key.link);
        wl_list_init(&manager->keyboard_key.link);
        wl_list_remove(&manager->keyboard_modifiers.link);
        wl_list_init(&manager->keyboard_modifiers.link);
    }

    manager->seat = seat;
    if (!seat) {
        return;
    }

    wl_signal_add(&seat->events.keyboard_key, &manager->keyboard_key);
    wl_signal_add(&seat->events.keyboard_modifiers, &manager->keyboard_modifiers);
}