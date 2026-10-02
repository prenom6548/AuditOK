// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>

#include "backend/libinput.h"
#include "libinput_p.h"
#include "util/time.h"

const struct wlr_touch_impl libinput_touch_impl = {
    .name = "libinput-touch",
};

void init_device_touch(struct libinput_input_device *dev)
{
    const char *name = libinput_device_get_name(dev->handle);
    struct wlr_touch *wlr_touch = &dev->touch;
    wlr_touch_init(wlr_touch, &libinput_touch_impl, name);
    wlr_touch->base.vendor = libinput_device_get_id_vendor(dev->handle);
    wlr_touch->base.product = libinput_device_get_id_product(dev->handle);

    libinput_device_get_size(dev->handle, &wlr_touch->width_mm, &wlr_touch->height_mm);

    dev->touch_abs = (struct touch_abs){
        .major_minimum = -1,
        .major_maximum = -1,
        .major_resolution = -1,
        .minor_minimum = -1,
        .minor_maximum = -1,
        .minor_resolution = -1,
    };

    const struct libinput_property_exts *property_exts = &dev->backend->property_exts;
    if (property_exts->support_touch_abs) {
        struct touch_abs touch_abs;
        if (property_exts->device_get_touch_major_minimum(dev->handle, &touch_abs.major_minimum) >=
                0 &&
            property_exts->device_get_touch_major_maximum(dev->handle, &touch_abs.major_maximum) >=
                0 &&
            property_exts->device_get_touch_major_resolution(dev->handle,
                                                             &touch_abs.major_resolution) >= 0 &&
            property_exts->device_get_touch_minor_minimum(dev->handle, &touch_abs.minor_minimum) >=
                0 &&
            property_exts->device_get_touch_minor_maximum(dev->handle, &touch_abs.minor_maximum) >=
                0 &&
            property_exts->device_get_touch_minor_resolution(dev->handle,
                                                             &touch_abs.minor_resolution) >= 0) {
            dev->touch_abs = touch_abs;
        }
    }
}

struct libinput_input_device *device_from_touch(struct wlr_touch *wlr_touch)
{
    assert(wlr_touch->impl == &libinput_touch_impl);
    struct libinput_input_device *dev = wl_container_of(wlr_touch, dev, touch);
    return dev;
}

bool wlr_libinput_get_touch_size_value(struct wlr_touch *touch, int32_t touch_id, double *major,
                                       double *minor)
{
    if (!touch || touch->impl != &libinput_touch_impl || !major || !minor) {
        return false;
    }

    struct libinput_input_device *dev = device_from_touch(touch);
    if (dev->touch_size_slot != touch_id) {
        return false;
    }

    *major = dev->touch_major;
    *minor = dev->touch_minor;
    return dev->touch_size_valid;
}

bool wlr_libinput_get_touch_size_abs(struct wlr_touch *touch, int32_t *major_minimum,
                                     int32_t *major_maximum, int32_t *major_resolution,
                                     int32_t *minor_minimum, int32_t *minor_maximum,
                                     int32_t *minor_resolution)
{
    if (!touch || touch->impl != &libinput_touch_impl || !major_minimum || !major_maximum ||
        !major_resolution || !minor_minimum || !minor_maximum || !minor_resolution) {
        return false;
    }

    struct libinput_input_device *dev = device_from_touch(touch);
    const struct touch_abs *touch_abs = &dev->touch_abs;
    if (touch_abs->major_minimum < 0 || touch_abs->major_maximum < 0 ||
        touch_abs->major_resolution < 0 || touch_abs->minor_minimum < 0 ||
        touch_abs->minor_maximum < 0 || touch_abs->minor_resolution < 0) {
        return false;
    }

    *major_minimum = touch_abs->major_minimum;
    *major_maximum = touch_abs->major_maximum;
    *major_resolution = touch_abs->major_resolution;
    *minor_minimum = touch_abs->minor_minimum;
    *minor_maximum = touch_abs->minor_maximum;
    *minor_resolution = touch_abs->minor_resolution;
    return true;
}

void handle_touch_down(struct libinput_event *event, struct wlr_touch *touch)
{
    struct libinput_event_touch *tevent = libinput_event_get_touch_event(event);
    struct libinput_input_device *dev = device_from_touch(touch);
    const struct libinput_property_exts *exts = &dev->backend->property_exts;
    double major = 0, minor = 0;
    bool has_size = false;

    if (exts->support_touch_size && exts->touch_has_major(tevent) &&
        exts->touch_has_minor(tevent) &&
        exts->touch_get_major_value(tevent, &major) == 0 &&
        exts->touch_get_minor_value(tevent, &minor) == 0) {
        has_size = true;
    }

    struct input_event ie = {
        .type = INPUT_EVENT_TOUCH_DOWN,
        .time_msec = usec_to_msec(libinput_event_touch_get_time_usec(tevent)),
        .touch = {
            .touch_id = libinput_event_touch_get_seat_slot(tevent),
            .x = libinput_event_touch_get_x_transformed(tevent, 1),
            .y = libinput_event_touch_get_y_transformed(tevent, 1),
            .major = major,
            .minor = minor,
            .has_size = has_size,
        },
    };
    queue_input_event(dev, &ie);
}

void handle_touch_up(struct libinput_event *event, struct wlr_touch *touch)
{
    struct libinput_event_touch *tevent = libinput_event_get_touch_event(event);
    struct libinput_input_device *dev = device_from_touch(touch);
    struct input_event ie = {
        .type = INPUT_EVENT_TOUCH_UP,
        .time_msec = usec_to_msec(libinput_event_touch_get_time_usec(tevent)),
        .touch = {
            .touch_id = libinput_event_touch_get_seat_slot(tevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_touch_motion(struct libinput_event *event, struct wlr_touch *touch)
{
    struct libinput_event_touch *tevent = libinput_event_get_touch_event(event);
    struct libinput_input_device *dev = device_from_touch(touch);
    const struct libinput_property_exts *exts = &dev->backend->property_exts;
    double major = 0, minor = 0;
    bool has_size = false;

    if (exts->support_touch_size && exts->touch_has_major(tevent) &&
        exts->touch_has_minor(tevent) &&
        exts->touch_get_major_value(tevent, &major) == 0 &&
        exts->touch_get_minor_value(tevent, &minor) == 0) {
        has_size = true;
    }

    struct input_event ie = {
        .type = INPUT_EVENT_TOUCH_MOTION,
        .time_msec = usec_to_msec(libinput_event_touch_get_time_usec(tevent)),
        .touch = {
            .touch_id = libinput_event_touch_get_seat_slot(tevent),
            .x = libinput_event_touch_get_x_transformed(tevent, 1),
            .y = libinput_event_touch_get_y_transformed(tevent, 1),
            .major = major,
            .minor = minor,
            .has_size = has_size,
        },
    };
    queue_input_event(dev, &ie);
}

void handle_touch_cancel(struct libinput_event *event, struct wlr_touch *touch)
{
    struct libinput_event_touch *tevent = libinput_event_get_touch_event(event);
    struct libinput_input_device *dev = device_from_touch(touch);
    struct input_event ie = {
        .type = INPUT_EVENT_TOUCH_CANCEL,
        .time_msec = usec_to_msec(libinput_event_touch_get_time_usec(tevent)),
        .touch = {
            .touch_id = libinput_event_touch_get_seat_slot(tevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_touch_frame(struct libinput_event *event, struct wlr_touch *touch)
{
    struct libinput_input_device *dev = device_from_touch(touch);
    struct input_event ie = { .type = INPUT_EVENT_TOUCH_FRAME };
    queue_input_event(dev, &ie);
}
