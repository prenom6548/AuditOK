// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/backend/multi.h>
#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_cursor_shape_v1.h>
#include <wlr/types/wlr_keyboard_shortcuts_inhibit_v1.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/types/wlr_virtual_pointer_v1.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "backend/eis.h"
#include "backend/libinput.h"
#include "backend/session.h"
#include "input/function.h"
#include "input/input.h"
#include "input/seat.h"
#include "input_p.h"
#include "output.h"
#include "server.h"
#include "util/time.h"

static struct input_manager *input_manager = NULL;

void input_set_all_cursor(const char *cursor_theme, uint32_t cursor_size)
{
    struct seat *seat;
    wl_list_for_each(seat, &input_manager->seats, link) {
        seat_set_cursor(seat, cursor_theme, cursor_size);
    }
}

void input_add_new_listener(struct wl_listener *listener)
{
    wl_signal_add(&input_manager->events.new_input, listener);
}

static void handle_touchpad_raw_tap(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, raw_tap);
    struct pointer_raw_tap_event *raw_tap_event = data;
    kywc_log(KYWC_DEBUG, "handle raw_tap fingers: %d", raw_tap_event->fingers);
    gesture_state_begin(input->tap_gestures, input->seat, GESTURE_TYPE_TAP, GESTURE_DEVICE_TOUCHPAD,
                        GESTURE_EDGE_NONE, raw_tap_event->fingers);
}

static enum gesture_edge gesture_edge_from_zone(enum pointer_zone_gesture_zone zone)
{
    switch (zone) {
    case POINTER_ZONE_GESTURE_LEFT_EDGE:
        return GESTURE_EDGE_LEFT;
    case POINTER_ZONE_GESTURE_RIGHT_EDGE:
        return GESTURE_EDGE_RIGHT;
    case POINTER_ZONE_GESTURE_TOP_EDGE:
        return GESTURE_EDGE_TOP;
    case POINTER_ZONE_GESTURE_NORMAL:
    case POINTER_ZONE_GESTURE_OUTSIDE:
    default:
        return GESTURE_EDGE_NONE;
    }
}

static void handle_touchpad_zone_gesture_begin(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, zone_gesture_begin);
    struct pointer_zone_gesture_begin_event *event = data;
    enum gesture_edge edge = gesture_edge_from_zone(event->zone);
    if (edge == GESTURE_EDGE_NONE) {
        return;
    }
    if (!input->seat || !input->seat->cursor) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Input %s zone gesture begin zone=%u edge=%d fingers=%u", input->name,
             event->zone, edge, event->fingers);
    gesture_state_begin(&input->seat->cursor->gestures, input->seat, GESTURE_TYPE_SWIPE,
                        GESTURE_DEVICE_TOUCHPAD, edge, event->fingers);
}

static void handle_touchpad_zone_gesture_update(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, zone_gesture_update);
    struct pointer_zone_gesture_update_event *event = data;
    if (!input->seat || !input->seat->cursor) {
        return;
    }

    enum gesture_edge edge = gesture_edge_from_zone(event->zone);
    if (edge == GESTURE_EDGE_NONE || input->seat->cursor->gestures.edge != edge) {
        return;
    }

    double gesture_dx = edge == GESTURE_EDGE_TOP ? event->delta : 0.0;
    double gesture_dy = edge == GESTURE_EDGE_TOP ? 0.0 : event->delta;
    kywc_log(KYWC_DEBUG, "Input %s zone gesture update zone=%u edge=%d delta=%.3f dx=%.3f dy=%.3f",
             input->name, event->zone, edge, event->delta, gesture_dx, gesture_dy);
    gesture_state_update(&input->seat->cursor->gestures, GESTURE_TYPE_SWIPE,
                         GESTURE_DEVICE_TOUCHPAD, gesture_dx, gesture_dy, NAN, NAN);
}

