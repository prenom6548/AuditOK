// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>
#include <strings.h>

#include <xkbcommon/xkbcommon.h>

#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

#include <kywc/log.h>

#include "input/keyboard_group.h"
#include "input_p.h"
#include "util/macros.h"
#include "util/time.h"

static struct modifier {
    char *name;
    uint32_t mod;
} modifiers[] = {
    { XKB_MOD_NAME_SHIFT, WLR_MODIFIER_SHIFT },
    { XKB_MOD_NAME_CAPS, WLR_MODIFIER_CAPS },
    { "Ctrl", WLR_MODIFIER_CTRL },
    { XKB_MOD_NAME_CTRL, WLR_MODIFIER_CTRL },
    { "Alt", WLR_MODIFIER_ALT },
    { XKB_MOD_NAME_ALT, WLR_MODIFIER_ALT },
    { XKB_MOD_NAME_NUM, WLR_MODIFIER_MOD2 },
    { "Mod3", WLR_MODIFIER_MOD3 },
    { "Win", WLR_MODIFIER_LOGO },
    { XKB_MOD_NAME_LOGO, WLR_MODIFIER_LOGO },
    { "Mod5", WLR_MODIFIER_MOD5 },
};

uint32_t keyboard_get_modifier_mask_by_name(const char *name)
{
    for (size_t i = 0; i < ARRAY_SIZE(modifiers); i++) {
        if (strcasecmp(modifiers[i].name, name) == 0) {
            return modifiers[i].mod;
        }
    }
    return 0;
}

const char *keyboard_get_modifier_name_by_mask(uint32_t modifier)
{
    for (size_t i = 0; i < ARRAY_SIZE(modifiers); i++) {
        if (modifiers[i].mod == modifier) {
            return modifiers[i].name;
        }
    }

    return NULL;
}

const char *keyboard_get_modifier_names(uint32_t modifier_masks, char split)
{
    static char names[128] = { 0 };
    char *p = names;

    for (size_t i = 0; i < ARRAY_SIZE(modifiers); i++) {
        if ((modifier_masks & modifiers[i].mod) != 0) {
            p += sprintf(p, "%s%c", modifiers[i].name, split);
            modifier_masks ^= modifiers[i].mod;
        }
    }

    p != names ? (*--p = '\0') : (*p = '\0');
    return names;
}

static void modifiers_mask_debug(uint32_t mask, const char *mask_name)
{
    const char *names = keyboard_get_modifier_names(mask, ' ');
    kywc_log(KYWC_DEBUG, "\t%s: %s", mask_name, names);
}

// TODO: compose and dead key support
static void keyboard_state_erase_key(struct keyboard_state *keyboard_state, uint32_t keysym)
{
    uint32_t idx = 0;
    for (size_t i = 0; i < keyboard_state->npressed; i++) {
        if (i > idx) {
            keyboard_state->pressed_keysyms[idx] = keyboard_state->pressed_keysyms[i];
        }
        if (keyboard_state->pressed_keysyms[i] != keysym) {
            idx++;
        }
    }

    while (keyboard_state->npressed > idx) {
        keyboard_state->npressed--;
        keyboard_state->pressed_keysyms[keyboard_state->npressed] = 0;
    }

    if (kywc_log_get_level() == KYWC_DEBUG) {
        kywc_log(KYWC_DEBUG, "Erase key 0x%x, %lu", keysym, keyboard_state->npressed);
        for (size_t i = 0; i < keyboard_state->npressed; i++) {
            kywc_log(KYWC_DEBUG, "\tcurrent keysym %lu: 0x%x", i,
                     keyboard_state->pressed_keysyms[i]);
        }
    }
}

static void keyboard_state_add_key(struct keyboard_state *keyboard_state, uint32_t keysym)
{
    if (keyboard_state->npressed >= MAX_PRESSED_KEY) {
        return;
    }
    if (keyboard_state->npressed > 0 &&
        keyboard_state->pressed_keysyms[keyboard_state->npressed - 1] == keysym) {
        return;
    }

    keyboard_state->pressed_keysyms[keyboard_state->npressed] = keysym;
    keyboard_state->npressed++;

    if (kywc_log_get_level() == KYWC_DEBUG) {
        kywc_log(KYWC_DEBUG, "Add key 0x%x, %lu", keysym, keyboard_state->npressed);
        for (size_t i = 0; i < keyboard_state->npressed; i++) {
            kywc_log(KYWC_DEBUG, "\tcurrent keysym %lu: 0x%x", i,
                     keyboard_state->pressed_keysyms[i]);
        }
    }
}

