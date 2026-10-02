// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

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

    libinput_device_unref(dev->handle);
    wl_list_remove(&dev->link);
    free(dev);
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
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->keyboard.base);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_POINTER)) {
        init_device_pointer(dev);
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->pointer.base);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_SWITCH)) {
        init_device_switch(dev);
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->switch_device.base);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TOUCH)) {
        init_device_touch(dev);
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->touch.base);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {
        init_device_tablet(dev);
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->tablet.base);
    }

    if (libinput_device_has_capability(libinput_dev, LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
        init_device_tablet_pad(dev);
        wl_signal_emit_mutable(&backend->backend.events.new_input, &dev->tablet_pad.base);
    }
}

static void handle_device_removed(struct libinput_backend *backend,
                                  struct libinput_input_device *dev)
{
    int vendor = libinput_device_get_id_vendor(dev->handle);
    int product = libinput_device_get_id_product(dev->handle);
    const char *name = libinput_device_get_name(dev->handle);
    kywc_log(KYWC_DEBUG, "Removing %s [%d:%d]", name, vendor, product);

    destroy_libinput_input_device(dev);
}

#if HAVE_LIBINPUT_RAW_TAP
static void handle_pointer_raw_tap(struct libinput_event *event,
                                   struct libinput_input_device *input_device)
{
    struct libinput_event_raw_tap *gevent = libinput_event_get_raw_tap_event(event);
    struct pointer_raw_tap_event wlr_event = {
        .time_msec = usec_to_msec(libinput_event_raw_tap_get_time(gevent)),
        .fingers = libinput_event_raw_tap_get_finger_count(gevent)
    };
    wl_signal_emit_mutable(&input_device->events.raw_tap, &wlr_event);
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

    struct pointer_zone_gesture_begin_event wlr_event = {
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .fingers = libinput_event_gesture_get_finger_count(gevent),
        .zone = zone,
    };
    wl_signal_emit_mutable(&input_device->events.zone_begin, &wlr_event);
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
    struct pointer_zone_gesture_update_event wlr_event = {
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .fingers = libinput_event_gesture_get_finger_count(gevent),
        .zone = zone,
        .delta = delta,
    };
    wl_signal_emit_mutable(&input_device->events.zone_update, &wlr_event);
}

static void handle_pointer_zone_end(struct libinput_event *event,
                                    struct libinput_input_device *input_device)
{
    struct libinput_event_gesture *gevent = libinput_event_get_gesture_event(event);
    struct pointer_zone_gesture_end_event wlr_event = {
        .time_msec = usec_to_msec(libinput_event_gesture_get_time_usec(gevent)),
        .cancelled = libinput_event_gesture_get_cancelled(gevent),
    };
    wl_signal_emit_mutable(&input_device->events.zone_end, &wlr_event);
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
