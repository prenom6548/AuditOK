// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE

#include <libei.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
#include <fcntl.h>

#include "ei_sender.h"
#include "util/event.h"

#define AXIS_VERTICAL_SCROLL 0
#define AXIS_HORIZONTAL_SCROLL 1

static void xkb_keyboard_init(struct ei_sender *sender, struct ei_device *device)
{
    struct ei_keymap *ei_keymap = ei_device_keyboard_get_keymap(device);
    if (ei_keymap && ei_keymap_get_type(ei_keymap) == EI_KEYMAP_TYPE_XKB) {
        int fd = ei_keymap_get_fd(ei_keymap);
        size_t len = ei_keymap_get_size(ei_keymap);
        char *buffer = (char *)malloc(len + 1);
        if (!buffer) {
            kywc_log(KYWC_WARN, "failed to allocate buffer for ei_keymap");
            return;
        }

        lseek(fd, 0, SEEK_SET);
        ssize_t sz = read(fd, buffer, len);
        if (sz < 0 || (size_t)sz < len) {
            kywc_log(KYWC_WARN, "failed to read ei_keymap: %s", strerror(errno));
            free(buffer);
            return;
        }
        buffer[len] = '\0';

        if (!sender->keyboard_info.context) {
            return;
        }
        struct xkb_keymap *keymap =
            xkb_keymap_new_from_string(sender->keyboard_info.context, buffer,
                                       XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        free(buffer);
        if (!keymap) {
            kywc_log(KYWC_WARN, "failed to create xkb_keymap");
            return;
        }

        if (sender->keyboard_info.keymap) {
            xkb_keymap_unref(sender->keyboard_info.keymap);
        }
        sender->keyboard_info.keymap = keymap;
        if (sender->keyboard_info.state) {
            xkb_state_unref(sender->keyboard_info.state);
        }
        sender->keyboard_info.state = xkb_state_new(keymap);
    }
}

static void ei_device_regions_debug(struct ei_device *device)
{
    int idx = 0;
    struct ei_region *r;
    while ((r = ei_device_get_region(device, idx++))) {
        int x, y, w, h;
        x = ei_region_get_x(r);
        y = ei_region_get_y(r);
        w = ei_region_get_width(r);
        h = ei_region_get_height(r);

        kywc_log(KYWC_DEBUG, "%s has region %dx%d@%d,%d\n", ei_device_get_name(device), w, h, x, y);
    }
}

static void ei_device_add(struct ei_sender *sender, struct ei_device *device)
{
    if (!sender->pointer && ei_device_has_capability(device, EI_DEVICE_CAP_POINTER) &&
        ei_device_has_capability(device, EI_DEVICE_CAP_BUTTON) &&
        ei_device_has_capability(device, EI_DEVICE_CAP_SCROLL)) {
        kywc_log(KYWC_INFO, "EI_DEVICE_CAP_POINTER added");
        sender->pointer = ei_device_ref(device);
    }
    if (!sender->keyboard && ei_device_has_capability(device, EI_DEVICE_CAP_KEYBOARD)) {
        kywc_log(KYWC_INFO, "EI_DEVICE_CAP_KEYBOARD added");
        sender->keyboard = ei_device_ref(device);
        xkb_keyboard_init(sender, device);
    }
    if (!sender->abs_pointer && ei_device_has_capability(device, EI_DEVICE_CAP_POINTER_ABSOLUTE) &&
        ei_device_has_capability(device, EI_DEVICE_CAP_BUTTON) &&
        ei_device_has_capability(device, EI_DEVICE_CAP_SCROLL)) {
        kywc_log(KYWC_INFO, "EI_DEVICE_CAP_POINTER_ABSOLUTE added");
        sender->abs_pointer = ei_device_ref(device);
        ei_device_regions_debug(device);
    }
}

static void ei_device_resumed(struct ei_event *event, struct ei_sender *sender,
                              struct ei_device *device)
{
    ++sender->sequence_num;
    if (ei_event_get_device(event) == sender->pointer) {
        ei_device_start_emulating(sender->pointer, sender->sequence_num);
    }
    if (ei_event_get_device(event) == sender->abs_pointer) {
        ei_device_start_emulating(sender->abs_pointer, sender->sequence_num);
    }
    if (ei_event_get_device(event) == sender->keyboard) {
        ei_device_start_emulating(sender->keyboard, sender->sequence_num);
    }
}

static void ei_device_remove(struct ei_sender *sender, struct ei_device *device)
{
    if (device == sender->pointer) {
        sender->pointer = ei_device_unref(device);
    }
    if (device == sender->abs_pointer) {
        sender->abs_pointer = ei_device_unref(device);
    }
    if (device == sender->keyboard) {
        sender->keyboard = ei_device_unref(device);
    }
}

static int handle_ei_event(uint32_t mask, void *data)
{
    if (!data) {
        return -1;
    }
    if (!(mask & EVENT_READABLE)) {
        return 0;
    }

    struct ei_sender *sender = data;
    ei_dispatch(sender->ei);

    struct ei_event *event;
    while ((event = ei_get_event(sender->ei)) != NULL) {
        enum ei_event_type type = ei_event_get_type(event);
        struct ei_seat *seat = ei_event_get_seat(event);
        struct ei_device *device = ei_event_get_device(event);

        switch (type) {
        case EI_EVENT_CONNECT:
            kywc_log(KYWC_DEBUG, "ei_fd %d connected", ei_get_fd(sender->ei));
            break;
        case EI_EVENT_DISCONNECT:
            kywc_log(KYWC_ERROR, "ei_fd %d disconnected", ei_get_fd(sender->ei));
            ei_event_unref(event);
            goto cleanup;
        case EI_EVENT_SEAT_ADDED:
            if (sender->seat) {
                kywc_log(KYWC_DEBUG, "ignoring other seats");
                break;
            }
            sender->seat = ei_seat_ref(ei_event_get_seat(event));
            kywc_log(KYWC_INFO, "ei_seat added: %s", ei_seat_get_name(sender->seat));
            ei_seat_bind_capabilities(sender->seat, EI_DEVICE_CAP_POINTER, EI_DEVICE_CAP_KEYBOARD,
                                      EI_DEVICE_CAP_POINTER_ABSOLUTE, EI_DEVICE_CAP_TOUCH,
                                      EI_DEVICE_CAP_BUTTON, EI_DEVICE_CAP_SCROLL, NULL);
            break;
        case EI_EVENT_SEAT_REMOVED:
            if (seat == sender->seat) {
                sender->seat = ei_seat_unref(sender->seat);
            }
            break;
        case EI_EVENT_DEVICE_ADDED:
            if (seat == sender->seat) {
                kywc_log(KYWC_INFO, "ei_event: new device %s added", ei_device_get_name(device));
                ei_device_add(sender, device);
            }
            break;
        case EI_EVENT_DEVICE_RESUMED:
            kywc_log(KYWC_DEBUG, "ei_event: %s device  was resumed", ei_device_get_name(device));
            ei_device_resumed(event, sender, device);
            break;
        case EI_EVENT_DEVICE_PAUSED:
            kywc_log(KYWC_DEBUG, "ei_event: %s device  was paused", ei_device_get_name(device));
            break;
        case EI_EVENT_DEVICE_REMOVED:
            kywc_log(KYWC_DEBUG, "ei_event: removing device %s", ei_device_get_name(device));
            ei_device_remove(sender, device);
            break;
        case EI_EVENT_FRAME:
            break;
        case EI_EVENT_DEVICE_START_EMULATING:
            kywc_log(KYWC_DEBUG, "ei_event: device %s is ready to send events",
                     ei_device_get_name(device));
            break;
        case EI_EVENT_DEVICE_STOP_EMULATING:
            kywc_log(KYWC_DEBUG, "ei_event: device %s stopped emulating",
                     ei_device_get_name(device));
            break;
        case EI_EVENT_POINTER_MOTION:
            kywc_log(KYWC_DEBUG, "ei_event: motion by %.2f/%.2f", ei_event_pointer_get_dx(event),
                     ei_event_pointer_get_dy(event));
            break;
        case EI_EVENT_POINTER_MOTION_ABSOLUTE:
            kywc_log(KYWC_DEBUG, "ei_event: absmotion to %.2f/%.2f",
                     ei_event_pointer_get_absolute_x(event),
                     ei_event_pointer_get_absolute_y(event));
            break;
        case EI_EVENT_BUTTON_BUTTON:
            kywc_log(KYWC_DEBUG, "ei_event: button %u (%s)", ei_event_button_get_button(event),
                     ei_event_button_get_is_press(event) ? "press" : "release");
            break;
        case EI_EVENT_SCROLL_DELTA:
            kywc_log(KYWC_DEBUG, "ei_event: scroll %.2f/%.2f", ei_event_scroll_get_dx(event),
                     ei_event_scroll_get_dy(event));
            break;
        case EI_EVENT_SCROLL_DISCRETE:
            kywc_log(KYWC_DEBUG, "ei_event: scroll discrete %d/%d",
                     ei_event_scroll_get_discrete_dx(event),
                     ei_event_scroll_get_discrete_dy(event));
            break;
        case EI_EVENT_KEYBOARD_KEY:
            kywc_log(KYWC_DEBUG, "ei_event: key %u (%s)", ei_event_keyboard_get_key(event),
                     ei_event_keyboard_get_key_is_press(event) ? "press" : "release");
            break;
        case EI_EVENT_TOUCH_DOWN:
        case EI_EVENT_TOUCH_MOTION:
        case EI_EVENT_TOUCH_UP:
            break;
        default:
            break;
        }
        ei_event_unref(event);
    }
    return 0;

cleanup:
    if (sender->event_loop) {
        event_loop_remove_source(sender->event_loop, ei_get_fd(sender->ei));
    }
    ei_sender_close(sender);
    return 0;
}

struct ei_sender *ei_sender_create(struct portal_session *session, char *name)
{
    if (!session) {
        return NULL;
    }

    struct portal_context *ctx = session->ctx;
    sd_bus_message *reply = NULL;
    int ret =
        sd_bus_call_method(ctx->bus, "com.kylin.Wlcom", "/com/kylin/Wlcom/EIS/RemoteDesktop",
                           "com.kylin.Wlcom.EIS.RemoteDesktop", "ConnectToEIS", NULL, &reply, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR,
                 "Failed to call method com.kylin.Wlcom.EIS.RemoteDesktop ConnectToEIS");
        return NULL;
    }

    int eis_fd = -1;
    ret = sd_bus_message_read(reply, "h", &eis_fd);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Get EIS fd failed");
        sd_bus_message_unref(reply);
        return NULL;
    }

    struct ei_sender *sender = calloc(1, sizeof(*sender));
    if (!sender) {
        kywc_log(KYWC_ERROR, "Create ei_sender failed");
        close(eis_fd);
        sd_bus_message_unref(reply);
        return NULL;
    }

    sender->eis_fd = fcntl(eis_fd, F_DUPFD_CLOEXEC, 0);
    sd_bus_message_unref(reply);
    if (sender->eis_fd < 0) {
        kywc_log(KYWC_ERROR, "dup EIS fd failed");
        ei_sender_close(sender);
        return NULL;
    }

    sender->ei = ei_new_sender(NULL);
    sender->sequence_num = 0;
    sender->keyboard_info.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    ei_configure_name(sender->ei, name);

    int rc = ei_setup_backend_fd(sender->ei, sender->eis_fd);
    if (rc != 0) {
        kywc_log(KYWC_ERROR, "ei_sender init failed: %s\n", strerror(-rc));
        close(sender->eis_fd);
        sender->eis_fd = -1;
        ei_sender_close(sender);
        return NULL;
    }

    sender->event_loop = ctx->event_loop;
    if (event_loop_add_source(ctx->event_loop, ei_get_fd(sender->ei), handle_ei_event, sender) <
        0) {
        kywc_log(KYWC_ERROR, "Failed to add ei_fd to event loop");
        ei_sender_close(sender);
        return NULL;
    }

    return sender;
}