static void handle_touchpad_zone_gesture_end(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, zone_gesture_end);
    struct pointer_zone_gesture_end_event *event = data;
    if (!input->seat || !input->seat->cursor) {
        return;
    }

    if (input->seat->cursor->gestures.edge == GESTURE_EDGE_NONE) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Input %s zone gesture end edge=%d cancelled=%d", input->name,
             input->seat->cursor->gestures.edge, event->cancelled);
    gesture_state_end(&input->seat->cursor->gestures, GESTURE_TYPE_SWIPE, GESTURE_DEVICE_TOUCHPAD,
                      event->cancelled);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    assert(wl_list_empty(&input_manager->events.new_input.listener_list));
    assert(wl_list_empty(&input_manager->events.new_seat.listener_list));

    wl_list_remove(&input_manager->server_destroy.link);

    struct seat *seat, *seat_tmp;
    wl_list_for_each_safe(seat, seat_tmp, &input_manager->seats, link) {
        seat_destroy(seat);
    }

    struct input *input, *input_tmp;
    wl_list_for_each_safe(input, input_tmp, &input_manager->inputs, link) {
        input_destroy(input);
    }

    struct input_keymap *keymap, *keymap_tmp;
    wl_list_for_each_safe(keymap, keymap_tmp, &input_manager->keymaps, link) {
        wl_list_remove(&keymap->link);
        xkb_keymap_unref(keymap->keymap);
        free(keymap);
    }
    queue_fence_finish(&input_manager->fence);

    free(input_manager);
    input_manager = NULL;
}

static void handle_primary_output(struct wl_listener *listener, void *data)
{
    struct kywc_output *primary = data;
    if (!primary) {
        return;
    }

    struct input *input = wl_container_of(listener, input, primary_output);
    wl_list_remove(&input->primary_output.link);
    wl_list_init(&input->primary_output.link);

    struct input_state state = input->state;
    state.mapped_to_output = primary->name;
    input_set_state(input, &state);
}

static void handle_mapped_output_disable(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, mapped_output_disable);
    /* if output backend is destroyed before input backend */
    if (input_manager->server->terminate) {
        wl_list_remove(&input->mapped_output_disable.link);
        wl_list_init(&input->mapped_output_disable.link);
        wl_list_remove(&input->viewport.link);
        wl_list_init(&input->viewport.link);
        return;
    }

    /* current mapped output is being off or destroyed */
    struct input_state state = input->state;
    state.mapped_to_output = NULL;

    if (input->prop.type == WLR_INPUT_DEVICE_TOUCH) {
        struct kywc_output *primary = kywc_output_get_primary();
        /* if it is primary, map to NULL and listen for primary change to remap */
        /* otherwise, map to the primary */
        if (!primary || primary->destroying) {
            kywc_output_add_primary_listener(&input->primary_output);
        } else {
            state.mapped_to_output = primary->name;
        }
    }

    input_set_state(input, &state);
}

static void handle_mapped_output_viewport(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, viewport);
    struct wlr_cursor *wlr_cursor = input->seat->cursor->wlr_cursor;
    struct output *output = output_from_kywc_output(input->mapped_output);
    if (kywc_box_not_empty(&output->scene_output->viewport.src)) {
        struct wlr_box box = {
            output->scene_output->viewport.src.x,
            output->scene_output->viewport.src.y,
            output->scene_output->viewport.src.width,
            output->scene_output->viewport.src.height,
        };
        wlr_cursor_map_input_to_region(wlr_cursor, input->wlr_input, &box);
    } else {
        wlr_cursor_map_input_to_output(wlr_cursor, input->wlr_input, output->wlr_output);
    }
}

static void _input_destroy(struct input *input)
{
    /* Tell the user the name of the removed device. */
    input_notify_destroy(input);

    wl_signal_emit_mutable(&input->events.destroy, NULL);
    assert(wl_list_empty(&input->events.destroy.listener_list));

    wl_list_remove(&input->link);
    wl_list_remove(&input->destroy.link);
    wl_list_remove(&input->mapped_output_disable.link);
    wl_list_remove(&input->primary_output.link);
    wl_list_remove(&input->viewport.link);
    wl_list_remove(&input->raw_tap.link);
    wl_list_remove(&input->zone_gesture_begin.link);
    wl_list_remove(&input->zone_gesture_update.link);
    wl_list_remove(&input->zone_gesture_end.link);

    kywc_log(KYWC_DEBUG, "Input device %s destroy", input->name);

    if (input->seat) {
        seat_remove_input(input);
    }

    if (input->tap_gestures) {
        gesture_state_finish(input->tap_gestures);
        free(input->tap_gestures);
    }

    free((void *)input->name);
    free(input);
}

static void handle_input_destroy(struct wl_listener *listener, void *data)
{
    struct input *input = wl_container_of(listener, input, destroy);
    _input_destroy(input);
}

