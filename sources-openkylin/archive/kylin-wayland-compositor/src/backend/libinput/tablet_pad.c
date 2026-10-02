// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>

#include <kywc/log.h>

#include "libinput_p.h"
#include "util/time.h"

const struct wlr_tablet_pad_impl libinput_tablet_pad_impl = {
    .name = "libinput-tablet-pad",
};

static void group_destroy(struct wlr_tablet_pad_group *group)
{
    free(group->buttons);
    free(group->strips);
    free(group->rings);
    free(group);
}

static void add_pad_group_from_libinput(struct wlr_tablet_pad *pad, struct libinput_device *device,
                                        unsigned int index)
{
    struct libinput_tablet_pad_mode_group *li_group =
        libinput_device_tablet_pad_get_mode_group(device, index);
    struct wlr_tablet_pad_group *group = calloc(1, sizeof(*group));
    if (!group) {
        kywc_log_errno(KYWC_ERROR, "failed to allocate wlr_tablet_pad_group");
        return;
    }

    for (size_t i = 0; i < pad->ring_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_ring(li_group, i)) {
            ++group->ring_count;
        }
    }
    group->rings = calloc(group->ring_count, sizeof(unsigned int));
    if (!group->rings) {
        goto group_fail;
    }

    size_t ring = 0;
    for (size_t i = 0; i < pad->ring_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_ring(li_group, i)) {
            group->rings[ring++] = i;
        }
    }

    for (size_t i = 0; i < pad->strip_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_strip(li_group, i)) {
            ++group->strip_count;
        }
    }
    group->strips = calloc(group->strip_count, sizeof(unsigned int));
    if (!group->strips) {
        goto group_fail;
    }

    size_t strip = 0;
    for (size_t i = 0; i < pad->strip_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_strip(li_group, i)) {
            group->strips[strip++] = i;
        }
    }

    for (size_t i = 0; i < pad->button_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_button(li_group, i)) {
            ++group->button_count;
        }
    }
    group->buttons = calloc(group->button_count, sizeof(unsigned int));
    if (!group->buttons) {
        goto group_fail;
    }

    size_t button = 0;
    for (size_t i = 0; i < pad->button_count; ++i) {
        if (libinput_tablet_pad_mode_group_has_button(li_group, i)) {
            group->buttons[button++] = i;
        }
    }
    group->mode_count = libinput_tablet_pad_mode_group_get_num_modes(li_group);

    libinput_tablet_pad_mode_group_ref(li_group);
    wl_list_insert(&pad->groups, &group->link);

    return;

group_fail:
    kywc_log(KYWC_ERROR, "failed to configure wlr_tablet_pad_group");
    group_destroy(group);
}

void init_device_tablet_pad(struct libinput_input_device *dev)
{
    struct libinput_device *handle = dev->handle;
    const char *name = libinput_device_get_name(handle);
    struct wlr_tablet_pad *wlr_tablet_pad = &dev->tablet_pad;
    wlr_tablet_pad_init(wlr_tablet_pad, &libinput_tablet_pad_impl, name);
    wlr_tablet_pad->base.vendor = libinput_device_get_id_vendor(handle);
    wlr_tablet_pad->base.product = libinput_device_get_id_product(handle);

    wlr_tablet_pad->button_count = libinput_device_tablet_pad_get_num_buttons(handle);
    wlr_tablet_pad->ring_count = libinput_device_tablet_pad_get_num_rings(handle);
    wlr_tablet_pad->strip_count = libinput_device_tablet_pad_get_num_strips(handle);

    struct udev_device *udev = libinput_device_get_udev_device(handle);
    char **dst = wl_array_add(&wlr_tablet_pad->paths, sizeof(char *));
    *dst = strdup(udev_device_get_syspath(udev));
    udev_device_unref(udev);

    int groups = libinput_device_tablet_pad_get_num_mode_groups(handle);
    for (int i = 0; i < groups; ++i) {
        add_pad_group_from_libinput(wlr_tablet_pad, handle, i);
    }
}

