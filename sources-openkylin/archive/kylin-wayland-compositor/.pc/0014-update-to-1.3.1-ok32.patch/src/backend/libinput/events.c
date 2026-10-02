// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <libinput.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "backend/libinput.h"
#include "libinput_p.h"
#include "util/time.h"

void destroy_libinput_input_device(struct libinput_input_device *dev)
{
    if (dev->keyboard.impl) {
        wlr_keyboard_finish(&dev->keyboard);
    }
    if (dev->pointer.impl) {
        wlr_pointer_finish(&dev->pointer);
    }
    if (dev->switch_device.impl) {
        wlr_switch_finish(&dev->switch_device);
    }
    if (dev->touch.impl) {
        wlr_touch_finish(&dev->touch);
    }
    if (dev->tablet.impl) {
        finish_device_tablet(dev);
    }
    if (dev->tablet_pad.impl) {
        finish_device_tablet_pad(dev);
    }

    struct libinput_backend *backend = dev->backend;
    pthread_mutex_lock(&backend->mutex);
    libinput_device_unref(dev->handle);
    wl_list_remove(&dev->link);
    pthread_mutex_unlock(&backend->mutex);
    free(dev);
}

void queue_input_event(struct libinput_input_device *dev, const struct input_event *e)
{
    if (!dev) {
        return;
    }

    struct libinput_backend *backend = dev->backend;

    struct input_event copy = *e;
    copy.dev = dev;
    if (!input_queue_push(&backend->queue, &copy)) {
        kywc_log(KYWC_ERROR, "Input queue push failed (OOM), dropping event type=%u", e->type);
    }
}

static void handle_device_added(struct libinput_backend *backend,
                                struct libinput_device *libinput_dev)
{
    int vendor = libinput_device_get_id_vendor(libinput_dev);
    int product = libinput_device_get_id_product(libinput_dev);
    const char *name = libinput_device_get_name(libinput_dev);
    kywc_log(KYWC_DEBUG, "Adding %s [%d:%d]", name, vendor, product);

    struct libinput_input_device *dev = calloc(1, sizeof(*dev));
    if (!dev) {
        kywc_log_errno(KYWC_ERROR, "failed to allocate libinput_input_device");
        return;
    }

    dev->handle = libinput_dev;
    dev->backend = backend;
    libinput_device_ref(libinput_dev);
    libinput_device_set_user_data(libinput_dev, dev);

    wl_list_insert(&backend->devices, &dev->link);
    wl_signal_init(&dev->events.raw_tap);
    wl_signal_init(&dev->events.zone_begin);
    wl_signal_init(&dev->events.zone_update);
    wl_signal_init(&dev->events.zone_end);

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
        init_device_keyboard(dev);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_POINTER)) {
        init_device_pointer(dev);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_SWITCH)) {
        init_device_switch(dev);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TOUCH)) {
        init_device_touch(dev);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {
        init_device_tablet(dev);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
        init_device_tablet_pad(dev);
    }

    struct input_event ie = { .type = INPUT_EVENT_DEVICE_ADDED };
    queue_input_event(dev, &ie);
}

static void handle_device_removed(struct libinput_backend *backend,
                                  struct libinput_input_device *dev)
{
    struct input_event ie = { .type = INPUT_EVENT_DEVICE_REMOVED };
    queue_input_event(dev, &ie);
}

#if HAVE_LIBINPUT_RAW_TAP
static void handle_pointer_raw_tap(struct libinput_event *event,
                                   struct libinput_input_device *input_device)
{
    struct libinput_event_raw_tap *gevent = libinput_event_get_raw_tap_event(event);
    struct input_event ie = {
        .type = INPUT_EVENT_RAW_TAP,
        .time_msec = usec_to_msec(libinput_event_raw_tap_get_time(gevent)),
        .raw_tap = { .fingers = libinput_event_raw_tap_get_finger_count(gevent) },
    };
    queue_input_event(input_device, &ie);
}
#endif