void ei_sender_close(struct ei_sender *sender)
{
    if (!sender) {
        return;
    }

    if (sender->pointer) {
        ei_device_close(sender->pointer);
        ei_device_unref(sender->pointer);
    }
    if (sender->abs_pointer) {
        ei_device_close(sender->abs_pointer);
        ei_device_unref(sender->abs_pointer);
    }
    if (sender->keyboard) {
        ei_device_close(sender->keyboard);
        ei_device_unref(sender->keyboard);
    }

    if (sender->seat) {
        ei_seat_unref(sender->seat);
    }

    if (sender->ei) {
        ei_unref(sender->ei);
    }

    if (sender->keyboard_info.state) {
        xkb_state_unref(sender->keyboard_info.state);
    }
    if (sender->keyboard_info.keymap) {
        xkb_keymap_unref(sender->keyboard_info.keymap);
    }
    if (sender->keyboard_info.context) {
        xkb_context_unref(sender->keyboard_info.context);
    }

    free(sender);
}

void ei_sender_pointer_motion(struct ei_sender *sender, double x, double y)
{
    if (!sender || !sender->pointer) {
        return;
    }

    ei_device_pointer_motion(sender->pointer, x, y);
    ei_device_frame(sender->pointer, ei_now(sender->ei));
}

void ei_sender_pointer_motion_absolute(struct ei_sender *sender, double x, double y)
{
    if (!sender || !sender->abs_pointer) {
        return;
    }

    ei_device_pointer_motion_absolute(sender->abs_pointer, x, y);
    ei_device_frame(sender->abs_pointer, ei_now(sender->ei));
}