static void keyboard_state_clear(struct keyboard_state *keyboard_state)
{
    if (keyboard_state->npressed > 0) {
        *keyboard_state = (struct keyboard_state){ 0 };
    }
}

static void handle_keyboard_state(struct keyboard_state *keyboard_state, uint32_t modifiers,
                                  uint32_t keysym, bool pressed)
{

    bool last_key_is_modifiers = modifiers != keyboard_state->last_modifiers;

    keyboard_state->only_one_modifier = modifiers && keyboard_state->last_modifiers == 0 &&
                                        !pressed && keyboard_state->npressed == 1;

    keyboard_state->last_modifiers = modifiers;

    if (last_key_is_modifiers && keyboard_state->last_keysym) {
        // a modifiier key preesed before this key, erase it
        keyboard_state_erase_key(keyboard_state, keyboard_state->last_keysym);
        keyboard_state->last_keysym = 0;
    }

    if (pressed) {
        keyboard_state_add_key(keyboard_state, keysym);
        keyboard_state->last_keysym = keysym;
    } else {
        keyboard_state_erase_key(keyboard_state, xkb_keysym_to_upper(keysym));
        keyboard_state_erase_key(keyboard_state, xkb_keysym_to_lower(keysym));
    }
}

static bool keyboard_update_keyboard_state(struct keyboard *keyboard, uint32_t key,
                                           uint32_t modifiers, bool pressed)
{
    struct keyboard_state *keyboard_state = &keyboard->state;
    /* Translate libinput keycode -> xkbcommon keysym */
    const xkb_keysym_t *keysyms;
    size_t len = 0; // xkb_state may be null
    if (keyboard->wlr_keyboard->xkb_state) {
        len = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, key + 8, &keysyms);
    }

    for (size_t i = 0; i < len; ++i) {
        handle_keyboard_state(keyboard_state, modifiers, keysyms[i], pressed);
    }

    for (size_t i = 0; i < len; ++i) {
        xkb_keysym_t keysym = keysyms[i];
        if (keysym >= XKB_KEY_XF86Switch_VT_1 && keysym <= XKB_KEY_XF86Switch_VT_12) {
            input_manager_switch_vt(keysym - XKB_KEY_XF86Switch_VT_1 + 1);
            return false;
        }
    }

    return true;
}

static struct key_binding *keyboard_get_key_binding(struct keyboard *keyboard, bool pressed)
{
    struct keyboard_state *keyboard_state = &keyboard->state;

    struct seat *seat = keyboard->seat;
    if (seat_is_keyboard_shortcuts_inhibited(seat)) {
        return NULL;
    }

    if (!pressed && !keyboard->state.only_one_modifier) {
        return NULL;
    }

    return bindings_get_key_binding(keyboard_state);
}

static void keyboard_repeat_stop(struct keyboard *keyboard)
{
    if (!keyboard->repeat.timer) {
        return;
    }
    if (keyboard->repeat.key == 0) {
        return;
    }

    keyboard->repeat.key = 0;
    wl_event_source_timer_update(keyboard->repeat.timer, 0);
}

static void keyboard_repeat_start(struct keyboard *keyboard, uint32_t key, bool pressed)
{
    if (!keyboard->repeat.timer) {
        return;
    }

    if (keyboard->repeat.key > 0) {
        if (keyboard->repeat.key == key && !pressed) {
            keyboard_repeat_stop(keyboard);
        }
        return;
    }

    /* only enable key repeat when pressed state */
    if (!pressed) {
        return;
    }

    int32_t delay = keyboard->wlr_keyboard->repeat_info.delay;
    if (delay > 0) {
        keyboard->repeat.key = key;
        if (wl_event_source_timer_update(keyboard->repeat.timer, delay) < 0) {
            kywc_log(KYWC_DEBUG, "Failed to set key repeat timer");
        }
    } else if (keyboard->repeat.key > 0) {
        keyboard_repeat_stop(keyboard);
    }
}

static void keyboard_update_lock(struct keyboard *keyboard, uint32_t key, bool pressed)
{
    if (key != KEY_SCROLLLOCK) {
        return;
    }

    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(keyboard->wlr_keyboard);
    if (!group) {
        return;
    }

    if (group->scroll_lock == 0 && pressed) {
        group->scroll_lock = 2;
        group->keyboard.leds |= WLR_LED_SCROLL_LOCK;
    } else if (group->scroll_lock > 0 && !pressed) {
        if (--group->scroll_lock == 0) {
            // only used to bypass check in wlr_keyboard_led_update
            group->keyboard.leds |= WLR_LED_SCROLL_LOCK;
        }
    }
}