#if HAVE_LIBINPUT_ZONE_GESTURE
static bool zone_from_libinput(enum libinput_zone_gesture_zone zone,
                               enum pointer_zone_gesture_zone *out)
{
    switch (zone) {
    case LIBINPUT_ZONE_GESTURE_ZONE_LEFT_EDGE:
        *out = POINTER_ZONE_GESTURE_LEFT_EDGE;
        return true;
    case LIBINPUT_ZONE_GESTURE_ZONE_RIGHT_EDGE:
        *out = POINTER_ZONE_GESTURE_RIGHT_EDGE;
        return true;
    case LIBINPUT_ZONE_GESTURE_ZONE_TOP_EDGE:
        *out = POINTER_ZONE_GESTURE_TOP_EDGE;
        return true;
    case LIBINPUT_ZONE_GESTURE_ZONE_OUTSIDE:
        *out = POINTER_ZONE_GESTURE_OUTSIDE;
        return true;
    case LIBINPUT_ZONE_GESTURE_ZONE_NORMAL:
        *out = POINTER_ZONE_GESTURE_NORMAL;
        return true;
    }

    return false;
}

static void handle_pointer_zone_begin(struct libinput_event *event,
                                      struct libinput_input_device *input_device)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    enum pointer_zone_gesture_zone zone;
    if (!zone_from_libinput(libinput_event_gesture_get_zone(gevent), &zone)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput zone gesture zone");
        return;
    }

    struct input_event ie = {
        .type = INPUT_EVENT_ZONE_BEGIN,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .zone_gesture = {
            .zone = zone,
            .fingers = libinput_event_gesture_get_finger_count(gevent),
        },
    };
    queue_input_event(input_device, &ie);
}

static void handle_pointer_zone_update(struct libinput_event *event,
                                       struct libinput_input_device *input_device)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    enum pointer_zone_gesture_zone zone;
    if (!zone_from_libinput(libinput_event_gesture_get_zone(gevent), &zone)) {
        kywc_log(KYWC_DEBUG, "Unhandled libinput zone gesture zone");
        return;
    }

    double delta = zone == POINTER_ZONE_GESTURE_TOP_EDGE
                       ? libinput_event_gesture_get_dx_unaccelerated(gevent)
                       : libinput_event_gesture_get_dy_unaccelerated(gevent);
    struct input_event ie = {
        .type = INPUT_EVENT_ZONE_UPDATE,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .zone_gesture = {
            .zone = zone,
            .fingers = libinput_event_gesture_get_finger_count(gevent),
            .delta = delta,
        },
    };
    queue_input_event(input_device, &ie);
}

static void handle_pointer_zone_end(struct libinput_event *event,
                                    struct libinput_input_device *input_device)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct input_event ie = {
        .type = INPUT_EVENT_ZONE_END,
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .zone_gesture = {
            .cancelled = libinput_event_gesture_get_cancelled(gevent),
        },
    };
    queue_input_event(input_device, &ie);
}
#endif