static void input_get_prop(struct input *input, struct input_prop *prop)
{
    struct wlr_input_device *wlr_input = input->wlr_input;

    input->prop.type = wlr_input->type;
    input->prop.vendor = wlr_input->vendor;
    input->prop.product = wlr_input->product;
    input->prop.is_virtual = strncmp(input->name, "V_", 2) == 0;
    input->prop.support_mapped_to_output = wlr_input->type == WLR_INPUT_DEVICE_POINTER ||
                                           wlr_input->type == WLR_INPUT_DEVICE_TOUCH ||
                                           wlr_input->type == WLR_INPUT_DEVICE_TABLET_TOOL;

    if (input->device) {
        libinput_get_prop(input, prop);
    }
}

static void input_get_state(struct input *input, struct input_state *state)
{
    struct wlr_input_device *wlr_input = input->wlr_input;

    state->seat = input->seat ? input->seat->name : NULL;
    state->mapped_to_output = input->mapped_output ? input->mapped_output->name : NULL;

    state->scroll_factor = input->state.scroll_factor > 0 ? input->state.scroll_factor
                                                          : input->default_state.scroll_factor;
    state->double_click_time = input->state.double_click_time > 0
                                   ? input->state.double_click_time
                                   : input->default_state.double_click_time;

    if (input->prop.type == WLR_INPUT_DEVICE_KEYBOARD) {
        struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(wlr_input);
        state->repeat_rate = wlr_keyboard->repeat_info.rate;
        state->repeat_delay = wlr_keyboard->repeat_info.delay;
    }

    if (input->device) {
        libinput_get_state(input, state);
    }
}

static void input_get_default_state(struct input *input, struct input_state *state)
{
    state->seat = NULL;
    state->mapped_to_output = NULL;

    state->scroll_factor = 1.0;
    state->double_click_time = DEFAULT_DOUBLE_CLICK_TIME;

    if (input->prop.type == WLR_INPUT_DEVICE_KEYBOARD) {
        state->repeat_rate = 25;
        state->repeat_delay = 600;
    }

    if (input->device) {
        libinput_get_default_state(input, state);
    }
}

static void input_set_name(struct input *input, const char *name)
{
    static int index = 0;
    input->name = name;

    struct input *device;
    wl_list_for_each(device, &input_manager->inputs, link) {
        if (strcmp(device->name, name) == 0) {
            input->name = kywc_identifier_generate("%s_%d", name, ++index);
            free((void *)name);
            return;
        }
    }
}

static struct input *input_create(const char *name, struct wlr_input_device *wlr_input)
{
    struct input *input = calloc(1, sizeof(*input));
    if (!input) {
        return NULL;
    }

    input->wlr_input = wlr_input;
    wlr_input->data = input;
    input->destroy.notify = handle_input_destroy;
    wl_signal_add(&wlr_input->events.destroy, &input->destroy);
    /* avoid duplicate names */
    input_set_name(input, name);

    input->manager = input_manager;
    wl_signal_init(&input->events.destroy);
    wl_list_insert(&input_manager->inputs, &input->link);

    input->mapped_output_disable.notify = handle_mapped_output_disable;
    input->primary_output.notify = handle_primary_output;
    input->viewport.notify = handle_mapped_output_viewport;
    input->raw_tap.notify = handle_touchpad_raw_tap;
    input->zone_gesture_begin.notify = handle_touchpad_zone_gesture_begin;
    input->zone_gesture_update.notify = handle_touchpad_zone_gesture_update;
    input->zone_gesture_end.notify = handle_touchpad_zone_gesture_end;
    wl_list_init(&input->mapped_output_disable.link);
    wl_list_init(&input->primary_output.link);
    wl_list_init(&input->viewport.link);
    wl_list_init(&input->raw_tap.link);
    wl_list_init(&input->zone_gesture_begin.link);
    wl_list_init(&input->zone_gesture_update.link);
    wl_list_init(&input->zone_gesture_end.link);

    if (wlr_input_device_is_libinput(wlr_input)) {
        input->device = wlr_libinput_get_device_handle(wlr_input);
    }

    input_get_prop(input, &input->prop);
    input_get_default_state(input, &input->default_state);
    input_get_state(input, &input->state);

    // touchpad checked
    if (input->prop.type == WLR_INPUT_DEVICE_POINTER && input->prop.tap_finger_count &&
        input->device) {
        input->tap_gestures = calloc(1, sizeof(*input->tap_gestures));
        gesture_state_init(input->tap_gestures, input_manager->server->display);
        wlr_libinput_add_raw_tap_listener(input->wlr_input, &input->raw_tap);
    }

