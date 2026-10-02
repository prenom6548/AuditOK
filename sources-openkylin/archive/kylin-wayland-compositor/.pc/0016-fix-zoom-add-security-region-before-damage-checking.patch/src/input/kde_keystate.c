// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include <wlr/interfaces/wlr_keyboard.h>

#include "input/keyboard_group.h"
#include "input_p.h"
#include "kde-keystate-protocol.h"
#include "server.h"

#define ORG_KDE_KWIN_KEYSTATE_VERSION 5

struct keystate_manager {
    struct wl_global *global;

    struct keystate_seat *current_seat;

    struct wl_list resources;
    struct wl_list keystate_seats;

    struct wl_listener new_seat;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct keystate_seat {
    struct wl_list link;

    struct seat *seat;

    struct wl_listener key;
    struct wl_listener modifiers;
    struct wl_listener destroy;

    uint32_t keycode;
    uint32_t seat_lock;
};

static struct keystate_manager *keystate_manager = NULL;

static void kde_keystate_fetchstates(struct wl_client *client, struct wl_resource *resource)
{
    struct seat *seat;
    uint32_t seat_lock;
    uint32_t state = ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
    if (keystate_manager->current_seat) {
        seat = keystate_manager->current_seat->seat;
        seat_lock = keystate_manager->current_seat->seat_lock;
    } else {
        seat = input_manager_get_default_seat();
        seat_lock = keyboard_get_locks(seat->keyboard);
    }

    state = seat_lock & INPUT_KEY_CAPSLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                           : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
    org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_CAPSLOCK, state);

    state = seat_lock & INPUT_KEY_NUMLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                          : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
    org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_NUMLOCK, state);

    state = seat_lock & INPUT_KEY_SCROLLLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                             : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
    org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK, state);

    uint32_t version = wl_resource_get_version(resource);
    if (version < ORG_KDE_KWIN_KEYSTATE_VERSION) {
        return;
    }
    struct wlr_keyboard_modifiers *wlr_keyboard_modifiers =
        &seat->keyboard->wlr_keyboard->modifiers;

    if (wlr_keyboard_modifiers->depressed & WLR_MODIFIER_ALT) {
        org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_ALT,
                                                ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED);
    }
    if (wlr_keyboard_modifiers->depressed & WLR_MODIFIER_CTRL) {
        org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_CONTROL,
                                                ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED);
    }
    if (wlr_keyboard_modifiers->depressed & WLR_MODIFIER_SHIFT) {
        org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_SHIFT,
                                                ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED);
    }
    if (wlr_keyboard_modifiers->depressed & WLR_MODIFIER_LOGO) {
        org_kde_kwin_keystate_send_stateChanged(resource, ORG_KDE_KWIN_KEYSTATE_KEY_META,
                                                ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED);
    }
}

static void kde_keystate_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct org_kde_kwin_keystate_interface kde_keystate_impl = {
    .fetchStates = kde_keystate_fetchstates,
    .destroy = kde_keystate_destroy,
};

static void kde_keystate_unbind(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void kde_keystate_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &org_kde_kwin_keystate_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kde_keystate_impl, NULL, kde_keystate_unbind);
    wl_list_insert(&keystate_manager->resources, wl_resource_get_link(resource));
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&keystate_manager->display_destroy.link);
    wl_list_remove(&keystate_manager->new_seat.link);
    wl_global_destroy(keystate_manager->global);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&keystate_manager->server_destroy.link);
    free(keystate_manager);
}

static void keystate_send_state_change(uint32_t key, uint32_t state)
{
    struct wl_resource *resource;
    wl_resource_for_each(resource, &keystate_manager->resources) {
        org_kde_kwin_keystate_send_stateChanged(resource, key, state);
    }
}

static bool keystate_change_seat_lock(struct keystate_seat *keystate_seat, uint32_t current_locks,
                                      uint32_t key)
{
    bool get_lock = false, get_last_lock = false;
    if (key == ORG_KDE_KWIN_KEYSTATE_KEY_CAPSLOCK) {
        get_lock = current_locks & INPUT_KEY_CAPSLOCK;
        get_last_lock = keystate_seat->seat_lock & INPUT_KEY_CAPSLOCK;
    } else if (key == ORG_KDE_KWIN_KEYSTATE_KEY_NUMLOCK) {
        get_lock = current_locks & INPUT_KEY_NUMLOCK;
        get_last_lock = keystate_seat->seat_lock & INPUT_KEY_NUMLOCK;
    } else if (key == ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK) {
        get_lock = current_locks & INPUT_KEY_SCROLLLOCK;
        get_last_lock = keystate_seat->seat_lock & INPUT_KEY_SCROLLLOCK;
    }

    if (get_lock != get_last_lock) {
        keystate_seat->seat_lock ^= 1 << key;
    }

    return get_lock != get_last_lock;
}

static void seat_scrolllock_change(struct keystate_seat *keystate_seat)
{
    uint32_t locks = keyboard_get_locks(keystate_seat->seat->keyboard);

    if (keystate_change_seat_lock(keystate_seat, locks, ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK)) {
        uint32_t state = locks & INPUT_KEY_SCROLLLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                                      : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
        keystate_send_state_change(ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK, state);
    }
}

static void handle_key(struct wl_listener *listener, void *data)
{
    struct keystate_seat *keystate_seat = wl_container_of(listener, keystate_seat, key);
    struct seat_keyboard_key_event *event = data;

    keystate_seat->keycode = event->pressed ? event->keycode : 0;
    if (event->keycode == KEY_SCROLLLOCK) {
        seat_scrolllock_change(keystate_seat);
    }

    if (keystate_manager->current_seat != keystate_seat) {
        keystate_manager->current_seat = keystate_seat;
    }
}

