// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_seat.h>

#include <kywc/log.h>

#include "input_p.h"
#include "server.h"
#include "util/dbus.h"
#include "xwayland/xwayland.h"

struct kde_clipboard_manager {
    struct wl_listener new_seat;
    struct wl_listener server_destroy;

    struct wl_list seats;
};

struct kde_clipboard_seat {
    struct wl_list link;
    struct seat *seat;
    struct wl_listener destroy;

    struct wl_listener set_selection;
    struct wl_listener set_primary_selection;
    int clipboard_selection_pid;
    int primary_selection_pid;
};

static const char *service_bus = "org.kde.KWin";
static const char *service_path = "/Clipboard";
static const char *service_interface = "org.kde.KWin.Clipboard";

static struct kde_clipboard_seat *clipboard_seat_from_seat(struct wl_list *seats, struct seat *seat)
{
    struct kde_clipboard_seat *clipboard_seat;
    wl_list_for_each(clipboard_seat, seats, link) {
        if (clipboard_seat->seat == seat) {
            return clipboard_seat;
        }
    }
    return NULL;
}

// SD_BUS_METHOD("getClipboardSelectionPid", "", "i", get_clipboard_selection_pid, 0),
static int get_clipboard_selection_pid(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct seat *seat = input_manager_get_default_seat();
    struct kde_clipboard_manager *manager = userdata;
    int clipboard_selection_pid = 0;

    struct kde_clipboard_seat *clipboard_seat = clipboard_seat_from_seat(&manager->seats, seat);
    if (clipboard_seat) {
        clipboard_selection_pid = clipboard_seat->clipboard_selection_pid;
    }

    return sd_bus_reply_method_return(msg, "i", clipboard_selection_pid);
}

// SD_BUS_METHOD("getPrimarySelectionPid", "", "i", get_primary_selection_pid, 0),
static int get_primary_selection_pid(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct seat *seat = input_manager_get_default_seat();
    struct kde_clipboard_manager *manager = userdata;
    int primary_selection_pid = 0;

    struct kde_clipboard_seat *clipboard_seat = clipboard_seat_from_seat(&manager->seats, seat);
    if (clipboard_seat) {
        primary_selection_pid = clipboard_seat->primary_selection_pid;
    }

    return sd_bus_reply_method_return(msg, "i", primary_selection_pid);
}

// SD_BUS_PROPERTY("GetClipboardSelectionPid", "i", get_selection_pid, 0,
// SD_BUS_VTABLE_PROPERTY_CONST),
static int get_selection_pid(sd_bus *bus, const char *path, const char *interface,
                             const char *property, sd_bus_message *reply, void *userdata,
                             sd_bus_error *ret_error)
{
    struct seat *seat = input_manager_get_default_seat();
    struct kde_clipboard_manager *manager = userdata;
    int clipboard_selection_pid = 0;

    struct kde_clipboard_seat *clipboard_seat = clipboard_seat_from_seat(&manager->seats, seat);
    if (clipboard_seat) {
        clipboard_selection_pid = clipboard_seat->clipboard_selection_pid;
    }

    return sd_bus_message_append_basic(reply, 'i', &clipboard_selection_pid);
}

// SD_BUS_PROPERTY("GetPrimarySelectionPid", "i", get_primary_pid, 0, SD_BUS_VTABLE_PROPERTY_CONST),
static int get_primary_pid(sd_bus *bus, const char *path, const char *interface,
                           const char *property, sd_bus_message *reply, void *userdata,
                           sd_bus_error *ret_error)
{
    struct seat *seat = input_manager_get_default_seat();
    struct kde_clipboard_manager *manager = userdata;
    int primary_selection_pid = 0;

    struct kde_clipboard_seat *clipboard_seat = clipboard_seat_from_seat(&manager->seats, seat);
    if (clipboard_seat) {
        primary_selection_pid = clipboard_seat->primary_selection_pid;
    }

    return sd_bus_message_append_basic(reply, 'i', &primary_selection_pid);
}

