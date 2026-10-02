// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include "eis_p.h"
#include "util/dbus.h"
#include "util/string.h"

static void log_handler(struct eis *eis, enum eis_log_priority priority, const char *message,
                        struct eis_log_context *ctx)
{
    enum kywc_log_level level = KYWC_INFO;
    switch (priority) {
    case EIS_LOG_PRIORITY_DEBUG:
        level = KYWC_DEBUG;
        break;
    case EIS_LOG_PRIORITY_WARNING:
        level = KYWC_WARN;
        break;
    case EIS_LOG_PRIORITY_ERROR:
        level = KYWC_ERROR;
        break;
    case EIS_LOG_PRIORITY_INFO:
    default:
        break;
    }
    kywc_log(level, "[EIS]: %s", message);
}

static void handle_eis_event(struct eis_context *context, struct eis_event *event)
{
    enum eis_event_type type = eis_event_get_type(event);
    struct eis_client *eis_client = eis_event_get_client(event);
    const char *name = eis_client_get_name(eis_client);

    switch (type) {
    case EIS_EVENT_CLIENT_CONNECT:;
        bool sender = eis_client_is_sender(eis_client);

        // reject client
        if (sender == (context->type == EIS_CONTEXT_TYPE_INPUT_CAPTURE)) {
            kywc_log(KYWC_WARN, "Reject mismatch mode client %s", name);
            eis_client_disconnect(eis_client);
            break;
        }

        // only one client in receiver mode per context
        if (!sender && !wl_list_empty(&context->clients)) {
            kywc_log(KYWC_WARN, "Reject unexpected client %s", name);
            eis_client_disconnect(eis_client);
            break;
        }

        kywc_log(KYWC_DEBUG, "New %s client: %s", sender ? "sender" : "receiver", name);
        eis_context_add_client(context, eis_client);
        break;
    case EIS_EVENT_CLIENT_DISCONNECT:
        kywc_log(KYWC_DEBUG, "client: %s disconnect", name);
        eis_context_remove_client(context, eis_client);
        break;
    default:;
        struct eis_context_client *client = eis_context_get_client(context, eis_client);
        if (!client) {
            kywc_log(KYWC_WARN, "Event for unknown EIS client: %s", name);
            return;
        }
        eis_context_client_process_event(client, event);
        break;
    }
}

static int handle_eis_readable(int fd, uint32_t mask, void *_context)
{
    struct eis_context *context = _context;
    eis_dispatch(context->eis);

    struct eis_event *event;
    while ((event = eis_get_event(context->eis))) {
        handle_eis_event(context, event);
        eis_event_unref(event);
    }
    return 0;
}

static int connect_to_eis(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct eis_context *context = userdata;
    int fd = eis_backend_fd_add_client(context->eis);
    return sd_bus_reply_method_return(msg, "h", fd);
}

static const sd_bus_vtable remote_desktop_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ConnectToEIS", "", "h", connect_to_eis, 0),
    SD_BUS_VTABLE_END,
};

static int enable(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct eis_context *context = userdata;

    sd_bus_message_enter_container(msg, 'a', "(iiii)");
    while (sd_bus_message_enter_container(msg, 'r', "iiii") > 0) {
        int x1, y1, x2, y2;
        sd_bus_message_read(msg, "iiii", &x1, &y1, &x2, &y2);
        struct barrier *barrier = wl_array_add(&context->barriers, sizeof(*barrier));
        if (x1 == x2) {
            *barrier = (struct barrier){ x1, y1, y2, VERTICAL, 0 };
        } else {
            *barrier = (struct barrier){ y1, x1, x2, HORIZONTAL, 0 };
        }
        sd_bus_message_exit_container(msg);
    }
    sd_bus_message_exit_container(msg);

    return sd_bus_reply_method_return(msg, NULL);
}

void eis_context_set_activated(struct eis_context *context, bool activated, double lx, double ly)
{
    struct eis_backend *backend = context->backend;
    kywc_log(KYWC_DEBUG, "Eis backend %s %s", activated ? "Activate" : "Deactivate",
             context->dbus_path);

    // signal activated or deactivated
    if (activated) {
        backend->active_capture = context;
        dbus_emit_signal(context->dbus_path, "com.kylin.Wlcom.EIS.InputCapture", "Activated",
                         "u(dd)", ++context->activation_id, lx, ly);
    } else {
        backend->active_capture = NULL;
        dbus_emit_signal(context->dbus_path, "com.kylin.Wlcom.EIS.InputCapture", "Deactivated", "u",
                         context->activation_id);
    }

    if (wl_list_empty(&context->clients)) {
        return;
    }

    struct eis_context_client *client = wl_container_of(context->clients.next, client, link);

    if (client->pointer) {
        activated ? eis_device_start_emulating(client->pointer, context->activation_id)
                  : eis_device_stop_emulating(client->pointer);
    }
    if (client->keyboard) {
        activated ? eis_device_start_emulating(client->keyboard, context->activation_id)
                  : eis_device_stop_emulating(client->keyboard);
    }
    if (client->absolute) {
        activated ? eis_device_start_emulating(client->absolute, context->activation_id)
                  : eis_device_stop_emulating(client->absolute);
    }
}

static int disable(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct eis_context *context = userdata;

    if (context->backend->active_capture == context) {
        eis_context_set_activated(context, false, 0, 0);
    }

    // clear the barriers
    context->barriers.size = 0;

    dbus_emit_signal(context->dbus_path, "com.kylin.Wlcom.EIS.InputCapture", "Disabled", NULL);

    return sd_bus_reply_method_return(msg, NULL);
}