static void keyboard_feed_key(struct keyboard *keyboard, uint32_t key, uint32_t state,
                              uint32_t time, uint32_t modifiers)
{
    if (kywc_log_get_level() == KYWC_DEBUG) {
        modifiers_mask_debug(modifiers, "modifiers");
    }

    struct seat *seat = keyboard->seat;
    bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;

    keyboard_update_lock(keyboard, key, pressed);

    /* early return if key is sent by input method */
    if (keyboard_is_from_input_method(keyboard)) {
        wlr_seat_set_keyboard(seat->wlr_seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat->wlr_seat, time, key, state);
        return;
    }

    struct seat_keyboard_key_event event = {
        .device = input_from_wlr_input(&keyboard->wlr_keyboard->base),
        .time_msec = time,
        .keycode = key,
        .pressed = pressed,
    };
    wl_signal_emit_mutable(&seat->events.keyboard_key, &event);

    if (!keyboard_update_keyboard_state(keyboard, key, modifiers, pressed)) {
        return;
    }

    struct key_binding *binding = keyboard_get_key_binding(keyboard, pressed);
    bool bypass_grab = bindings_get_key_binding_bypass_grab(binding);

    if (!bypass_grab && seat->keyboard_grab && seat->keyboard_grab->interface->key &&
        seat->keyboard_grab->interface->key(seat->keyboard_grab, keyboard, time, key, pressed,
                                            modifiers)) {
        keyboard_state_clear(&keyboard->state);
        if (key != keyboard->repeat.key) {
            keyboard_repeat_stop(keyboard);
        }
        if (!bindings_get_key_binding_no_repeat(binding)) {
            keyboard_repeat_start(keyboard, key, pressed);
        }
        return;
    }

    /* don't auto repeat for some bindings, like vt switch */
    bool need_repeat = false;
    bool handled = bindings_handle_key_binding(keyboard, binding, key, pressed, &need_repeat);
    if (handled) {
        keyboard_update_keyboard_state(keyboard, key, modifiers, false);
        need_repeat ? keyboard_repeat_start(keyboard, key, pressed)
                    : keyboard_repeat_stop(keyboard);
        return;
    }

    keyboard_repeat_stop(keyboard);

    handled = input_method_handle_key(keyboard, time, key, state);
    if (handled) {
        return;
    }

    wlr_seat_set_keyboard(seat->wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_key(seat->wlr_seat, time, key, state);
}

static void keyboard_feed_modifiers(struct keyboard *keyboard,
                                    struct wlr_keyboard_modifiers *modifiers)
{
    if (kywc_log_get_level() == KYWC_DEBUG) {
        kywc_log(KYWC_DEBUG, "Keyboard modifiers update");
        modifiers_mask_debug(modifiers->depressed, "depressed");
        modifiers_mask_debug(modifiers->latched, "latched");
        modifiers_mask_debug(modifiers->locked, "locked");
        modifiers_mask_debug(modifiers->group, "group");
    }

    if (bindings_handle_key_binding_modifiers(keyboard,
                                              wlr_keyboard_get_modifiers(keyboard->wlr_keyboard))) {
        keyboard_repeat_stop(keyboard);
    }

    if (input_method_handle_modifiers(keyboard)) {
        return;
    }
    wl_signal_emit_mutable(&keyboard->seat->events.keyboard_modifiers, modifiers);

    struct wlr_seat *wlr_seat = keyboard->seat->wlr_seat;
    wlr_seat_set_keyboard(wlr_seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(wlr_seat, modifiers);
}

static bool keyboard_sync_physical_key(struct keyboard *keyboard, uint32_t key, bool state)
{
    if (!keyboard->is_virtual || keyboard_is_from_input_method(keyboard)) {
        return false;
    }

    if (key == KEY_CAPSLOCK) {
        struct seat *seat = keyboard->seat;
        struct keyboard *kb;
        wl_list_for_each(kb, &seat->keyboards, link) {
            if (keyboard_has_no_input(kb)) {
                continue;
            }
            keyboard_send_key(kb, key, state);
            return true;
        }
    }
    return false;
}

static void keyboard_handle_key(struct wl_listener *listener, void *data)
{
    struct keyboard *keyboard = wl_container_of(listener, keyboard, key);
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;
    struct wlr_keyboard_key_event *event = data;

    if (input_event_filter(keyboard->seat, NULL, INPUT_EVENT_KEYBOARD_KEY, event)) {
        return;
    }

    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(wlr_keyboard);
    if (group) {
        keyboard->seat->keyboard = keyboard;
    }

    uint32_t modifiers = wlr_keyboard_get_modifiers(wlr_keyboard);
    if (!keyboard_sync_physical_key(keyboard, event->keycode, event->state)) {
        keyboard_feed_key(keyboard, event->keycode, event->state, event->time_msec, modifiers);
    }
}

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data)
{
    struct keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;

    if (input_event_filter(keyboard->seat, NULL, INPUT_EVENT_KEYBOARD_MODIFIERS, data)) {
        return;
    }

    keyboard_feed_modifiers(keyboard, &wlr_keyboard->modifiers);
}

static int keyboard_handle_repeat(void *data)
{
    struct keyboard *keyboard = data;
    if (keyboard->repeat.key > 0) {
        if (keyboard->wlr_keyboard->repeat_info.rate > 0) {
            // We queue the next event first, as the command might cancel it
            if (wl_event_source_timer_update(keyboard->repeat.timer,
                                             1000 / keyboard->wlr_keyboard->repeat_info.rate) < 0) {
                kywc_log(KYWC_DEBUG, "Failed to update key repeat timer");
            }
        }
        keyboard_feed_key(keyboard, keyboard->repeat.key, true, current_time_msec(),
                          wlr_keyboard_get_modifiers(keyboard->wlr_keyboard));
    }
    return 0;
}

struct keyboard *keyboard_create(struct seat *seat, struct wlr_keyboard *wlr_keyboard)
{
    struct keyboard *keyboard = calloc(1, sizeof(*keyboard));
    if (!keyboard) {
        return NULL;
    }

    if (!wlr_keyboard) {
        struct keyboard_group *group = keyboard_group_create();
        keyboard->wlr_keyboard = &group->keyboard;
    } else {
        keyboard->wlr_keyboard = wlr_keyboard;
    }

    keyboard->is_virtual = !!wlr_keyboard;
    keyboard->wlr_keyboard->data = keyboard;

    /* insert new keyboard to seat keyboard list */
    keyboard->seat = seat;
    wl_list_insert(&seat->keyboards, &keyboard->link);

    /* create timer for internal key repeat */
    if (!keyboard->is_virtual) {
        struct wl_event_loop *loop = wl_display_get_event_loop(seat->wlr_seat->display);
        keyboard->repeat.timer = wl_event_loop_add_timer(loop, keyboard_handle_repeat, keyboard);
    }

    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&keyboard->wlr_keyboard->events.key, &keyboard->key);
    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&keyboard->wlr_keyboard->events.modifiers, &keyboard->modifiers);

    return keyboard;
}

