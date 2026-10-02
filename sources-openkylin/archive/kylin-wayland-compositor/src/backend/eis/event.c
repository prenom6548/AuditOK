// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "eis_p.h"
#include "util/time.h"

static void handle_frame(struct eis_context_client *client, struct eis_device *device,
                         struct eis_event *event)
{
    if (device == client->pointer) {
        wl_signal_emit_mutable(&client->wlr_pointer.events.frame, &client->wlr_pointer);
    } else if (device == client->absolute) {
        wl_signal_emit_mutable(&client->wlr_absolute.events.frame, &client->wlr_absolute);
    } else {
        kywc_log(KYWC_DEBUG, "Unhandled EIS frame event");
    }
}

static void handle_pointer_motion(struct eis_context_client *client, struct eis_device *device,
                                  struct eis_event *event)
{
    if (device != client->pointer) {
        kywc_log(KYWC_DEBUG, "Unhandled EIS pointer motion event");
        return;
    }

    struct wlr_pointer_motion_event wlr_event = {
        .pointer = &client->wlr_pointer,
        .time_msec = current_time_msec(),
        .delta_x = eis_event_pointer_get_dx(event),
        .delta_y = eis_event_pointer_get_dy(event),
        .unaccel_dx = eis_event_pointer_get_dx(event),
        .unaccel_dy = eis_event_pointer_get_dy(event),
    };

    wl_signal_emit_mutable(&client->wlr_pointer.events.motion, &wlr_event);
}

static bool eis_device_transform_coord(struct eis_device *device, double *x, double *y)
{
    struct eis_region *region = eis_device_get_region_at(device, *x, *y);
    if (!region) {
        return false;
    }

    // TODO: check when multi-output
    struct wlr_box box = {
        eis_region_get_x(region),
        eis_region_get_y(region),
        eis_region_get_width(region),
        eis_region_get_height(region),
    };
    *x = (*x - box.x) / box.width;
    *y = (*y - box.y) / box.height;

    return true;
}

static void handle_pointer_motion_absolute(struct eis_context_client *client,
                                           struct eis_device *device, struct eis_event *event)
{
    if (device != client->absolute) {
        kywc_log(KYWC_DEBUG, "Unhandled EIS pointer motion absolute event");
        return;
    }

    double x = eis_event_pointer_get_absolute_x(event);
    double y = eis_event_pointer_get_absolute_y(event);
    // transform to 0..1
    if (!eis_device_transform_coord(device, &x, &y)) {
        return;
    }

    struct wlr_pointer_motion_absolute_event wlr_event = {
        .pointer = &client->wlr_absolute,
        .time_msec = current_time_msec(),
        .x = x,
        .y = y,
    };

    wl_signal_emit_mutable(&client->wlr_absolute.events.motion_absolute, &wlr_event);
}

static void handle_pointer_button(struct eis_context_client *client, struct eis_device *device,
                                  struct eis_event *event)
{
    struct wlr_pointer *pointer = NULL;
    if (device == client->pointer) {
        pointer = &client->wlr_pointer;
    } else if (device == client->absolute) {
        pointer = &client->wlr_absolute;
    } else {
        kywc_log(KYWC_DEBUG, "Unhandled EIS pointer button event");
        return;
    }

    struct wlr_pointer_button_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .button = eis_event_button_get_button(event),
        .state = eis_event_button_get_is_press(event) ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED,
    };

    wl_signal_emit_mutable(&pointer->events.button, &wlr_event);
}

static void handle_scroll_delta(struct eis_context_client *client, struct eis_device *device,
                                struct eis_event *event)
{
    struct wlr_pointer *pointer = NULL;
    if (device == client->pointer) {
        pointer = &client->wlr_pointer;
    } else if (device == client->absolute) {
        pointer = &client->wlr_absolute;
    } else {
        kywc_log(KYWC_DEBUG, "Unhandled EIS scroll delta event");
        return;
    }

    struct wlr_pointer_axis_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .source = WLR_AXIS_SOURCE_WHEEL,
    };

    double x = eis_event_scroll_get_dx(event);
    if (x != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_HORIZONTAL;
        wlr_event.delta = x;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }

    double y = eis_event_scroll_get_dy(event);
    if (y != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_VERTICAL;
        wlr_event.delta = y;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }
}