void finish_device_tablet_pad(struct libinput_input_device *dev)
{
    struct wlr_tablet_pad_group *group, *tmp;
    wl_list_for_each_safe(group, tmp, &dev->tablet_pad.groups, link) {
        group_destroy(group);
    }

    wlr_tablet_pad_finish(&dev->tablet_pad);

    int groups = libinput_device_tablet_pad_get_num_mode_groups(dev->handle);
    for (int i = 0; i < groups; ++i) {
        struct libinput_tablet_pad_mode_group *li_group =
            libinput_device_tablet_pad_get_mode_group(dev->handle, i);
        libinput_tablet_pad_mode_group_unref(li_group);
    }
}

struct libinput_input_device *device_from_tablet_pad(struct wlr_tablet_pad *wlr_tablet_pad)
{
    assert(wlr_tablet_pad->impl == &libinput_tablet_pad_impl);
    struct libinput_input_device *dev = wl_container_of(wlr_tablet_pad, dev, tablet_pad);
    return dev;
}

void handle_tablet_pad_button(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad)
{
    struct libinput_event_tablet_pad *pevent = libinput_event_get_tablet_pad_event(event);
    struct libinput_input_device *dev = device_from_tablet_pad(tablet_pad);
    enum wlr_button_state state;
    if (!button_state_from_libinput(libinput_event_tablet_pad_get_button_state(pevent), &state)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput button state");
        return;
    }
    struct input_event ie = {
        .type = INPUT_EVENT_TABLET_PAD_BUTTON,
        .time_msec = usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent)),
        .tablet_pad = {
            .button = libinput_event_tablet_pad_get_button_number(pevent),
            .pressed = (state == WLR_BUTTON_PRESSED),
            .mode = libinput_event_tablet_pad_get_mode(pevent),
            .group = libinput_tablet_pad_mode_group_get_index(
                libinput_event_tablet_pad_get_mode_group(pevent)),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_tablet_pad_ring(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad)
{
    struct libinput_event_tablet_pad *pevent = libinput_event_get_tablet_pad_event(event);
    struct libinput_input_device *dev = device_from_tablet_pad(tablet_pad);
    uint32_t source = WLR_TABLET_PAD_RING_SOURCE_UNKNOWN;
    switch (libinput_event_tablet_pad_get_ring_source(pevent)) {
    case LIBINPUT_TABLET_PAD_RING_SOURCE_FINGER:
        source = WLR_TABLET_PAD_RING_SOURCE_FINGER;
        break;
    default:
        break;
    }
    struct input_event ie = {
        .type = INPUT_EVENT_TABLET_PAD_RING,
        .time_msec = usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent)),
        .tablet_pad = {
            .number = libinput_event_tablet_pad_get_ring_number(pevent),
            .position = libinput_event_tablet_pad_get_ring_position(pevent),
            .mode = libinput_event_tablet_pad_get_mode(pevent),
            .source = source,
        },
    };
    queue_input_event(dev, &ie);
}

void handle_tablet_pad_strip(struct libinput_event *event, struct wlr_tablet_pad *tablet_pad)
{
    struct libinput_event_tablet_pad *pevent = libinput_event_get_tablet_pad_event(event);
    struct libinput_input_device *dev = device_from_tablet_pad(tablet_pad);
    uint32_t source = WLR_TABLET_PAD_STRIP_SOURCE_UNKNOWN;
    switch (libinput_event_tablet_pad_get_strip_source(pevent)) {
    case LIBINPUT_TABLET_PAD_STRIP_SOURCE_FINGER:
        source = WLR_TABLET_PAD_STRIP_SOURCE_FINGER;
        break;
    default:
        break;
    }
    struct input_event ie = {
        .type = INPUT_EVENT_TABLET_PAD_STRIP,
        .time_msec = usec_to_msec(libinput_event_tablet_pad_get_time_usec(pevent)),
        .tablet_pad = {
            .number = libinput_event_tablet_pad_get_strip_number(pevent),
            .position = libinput_event_tablet_pad_get_strip_position(pevent),
            .mode = libinput_event_tablet_pad_get_mode(pevent),
            .source = source,
        },
    };
    queue_input_event(dev, &ie);
}