    if (input->prop.type == WLR_INPUT_DEVICE_POINTER && input->prop.has_zone_gesture &&
        input->device) {
        wlr_libinput_add_zone_gesture_begin_listener(input->wlr_input, &input->zone_gesture_begin);
        wlr_libinput_add_zone_gesture_update_listener(input->wlr_input,
                                                      &input->zone_gesture_update);
        wlr_libinput_add_zone_gesture_end_listener(input->wlr_input, &input->zone_gesture_end);
    }

    struct input_state state = input->state;
    bool found = input_read_config(input, &state);
    if (!found) {
        // keep default
    }

    // map touch screen to primary output by default
    if (input->prop.type == WLR_INPUT_DEVICE_TOUCH) {
        struct kywc_output *primary = kywc_output_get_primary();
        state.mapped_to_output = primary ? primary->name : NULL;
    }

    input_set_state(input, &state);

    wl_signal_emit_mutable(&input_manager->events.new_input, input);

    if (kywc_log_get_level() == KYWC_DEBUG) {
        kywc_log(KYWC_DEBUG, "Input device %s create", input->name);
        input_prop_and_state_debug(input);
    }

    /* Tell the user the name of the added device. */
    input_notify_create(input);

    return input;
}

struct input *input_create_pointer(const char *name)
{
    static const struct wlr_pointer_impl pointer_impl = {
        .name = "pointer",
    };

    struct wlr_pointer *pointer = calloc(1, sizeof(*pointer));
    if (!pointer) {
        return NULL;
    }

    const char *pointer_name = kywc_identifier_generate("V_%s-%s", name, pointer_impl.name);
    if (!pointer_name) {
        free(pointer);
        return NULL;
    }

    wlr_pointer_init(pointer, &pointer_impl, pointer_name);

    struct input *input = input_create(pointer_name, &pointer->base);
    if (input) {
        input->own_device = true;
        return input;
    }

    wlr_pointer_finish(pointer);
    free((void *)pointer_name);
    free(pointer);
    return NULL;
}

struct input *input_create_keyboard(const char *name)
{
    static const struct wlr_keyboard_impl keyboard_impl = {
        .name = "keyboard",
    };

    struct wlr_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    if (!keyboard) {
        return NULL;
    }

    const char *keyboard_name = kywc_identifier_generate("V_%s-%s", name, keyboard_impl.name);
    if (!keyboard_name) {
        free(keyboard);
        return NULL;
    }

    wlr_keyboard_init(keyboard, &keyboard_impl, keyboard_name);

    struct input *input = input_create(keyboard_name, &keyboard->base);
    if (input) {
        input->own_device = true;
        return input;
    }

    wlr_keyboard_finish(keyboard);
    free((void *)keyboard_name);
    free(keyboard);
    return NULL;
}

void input_destroy(struct input *input)
{
    if (!input || !input->own_device) {
        return;
    }

    void *device = NULL;

    switch (input->prop.type) {
    case WLR_INPUT_DEVICE_POINTER:;
        struct wlr_pointer *pointer = wlr_pointer_from_input_device(input->wlr_input);
        wlr_pointer_finish(pointer);
        device = pointer;
        break;
    case WLR_INPUT_DEVICE_KEYBOARD:;
        struct wlr_keyboard *keyboard = wlr_keyboard_from_input_device(input->wlr_input);
        wlr_keyboard_finish(keyboard);
        device = keyboard;
    default:
        break;
    }

    free(device);
}

void input_feed_pointer_motion(struct input *input, double x, double y, bool absolute)
{
    if (input->prop.type != WLR_INPUT_DEVICE_POINTER) {
        return;
    }

    struct wlr_pointer *pointer = wlr_pointer_from_input_device(input->wlr_input);

    if (absolute) {
        x -= input->seat->cursor->lx;
        y -= input->seat->cursor->ly;
    }

    struct wlr_pointer_motion_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .delta_x = x,
        .delta_y = y,
        .unaccel_dx = x,
        .unaccel_dy = y,
    };

    wl_signal_emit_mutable(&pointer->events.motion, &wlr_event);
    wl_signal_emit_mutable(&pointer->events.frame, pointer);
}

void input_feed_pointer_button(struct input *input, uint32_t button, bool pressed)
{
    if (input->prop.type != WLR_INPUT_DEVICE_POINTER) {
        return;
    }

    struct wlr_pointer *pointer = wlr_pointer_from_input_device(input->wlr_input);

    struct wlr_pointer_button_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .button = button,
        .state = pressed ? WLR_BUTTON_PRESSED : WLR_BUTTON_RELEASED,
    };

    wl_signal_emit_mutable(&pointer->events.button, &wlr_event);
    wl_signal_emit_mutable(&pointer->events.frame, pointer);
}