void handle_libinput_event(struct libinput_backend *backend, struct libinput_event *event)
{
    struct libinput_device *libinput_dev = libinput_event_get_device(event);
    struct libinput_input_device *dev = libinput_device_get_user_data(libinput_dev);
    enum libinput_event_type event_type = libinput_event_get_type(event);

    if (!dev && event_type != LIBINPUT_EVENT_DEVICE_ADDED) {
        kywc_log(KYWC_ERROR, "libinput_device has no libinput_input_device");
        return;
    }

    switch (event_type) {
    case LIBINPUT_EVENT_DEVICE_ADDED:
        handle_device_added(backend, libinput_dev);
        break;
    case LIBINPUT_EVENT_DEVICE_REMOVED:
        handle_device_removed(backend, dev);
        break;
    case LIBINPUT_EVENT_KEYBOARD_KEY:
        handle_keyboard_key(event, &dev->keyboard);
        break;
    case LIBINPUT_EVENT_POINTER_MOTION:
        handle_pointer_motion(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        handle_pointer_motion_abs(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_POINTER_BUTTON:
        handle_pointer_button(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_POINTER_AXIS:
        /* This event must be ignored in favour of the SCROLL_* events */
        break;
    case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
        handle_pointer_axis_value120(event, &dev->pointer, WLR_AXIS_SOURCE_WHEEL);
        break;
    case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
        handle_pointer_axis_value120(event, &dev->pointer, WLR_AXIS_SOURCE_FINGER);
        break;
    case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
        handle_pointer_axis_value120(event, &dev->pointer, WLR_AXIS_SOURCE_CONTINUOUS);
        break;
    case LIBINPUT_EVENT_TOUCH_DOWN:
        handle_touch_down(event, &dev->touch);
        break;
    case LIBINPUT_EVENT_TOUCH_UP:
        handle_touch_up(event, &dev->touch);
        break;
    case LIBINPUT_EVENT_TOUCH_MOTION:
        handle_touch_motion(event, &dev->touch);
        break;
    case LIBINPUT_EVENT_TOUCH_CANCEL:
        handle_touch_cancel(event, &dev->touch);
        break;
    case LIBINPUT_EVENT_TOUCH_FRAME:
        handle_touch_frame(event, &dev->touch);
        break;
    case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
        handle_tablet_tool_axis(event, &dev->tablet);
        break;
    case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
        handle_tablet_tool_proximity(event, &dev->tablet);
        break;
    case LIBINPUT_EVENT_TABLET_TOOL_TIP:
        handle_tablet_tool_tip(event, &dev->tablet);
        break;
    case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
        handle_tablet_tool_button(event, &dev->tablet);
        break;
    case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
        handle_tablet_pad_button(event, &dev->tablet_pad);
        break;
    case LIBINPUT_EVENT_TABLET_PAD_RING:
        handle_tablet_pad_ring(event, &dev->tablet_pad);
        break;
    case LIBINPUT_EVENT_TABLET_PAD_STRIP:
        handle_tablet_pad_strip(event, &dev->tablet_pad);
        break;
    case LIBINPUT_EVENT_SWITCH_TOGGLE:
        handle_switch_toggle(event, &dev->switch_device);
        break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        handle_pointer_swipe_begin(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        handle_pointer_swipe_update(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        handle_pointer_swipe_end(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        handle_pointer_pinch_begin(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        handle_pointer_pinch_update(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_PINCH_END:
        handle_pointer_pinch_end(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
        handle_pointer_hold_begin(event, &dev->pointer);
        break;
    case LIBINPUT_EVENT_GESTURE_HOLD_END:
        handle_pointer_hold_end(event, &dev->pointer);
        break;
#if HAVE_LIBINPUT_RAW_TAP
    case LIBINPUT_EVENT_RAW_TAP:
        handle_pointer_raw_tap(event, dev);
        break;
#endif
#if HAVE_LIBINPUT_ZONE_GESTURE
    case LIBINPUT_EVENT_GESTURE_ZONE_BEGIN:
        handle_pointer_zone_begin(event, dev);
        break;
    case LIBINPUT_EVENT_GESTURE_ZONE_UPDATE:
        handle_pointer_zone_update(event, dev);
        break;
    case LIBINPUT_EVENT_GESTURE_ZONE_END:
        handle_pointer_zone_end(event, dev);
        break;
#endif
    default:
        kywc_log(KYWC_DEBUG, "Unknown libinput event %d", event_type);
        break;
    }
}

void dispatch_input_events(struct libinput_backend *backend)
{
    struct input_event ie;
    while (input_queue_pop(&backend->queue, &ie)) {
        struct libinput_input_device *dev = ie.dev;
        if (!dev) {
            continue;
        }

        switch (ie.type) {
        case INPUT_EVENT_DEVICE_ADDED:
            if (dev->keyboard.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->keyboard.base);
            }
            if (dev->pointer.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->pointer.base);
            }
            if (dev->switch_device.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input,
                                       &dev->switch_device.base);
            }
            if (dev->touch.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->touch.base);
            }
            if (dev->tablet.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->tablet.base);
            }
            if (dev->tablet_pad.impl) {
                wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->tablet_pad.base);
            }
            break;
        case INPUT_EVENT_DEVICE_REMOVED:
            int vendor = libinput_device_get_id_vendor(dev->handle);
            int product = libinput_device_get_id_product(dev->handle);
            const char *name = libinput_device_get_name(dev->handle);
            kywc_log(KYWC_DEBUG, "Removing %s [%d:%d]", name, vendor, product);

            destroy_libinput_input_device(dev);
            break;
        case INPUT_EVENT_KEYBOARD_KEY:
            struct wlr_keyboard_key_event key_ev = {
                .keycode = ie.key.key,
                .state =
                    ie.key.pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED,
                .time_msec = ie.time_msec,
                .update_state = true,
            };
            wlr_keyboard_notify_key(&dev->keyboard, &key_ev);
            break;
        case INPUT_EVENT_POINTER_MOTION:
            struct wlr_pointer_motion_event motion_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .delta_x = ie.motion.dx,
                .delta_y = ie.motion.dy,
                .unaccel_dx = ie.motion.dx_unaccel,
                .unaccel_dy = ie.motion.dy_unaccel,
            };
            wl_signal_emit_mutable(&dev->pointer.events.motion, &motion_ev);
            wl_signal_emit_mutable(&dev->pointer.events.frame, &dev->pointer);
            break;
        case INPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            struct wlr_pointer_motion_absolute_event motion_abs_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .x = ie.motion_abs.x,
                .y = ie.motion_abs.y,
            };
            wl_signal_emit_mutable(&dev->pointer.events.motion_absolute, &motion_abs_ev);
            wl_signal_emit_mutable(&dev->pointer.events.frame, &dev->pointer);
            break;
        case INPUT_EVENT_POINTER_BUTTON:
            struct wlr_pointer_button_event button_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .button = ie.button.button,
                .state = ie.button.pressed ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED,
            };
            wl_signal_emit_mutable(&dev->pointer.events.button, &button_ev);
            wl_signal_emit_mutable(&dev->pointer.events.frame, &dev->pointer);
            break;
        case INPUT_EVENT_POINTER_AXIS:
            struct wlr_pointer_axis_event axis_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .source = ie.axis.source,
                .orientation = ie.axis.orientation,
                .delta = ie.axis.delta,
                .delta_discrete = ie.axis.delta_discrete,
            };
            wl_signal_emit_mutable(&dev->pointer.events.axis, &axis_ev);
            wl_signal_emit_mutable(&dev->pointer.events.frame, &dev->pointer);
            break;
        case INPUT_EVENT_TOUCH_DOWN:
            dev->touch_size_slot = ie.touch.touch_id;
            dev->touch_major = ie.touch.major;
            dev->touch_minor = ie.touch.minor;
            dev->touch_size_valid = ie.touch.has_size;
            struct wlr_touch_down_event down_ev = {
                .touch = &dev->touch,
                .time_msec = ie.time_msec,
                .touch_id = ie.touch.touch_id,
                .x = ie.touch.x,
                .y = ie.touch.y,
            };
            wl_signal_emit_mutable(&dev->touch.events.down, &down_ev);
            break;
        case INPUT_EVENT_TOUCH_UP:
            struct wlr_touch_up_event up_ev = {
                .touch = &dev->touch,
                .time_msec = ie.time_msec,
                .touch_id = ie.touch.touch_id,
            };
            wl_signal_emit_mutable(&dev->touch.events.up, &up_ev);
            break;
        case INPUT_EVENT_TOUCH_MOTION:
            dev->touch_size_slot = ie.touch.touch_id;
            dev->touch_major = ie.touch.major;
            dev->touch_minor = ie.touch.minor;
            dev->touch_size_valid = ie.touch.has_size;
            struct wlr_touch_motion_event touch_motion_ev = {
                .touch = &dev->touch,
                .time_msec = ie.time_msec,
                .touch_id = ie.touch.touch_id,
                .x = ie.touch.x,
                .y = ie.touch.y,
            };
            wl_signal_emit_mutable(&dev->touch.events.motion, &touch_motion_ev);
            break;
        case INPUT_EVENT_TOUCH_CANCEL:
            struct wlr_touch_cancel_event cancel_ev = {
                .touch = &dev->touch,
                .time_msec = ie.time_msec,
                .touch_id = ie.touch.touch_id,
            };
            wl_signal_emit_mutable(&dev->touch.events.cancel, &cancel_ev);
            break;
        case INPUT_EVENT_TOUCH_FRAME:
            wl_signal_emit_mutable(&dev->touch.events.frame, NULL);
            break;
        case INPUT_EVENT_SWITCH_TOGGLE:
            struct wlr_switch_toggle_event switch_ev = {
                .time_msec = ie.time_msec,
                .switch_type = ie.sw.switch_type,
                .switch_state = ie.sw.switch_state,
            };
            wl_signal_emit_mutable(&dev->switch_device.events.toggle, &switch_ev);
            break;
        case INPUT_EVENT_GESTURE_SWIPE_BEGIN:
            struct wlr_pointer_swipe_begin_event swipe_begin_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .fingers = ie.gesture.fingers,
            };
            wl_signal_emit_mutable(&dev->pointer.events.swipe_begin, &swipe_begin_ev);
            break;
        case INPUT_EVENT_GESTURE_SWIPE_UPDATE:
            struct wlr_pointer_swipe_update_event swipe_update_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .fingers = ie.gesture.fingers,
                .dx = ie.gesture.dx,
                .dy = ie.gesture.dy,
            };
            wl_signal_emit_mutable(&dev->pointer.events.swipe_update, &swipe_update_ev);
            break;
        case INPUT_EVENT_GESTURE_SWIPE_END:
            struct wlr_pointer_swipe_end_event swipe_end_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .cancelled = ie.gesture.cancelled,
            };
            wl_signal_emit_mutable(&dev->pointer.events.swipe_end, &swipe_end_ev);
            break;
        case INPUT_EVENT_GESTURE_PINCH_BEGIN:
            struct wlr_pointer_pinch_begin_event pinch_begin_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .fingers = ie.gesture.fingers,
            };
            wl_signal_emit_mutable(&dev->pointer.events.pinch_begin, &pinch_begin_ev);
            break;
        case INPUT_EVENT_GESTURE_PINCH_UPDATE:
            struct wlr_pointer_pinch_update_event pinch_update_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .fingers = ie.gesture.fingers,
                .dx = ie.gesture.dx,
                .dy = ie.gesture.dy,
                .scale = ie.gesture.scale,
                .rotation = ie.gesture.angle_delta,
            };
            wl_signal_emit_mutable(&dev->pointer.events.pinch_update, &pinch_update_ev);
            break;
        case INPUT_EVENT_GESTURE_PINCH_END:
            struct wlr_pointer_pinch_end_event pinch_end_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .cancelled = ie.gesture.cancelled,
            };
            wl_signal_emit_mutable(&dev->pointer.events.pinch_end, &pinch_end_ev);
            break;
        case INPUT_EVENT_GESTURE_HOLD_BEGIN:
            struct wlr_pointer_hold_begin_event hold_begin_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .fingers = ie.gesture.fingers,
            };
            wl_signal_emit_mutable(&dev->pointer.events.hold_begin, &hold_begin_ev);
            break;
        case INPUT_EVENT_GESTURE_HOLD_END:
            struct wlr_pointer_hold_end_event hold_end_ev = {
                .pointer = &dev->pointer,
                .time_msec = ie.time_msec,
                .cancelled = ie.gesture.cancelled,
            };
            wl_signal_emit_mutable(&dev->pointer.events.hold_end, &hold_end_ev);
            break;
        case INPUT_EVENT_TABLET_TOOL_AXIS:
            struct wlr_tablet_tool_axis_event tablet_axis_ev = {
                .tablet = &dev->tablet,
                .tool = (struct wlr_tablet_tool *)ie.tablet_axis.tool,
                .time_msec = ie.time_msec,
                .x = ie.tablet_axis.x,
                .y = ie.tablet_axis.y,
                .dx = ie.tablet_axis.dx,
                .dy = ie.tablet_axis.dy,
                .pressure = ie.tablet_axis.pressure,
                .distance = ie.tablet_axis.distance,
                .tilt_x = ie.tablet_axis.tilt_x,
                .tilt_y = ie.tablet_axis.tilt_y,
                .rotation = ie.tablet_axis.rotation,
                .slider = ie.tablet_axis.slider,
                .wheel_delta = ie.tablet_axis.wheel_delta,
                .updated_axes = ie.tablet_axis.updated_axes,
            };
            wl_signal_emit_mutable(&dev->tablet.events.axis, &tablet_axis_ev);
            break;
        case INPUT_EVENT_TABLET_TOOL_PROXIMITY:
            struct wlr_tablet_tool_proximity_event prox_ev = {
                .tablet = &dev->tablet,
                .tool = (struct wlr_tablet_tool *)ie.tablet_proximity.tool,
                .time_msec = ie.time_msec,
                .x = ie.tablet_proximity.x,
                .y = ie.tablet_proximity.y,
                .state = ie.tablet_proximity.in_proximity ? WLR_TABLET_TOOL_PROXIMITY_IN
                                                          : WLR_TABLET_TOOL_PROXIMITY_OUT,
            };
            wl_signal_emit_mutable(&dev->tablet.events.proximity, &prox_ev);
            break;
        case INPUT_EVENT_TABLET_TOOL_TIP:
            struct wlr_tablet_tool_tip_event tip_ev = {
                .tablet = &dev->tablet,
                .tool = (struct wlr_tablet_tool *)ie.tablet_tip.tool,
                .time_msec = ie.time_msec,
                .x = ie.tablet_tip.x,
                .y = ie.tablet_tip.y,
                .state = ie.tablet_tip.tip_down ? WLR_TABLET_TOOL_TIP_DOWN : WLR_TABLET_TOOL_TIP_UP,
            };
            wl_signal_emit_mutable(&dev->tablet.events.tip, &tip_ev);
            break;
        case INPUT_EVENT_TABLET_TOOL_BUTTON:
            struct wlr_tablet_tool_button_event tablet_button_ev = {
                .tablet = &dev->tablet,
                .tool = (struct wlr_tablet_tool *)ie.tablet_button.tool,
                .time_msec = ie.time_msec,
                .button = ie.tablet_button.button,
                .state = ie.tablet_button.pressed ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED,
            };
            wl_signal_emit_mutable(&dev->tablet.events.button, &tablet_button_ev);
            break;
        case INPUT_EVENT_TABLET_PAD_BUTTON:
            struct wlr_tablet_pad_button_event pad_button_ev = {
                .time_msec = ie.time_msec,
                .button = ie.tablet_pad.button,
                .state = ie.tablet_pad.pressed ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED,
                .mode = ie.tablet_pad.mode,
                .group = ie.tablet_pad.group,
            };
            wl_signal_emit_mutable(&dev->tablet_pad.events.button, &pad_button_ev);
            break;
        case INPUT_EVENT_TABLET_PAD_RING:
            struct wlr_tablet_pad_ring_event pad_ring_ev = {
                .time_msec = ie.time_msec,
                .ring = ie.tablet_pad.number,
                .position = ie.tablet_pad.position,
                .mode = ie.tablet_pad.mode,
                .source = ie.tablet_pad.source,
            };
            wl_signal_emit_mutable(&dev->tablet_pad.events.ring, &pad_ring_ev);
            break;
        case INPUT_EVENT_TABLET_PAD_STRIP:
            struct wlr_tablet_pad_strip_event pad_strip_ev = {
                .time_msec = ie.time_msec,
                .strip = ie.tablet_pad.number,
                .position = ie.tablet_pad.position,
                .mode = ie.tablet_pad.mode,
                .source = ie.tablet_pad.source,
            };
            wl_signal_emit_mutable(&dev->tablet_pad.events.strip, &pad_strip_ev);
            break;
        case INPUT_EVENT_TABLET_TOOL_DESTROY:
            tool_destroy((struct tablet_tool *)ie.tablet_tool_destroy.tool,
                         (struct libinput_input_device *)ie.tablet_tool_destroy.dev);
            break;
        case INPUT_EVENT_RAW_TAP:
            struct pointer_raw_tap_event tap_ev = {
                .time_msec = ie.time_msec,
                .fingers = ie.raw_tap.fingers,
            };
            wl_signal_emit_mutable(&dev->events.raw_tap, &tap_ev);
            break;
