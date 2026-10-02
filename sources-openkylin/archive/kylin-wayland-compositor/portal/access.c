// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <unistd.h>

#include "portal.h"
#include "util/dialog.h"
#include "util/event.h"
#include "util/spawn.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.Access";

struct permission_dialog_data {
    sd_bus_message *msg;
    int dialog_socket;
    struct event_context *event_loop;
};

static int handle_permission_dialog(uint32_t mask, void *data)
{
    if (!data) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: data is NULL");
        return 0;
    }

    struct portal_request *request = data;
    struct permission_dialog_data *dialog_data = request->data;
    struct portal_context *ctx = request->ctx;

    if (!dialog_data || !ctx || !dialog_data->msg) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return 0;
    }

    sd_bus_message *msg = dialog_data->msg;
    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_WARN, "Dialog socket hungup or error");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        goto cleanup;
    }

    int result_value = PORTAL_RESPONSE_ENDED;
    json_object *obj = dialog_get_reply(dialog_data->dialog_socket);
    if (obj) {
        json_object *result_obj;
        if (json_object_object_get_ex(obj, "user_action", &result_obj)) {
            result_value = json_object_get_int(result_obj);
        }
        json_object_put(obj);
    } else {
        result_value = PORTAL_RESPONSE_CANCELLED;
    }

    int ret = 0;
    ret = sd_bus_reply_method_return(msg, "ua{sv}", result_value, 0);
    if (ret < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to send D-Bus reply");
    }

cleanup:
    if (dialog_data->dialog_socket >= 0) {
        event_loop_remove_source(ctx->event_loop, dialog_data->dialog_socket);
        close(dialog_data->dialog_socket);
        dialog_data->dialog_socket = -1;
    }
    portal_request_destroy(request);

    return 0;
}

static void handle_request_destroy(void *data)
{
    struct permission_dialog_data *dialog_data = data;
    if (!dialog_data) {
        return;
    }

    if (dialog_data->msg) {
        sd_bus_message_unref(dialog_data->msg);
    }

    if (dialog_data->event_loop) {
        event_loop_remove_source(dialog_data->event_loop, dialog_data->dialog_socket);
    }
    if (dialog_data->dialog_socket >= 0) {
        dialog_close(dialog_data->dialog_socket);
        close(dialog_data->dialog_socket);
        dialog_data->dialog_socket = -1;
    }

    free(dialog_data);
}

static int handle_access_dialog(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "Access: AccessDialog called");

    char *handle, *app_id, *parent_window, *title, *subtitle, *body;
    int ret = sd_bus_message_read(msg, "osssss", &handle, &app_id, &parent_window, &title,
                                  &subtitle, &body);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG,
             "Access by handle %s app_id %s parent_window %s, title: %s subtitle: %s "
             "body: %s",
             handle, app_id, parent_window, title, subtitle, body);

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    char *key;
    char *deny_label = NULL;
    char *grant_label = NULL;
    char *icon = NULL;
    int modal = 0;
    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        int inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }
        if (strcmp(key, "modal") == 0) {
            sd_bus_message_read(msg, "v", "b", &modal);
            kywc_log(KYWC_DEBUG, "Access: option modal: %d", modal);
        } else if (strcmp(key, "deny_label") == 0) {
            sd_bus_message_read(msg, "v", "s", &deny_label);
            kywc_log(KYWC_DEBUG, "Access: option deny_label: %s", deny_label);
        } else if (strcmp(key, "grant_label") == 0) {
            sd_bus_message_read(msg, "v", "s", &grant_label);
            kywc_log(KYWC_DEBUG, "Access: option grant_label: %s", grant_label);
        } else if (strcmp(key, "icon") == 0) {
            sd_bus_message_read(msg, "v", "s", &icon);
            kywc_log(KYWC_DEBUG, "Access: option icon: %s", icon);
        } else if (strcmp(key, "choices") == 0) {
            /*TODO: add openFile*/
            sd_bus_message_skip(msg, "v");
            kywc_log(KYWC_DEBUG, "Access: option choices");
        } else {
            sd_bus_message_skip(msg, "v");
        }

        inner_ret = sd_bus_message_exit_container(msg);
        if (inner_ret < 0) {
            return inner_ret;
        }
    }

    if (ret < 0) {
        return ret;
    }
    ret = sd_bus_message_exit_container(msg);
    if (ret < 0) {
        return ret;
    }

    json_object *json_req = json_object_new_object();
    if (parent_window) {
        json_object_object_add(json_req, "parent_window", json_object_new_string(parent_window));
    }
    json_object_object_add(json_req, "modal", json_object_new_boolean(modal));
    json_object_object_add(json_req, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(json_req, "window_title", json_object_new_string(title));

    json_object *title_obj = json_object_new_object();
    json_object_object_add(title_obj, "text", json_object_new_string(title));
    json_object_object_add(json_req, "title", title_obj);

    json_object *subtitle_obj = json_object_new_object();
    json_object_object_add(subtitle_obj, "text", json_object_new_string(subtitle));
    json_object_object_add(json_req, "sub_title", subtitle_obj);

    json_object_object_add(json_req, "message", json_object_new_string(body));

    json_object_object_add(json_req, "button_mode", json_object_new_int64(0));

    json_object_object_add(
        json_req, "window_icon",
        json_object_new_string(icon ? icon : (app_id ? app_id : "dialog-warning")));
    json_object_object_add(
        json_req, "title_icon",
        json_object_new_string(icon ? icon : (app_id ? app_id : "dialog-warning")));
    if (grant_label) {
        json_object_object_add(json_req, "allow_text", json_object_new_string(grant_label));
    }
    if (deny_label) {
        json_object_object_add(json_req, "deny_text", json_object_new_string(deny_label));
    }
    json_object *dialog_json = json_object_new_object();
    json_object_object_add(dialog_json, "dialog", json_req);
    int fd = spawn_dialog("wldialog");
    if (!dialog_send_request(fd, dialog_json)) {
        close(fd);
        json_object_put(dialog_json);
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
    }
    json_object_put(dialog_json);

    struct permission_dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        close(fd);
        return -ENOMEM;
    }

    dialog_data->dialog_socket = fd;
    dialog_data->msg = sd_bus_message_ref(msg);

    struct portal_context *ctx = data;
    struct portal_request *request = portal_request_create(ctx, handle, dialog_data);
    if (!request) {
        free(dialog_data);
        close(fd);
        return -ENOMEM;
    }
    request->destroy = handle_request_destroy;
    dialog_data->event_loop = ctx->event_loop;

    if (event_loop_add_source(ctx->event_loop, dialog_data->dialog_socket, handle_permission_dialog,
                              request) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
        portal_request_destroy(request);
        return -EACCES;
    }

    return 1;
}

static const sd_bus_vtable access_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("AccessDialog", "osssssa{sv}", "ua{sv}", handle_access_dialog,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool portal_access_init(struct portal_context *ctx)
{
    int ret =
        sd_bus_add_object_vtable(ctx->bus, NULL, object_path, interface_name, access_vtable, ctx);
    if (ret < 0) {
        return false;
    }

    return true;
}

void portal_access_fini(struct portal_context *ctx)
{
    // nothing to do
}
