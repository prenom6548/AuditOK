// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "keyboard.h"
#include "output.h"
#include "wlcctrl.h"

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t id,
                                   const char *interface, uint32_t version)
{
    struct wlcom_ctrl_manager *manager = data;

    if ((manager->needs & NEED_KEYBOARD) &&
        strcmp(interface, zwp_virtual_keyboard_manager_v1_interface.name) == 0) {
        manager->k_manager =
            wl_registry_bind(registry, id, &zwp_virtual_keyboard_manager_v1_interface, version);
    } else if ((manager->needs & NEED_POINTER) &&
               strcmp(interface, zwlr_virtual_pointer_manager_v1_interface.name) == 0) {
        manager->p_manager =
            wl_registry_bind(registry, id, &zwlr_virtual_pointer_manager_v1_interface, version);
    } else if ((manager->needs & NEED_SEAT) && strcmp(interface, wl_seat_interface.name) == 0) {
        struct wl_seat *wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
        if (!wl_seat) {
            return;
        }

        manager->seat = new_seat(wl_seat, id);
        if (!manager->seat) {
            wl_seat_release(wl_seat);
            return;
        }
    } else if ((manager->needs & NEED_OUTPUTS) &&
               strcmp(interface, wl_output_interface.name) == 0) {
        struct wl_output *wl_output = wl_registry_bind(registry, id, &wl_output_interface, version);
        if (!wl_output) {
            return;
        }

        output *output = output_new(wl_output, id);
        if (!output) {
            return;
        }

        wl_list_insert(&manager->outputs, &output->link);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

struct wlcom_ctrl_manager *wayland_init(uint32_t needs)
{
    struct wlcom_ctrl_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return NULL;
    }
    wl_list_init(&manager->outputs);
    manager->needs = needs;

    manager->wl_display = wl_display_connect(NULL);
    if (manager->wl_display == NULL) {
        free(manager);
        return NULL;
    }

    manager->registry = wl_display_get_registry(manager->wl_display);
    wl_registry_add_listener(manager->registry, &registry_listener, manager);
    wl_display_roundtrip(manager->wl_display);

    return manager;
}

static bool find_primary_output(kywc_output *output, void *data)
{
    struct wlcom_ctrl_manager *manager = data;

    if (output->primary) {
        manager->primary_output = strdup(output->name);
        return true;
    }

    return false;
}

cursor *wlcctrl_cursor_init(struct wlcom_ctrl_manager *manager)
{
    if (!manager->seat || !manager->p_manager) {
        return NULL;
    }

    cursor *cursor = calloc(1, sizeof(*cursor));
    if (!cursor) {
        return NULL;
    }

    cursor->display = manager->wl_display;
    if (manager->kywc_ctx) {
        kywc_context_for_each_output(manager->kywc_ctx, find_primary_output, manager);
        cursor->output = output_find_by_name(&manager->outputs, manager->primary_output);
    }
    if (cursor->output) {
        int version = zwlr_virtual_pointer_manager_v1_get_version(manager->p_manager);
        cursor->zwlr_virtual_pointer_v1 =
            version >= 2
                ? zwlr_virtual_pointer_manager_v1_create_virtual_pointer_with_output(
                      manager->p_manager, manager->seat->wl_seat, cursor->output->wl_output)
                : zwlr_virtual_pointer_manager_v1_create_virtual_pointer(manager->p_manager,
                                                                         manager->seat->wl_seat);
    } else {
        cursor->zwlr_virtual_pointer_v1 = zwlr_virtual_pointer_manager_v1_create_virtual_pointer(
            manager->p_manager, manager->seat->wl_seat);
    }

    return cursor;
}

keyboard *wlcctrl_keyboard_init(struct wlcom_ctrl_manager *manager)
{
    if (!manager->seat || !manager->k_manager) {
        return NULL;
    }

    keyboard *keyboard = calloc(1, sizeof(*keyboard));
    if (!keyboard) {
        return NULL;
    }

    keyboard->virtual_keyboard = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
        manager->k_manager, manager->seat->wl_seat);
    keyboard->display = manager->wl_display;

    return keyboard;
}

void wayland_deinit(struct wlcom_ctrl_manager *manager)
{
    if (!manager) {
        return;
    }

    if (manager->seat) {
        seat_destroy(manager->seat);
    }
    if (manager->k_manager) {
        zwp_virtual_keyboard_manager_v1_destroy(manager->k_manager);
    }
    if (manager->p_manager) {
        zwlr_virtual_pointer_manager_v1_destroy(manager->p_manager);
    }

    output_list_destroy(&manager->outputs);
    wl_registry_destroy(manager->registry);
    wl_display_disconnect(manager->wl_display);
    free(manager->primary_output);
    free(manager);
}