#if HAVE_LIBINPUT_ZONE_GESTURE
        case INPUT_EVENT_ZONE_BEGIN:
            struct pointer_zone_gesture_begin_event zone_begin_ev = {
                .time_msec = ie.time_msec,
                .fingers = ie.zone_gesture.fingers,
                .zone = ie.zone_gesture.zone,
            };
            wl_signal_emit_mutable(&dev->events.zone_begin, &zone_begin_ev);
            break;
        case INPUT_EVENT_ZONE_UPDATE:
            struct pointer_zone_gesture_update_event zone_update_ev = {
                .time_msec = ie.time_msec,
                .fingers = ie.zone_gesture.fingers,
                .zone = ie.zone_gesture.zone,
                .delta = ie.zone_gesture.delta,
            };
            wl_signal_emit_mutable(&dev->events.zone_update, &zone_update_ev);
            break;
        case INPUT_EVENT_ZONE_END:
            struct pointer_zone_gesture_end_event zone_end_ev = {
                .time_msec = ie.time_msec,
                .cancelled = ie.zone_gesture.cancelled,
            };
            wl_signal_emit_mutable(&dev->events.zone_end, &zone_end_ev);
            break;
#endif
        default:
            break;
        }
    }
}

bool button_state_from_libinput(enum libinput_button_state state, enum wlr_button_state *out)
{
    switch (state) {
    case LIBINPUT_BUTTON_STATE_RELEASED:
        *out = WLR_BUTTON_RELEASED;
        return true;
    case LIBINPUT_BUTTON_STATE_PRESSED:
        *out = WLR_BUTTON_PRESSED;
        return true;
    default:
        return false;
    }
}