void input_feed_pointer_axis(struct input *input, uint32_t axis, double step)
{
    if (input->prop.type != WLR_INPUT_DEVICE_POINTER) {
        return;
    }

    struct wlr_pointer *pointer = wlr_pointer_from_input_device(input->wlr_input);

    struct wlr_pointer_axis_event wlr_event = {
        .pointer = pointer,
        .time_msec = current_time_msec(),
        .source = WLR_AXIS_SOURCE_WHEEL,
        .orientation = axis,
        .delta = step,
    };

    wl_signal_emit_mutable(&pointer->events.axis, &wlr_event);
    wl_signal_emit_mutable(&pointer->events.frame, pointer);
}

void input_feed_keyboard_key(struct input *input, uint32_t key, bool pressed)
{
    if (input->prop.type != WLR_INPUT_DEVICE_KEYBOARD) {
        return;
    }

    struct wlr_keyboard *keyboard = wlr_keyboard_from_input_device(input->wlr_input);

    struct wlr_keyboard_key_event wlr_event = {
        .time_msec = current_time_msec(),
        .keycode = key,
        .state = pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED,
        .update_state = true,
    };

    wlr_keyboard_notify_key(keyboard, &wlr_event);
}

static void handle_new_input(struct wl_listener *listener, void *data)
{
    struct wlr_input_device *wlr_input = data;
    const char *name;

    if (wlr_input_device_is_eis(wlr_input)) {
        name = kywc_identifier_generate("V_%s", wlr_input->name);
    } else {
        name = kywc_identifier_generate("%d:%d:%d:%s", wlr_input->type, wlr_input->vendor,
                                        wlr_input->product, wlr_input->name);
    }
    if (!name) {
        return;
    }

    if (!input_create(name, wlr_input)) {
        free((void *)name);
    }
}

static void handle_new_virtual_pointer(struct wl_listener *listener, void *data)
{
    struct wlr_virtual_pointer_v1_new_pointer_event *event = data;
    struct wlr_virtual_pointer_v1 *pointer = event->new_pointer;
    struct wlr_input_device *wlr_input = &pointer->pointer.base;

    const char *name = kywc_identifier_generate("V_%s", wlr_input->name);
    if (!name) {
        return;
    }

    struct input *input = input_create(name, wlr_input);
    if (!input) {
        free((void *)name);
        return;
    }

    /* apply suggested seat and output */
    if (event->suggested_seat || event->suggested_output) {
        struct input_state state = input->state;
        if (event->suggested_seat) {
            state.seat = event->suggested_seat->name;
        }
        if (event->suggested_output) {
            state.mapped_to_output = event->suggested_output->name;
        }
        input_set_state(input, &state);
    }
}

static void handle_new_virtual_keyboard(struct wl_listener *listener, void *data)
{
    struct wlr_virtual_keyboard_v1 *keyboard = data;
    struct wlr_input_device *wlr_input = &keyboard->keyboard.base;

    const char *name = kywc_identifier_generate("V_%s", wlr_input->name);
    if (!name) {
        return;
    }

    struct input *input = input_create(name, wlr_input);
    if (!input) {
        free((void *)name);
        return;
    }

    /* apply keyboard seat */
    if (strcmp(input->seat->name, keyboard->seat->name)) {
        struct input_state state = input->state;
        state.seat = keyboard->seat->name;
        input_set_state(input, &state);
    }
}

uint32_t input_manager_for_each_seat(seat_iterator_func_t iterator, void *data)
{
    uint32_t index = 0;
    struct seat *seat;
    wl_list_for_each(seat, &input_manager->seats, link) {
        if (iterator(seat, index++, data)) {
            break;
        }
    }
    return index;
}

static void handle_keyboard_shortcuts_inhibitor_destroy(struct wl_listener *listener, void *data)
{
    struct seat_keyboard_shortcuts_inhibitor *shortcuts_inhibitor =
        wl_container_of(listener, shortcuts_inhibitor, destroy);

    wl_list_remove(&shortcuts_inhibitor->link);
    wl_list_remove(&shortcuts_inhibitor->destroy.link);
    free(shortcuts_inhibitor);
}

