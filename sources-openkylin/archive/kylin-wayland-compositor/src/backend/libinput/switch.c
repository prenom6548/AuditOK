// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>

#include <kywc/log.h>

#include "libinput_p.h"
#include "util/time.h"

const struct wlr_switch_impl libinput_switch_impl = {
    .name = "libinput-switch",
};

void init_device_switch(struct libinput_input_device *dev)
{
    const char *name = libinput_device_get_name(dev->handle);
    struct wlr_switch *wlr_switch = &dev->switch_device;
    wlr_switch_init(wlr_switch, &libinput_switch_impl, name);
    wlr_switch->base.vendor = libinput_device_get_id_vendor(dev->handle);
    wlr_switch->base.product = libinput_device_get_id_product(dev->handle);
}

struct libinput_input_device *device_from_switch(struct wlr_switch *wlr_switch)
{
    assert(wlr_switch->impl == &libinput_switch_impl);
    struct libinput_input_device *dev = wl_container_of(wlr_switch, dev, switch_device);
    return dev;
}

static bool switch_type_from_libinput(enum libinput_switch type, enum wlr_switch_type *out)
{
    switch (type) {
    case LIBINPUT_SWITCH_LID:
        *out = WLR_SWITCH_TYPE_LID;
        return true;
    case LIBINPUT_SWITCH_TABLET_MODE:
        *out = WLR_SWITCH_TYPE_TABLET_MODE;
        return true;
    default:
        return false;
    }
}

static bool switch_state_from_libinput(enum libinput_switch_state state, enum wlr_switch_state *out)
{
    switch (state) {
    case LIBINPUT_SWITCH_STATE_OFF:
        *out = WLR_SWITCH_STATE_OFF;
        return true;
    case LIBINPUT_SWITCH_STATE_ON:
        *out = WLR_SWITCH_STATE_ON;
        return true;
    default:
        return false;
    }
}

void handle_switch_toggle(struct libinput_event *event, struct wlr_switch *wlr_switch)
{
    struct libinput_event_switch *sevent = libinput_event_get_switch_event(event);
    struct libinput_input_device *dev = device_from_switch(wlr_switch);
    enum wlr_switch_type sw_type;
    enum wlr_switch_state sw_state;
    if (!switch_type_from_libinput(libinput_event_switch_get_switch(sevent), &sw_type)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput switch type");
        return;
    }
    if (!switch_state_from_libinput(libinput_event_switch_get_switch_state(sevent), &sw_state)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput switch state");
        return;
    }

    struct input_event ie = {
        .type = INPUT_EVENT_SWITCH_TOGGLE,
        .time_msec = usec_to_msec(libinput_event_switch_get_time_usec(sevent)),
        .sw = { .switch_type = sw_type, .switch_state = sw_state },
    };
    queue_input_event(dev, &ie);
}