static void handle_scroll_stop_or_cancel(struct eis_context_client *client,
                                         struct eis_device *device, struct eis_event *event)
{
    struct wlr_pointer *pointer = NULL;
    if (device == client->pointer) {
        pointer = &client->wlr_pointer;
    } else if (device == client->absolute) {
        pointer = &client->wlr_absolute;
    } else {
        kywc_log(KYWC_DEBUG, "Unhandled EIS scroll stop or cancel event");
        return;
    }

    struct wlr_pointer_axis_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .source = WLR_AXIS_SOURCE_WHEEL,
    };

    double x = eis_event_scroll_get_stop_x(event);
    if (x != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_HORIZONTAL;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }

    double y = eis_event_scroll_get_stop_y(event);
    if (y != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_VERTICAL;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }
}

static void handle_scroll_discrete(struct eis_context_client *client, struct eis_device *device,
                                   struct eis_event *event)
{
    struct wlr_pointer *pointer = NULL;
    if (device == client->pointer) {
        pointer = &client->wlr_pointer;
    } else if (device == client->absolute) {
        pointer = &client->wlr_absolute;
    } else {
        kywc_log(KYWC_DEBUG, "Unhandled EIS scroll discrete event");
        return;
    }

    struct wlr_pointer_axis_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .source = WLR_AXIS_SOURCE_WHEEL,
    };

    double x = eis_event_scroll_get_discrete_dx(event);
    if (x != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_HORIZONTAL;
        wlr_event.delta = x * 15 / 120.0;
        wlr_event.delta_discrete = x;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }

    double y = eis_event_scroll_get_discrete_dy(event);
    if (y != 0) {
        wlr_event.orientation = WLR_AXIS_ORIENTATION_VERTICAL;
        wlr_event.delta = y * 15 / 120.0;
        wlr_event.delta_discrete = y;
        wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    }
}

static void handle_keyboard_key(struct eis_context_client *client, struct eis_device *device,
                                struct eis_event *event)
{
    if (device != client->keyboard) {
        kywc_log(KYWC_DEBUG, "Unhandled EIS keyboard key event");
        return;
    }

    struct wlr_keyboard_key_event wlr_event = {
        .time_msec = current_time_msec(),
        .keycode = eis_event_keyboard_get_key(event),
        .state = eis_event_keyboard_get_key_is_press(event) ? WL_KEYBOARD_KEY_STATE_PRESSED
                                                            : WL_KEYBOARD_KEY_STATE_RELEASED,
        .update_state = true,
    };

    wlr_keyboard_notify_key(&client->wlr_keyboard, &wlr_event);
}

void eis_context_client_feed_event(struct eis_context_client *client, struct eis_event *event)
{
    enum eis_event_type type = eis_event_get_type(event);
    struct eis_device *device = eis_event_get_device(event);

    switch (type) {
    case EIS_EVENT_FRAME:
        handle_frame(client, device, event);
        break;
    case EIS_EVENT_DEVICE_START_EMULATING:;
        kywc_log(KYWC_DEBUG, "Device %s is ready to send events", eis_device_get_name(device));
        break;
    case EIS_EVENT_DEVICE_STOP_EMULATING:
        kywc_log(KYWC_DEBUG, "Device %s will no longer send events", eis_device_get_name(device));
        break;
    case EIS_EVENT_POINTER_MOTION:
        handle_pointer_motion(client, device, event);
        break;
    case EIS_EVENT_POINTER_MOTION_ABSOLUTE:
        handle_pointer_motion_absolute(client, device, event);
        break;
    case EIS_EVENT_BUTTON_BUTTON:
        handle_pointer_button(client, device, event);
        break;
    case EIS_EVENT_SCROLL_DELTA:
        handle_scroll_delta(client, device, event);
        break;
    case EIS_EVENT_SCROLL_STOP:
    case EIS_EVENT_SCROLL_CANCEL:
        handle_scroll_stop_or_cancel(client, device, event);
        break;
    case EIS_EVENT_SCROLL_DISCRETE:
        handle_scroll_discrete(client, device, event);
        break;
    case EIS_EVENT_KEYBOARD_KEY:
        handle_keyboard_key(client, device, event);
        break;
    case EIS_EVENT_TOUCH_DOWN:
    case EIS_EVENT_TOUCH_UP:
    case EIS_EVENT_TOUCH_MOTION:
        kywc_log(KYWC_DEBUG, "Unsupport EIS event type %d", type);
        break;
    default:
        kywc_log(KYWC_DEBUG, "Unhandled EIS event type %d", type);
    }
}
