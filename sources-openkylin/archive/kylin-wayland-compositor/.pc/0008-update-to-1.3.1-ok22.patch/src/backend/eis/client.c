// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "backend/eis.h"
#include "eis_p.h"
#include "input/keyboard.h"
#include "input/seat.h"

static const struct wlr_pointer_impl eis_pointer_impl = {
    .name = "eis-pointer",
};

static void eis_context_client_create_pointer(struct eis_context_client *client)
{
    if (client->pointer) {
        return;
    }

    char device_name[256];
    snprintf(device_name, sizeof(device_name), "%s %s%s", eis_client_get_name(client->client),
             client->sender ? "" : "capture", "eis pointer");

    client->pointer = eis_seat_new_device(client->seat);

    eis_device_configure_name(client->pointer, device_name);
    eis_device_configure_capability(client->pointer, EIS_DEVICE_CAP_POINTER);
    eis_device_configure_capability(client->pointer, EIS_DEVICE_CAP_SCROLL);
    eis_device_configure_capability(client->pointer, EIS_DEVICE_CAP_BUTTON);

    if (client->sender) {
        wlr_pointer_init(&client->wlr_pointer, &eis_pointer_impl, device_name);
        wl_signal_emit_mutable(&client->context->backend->backend.events.new_input,
                               &client->wlr_pointer.base);
    }

    eis_device_add(client->pointer);
    eis_device_resume(client->pointer);
}

static void eis_context_client_destroy_pointer(struct eis_context_client *client)
{
    if (!client->pointer) {
        return;
    }

    if (client->wlr_pointer.impl) {
        wlr_pointer_finish(&client->wlr_pointer);
    }

    eis_device_pause(client->pointer);
    eis_device_remove(client->pointer);
    eis_device_unref(client->pointer);
    client->pointer = NULL;
}

static const struct wlr_pointer_impl eis_abs_pointer_impl = {
    .name = "eis-abs-pointer",
};

static void eis_context_client_create_absolute(struct eis_context_client *client)
{
    if (client->absolute) {
        return;
    }

    char device_name[256];
    snprintf(device_name, sizeof(device_name), "%s %s%s", eis_client_get_name(client->client),
             client->sender ? "" : "capture", "eis abs pointer");

    client->absolute = eis_seat_new_device(client->seat);

    eis_device_configure_name(client->absolute, device_name);
    eis_device_configure_capability(client->absolute, EIS_DEVICE_CAP_POINTER_ABSOLUTE);
    eis_device_configure_capability(client->absolute, EIS_DEVICE_CAP_SCROLL);
    eis_device_configure_capability(client->absolute, EIS_DEVICE_CAP_BUTTON);

    struct wlr_output_layout *layout = client->context->backend->layout;
    struct wlr_output_layout_output *output_layout;
    wl_list_for_each(output_layout, &layout->outputs, link) {
        struct eis_region *region = eis_device_new_region(client->absolute);
        int width, height;
        wlr_output_effective_resolution(output_layout->output, &width, &height);
        eis_region_set_offset(region, output_layout->x, output_layout->y);
        eis_region_set_size(region, width, height);
        eis_region_set_physical_scale(region, output_layout->output->scale);
        eis_region_add(region);
        eis_region_unref(region);
    }

    wl_signal_add(&layout->events.change, &client->layout_update);

    if (client->sender) {
        wlr_pointer_init(&client->wlr_absolute, &eis_abs_pointer_impl, device_name);
        wl_signal_emit_mutable(&client->context->backend->backend.events.new_input,
                               &client->wlr_absolute.base);
    }

    eis_device_add(client->absolute);
    eis_device_resume(client->absolute);
}

static void eis_context_client_destroy_absolute(struct eis_context_client *client)
{
    if (!client->absolute) {
        return;
    }

    if (client->wlr_absolute.impl) {
        wlr_pointer_finish(&client->wlr_absolute);
    }

    wl_list_remove(&client->layout_update.link);
    wl_list_init(&client->layout_update.link);

    eis_device_pause(client->absolute);
    eis_device_remove(client->absolute);
    eis_device_unref(client->absolute);
    client->absolute = NULL;
}