static void handle_new_shortcuts_inhibitor(struct wl_listener *listener, void *data)
{
    struct wlr_keyboard_shortcuts_inhibitor_v1 *inhibitor = data;

    struct seat_keyboard_shortcuts_inhibitor *shortcuts_inhibitor =
        calloc(1, sizeof(*shortcuts_inhibitor));
    if (!shortcuts_inhibitor) {
        return;
    }

    shortcuts_inhibitor->inhibitor = inhibitor;
    shortcuts_inhibitor->destroy.notify = handle_keyboard_shortcuts_inhibitor_destroy;
    wl_signal_add(&inhibitor->events.destroy, &shortcuts_inhibitor->destroy);

    struct seat *seat = seat_from_wlr_seat(inhibitor->seat);
    wl_list_insert(&seat->keyboard_shortcuts_inhibitors, &shortcuts_inhibitor->link);

    wlr_keyboard_shortcuts_inhibitor_v1_activate(inhibitor);
}

static void handle_new_pointer_constraint(struct wl_listener *listener, void *data)
{
    struct wlr_pointer_constraint_v1 *constraint = data;
    struct seat *seat = seat_from_wlr_seat(constraint->seat);
    cursor_constraint_create(seat->cursor, constraint);
}

static void handle_request_set_cursor_shape(struct wl_listener *listener, void *data)
{
    const struct wlr_cursor_shape_manager_v1_request_set_shape_event *event = data;
    struct seat *seat = seat_from_wlr_seat(event->seat_client->seat);

    struct wlr_seat_client *focused_client = seat->wlr_seat->pointer_state.focused_client;
    if (seat->pointer_grab || focused_client != event->seat_client) {
        return;
    }

    cursor_set_image(seat->cursor, (enum cursor_name)event->shape);
}

struct xkb_keymap *input_get_or_create_keymap(struct keymap_rules *rules, bool wait)
{
    if (wait) {
        queue_fence_wait(&input_manager->fence);
    }

    struct input_keymap *keymap;
    wl_list_for_each(keymap, &input_manager->keymaps, link) {
        if (!keyboard_check_keymap_rules(&keymap->rules, rules)) {
            return keymap->keymap;
        }
    }

    keymap = calloc(1, sizeof(*keymap));
    if (!keymap) {
        return NULL;
    }
    keymap->keymap = keyboard_compile_keymap(&keymap->rules);
    if (!keymap->keymap) {
        kywc_log(KYWC_ERROR, "Keymap compile failed, text input is broken");
        free(keymap);
        return NULL;
    }

    keymap->rules = *rules;
    wl_list_insert(&input_manager->keymaps, &keymap->link);

    return keymap->keymap;
}

static void compile_keymap(void *job, void *gdata, int index)
{
    struct keymap_rules rules = { 0 };
    input_get_or_create_keymap(&rules, false);
}

static void handle_backend_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&input_manager->backend_destroy.link);
    wl_list_remove(&input_manager->new_input.link);
    wl_list_remove(&input_manager->new_virtual_keyboard.link);
    wl_list_remove(&input_manager->new_virtual_pointer.link);
    wl_list_remove(&input_manager->new_shortcuts_inhibit.link);
    wl_list_remove(&input_manager->new_pointer_constraint.link);
    wl_list_remove(&input_manager->request_set_cursor_shape.link);
}

struct input_manager *input_manager_create(struct server *server)
{
    input_manager = calloc(1, sizeof(*input_manager));
    if (!input_manager) {
        return NULL;
    }

    input_manager->server = server;
    wl_list_init(&input_manager->seats);
    wl_list_init(&input_manager->inputs);
    wl_signal_init(&input_manager->events.new_input);
    wl_signal_init(&input_manager->events.new_seat);

    input_manager->backend_destroy.notify = handle_backend_destroy;
    wl_signal_add(&server->backend->events.destroy, &input_manager->backend_destroy);