void ei_sender_button(struct ei_sender *sender, uint32_t button, bool press)
{
    if (!sender || !sender->pointer) {
        return;
    }

    ei_device_button_button(sender->pointer, button, press);
    ei_device_frame(sender->pointer, ei_now(sender->ei));
}

void ei_sender_axis(struct ei_sender *sender, double x, double y, bool finish)
{
    if (!sender || !sender->pointer) {
        return;
    }

    if (finish) {
        ei_device_scroll_stop(sender->pointer, x != 0, y != 0);
    } else {
        ei_device_scroll_delta(sender->pointer, x, y);
    }
    ei_device_frame(sender->pointer, ei_now(sender->ei));
}

void ei_sender_axis_discrete(struct ei_sender *sender, uint32_t axis, int steps)
{
    if (!sender || !sender->pointer) {
        return;
    }

    if (axis == AXIS_VERTICAL_SCROLL) {
        ei_device_scroll_discrete(sender->pointer, 0, steps * 120);
        ei_device_frame(sender->pointer, ei_now(sender->ei));
    } else if (axis == AXIS_HORIZONTAL_SCROLL) {
        ei_device_scroll_discrete(sender->pointer, steps * 120, 0);
        ei_device_frame(sender->pointer, ei_now(sender->ei));
    }
}