static int release(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct eis_context *context = userdata;

    if (context->backend->active_capture != context) {
        return sd_bus_reply_method_return(msg, NULL);
    }

    double cursor_x, cursor_y;
    uint32_t apply;
    sd_bus_message_read(msg, "(dd)b", &cursor_x, &cursor_y, &apply);

    if (apply) {
        // TODO: apply position to default seat cursor
    }

    eis_context_set_activated(context, false, 0, 0);

    return sd_bus_reply_method_return(msg, NULL);
}

static const sd_bus_vtable input_capture_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ConnectToEIS", "", "h", connect_to_eis, 0),
    SD_BUS_METHOD("Enable", "a(iiii)", "", enable, 0),
    SD_BUS_METHOD("Disable", "", "", disable, 0),
    SD_BUS_METHOD("Release", "(dd)b", "", release, 0),
    SD_BUS_SIGNAL("Activated", "u(dd)", 0),
    SD_BUS_SIGNAL("Deactivated", "u", 0),
    SD_BUS_SIGNAL("Disabled", "", 0),
    SD_BUS_VTABLE_END,
};

static int handle_name_lost(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct eis_context *context = userdata;
    kywc_log(KYWC_INFO, "%s lost name, closing context", context->dbus_path);
    eis_context_destroy(context);
    return 0;
}

struct eis_context *eis_context_create(struct eis_backend *backend, enum eis_context_type type,
                                       uint32_t capabilities, const void *data)
{
    struct eis_context *context = calloc(1, sizeof(*context));
    if (!context) {
        return NULL;
    }

    context->eis = eis_new(context);
    if (!context->eis) {
        kywc_log(KYWC_ERROR, "Failed to create eis context");
        goto failed;
    }

    eis_log_set_priority(context->eis, EIS_LOG_PRIORITY_WARNING);
    eis_log_set_handler(context->eis, log_handler);

    if (type == EIS_CONTEXT_TYPE_NORMAL) {
        if (eis_setup_backend_socket(context->eis, data) != 0) {
            goto failed;
        }
    } else if (type == EIS_CONTEXT_TYPE_REMOTE_DESKTOP) {
        if (eis_setup_backend_fd(context->eis) != 0) {
            goto failed;
        }
        context->dbus = dbus_register_object(NULL, "/com/kylin/Wlcom/EIS/RemoteDesktop",
                                             "com.kylin.Wlcom.EIS.RemoteDesktop",
                                             remote_desktop_vtable, context);
        if (!context->dbus) {
            goto failed;
        }
    } else if (type == EIS_CONTEXT_TYPE_INPUT_CAPTURE) {
        static uint32_t counter = 0;
        if (eis_setup_backend_fd(context->eis) != 0) {
            goto failed;
        }
        context->dbus_path = string_create("/com/kylin/Wlcom/EIS/InputCapture/%d", ++counter);
        if (!context->dbus_path) {
            goto failed;
        }
        context->dbus =
            dbus_register_object(NULL, context->dbus_path, "com.kylin.Wlcom.EIS.InputCapture",
                                 input_capture_vtable, context);
        if (!context->dbus) {
            goto failed;
        }
    }

    int eis_fd = eis_get_fd(context->eis);
    context->event = wl_event_loop_add_fd(backend->event_loop, eis_fd, WL_EVENT_READABLE,
                                          handle_eis_readable, context);
    if (!context->event) {
        kywc_log(KYWC_ERROR, "Failed to create event on event loop");
        goto failed;
    }

    context->type = type;
    context->capabilities = capabilities;
    wl_list_init(&context->clients);
    wl_array_init(&context->barriers);
    context->backend = backend;
    wl_list_insert(&backend->contexts, &context->link);

    if (type == EIS_CONTEXT_TYPE_INPUT_CAPTURE) {
        static char match_rule[1024];
        snprintf(match_rule, sizeof(match_rule),
                 "sender='org.freedesktop.DBus',"
                 "type='signal',"
                 "interface='org.freedesktop.DBus',"
                 "member='NameOwnerChanged',"
                 "path='/org/freedesktop/DBus',"
                 "arg0='%s'",
                 (const char *)data);
        sd_bus_add_match(dbus_context_get_bus(false), &context->watcher, match_rule,
                         handle_name_lost, context);
        eis_backend_lock_filter(backend, true);
    }

    return context;

failed:
    eis_unref(context->eis);
    dbus_unregister_object(context->dbus);
    free((void *)context->dbus_path);
    free(context);
    return NULL;
}

void eis_context_destroy(struct eis_context *context)
{
    if (!context) {
        return;
    }

    struct eis_context_client *client, *tmp;
    wl_list_for_each_safe(client, tmp, &context->clients, link) {
        eis_context_remove_client(context, client->client);
    }

    wl_list_remove(&context->link);
    wl_event_source_remove(context->event);
    eis_unref(context->eis);

    if (context->watcher) {
        sd_bus_slot_unref(context->watcher);
    }
    dbus_unregister_object(context->dbus);

    if (context->type == EIS_CONTEXT_TYPE_INPUT_CAPTURE) {
        struct eis_backend *backend = context->backend;
        eis_backend_lock_filter(backend, false);
        if (backend->active_capture == context) {
            eis_context_set_activated(context, false, 0, 0);
        }
    }

    wl_array_release(&context->barriers);
    free((void *)context->dbus_path);
    free(context);
}