    input_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &input_manager->server_destroy);

    wl_list_init(&input_manager->keymaps);
    queue_fence_init(&input_manager->fence);
    if (!queue_add_job(server->queue, input_manager, &input_manager->fence, compile_keymap, NULL)) {
        compile_keymap(input_manager, server, -1);
    }

    input_manager->new_input.notify = handle_new_input;
    wl_signal_add(&server->backend->events.new_input, &input_manager->new_input);

    struct wlr_backend *eis = eis_backend_create(server->display, server->layout);
    if (eis) {
        wlr_multi_backend_add(server->backend, eis);
    }

    input_manager->virtual_pointer = wlr_virtual_pointer_manager_v1_create(server->display);
    input_manager->new_virtual_pointer.notify = handle_new_virtual_pointer;
    wl_signal_add(&input_manager->virtual_pointer->events.new_virtual_pointer,
                  &input_manager->new_virtual_pointer);

    input_manager->virtual_keyboard = wlr_virtual_keyboard_manager_v1_create(server->display);
    input_manager->new_virtual_keyboard.notify = handle_new_virtual_keyboard;
    wl_signal_add(&input_manager->virtual_keyboard->events.new_virtual_keyboard,
                  &input_manager->new_virtual_keyboard);

    input_manager->pointer_gestures = wlr_pointer_gestures_v1_create(server->display);
    input_manager->relative_pointer = wlr_relative_pointer_manager_v1_create(server->display);

    input_manager->shortcuts_inhibit = wlr_keyboard_shortcuts_inhibit_v1_create(server->display);
    input_manager->new_shortcuts_inhibit.notify = handle_new_shortcuts_inhibitor;
    wl_signal_add(&input_manager->shortcuts_inhibit->events.new_inhibitor,
                  &input_manager->new_shortcuts_inhibit);

    input_manager->pointer_constraints = wlr_pointer_constraints_v1_create(server->display);
    input_manager->new_pointer_constraint.notify = handle_new_pointer_constraint;
    wl_signal_add(&input_manager->pointer_constraints->events.new_constraint,
                  &input_manager->new_pointer_constraint);

    input_manager->cursor_shape = wlr_cursor_shape_manager_v1_create(server->display, 1);
    input_manager->request_set_cursor_shape.notify = handle_request_set_cursor_shape;
    wl_signal_add(&input_manager->cursor_shape->events.request_set_shape,
                  &input_manager->request_set_cursor_shape);

    function_manager_init();
    input_manager_config_init(input_manager);
    selection_manager_create(input_manager);
    input_monitor_create(input_manager);
    bindings_create(input_manager);
    input_method_manager_create(input_manager);

    touch_manager_create(input_manager);
    tablet_manager_create(input_manager);

    kde_keystate_manager_create(input_manager);
    transient_seat_manager_create(input_manager);

    global_shortcuts_manager_create(server);
    global_gestures_manager_create(server);

    idle_manager_create(server);
    idle_inhibit_manager_create(server);

    /* create the default seat */
    input_manager->default_seat = seat_create(input_manager, "seat0");
    input_manager->bind_seat = input_set_seat;

    return input_manager;
}

void input_set_seat(struct input *input, const char *seat)
{
    /* already have attached to seat */
    if (input->seat) {
        if (!strcmp(seat, input->seat->name)) {
            return;
        } else {
            /* remove from prev seat */
            seat_remove_input(input);
        }
    }

    input->seat = seat_by_name(seat);
    /* create a new seat */
    if (!input->seat) {
        input->seat = seat_create(input_manager, seat);
    }
    seat_add_input(input->seat, input);
}

static bool _input_set_state(struct input *input, struct input_state *state)
{
    struct wlr_input_device *wlr_input = input->wlr_input;

    /* config keyboard with input state */
    if (input->prop.type == WLR_INPUT_DEVICE_KEYBOARD) {
        struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(wlr_input);
        bool keymap_changed = !wlr_keyboard->keymap ||
                              keyboard_check_keymap_rules(&input->state.rules, &state->rules);
        bool repeat_info_changed = wlr_keyboard->repeat_info.rate != state->repeat_rate ||
                                   wlr_keyboard->repeat_info.delay != state->repeat_delay;

        /* we need remove this input and add later */
        if (!input->prop.is_virtual && wlr_keyboard->group &&
            (keymap_changed || repeat_info_changed)) {
            kywc_log(KYWC_DEBUG, "Input %s is removed and be added later", input->name);
            seat_remove_input(input);
        }

        if (keymap_changed) {
            struct xkb_keymap *keymap = input_get_or_create_keymap(&state->rules, true);
            wlr_keyboard_set_keymap(wlr_keyboard, keymap);
        }
        if (repeat_info_changed) {
            wlr_keyboard_set_repeat_info(wlr_keyboard, state->repeat_rate, state->repeat_delay);
        }
    }

    input->state.scroll_factor = state->scroll_factor;
    input->state.double_click_time = state->double_click_time;

    /* choose a suitable seat, add the input device to the seat */
    input_manager->bind_seat(input, state->seat ? state->seat : "seat0");

    if (input->prop.support_mapped_to_output) {
        struct kywc_output *mapped_output = NULL;
        if (state->mapped_to_output) {
            mapped_output = kywc_output_by_name(state->mapped_to_output);
            if (mapped_output && (!mapped_output->state.enabled || mapped_output->destroying)) {
                kywc_log(KYWC_WARN, "mapped output %s is not available", mapped_output->name);
                mapped_output = NULL;
            }
        }

        struct wlr_cursor *wlr_cursor = input->seat->cursor->wlr_cursor;
        struct wlr_output *wlr_output =
            mapped_output ? output_from_kywc_output(mapped_output)->wlr_output : NULL;
        wlr_cursor_map_input_to_output(wlr_cursor, wlr_input, wlr_output);
        input->mapped_output = mapped_output;
    }

    if (input->device) {
        return libinput_set_state(input, state);
    }

    return true;
}

