// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>

#include "eis_p.h"
#include "input/cursor.h"
#include "input/seat.h"

static inline bool hit_test(struct barrier *barrier, int lx, int ly)
{
    if (barrier->orientation == VERTICAL) {
        return lx == barrier->position && barrier->start <= ly && ly <= barrier->end;
    } else {
        return ly == barrier->position && barrier->start <= lx && lx <= barrier->end;
    }
}

static bool barrier_hit_test(struct barrier *barrier, int lx, int ly, int dx, int dy)
{
    if (dx == 0 && dy == 0) {
        return false;
    }
    if (!hit_test(barrier, lx, ly)) {
        barrier->hit = 0;
        return false;
    }
    if (++barrier->hit <= 1) {
        return false;
    }

    bool hit = (barrier->orientation == VERTICAL) ? (dx != 0) : (dy != 0);
    if (hit) {
        barrier->hit = 0;
    }
    return hit;
}

static void barrier_hint(struct eis_backend *backend, int lx, int ly, int dx, int dy)
{
    struct eis_context *context;
    wl_list_for_each(context, &backend->contexts, link) {
        if (context->type != EIS_CONTEXT_TYPE_INPUT_CAPTURE) {
            continue;
        }

        struct barrier *barrier;
        wl_array_for_each(barrier, &context->barriers) {
            if (barrier_hit_test(barrier, lx, ly, dx, dy)) {
                eis_context_set_activated(context, true, lx + dx, ly + dy);
                return;
            }
        }
    }
}

static void handle_motion(struct eis_context_client *client, struct wlr_pointer_motion_event *event)
{
    if (!client->pointer) {
        return;
    }

    eis_device_pointer_motion(client->pointer, event->delta_x, event->delta_y);
}

static void handle_button(struct eis_context_client *client, struct wlr_pointer_button_event *event)
{
    if (!client->pointer) {
        return;
    }

    eis_device_button_button(client->pointer, event->button, event->state == WLR_BUTTON_PRESSED);
}

static void handle_axis(struct eis_context_client *client, struct wlr_pointer_axis_event *event)
{
    if (!client->pointer) {
        return;
    }

    if (event->delta) {
        if (event->delta_discrete) {
            if (event->orientation == WLR_AXIS_ORIENTATION_HORIZONTAL) {
                eis_device_scroll_discrete(client->pointer, event->delta_discrete, 0);
            } else {
                eis_device_scroll_discrete(client->pointer, 0, event->delta_discrete);
            }
        } else {
            if (event->orientation == WLR_AXIS_ORIENTATION_HORIZONTAL) {
                eis_device_scroll_delta(client->pointer, event->delta, 0);
            } else {
                eis_device_scroll_delta(client->pointer, 0, event->delta);
            }
        }
    } else {
        if (event->orientation == WLR_AXIS_ORIENTATION_HORIZONTAL) {
            eis_device_scroll_stop(client->pointer, true, false);
        } else {
            eis_device_scroll_stop(client->pointer, false, true);
        }
    }
}

static void handle_frame(struct eis_context_client *client)
{
    if (!client->pointer) {
        return;
    }

    eis_device_frame(client->pointer, eis_now(client->context->eis));
}

static void handle_key(struct eis_context_client *client, struct wlr_keyboard_key_event *event)
{
    if (!client->keyboard) {
        return;
    }

    eis_device_keyboard_key(client->keyboard, event->keycode,
                            event->state != WL_KEYBOARD_KEY_STATE_RELEASED);
    eis_device_frame(client->keyboard, eis_now(client->context->eis));
}

static bool backend_handle_event(struct input_event *event, void *data)
{
    struct eis_backend *backend = data;

    if (!backend->active_capture && event->type == INPUT_EVENT_POINTER_MOTION) {
        struct cursor *cursor = event->input->seat->cursor;
        struct wlr_pointer_motion_event *motion = event->event;
        barrier_hint(backend, cursor->lx, cursor->ly, motion->delta_x, motion->delta_y);
    }

    struct eis_context *context = backend->active_capture;
    if (!context || wl_list_empty(&context->clients)) {
        return false;
    }

    struct eis_context_client *client = wl_container_of(context->clients.next, client, link);

    switch (event->type) {
    case INPUT_EVENT_KEYBOARD_KEY:
        handle_key(client, event->event);
        break;
    case INPUT_EVENT_POINTER_MOTION:
        handle_motion(client, event->event);
        break;
    case INPUT_EVENT_POINTER_BUTTON:
        handle_button(client, event->event);
        break;
    case INPUT_EVENT_POINTER_AXIS:
        handle_axis(client, event->event);
        break;
    case INPUT_EVENT_POINTER_FRAME:
        handle_frame(client);
        break;
    default:
        break;
    }

    return true;
}

void eis_backend_lock_filter(struct eis_backend *backend, bool lock)
{
    if (lock) {
        backend->filter_locks++;
    } else {
        assert(backend->filter_locks > 0);
        --backend->filter_locks;
    }

    kywc_log(KYWC_INFO, "Eis backend input filter %s (locks: %d)", lock ? "Enabling" : "Disabling",
             backend->filter_locks);

    if (backend->filter_locks > 0) {
        input_manager_set_event_filter(backend_handle_event, backend);
    } else {
        input_manager_set_event_filter(NULL, NULL);
    }
}