static void keystate_check_lock_state(struct keystate_seat *keystate_seat)
{
    uint32_t locks = keyboard_get_locks(keystate_seat->seat->keyboard);
    uint32_t key, state;
    if (keystate_change_seat_lock(keystate_seat, locks, ORG_KDE_KWIN_KEYSTATE_KEY_CAPSLOCK)) {
        key = ORG_KDE_KWIN_KEYSTATE_KEY_CAPSLOCK;
        state = locks & INPUT_KEY_CAPSLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                           : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
        keystate_send_state_change(key, state);
    }

    if (keystate_change_seat_lock(keystate_seat, locks, ORG_KDE_KWIN_KEYSTATE_KEY_NUMLOCK)) {
        key = ORG_KDE_KWIN_KEYSTATE_KEY_NUMLOCK;
        state = locks & INPUT_KEY_NUMLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                          : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
        keystate_send_state_change(key, state);
    }

    if (keystate_change_seat_lock(keystate_seat, locks, ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK)) {
        key = ORG_KDE_KWIN_KEYSTATE_KEY_SCROLLLOCK;
        state = locks & INPUT_KEY_SCROLLLOCK ? ORG_KDE_KWIN_KEYSTATE_STATE_LOCKED
                                             : ORG_KDE_KWIN_KEYSTATE_STATE_UNLOCKED;
        keystate_send_state_change(key, state);
    }
}

static void handle_modifiers(struct wl_listener *listener, void *data)
{
    struct keystate_seat *keystate_seat = wl_container_of(listener, keystate_seat, modifiers);
    struct wlr_keyboard_modifiers *modifiers = data;
    uint32_t key = 0;

    switch (keystate_seat->keycode) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
        if (modifiers->depressed & WLR_MODIFIER_SHIFT) {
            key = ORG_KDE_KWIN_KEYSTATE_KEY_SHIFT;
        }
        break;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
        if (modifiers->depressed & WLR_MODIFIER_CTRL) {
            key = ORG_KDE_KWIN_KEYSTATE_KEY_CONTROL;
        }
        break;
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
        if (modifiers->depressed & WLR_MODIFIER_LOGO) {
            key = ORG_KDE_KWIN_KEYSTATE_KEY_META;
        }
        break;
    case KEY_LEFTALT:
        if (modifiers->depressed & WLR_MODIFIER_ALT) {
            key = ORG_KDE_KWIN_KEYSTATE_KEY_ALT;
        }
        break;
    case KEY_RIGHTALT:
        if (modifiers->depressed & WLR_MODIFIER_ALT) {
            key = ORG_KDE_KWIN_KEYSTATE_KEY_ALTGR;
        }
        break;
    default:
        break;
    }

    if (!key) {
        keystate_check_lock_state(keystate_seat);
        return;
    }

    struct wl_resource *resource;
    wl_resource_for_each(resource, &keystate_manager->resources) {
        uint32_t version = wl_resource_get_version(resource);
        if (version >= ORG_KDE_KWIN_KEYSTATE_VERSION) {
            org_kde_kwin_keystate_send_stateChanged(resource, key,
                                                    ORG_KDE_KWIN_KEYSTATE_STATE_PRESSED);
        }
    }
}

static void handle_destroy(struct wl_listener *listener, void *data)
{
    struct keystate_seat *keystate_seat = wl_container_of(listener, keystate_seat, destroy);
    wl_list_remove(&keystate_seat->destroy.link);
    wl_list_remove(&keystate_seat->modifiers.link);
    wl_list_remove(&keystate_seat->key.link);
    wl_list_remove(&keystate_seat->link);
    free(keystate_seat);
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct seat *seat = data;
    struct keystate_seat *keystate_seat = calloc(1, sizeof(*keystate_seat));
    if (!keystate_seat) {
        return;
    }

    keystate_seat->seat = seat;
    keystate_seat->keycode = 0;
    keystate_seat->seat_lock = 0;
    wl_list_insert(&keystate_manager->keystate_seats, &keystate_seat->link);
    if (!keystate_manager->current_seat) {
        keystate_manager->current_seat = keystate_seat;
    }

    keystate_seat->key.notify = handle_key;
    wl_signal_add(&seat->events.keyboard_key, &keystate_seat->key);
    keystate_seat->modifiers.notify = handle_modifiers;
    wl_signal_add(&seat->events.keyboard_modifiers, &keystate_seat->modifiers);
    keystate_seat->destroy.notify = handle_destroy;
    wl_signal_add(&seat->events.destroy, &keystate_seat->destroy);
}

bool kde_keystate_manager_create(struct input_manager *input_manager)
{
    keystate_manager = calloc(1, sizeof(*keystate_manager));
    if (!keystate_manager) {
        return false;
    }

    wl_list_init(&keystate_manager->resources);
    wl_list_init(&keystate_manager->keystate_seats);

    keystate_manager->global =
        wl_global_create(input_manager->server->display, &org_kde_kwin_keystate_interface,
                         ORG_KDE_KWIN_KEYSTATE_VERSION, keystate_manager, kde_keystate_bind);
    if (!keystate_manager->global) {
        kywc_log(KYWC_WARN, "Failed to create %s global", org_kde_kwin_keystate_interface.name);
        free(keystate_manager);
        return false;
    }

    keystate_manager->current_seat = NULL;
    keystate_manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(input_manager->server->display,
                                    &keystate_manager->display_destroy);
    keystate_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &keystate_manager->server_destroy);

    keystate_manager->new_seat.notify = handle_new_seat;
    wl_signal_add(&input_manager->events.new_seat, &keystate_manager->new_seat);
    return true;
}