void keyboard_add_input(struct seat *seat, struct input *input)
{
    struct wlr_input_device *wlr_input = input->wlr_input;
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(wlr_input);

    /* virtual keyboard is not managed by group */
    if (input->prop.is_virtual) {
        keyboard_create(seat, wlr_keyboard);
        return;
    }

    /* find a suitable group */
    struct keyboard *keyboard, *empty_keyboard = NULL;
    wl_list_for_each(keyboard, &seat->keyboards, link) {
        if (keyboard->is_virtual) {
            continue;
        }

        struct wlr_keyboard *dst_keyboard = keyboard->wlr_keyboard;
        struct keyboard_group *group = keyboard_group_from_wlr_keyboard(dst_keyboard);
        if (wl_list_empty(&group->devices)) {
            empty_keyboard = keyboard;
            continue;
        }

        if (keyboard_keymaps_match(wlr_keyboard, dst_keyboard) &&
            wlr_keyboard->repeat_info.rate == dst_keyboard->repeat_info.rate &&
            wlr_keyboard->repeat_info.delay == dst_keyboard->repeat_info.delay) {
            keyboard_group_add_keyboard(group, wlr_keyboard);
            return;
        }
    }

    if (empty_keyboard) {
        struct wlr_keyboard *dst_keyboard = empty_keyboard->wlr_keyboard;
        if (!dst_keyboard->keymap || !keyboard_keymaps_match(wlr_keyboard, dst_keyboard)) {
            wlr_keyboard_set_keymap(dst_keyboard, wlr_keyboard->keymap);
        }
        wlr_keyboard_set_repeat_info(dst_keyboard, wlr_keyboard->repeat_info.rate,
                                     wlr_keyboard->repeat_info.delay);

        struct keyboard_group *group = keyboard_group_from_wlr_keyboard(dst_keyboard);
        keyboard_group_add_keyboard(group, wlr_keyboard);
        return;
    }

    /* create a new keyboard group with keyboard configuration */
    keyboard = keyboard_create(seat, NULL);
    if (!keyboard) {
        return;
    }

    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(keyboard->wlr_keyboard);
    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, wlr_keyboard->keymap);
    wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, wlr_keyboard->repeat_info.rate,
                                 wlr_keyboard->repeat_info.delay);
    keyboard_group_add_keyboard(group, wlr_keyboard);
}