bool input_set_state(struct input *input, struct input_state *state)
{
    struct kywc_output *old_mapped_output = input->mapped_output;

    bool success = _input_set_state(input, state);
    /* update state anyway */
    input_get_state(input, &input->state);

    if (old_mapped_output != input->mapped_output) {
        wl_list_remove(&input->mapped_output_disable.link);
        wl_list_init(&input->mapped_output_disable.link);
        wl_list_remove(&input->viewport.link);
        wl_list_init(&input->viewport.link);

        if (input->mapped_output) {
            struct output *output = output_from_kywc_output(input->mapped_output);
            struct scene_output *scene_output = output->scene_output;
            wl_signal_add(&scene_output->events.destroy, &input->mapped_output_disable);
            wl_signal_add(&scene_output->events.viewport, &input->viewport);

            handle_mapped_output_viewport(&input->viewport, NULL);
        }
    }

    if (!input->prop.is_virtual) {
        input_write_config(input);
    }
    return success;
}

struct input *input_by_name(const char *name)
{
    struct input *input;
    wl_list_for_each(input, &input_manager->inputs, link) {
        if (!strcmp(input->name, name)) {
            return input;
        }
    }

    return NULL;
}

struct input *input_from_wlr_input(struct wlr_input_device *wlr_input)
{
    return wlr_input->data;
}

struct seat *input_manager_get_default_seat(void)
{
    return input_manager->default_seat;
}

void input_manager_set_event_filter(event_filter_func_t filter, void *data)
{
    input_manager->event_filter = filter;
    input_manager->event_filter_data = data;
}

bool input_event_filter(struct seat *seat, struct input *input, enum input_event_type type,
                        void *event)
{
    idle_manager_notify_activity(seat);

    if (!input_manager->event_filter) {
        return false;
    }

    struct input_event ev = {
        .seat = seat,
        .input = input,
        .type = type,
        .event = event,
    };

    return input_manager->event_filter(&ev, input_manager->event_filter_data);
}

struct output *input_current_output(struct seat *seat)
{
    struct wlr_output *wlr_output =
        wlr_output_layout_output_at(seat->layout, seat->cursor->lx, seat->cursor->ly);
    if (!wlr_output) {
        /* sync the position in wlr_cursor */
        cursor_move(seat->cursor, NULL, 0, 0, true, false);
        wlr_output = wlr_output_layout_output_at(seat->layout, seat->cursor->lx, seat->cursor->ly);
    }
    return wlr_output ? output_from_wlr_output(wlr_output) : NULL;
}

void input_get_mapped_size(struct input *input, int *width, int *height)
{
    if (input->mapped_output) {
        struct output *output = output_from_kywc_output(input->mapped_output);
        if (width) {
            *width = output->geometry.width;
        }
        if (height) {
            *height = output->geometry.height;
        }
    } else {
        output_layout_get_size(width, height);
    }
}

void input_manager_switch_vt(unsigned vt)
{
    struct server *server = input_manager->server;
    if (server->vtnr == vt) {
        return;
    }

    struct session *session = server->session;
    if (session) {
        server->active = false;
        if (!session_change_vt(session, vt)) {
            server->active = true;
        }
    }
}

struct seat *seat_by_name(const char *seat_name)
{
    struct seat *seat;
    wl_list_for_each(seat, &input_manager->seats, link) {
        if (strcmp(seat->name, seat_name) == 0) {
            return seat;
        }
    }
    return NULL;
}

void seat_add_new_listener(struct wl_listener *listener)
{
    wl_signal_add(&input_manager->events.new_seat, listener);
}