static const sd_bus_vtable clipboard_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("getClipboardSelectionPid", "", "i", get_clipboard_selection_pid,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("getPrimarySelectionPid", "", "i", get_primary_selection_pid,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_PROPERTY("GetClipboardSelectionPid", "i", get_selection_pid, 0,
                    SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY("GetPrimarySelectionPid", "i", get_primary_pid, 0,
                    SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_SIGNAL("clipboardSelectionPidChanged", "i", 0),
    SD_BUS_SIGNAL("primarySelectionPidChanged", "i", 0),
    SD_BUS_VTABLE_END,
};

static pid_t get_pid_by_wlr_surface(struct wlr_surface *surface)
{
    pid_t pid = 0;
    if (!surface) {
        return pid;
    }

    pid = xwayland_get_pid_from_wlr_surface(surface);
    if (pid != -1) {
        return pid;
    }

    struct wl_client *client = wl_resource_get_client(surface->resource);
    wl_client_get_credentials(client, &pid, NULL, NULL);
    return pid;
}

static void send_selection_pid_changed(struct kde_clipboard_seat *seat, bool is_primary)
{
    pid_t pid = 0, current_pid = 0;
    char *signal_name = NULL;
    struct wlr_surface *focused_surface = seat->seat->wlr_seat->keyboard_state.focused_surface;

    pid = get_pid_by_wlr_surface(focused_surface);
    if (!is_primary && pid != seat->clipboard_selection_pid) {
        current_pid = pid;
        seat->clipboard_selection_pid = pid;
        signal_name = "clipboardSelectionPidChanged";
    } else if (is_primary && pid != seat->primary_selection_pid) {
        current_pid = pid;
        seat->primary_selection_pid = pid;
        signal_name = "primarySelectionPidChanged";
    }

    if (signal_name) {
        dbus_emit_signal(service_path, service_interface, signal_name, "i", current_pid);
    }
}

static void handle_set_selection(struct wl_listener *listener, void *data)
{
    struct kde_clipboard_seat *clipboard_seat =
        wl_container_of(listener, clipboard_seat, set_selection);
    send_selection_pid_changed(clipboard_seat, false);
}

static void handle_set_primary_selection(struct wl_listener *listener, void *data)
{
    struct kde_clipboard_seat *clipboard_seat =
        wl_container_of(listener, clipboard_seat, set_primary_selection);
    send_selection_pid_changed(clipboard_seat, true);
}

static void handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct kde_clipboard_seat *clipboard_seat = wl_container_of(listener, clipboard_seat, destroy);
    wl_list_remove(&clipboard_seat->destroy.link);
    wl_list_remove(&clipboard_seat->set_selection.link);
    wl_list_remove(&clipboard_seat->set_primary_selection.link);
    free(clipboard_seat);
}

static void handle_new_seat(struct wl_listener *listener, void *data)
{
    struct kde_clipboard_seat *clipboard_seat = calloc(1, sizeof(*clipboard_seat));
    if (!clipboard_seat) {
        return;
    }

    struct kde_clipboard_manager *manager = wl_container_of(listener, manager, new_seat);
    wl_list_insert(&manager->seats, &clipboard_seat->link);

    struct seat *seat = data;
    clipboard_seat->seat = seat;
    clipboard_seat->destroy.notify = handle_seat_destroy;
    wl_signal_add(&seat->events.destroy, &clipboard_seat->destroy);

    clipboard_seat->set_selection.notify = handle_set_selection;
    wl_signal_add(&seat->events.set_selection, &clipboard_seat->set_selection);
    clipboard_seat->set_primary_selection.notify = handle_set_primary_selection;
    wl_signal_add(&seat->events.set_primary_selection, &clipboard_seat->set_primary_selection);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct kde_clipboard_manager *manager = wl_container_of(listener, manager, server_destroy);
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->new_seat.link);
    free(manager);
}

bool kde_clipboard_manager_create(struct input_manager *input_manager)
{
    struct kde_clipboard_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    if (!dbus_register_object(service_bus, service_path, service_interface, clipboard_vtable,
                              manager)) {
        kywc_log(KYWC_WARN, "Failed to register org.kde.KWin.Clipboard");
        free(manager);
        return false;
    }

    wl_list_init(&manager->seats);

    manager->new_seat.notify = handle_new_seat;
    wl_signal_add(&input_manager->events.new_seat, &manager->new_seat);
    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &manager->server_destroy);

    return true;
}