void keyboard_destroy(struct keyboard *keyboard)
{
    struct wlr_seat *wlr_seat = keyboard->seat->wlr_seat;
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;

    wl_list_remove(&keyboard->link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->modifiers.link);

    if (keyboard->seat->keyboard == keyboard) {
        keyboard->seat->keyboard = NULL;
    }

    if (wlr_seat_get_keyboard(wlr_seat) == wlr_keyboard) {
        struct wlr_keyboard *kb =
            keyboard->seat->keyboard ? keyboard->seat->keyboard->wlr_keyboard : NULL;
        wlr_seat_set_keyboard(wlr_seat, kb);
    }

    if (!keyboard->is_virtual) {
        struct keyboard_group *group = keyboard_group_from_wlr_keyboard(wlr_keyboard);
        keyboard_group_destroy(group);
    }

    if (keyboard->repeat.timer) {
        wl_event_source_remove(keyboard->repeat.timer);
    }
    free(keyboard);
}

void keyboard_remove_input(struct input *input)
{
    struct wlr_input_device *wlr_input = input->wlr_input;
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(wlr_input);
    struct keyboard *keyboard;

    if (input->prop.is_virtual) {
        keyboard = wlr_keyboard->data;
        keyboard_destroy(keyboard);
        return;
    }

    struct keyboard_group *group = (struct keyboard_group *)wlr_keyboard->group;
    /* may removed when input destroy at keyboard group */
    if (group) {
        keyboard_group_remove_keyboard(group, wlr_keyboard);
    }
}

void keyboard_send_key(struct keyboard *keyboard, uint32_t key, bool pressed)
{
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;
    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(wlr_keyboard);
    if (!group) {
        return;
    }

    struct wlr_keyboard_key_event wlr_event = {
        .time_msec = current_time_msec(),
        .keycode = key,
        .update_state = true,
        .state = pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED,
    };

    wlr_keyboard = keyboard_group_pick_keyboard(group);
    wlr_keyboard_notify_key(wlr_keyboard, &wlr_event);
}

uint32_t keyboard_get_locks(struct keyboard *keyboard)
{
    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;

    uint32_t locks = 0;
    if (wlr_keyboard->modifiers.locked & WLR_MODIFIER_CAPS) {
        locks |= INPUT_KEY_CAPSLOCK;
    }
    if (wlr_keyboard->modifiers.locked & WLR_MODIFIER_MOD2) {
        locks |= INPUT_KEY_NUMLOCK;
    }
    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(wlr_keyboard);
    if (group && group->scroll_lock) {
        locks |= INPUT_KEY_SCROLLLOCK;
    }
    return locks;
}

bool keyboard_has_no_input(struct keyboard *keyboard)
{
    if (keyboard->is_virtual) {
        return true;
    }

    struct wlr_keyboard *wlr_keyboard = keyboard->wlr_keyboard;
    struct keyboard_group *group = keyboard_group_from_wlr_keyboard(wlr_keyboard);
    return wl_list_empty(&group->devices);
}

bool keyboard_keymaps_match(struct wlr_keyboard *kb1, struct wlr_keyboard *kb2)
{
    const char *km1 = kb1->keymap_string;
    const char *km2 = kb2->keymap_string;

    if (!km1 && !km2) {
        return true;
    }
    if (!km1 || !km2) {
        return false;
    }
    return strcmp(km1, km2) == 0;
}

static bool compare_string(const char *a, const char *b)
{
    if (!a && !b) {
        return false;
    }
    if (!a || !b) {
        return true;
    }
    return strcmp(a, b) != 0;
}

bool keyboard_check_keymap_rules(struct keymap_rules *old, struct keymap_rules *new)
{
    return compare_string(old->xkb_rules, new->xkb_rules) ||
           compare_string(old->xkb_model, new->xkb_model) ||
           compare_string(old->xkb_layout, new->xkb_layout) ||
           compare_string(old->xkb_variant, new->xkb_variant) ||
           compare_string(old->xkb_options, new->xkb_options);
}

struct xkb_keymap *keyboard_compile_keymap(struct keymap_rules *rules)
{
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_SECURE_GETENV);
    struct xkb_rule_names names = {
        .layout = rules->xkb_layout,
        .model = rules->xkb_model,
        .options = rules->xkb_options,
        .rules = rules->xkb_rules,
        .variant = rules->xkb_variant,
    };
    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    xkb_context_unref(context);
    return keymap;
}