void ei_sender_keycode(struct ei_sender *sender, uint32_t keycode, bool press)
{
    if (!sender || !sender->keyboard) {
        return;
    }

    ei_device_keyboard_key(sender->keyboard, keycode, press);
    ei_device_frame(sender->keyboard, ei_now(sender->ei));
}

static xkb_keycode_t xkb_get_keycode_from_keysym(struct ei_sender *sender, xkb_keysym_t keysym)
{
    if (!sender->keyboard_info.state || !sender->keyboard_info.keymap) {
        return XKB_KEYCODE_INVALID;
    }

    xkb_layout_index_t layout =
        xkb_state_serialize_layout(sender->keyboard_info.state, XKB_STATE_LAYOUT_EFFECTIVE);
    xkb_keycode_t min_keycode = xkb_keymap_min_keycode(sender->keyboard_info.keymap);
    xkb_keycode_t max_keycode = xkb_keymap_max_keycode(sender->keyboard_info.keymap);

    uint32_t code;
    for (code = min_keycode; code < max_keycode; code++) {
        size_t n_levels = xkb_keymap_num_levels_for_key(sender->keyboard_info.keymap, code, layout);
        for (size_t level = 0; level < n_levels; level++) {
            const xkb_keysym_t *symbols;
            size_t n_syms = xkb_keymap_key_get_syms_by_level(sender->keyboard_info.keymap, code,
                                                             layout, level, &symbols);
            for (size_t sym_idx = 0; sym_idx < n_syms; sym_idx++) {
                if (symbols[sym_idx] == keysym) {
                    return code;
                }
            }
        }
    }

    kywc_log(KYWC_DEBUG, "Failed to look up keyboard symbol: %x", keysym);
    return XKB_KEYCODE_INVALID;
}

void ei_sender_keysym(struct ei_sender *sender, uint32_t keysym, bool press)
{
    if (!sender || !sender->keyboard) {
        return;
    }

    char keysym_name[64];
    int ret = xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    if (ret < 0) {
        kywc_log(KYWC_DEBUG, "xkb_keysym_get_name failed: %x", keysym);
        return;
    }

    if (!sender->keyboard_info.keymap) {
        return;
    }

    xkb_keycode_t keycode = xkb_keymap_key_by_name(sender->keyboard_info.keymap, keysym_name);
    if (keycode == XKB_KEYCODE_INVALID) {
        keycode = xkb_get_keycode_from_keysym(sender, keysym);
        if (keycode == XKB_KEYCODE_INVALID) {
            return;
        }
    }

    uint32_t evdev_keycode = keycode - 8;
    ei_device_keyboard_key(sender->keyboard, evdev_keycode, press);
    ei_device_frame(sender->keyboard, ei_now(sender->ei));
}