static const struct wlr_keyboard_impl eis_keyboard_impl = {
    .name = "eis-keyboard",
};

static void eis_context_client_create_keyboard(struct eis_context_client *client)
{
    if (client->keyboard) {
        return;
    }

    char device_name[256];
    snprintf(device_name, 256, "%s %s%s", eis_client_get_name(client->client),
             client->sender ? "" : "capture ", "eis keyboard");

    client->keyboard = eis_seat_new_device(client->seat);

    eis_device_configure_name(client->keyboard, device_name);
    eis_device_configure_capability(client->keyboard, EIS_DEVICE_CAP_KEYBOARD);

    struct wlr_keyboard *wlr_keyboard = NULL;

    if (client->sender) {
        wlr_keyboard_init(&client->wlr_keyboard, &eis_keyboard_impl, device_name);
        wl_signal_emit_mutable(&client->context->backend->backend.events.new_input,
                               &client->wlr_keyboard.base);
        wlr_keyboard = &client->wlr_keyboard;
    } else {
        wlr_keyboard = input_manager_get_default_seat()->keyboard->wlr_keyboard;
    }

    if (wlr_keyboard->keymap) {
        struct eis_keymap *keymap =
            eis_device_new_keymap(client->keyboard, EIS_KEYMAP_TYPE_XKB, wlr_keyboard->keymap_fd,
                                  wlr_keyboard->keymap_size);
        eis_keymap_add(keymap);
        eis_keymap_unref(keymap);
    }

    wl_signal_add(&wlr_keyboard->events.keymap, &client->keymap_update);
    wl_signal_add(&wlr_keyboard->events.modifiers, &client->modifiers_update);

    eis_device_add(client->keyboard);
    eis_device_resume(client->keyboard);
}

static void eis_context_client_destroy_keyboard(struct eis_context_client *client)
{
    if (!client->keyboard) {
        return;
    }

    if (client->wlr_keyboard.impl) {
        wlr_keyboard_finish(&client->wlr_keyboard);
    }

    wl_list_remove(&client->keymap_update.link);
    wl_list_init(&client->keymap_update.link);
    wl_list_remove(&client->modifiers_update.link);
    wl_list_init(&client->modifiers_update.link);

    eis_device_pause(client->keyboard);
    eis_device_remove(client->keyboard);
    eis_device_unref(client->keyboard);
    client->keyboard = NULL;
}

static void client_handle_layout_change(struct wl_listener *listener, void *data)
{
    struct eis_context_client *client = wl_container_of(listener, client, layout_update);
    if (!client->absolute) {
        return;
    }

    eis_context_client_destroy_absolute(client);
    eis_context_client_create_absolute(client);
}

static void client_handle_keymap_change(struct wl_listener *listener, void *data)
{
    struct eis_context_client *client = wl_container_of(listener, client, keymap_update);
    if (!client->keyboard) {
        return;
    }

    eis_context_client_destroy_keyboard(client);
    eis_context_client_create_keyboard(client);
}

static void client_handle_modifiers_change(struct wl_listener *listener, void *data)
{
    struct eis_context_client *client = wl_container_of(listener, client, modifiers_update);
    if (!client->keyboard) {
        return;
    }

    // TODO: check enabled if input capture context

    struct wlr_keyboard_modifiers *modifiers = &client->wlr_keyboard.modifiers;
    eis_device_keyboard_send_xkb_modifiers(client->keyboard, modifiers->depressed,
                                           modifiers->latched, modifiers->locked, modifiers->group);
}

struct eis_context_client *eis_context_add_client(struct eis_context *context,
                                                  struct eis_client *eis_client)
{
    struct eis_context_client *client = calloc(1, sizeof(*client));
    if (!client) {
        return NULL;
    }

    client->context = context;
    wl_list_insert(&context->clients, &client->link);

    client->client = eis_client_ref(eis_client);
    eis_client_set_user_data(eis_client, client);

    eis_client_connect(eis_client);

    struct eis_seat *eis_seat = eis_client_new_seat(eis_client, "eis-0");

