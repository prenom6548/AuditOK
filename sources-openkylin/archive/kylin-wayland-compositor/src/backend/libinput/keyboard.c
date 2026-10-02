// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>

#include <kywc/log.h>

#include "libinput_p.h"
#include "util/time.h"

struct libinput_input_device *device_from_keyboard(struct wlr_keyboard *kb)
{
    assert(kb->impl == &libinput_keyboard_impl);
    struct libinput_input_device *dev = wl_container_of(kb, dev, keyboard);
    return dev;
}

static void keyboard_set_leds(struct wlr_keyboard *wlr_kb, uint32_t leds)
{
    struct libinput_input_device *dev = device_from_keyboard(wlr_kb);
    libinput_device_led_update(dev->handle, leds);
}

const struct wlr_keyboard_impl libinput_keyboard_impl = {
    .name = "libinput-keyboard",
    .led_update = keyboard_set_leds,
};

void init_device_keyboard(struct libinput_input_device *dev)
{
    const char *name = libinput_device_get_name(dev->handle);
    struct wlr_keyboard *wlr_kb = &dev->keyboard;
    wlr_keyboard_init(wlr_kb, &libinput_keyboard_impl, name);
    wlr_kb->base.vendor = libinput_device_get_id_vendor(dev->handle);
    wlr_kb->base.product = libinput_device_get_id_product(dev->handle);

    libinput_device_led_update(dev->handle, 0);
}

static bool key_state_from_libinput(enum libinput_key_state state, enum wl_keyboard_key_state *out)
{
    switch (state) {
    case LIBINPUT_KEY_STATE_RELEASED:
        *out = WL_KEYBOARD_KEY_STATE_RELEASED;
        return true;
    case LIBINPUT_KEY_STATE_PRESSED:
        *out = WL_KEYBOARD_KEY_STATE_PRESSED;
        return true;
    default:
        return false;
    }
}

void handle_keyboard_key(struct libinput_event *event, struct wlr_keyboard *kb)
{
    struct libinput_event_keyboard *kbevent = libinput_event_get_keyboard_event(event);
    struct libinput_input_device *dev = device_from_keyboard(kb);
    enum wl_keyboard_key_state state;
    if (!key_state_from_libinput(libinput_event_keyboard_get_key_state(kbevent), &state)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput key state");
        return;
    }
    struct input_event ie = {
        .type = INPUT_EVENT_KEYBOARD_KEY,
        .time_msec = usec_to_msec(libinput_event_keyboard_get_time_usec(kbevent)),
        .key = {
            .key = libinput_event_keyboard_get_key(kbevent),
            .pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED),
        },
    };
    queue_input_event(dev, &ie);
}
