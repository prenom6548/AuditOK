// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>

#include <kywc/log.h>

#include "libinput_p.h"
#include "util/time.h"

const struct wlr_pointer_impl libinput_pointer_impl = {
    .name = "libinput-pointer",
};

void init_device_pointer(struct libinput_input_device *dev)
{
    const char *name = libinput_device_get_name(dev->handle);
    struct wlr_pointer *wlr_pointer = &dev->pointer;
    wlr_pointer_init(wlr_pointer, &libinput_pointer_impl, name);
    wlr_pointer->base.vendor = libinput_device_get_id_vendor(dev->handle);
    wlr_pointer->base.product = libinput_device_get_id_product(dev->handle);
}

struct libinput_input_device *device_from_pointer(struct wlr_pointer *wlr_pointer)
{
    assert(wlr_pointer->impl == &libinput_pointer_impl);
    struct libinput_input_device *dev = wl_container_of(wlr_pointer, dev, pointer);
    return dev;
}

void handle_pointer_motion(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_pointer *pevent = libinput_event_get_pointer_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_POINTER_MOTION,
        .time_msec = usec_to_msec(libinput_event_pointer_get_time_usec(pevent)),
        .motion = {
            .dx = libinput_event_pointer_get_dx(pevent),
            .dy = libinput_event_pointer_get_dy(pevent),
            .dx_unaccel = libinput_event_pointer_get_dx_unaccelerated(pevent),
            .dy_unaccel = libinput_event_pointer_get_dy_unaccelerated(pevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_motion_abs(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_pointer *pevent = libinput_event_get_pointer_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_POINTER_MOTION_ABSOLUTE,
        .time_msec = usec_to_msec(libinput_event_pointer_get_time_usec(pevent)),
        .motion_abs = {
            .x = libinput_event_pointer_get_absolute_x_transformed(pevent, 1),
            .y = libinput_event_pointer_get_absolute_y_transformed(pevent, 1),
        },
    };
    queue_input_event(dev, &ie);
}

static bool axis_source_from_libinput(enum libinput_pointer_axis_source source,
                                      enum wlr_axis_source *out)
{
    switch (source) {
    case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
        *out = WLR_AXIS_SOURCE_WHEEL;
        return true;
    case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
        *out = WLR_AXIS_SOURCE_FINGER;
        return true;
    case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
        *out = WLR_AXIS_SOURCE_CONTINUOUS;
        return true;
    case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT:
        *out = WLR_AXIS_SOURCE_WHEEL_TILT;
        return true;
    default:
        return false;
    }
}

void handle_pointer_button(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_pointer *pevent = libinput_event_get_pointer_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_POINTER_BUTTON,
        .time_msec = usec_to_msec(libinput_event_pointer_get_time_usec(pevent)),
        .button = {
            .button = libinput_event_pointer_get_button(pevent),
        },
    };
    enum wlr_button_state state;
    if (!button_state_from_libinput(libinput_event_pointer_get_button_state(pevent), &state)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput button state");
        return;
    }
    ie.button.pressed = (state == WLR_BUTTON_PRESSED);
    queue_input_event(dev, &ie);
}

void handle_pointer_axis(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_pointer *pevent = libinput_event_get_pointer_event(event);
    struct wlr_pointer_axis_event wlr_event = {
        .pointer = pointer,
        .time_msec = usec_to_msec(libinput_event_pointer_get_time_usec(pevent)),
    };
    if (!axis_source_from_libinput(libinput_event_pointer_get_axis_source(pevent),
                                   &wlr_event.source)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput pointer axis source");
        return;
    }
    const enum libinput_pointer_axis axes[] = {
        LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
        LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
    };
    for (size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) {
        if (!libinput_event_pointer_has_axis(pevent, axes[i])) {
            continue;
        }

        switch (axes[i]) {
        case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
            wlr_event.orientation = WLR_AXIS_ORIENTATION_VERTICAL;
            break;
        case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
            wlr_event.orientation = WLR_AXIS_ORIENTATION_HORIZONTAL;
            break;
        }
        wlr_event.delta = libinput_event_pointer_get_axis_value(pevent, axes[i]);
        wlr_event.delta_discrete = libinput_event_pointer_get_axis_value_discrete(pevent, axes[i]);
        wlr_event.delta_discrete *= WLR_POINTER_AXIS_DISCRETE_STEP;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }
    wl_signal_emit_mutable(&pointer->events.frame, pointer);
}

void handle_pointer_axis_value120(struct libinput_event *event, struct wlr_pointer *pointer,
                                  enum wlr_axis_source source)
{
    struct libinput_event_pointer *pevent = libinput_event_get_pointer_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);

    const enum libinput_pointer_axis axes[] = {
        LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL,
        LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL,
    };
    for (size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) {
        if (!libinput_event_pointer_has_axis(pevent, axes[i])) {
            continue;
        }
        struct input_event ie = {
            .type = INPUT_EVENT_POINTER_AXIS,
            .time_msec = usec_to_msec(libinput_event_pointer_get_time_usec(pevent)),
            .axis = {
                .source = source,
            },
        };
        switch (axes[i]) {
        case LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL:
            ie.axis.orientation = WLR_AXIS_ORIENTATION_VERTICAL;
            break;
        case LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL:
            ie.axis.orientation = WLR_AXIS_ORIENTATION_HORIZONTAL;
            break;
        }
        ie.axis.delta = libinput_event_pointer_get_scroll_value(pevent, axes[i]);
        if (source == WLR_AXIS_SOURCE_WHEEL) {
            ie.axis.delta_discrete = libinput_event_pointer_get_scroll_value_v120(pevent, axes[i]);
        }
        queue_input_event(dev, &ie);
    }
}

void handle_pointer_swipe_begin(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_SWIPE_BEGIN,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .fingers = libinput_event_gesture_get_finger_count(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_swipe_update(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_SWIPE_UPDATE,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .fingers = libinput_event_gesture_get_finger_count(gevent),
        .dx = libinput_event_gesture_get_dx_unaccelerated(gevent),
        .dy = libinput_event_gesture_get_dy_unaccelerated(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_swipe_end(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_SWIPE_END,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .cancelled = libinput_event_gesture_get_cancelled(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_pinch_begin(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_PINCH_BEGIN,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .fingers = libinput_event_gesture_get_finger_count(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_pinch_update(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_PINCH_UPDATE,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .fingers = libinput_event_gesture_get_finger_count(gevent),
            .dx = libinput_event_gesture_get_dx_unaccelerated(gevent),
            .dy = libinput_event_gesture_get_dy_unaccelerated(gevent),
            .scale = libinput_event_gesture_get_scale(gevent),
            .angle_delta = libinput_event_gesture_get_angle_delta(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_pinch_end(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_PINCH_END,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .cancelled = libinput_event_gesture_get_cancelled(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_hold_begin(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_HOLD_BEGIN,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .fingers = libinput_event_gesture_get_finger_count(gevent),
        },
    };
    queue_input_event(dev, &ie);
}

void handle_pointer_hold_end(struct libinput_event *event, struct wlr_pointer *pointer)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct libinput_input_device *dev = device_from_pointer(pointer);
    struct input_event ie = {
        .type = INPUT_EVENT_GESTURE_HOLD_END,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .gesture = {
            .cancelled = libinput_event_gesture_get_cancelled(gevent),
        },
    };
    queue_input_event(dev, &ie);
}