    if (context->capabilities & EIS_DEVICE_CAP_POINTER) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_POINTER);
    }
    if (context->capabilities & EIS_DEVICE_CAP_POINTER_ABSOLUTE) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_POINTER_ABSOLUTE);
    }
    if (context->capabilities & EIS_DEVICE_CAP_KEYBOARD) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_KEYBOARD);
    }
    if (context->capabilities & EIS_DEVICE_CAP_TOUCH) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_TOUCH);
    }
    if (context->capabilities & EIS_DEVICE_CAP_SCROLL) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_SCROLL);
    }
    if (context->capabilities & EIS_DEVICE_CAP_BUTTON) {
        eis_seat_configure_capability(eis_seat, EIS_DEVICE_CAP_BUTTON);
    }

    eis_seat_add(eis_seat);
    client->seat = eis_seat;

    // layout changed for abs pointer
    client->layout_update.notify = client_handle_layout_change;
    wl_list_init(&client->layout_update.link);

    // keymap and modifiers changed for keyboard
    client->keymap_update.notify = client_handle_keymap_change;
    wl_list_init(&client->keymap_update.link);
    client->modifiers_update.notify = client_handle_modifiers_change;
    wl_list_init(&client->modifiers_update.link);

    client->sender = eis_client_is_sender(eis_client);

    return client;
}

struct eis_context_client *eis_context_get_client(struct eis_context *context,
                                                  struct eis_client *eis_client)
{
    if (!eis_client) {
        return NULL;
    }

    return eis_client_get_user_data(eis_client);
}

void eis_context_client_process_event(struct eis_context_client *client, struct eis_event *event)
{
    enum eis_event_type type = eis_event_get_type(event);

    if (type == EIS_EVENT_SEAT_BIND) {
        if (eis_event_seat_has_capability(event, EIS_DEVICE_CAP_POINTER)) {
            eis_context_client_create_pointer(client);
        } else {
            eis_context_client_destroy_pointer(client);
        }

        if (eis_event_seat_has_capability(event, EIS_DEVICE_CAP_POINTER_ABSOLUTE)) {
            eis_context_client_create_absolute(client);
        } else {
            eis_context_client_destroy_absolute(client);
        }

        if (eis_event_seat_has_capability(event, EIS_DEVICE_CAP_KEYBOARD)) {
            eis_context_client_create_keyboard(client);
        } else {
            eis_context_client_destroy_keyboard(client);
        }
    } else if (type == EIS_EVENT_DEVICE_CLOSED) {
        struct eis_device *device = eis_event_get_device(event);
        if (device == client->pointer) {
            eis_context_client_destroy_pointer(client);
        } else if (device == client->absolute) {
            eis_context_client_destroy_absolute(client);
        } else if (device == client->keyboard) {
            eis_context_client_destroy_keyboard(client);
        }
    } else if (!client->sender) {
        kywc_log(KYWC_DEBUG, "Skip EIS event type %d from receiver client", type);
    } else {
        eis_context_client_feed_event(client, event);
    }
}

void eis_context_remove_client(struct eis_context *context, struct eis_client *eis_client)
{
    struct eis_context_client *client = eis_context_get_client(context, eis_client);
    if (!client) {
        return;
    }

    eis_context_client_destroy_pointer(client);
    eis_context_client_destroy_absolute(client);
    eis_context_client_destroy_keyboard(client);

    eis_seat_unref(client->seat);
    eis_client_disconnect(eis_client);
    eis_client_unref(eis_client);

    wl_list_remove(&client->link);
    free(client);
}

bool wlr_input_device_is_eis(struct wlr_input_device *wlr_dev)
{
    switch (wlr_dev->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        return wlr_keyboard_from_input_device(wlr_dev)->impl == &eis_keyboard_impl;
    case WLR_INPUT_DEVICE_POINTER:;
        struct wlr_pointer *pointer = wlr_pointer_from_input_device(wlr_dev);
        return pointer->impl == &eis_pointer_impl || pointer->impl == &eis_abs_pointer_impl;
    default:
        return false;
    }
}
