// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ei_sender.h"
#include "portal.h"
#include "stream.h"
#include "util/dialog.h"
#include "util/event.h"
#include "util/macros.h"
#include "util/nls.h"
#include "util/spawn.h"
#include "util/string.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.RemoteDesktop";

enum remote_desktop_persist_mode {
    REMOTE_DESKTOP_PERSIST_MODE_NONE = 0,
    REMOTE_DESKTOP_PERSIST_MODE_TEMPORARY = 1,
    REMOTE_DESKTOP_PERSIST_MODE_PERSISTENT = 2,
};

struct remote_restore_data {
    char *vendor_name;
    uint32_t version;
    uint32_t allow_devices;
    bool screen_share_enabled;
    bool clipboard_enabled;
};

struct permission_dialog_data {
    sd_bus_message *msg;
    struct portal_session *session;
    int socket_fd;
    struct event_context *event_loop;
    char *handle;
    char *app_id;
};

static void remote_desktop_restore_data_free(void *data)
{
    struct remote_restore_data *restore = data;
    if (!restore) {
        return;
    }

    free(restore->vendor_name);
    free(restore);
}

static int handle_create_session(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: CreateSession called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "CreateSession by handle %s session_handle %s app_id %s", handle,
             session_handle, app_id);

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_create(ctx, PORTAL_SESSION_TYPE_REMOTE_DESKTOP, session_handle);
    if (!session) {
        return -ENOMEM;
    }

    session->remote_desktop.ei_sender = ei_sender_create(session, app_id);

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static bool get_restore_data(sd_bus_message *msg, struct remote_restore_data *restore_data)
{
    int ret;
    ret = sd_bus_message_enter_container(msg, 'v', NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter VARIANT container for restore_data");
        return false;
    }

    ret = sd_bus_message_enter_container(msg, 'r', "suv");
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter STRUCT (sua{sv}) container");
        sd_bus_message_exit_container(msg);
        return false;
    }

    const char *vendor_name;
    ret = sd_bus_message_read(msg, "su", &vendor_name, &restore_data->version);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to read session name or version");
        goto error_exit;
    }

    restore_data->vendor_name = strdup(vendor_name);

    ret = sd_bus_message_enter_container(msg, 'v', "a{sv}");
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enter VARIANT wrapping the dictionary");
        goto error_exit;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        sd_bus_message_exit_container(msg);
        goto error_cleanup;
    }

    const char *key;
    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        ret = sd_bus_message_read(msg, "s", &key);
        if (ret < 0) {
            break;
        }

        if (strcmp(key, "devices") == 0) {
            uint32_t val = 0;
            sd_bus_message_read(msg, "v", "u", &val);
            restore_data->allow_devices = val;
            kywc_log(KYWC_DEBUG, "Parsed devices: %u", val);
        } else if (strcmp(key, "screen_share_enabled") == 0) {
            int val = 0;
            sd_bus_message_read(msg, "v", "b", &val);
            restore_data->screen_share_enabled = (val != 0);
            kywc_log(KYWC_DEBUG, "Parsed screen_share_enabled: %d", val);
        } else if (strcmp(key, "clipboard_enabled") == 0) {
            int val = 0;
            sd_bus_message_read(msg, "v", "b", &val);
            restore_data->clipboard_enabled = (val != 0);
            kywc_log(KYWC_DEBUG, "Parsed clipboard_enabled: %d", val);
        } else {
            kywc_log(KYWC_WARN, "RestoreData: unknown option %s", key);
            sd_bus_message_skip(msg, "v");
        }

        int inner_ret = sd_bus_message_exit_container(msg);
        if (inner_ret < 0) {
            break;
        }
    }

    sd_bus_message_exit_container(msg);
    sd_bus_message_exit_container(msg);
    sd_bus_message_exit_container(msg);
    sd_bus_message_exit_container(msg);

    kywc_log(KYWC_DEBUG,
             "Restore data parsed: session=%s, version=%u, devices=%u, screen=%d, clipboard=%d",
             restore_data->vendor_name, restore_data->version, restore_data->allow_devices,
             restore_data->screen_share_enabled, restore_data->clipboard_enabled);
    return true;
error_cleanup:
    sd_bus_message_exit_container(msg);
error_exit:
    sd_bus_message_exit_container(msg);
    sd_bus_message_exit_container(msg);
    return false;
}

static int handle_select_devices(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: SelectDevices called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to select device on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    uint32_t types = ctx->remote_desktop.devices;
    uint32_t persist_mode = 0;

    char *key;
    struct remote_restore_data *restore_data = NULL;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        int inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "types") == 0) {
            sd_bus_message_read(msg, "v", "u", &types);
            kywc_log(KYWC_DEBUG, "SelectDevices: option types: %d", types);
        } else if (strcmp(key, "persist_mode") == 0) {
            sd_bus_message_read(msg, "v", "u", &persist_mode);
            kywc_log(KYWC_DEBUG, "SelectDevices: option persist_mode: %d", persist_mode);
        } else if (strcmp(key, "restore_data") == 0) {
            restore_data = calloc(1, sizeof(*restore_data));
            if (restore_data) {
                if (!get_restore_data(msg, restore_data)) {
                    remote_desktop_restore_data_free(restore_data);
                    restore_data = NULL;
                    kywc_log(KYWC_ERROR, "RestoreData: get_restore_data failed");
                }
            }
            kywc_log(KYWC_DEBUG, "RestoreData: option restore_data");
        } else {
            kywc_log(KYWC_WARN, "SelectDevices: unknown option %s", key);
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

    kywc_log(KYWC_DEBUG,
             "SelectDevices by handle %s session_handle %s app_id %s , types %d persist_mode %d",
             handle, session_handle, app_id, types, persist_mode);
    session->remote_desktop.persist_mode = persist_mode;
    session->remote_desktop.select_devices = types;
    session->remote_desktop.restore_data = restore_data;
    session->remote_desktop.restore_data_destroy = remote_desktop_restore_data_free;

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static char *add_capabilities_text(char *full_content, const char *name, const char *desc)
{
    const char *prefix = full_content ? full_content : "";
    char *new_content = string_create("%s• %s: %s\n", prefix, name, desc);
    free(full_content);
    return new_content;
}

static char *build_capabilities_text(uint32_t caps, bool screen_share)
{
    struct {
        uint32_t flag;
        const char *name;
        const char *desc;
    } infos[] = { { 1, tr("Keyboard"), tr("Control keyboard input") },
                  { 2, tr("Pointer"), tr("Control pointer input") },
                  { 4, tr("Touch"), tr("Control touch screen input") } };

    char *full_content = NULL;
    for (size_t i = 0; i < ARRAY_SIZE(infos); i++) {
        if (caps & infos[i].flag) {
            full_content = add_capabilities_text(full_content, infos[i].name, infos[i].desc);
        }
    }

    if (screen_share) {
        full_content =
            add_capabilities_text(full_content, tr("Screen"), tr("Share screen content"));
    }

    return full_content;
}

static void build_permission_request_json(json_object **out_obj, const char *app_id,
                                          const char *parent_window, uint32_t capabilities,
                                          bool screen_sharing, uint32_t persist_mode)
{
    json_object *check_boxes = NULL;
    json_object *json_req = json_object_new_object();
    if (parent_window) {
        json_object_object_add(json_req, "parent_window", json_object_new_string(parent_window));
    }
    json_object_object_add(json_req, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(json_req, "window_title",
                           json_object_new_string(tr("Remote Desktop Permission")));
    json_object_object_add(json_req, "window_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));
    json_object_object_add(json_req, "title_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));

    json_object *title_obj = json_object_new_object();
    json_object_object_add(title_obj, "text", json_object_new_string(tr("Remote Desktop Request")));
    json_object_object_add(json_req, "title", title_obj);

    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), tr("This action requested by %s"), app_id);
    json_object *subtitle_obj = json_object_new_object();
    json_object_object_add(subtitle_obj, "text", json_object_new_string(temp_str));
    json_object_object_add(json_req, "sub_title", subtitle_obj);

    snprintf(temp_str, sizeof(temp_str), tr("%s wants to remotely control: "), app_id);
    json_object_object_add(json_req, "message", json_object_new_string(temp_str));

    char *capabilities_buf = build_capabilities_text(capabilities, screen_sharing);
    if (capabilities_buf) {
        json_object_object_add(json_req, "details", json_object_new_string(capabilities_buf));
        free(capabilities_buf);
    }

    if (persist_mode != REMOTE_DESKTOP_PERSIST_MODE_NONE) {
        check_boxes = json_object_new_array();
        json_object *box = json_object_new_object();
        json_object_object_add(box, "name", json_object_new_string("persist_mode"));
        json_object_object_add(
            box, "text",
            json_object_new_string(tr("Remember this permission for future sessions")));
        if (persist_mode == REMOTE_DESKTOP_PERSIST_MODE_TEMPORARY) {
            json_object_object_add(box, "default", json_object_new_boolean(false));
        } else {
            json_object_object_add(box, "default", json_object_new_boolean(true));
        }
        json_object_array_add(check_boxes, box);
        json_object_object_add(json_req, "check_boxes", check_boxes);
    }

    json_object_object_add(json_req, "button_mode", json_object_new_int64(0));

    json_object *dialog = json_object_new_object();
    json_object_object_add(dialog, "dialog", json_req);
    *out_obj = dialog;
}

void remote_desktop_restore_send(sd_bus_message **reply, struct portal_session *session)
{
    sd_bus_message_append(*reply, "{sv}", "devices", "u", session->remote_desktop.select_devices);
    sd_bus_message_append(*reply, "{sv}", "clipboard_enabled", "b",
                          session->remote_desktop.clipboard_enabled ? 1 : 0);
    sd_bus_message_append(*reply, "{sv}", "persist_mode", "u",
                          session->remote_desktop.persist_mode);

    if (session->remote_desktop.persist_mode != REMOTE_DESKTOP_PERSIST_MODE_NONE) {
        sd_bus_message_open_container(*reply, 'e', "sv");
        sd_bus_message_append_basic(*reply, 's', "restore_data");
        sd_bus_message_open_container(*reply, 'v', "(sua{sv})");
        sd_bus_message_open_container(*reply, 'r', "sua{sv}");
        sd_bus_message_append(*reply, "s", "wlcom");
        sd_bus_message_append(*reply, "u", session->ctx->remote_desktop.version);
        sd_bus_message_open_container(*reply, 'a', "{sv}");
        sd_bus_message_append(*reply, "{sv}", "devices", "u",
                              session->remote_desktop.select_devices);
        sd_bus_message_append(*reply, "{sv}", "screen_share_enabled", "b",
                              session->remote_desktop.screen_share_enabled ? 1 : 0);
        sd_bus_message_append(*reply, "{sv}", "clipboard_enabled", "b",
                              session->remote_desktop.clipboard_enabled ? 1 : 0);
        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
        sd_bus_message_close_container(*reply);
    }
}

static void handle_result_send(sd_bus_message *msg, struct portal_session *session,
                               int result_value)
{
    sd_bus_message *reply = NULL;
    sd_bus_message_new_method_return(msg, &reply);

    sd_bus_message_append(reply, "u", result_value);
    sd_bus_message_open_container(reply, 'a', "{sv}");

    remote_desktop_restore_send(&reply, session);

    sd_bus_message_close_container(reply);

    sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);
}

static int handle_permission_dialog(uint32_t mask, void *data)
{
    if (!data) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: data is NULL");
        return -1;
    }

    struct portal_request *request = data;
    struct permission_dialog_data *dialog_data = request->data;
    struct portal_context *ctx = request->ctx;
    if (!dialog_data || !ctx || !dialog_data->session) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return -1;
    }

    sd_bus_message *msg = dialog_data->msg;
    if (mask & (EVENT_HANGUP | EVENT_ERROR)) {
        kywc_log(KYWC_DEBUG, "Dialog socket hungup or error");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        sd_bus_message_unref(msg);
        dialog_data->msg = NULL;
        goto cleanup;
    }

    struct portal_session *session = dialog_data->session;
    if (!session) {
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        sd_bus_message_unref(msg);
        dialog_data->msg = NULL;
        goto cleanup;
    }

    int persist_mode = REMOTE_DESKTOP_PERSIST_MODE_NONE;
    int result_value = PORTAL_RESPONSE_ENDED;
    json_object *obj = dialog_get_reply(dialog_data->socket_fd);
    if (obj) {
        json_object *result_obj, *checkboxes_obj;
        if (json_object_object_get_ex(obj, "user_action", &result_obj)) {
            result_value = json_object_get_int(result_obj);
        }
        if (json_object_object_get_ex(obj, "check_boxes", &checkboxes_obj)) {
            if (json_object_is_type(checkboxes_obj, json_type_array)) {
                int array_len = json_object_array_length(checkboxes_obj);
                for (int i = 0; i < array_len; i++) {
                    json_object *item = json_object_array_get_idx(checkboxes_obj, i);
                    if (!item || !json_object_is_type(item, json_type_object)) {
                        continue;
                    }

                    json_object *name_obj, *checked_obj;
                    const char *name = NULL;
                    int checked = 0;

                    if (json_object_object_get_ex(item, "name", &name_obj)) {
                        name = json_object_get_string(name_obj);
                    }
                    if (json_object_object_get_ex(item, "checked", &checked_obj)) {
                        checked = json_object_get_boolean(checked_obj);
                    }

                    kywc_log(KYWC_DEBUG, "Checkbox[%d]: name='%s', checked=%s", i,
                             name ? name : "NULL", checked ? "true" : "false");

                    if (name && strcmp(name, "persist_mode") == 0) {
                        if (checked) {
                            persist_mode = REMOTE_DESKTOP_PERSIST_MODE_PERSISTENT;
                        } else {
                            persist_mode = REMOTE_DESKTOP_PERSIST_MODE_TEMPORARY;
                        }
                    }
                }
            } else {
                kywc_log(KYWC_ERROR, "check_boxes is not an array");
            }
        } else {
            kywc_log(KYWC_ERROR, "Failed to parse JSON response check_boxes.");
        }
        json_object_put(obj);
    }

    session->remote_desktop.persist_mode = persist_mode;
    if (result_value == PORTAL_RESPONSE_SUCCESS && session->remote_desktop.screen_share_enabled) {
        if (dialog_data->socket_fd >= 0) {
            event_loop_remove_source(ctx->event_loop, dialog_data->socket_fd);
            close(dialog_data->socket_fd);
            dialog_data->socket_fd = -1;
        }

        request->data = NULL;
        request->destroy = NULL;
        dialog_data->msg = NULL;
        portal_stream_select(session, dialog_data->app_id, request, msg);
        free(dialog_data);
        return 0;
    } else {
        handle_result_send(msg, session, result_value);
        sd_bus_message_unref(msg);
        dialog_data->msg = NULL;
    }

cleanup:
    if (dialog_data->socket_fd >= 0) {
        event_loop_remove_source(ctx->event_loop, dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
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
        dialog_data->msg = NULL;
    }

    if (dialog_data->event_loop) {
        event_loop_remove_source(dialog_data->event_loop, dialog_data->socket_fd);
    }
    if (dialog_data->socket_fd >= 0) {
        dialog_close(dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
    }

    free(dialog_data);
}

static int handle_start(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: Start called");

    char *handle, *session_handle, *app_id, *parent_window;
    int ret = sd_bus_message_read(msg, "ooss", &handle, &session_handle, &app_id, &parent_window);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to start on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    char *handle_token = NULL;

    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        int inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "handle_token") == 0) {
            sd_bus_message_read(msg, "v", "s", &handle_token);
            kywc_log(KYWC_DEBUG, "Start: option handle_token: %s", handle_token);
        } else {
            kywc_log(KYWC_WARN, "Start: unknown option %s", key);
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

    kywc_log(KYWC_DEBUG, "Start by session_handle %s, handle_token %s", session_handle,
             handle_token);

    bool restore_data_flag = false;
    struct remote_restore_data *restore_data =
        (struct remote_restore_data *)session->remote_desktop.restore_data;
    if ((session->remote_desktop.persist_mode != REMOTE_DESKTOP_PERSIST_MODE_NONE) &&
        restore_data) {
        if (session->remote_desktop.select_devices != restore_data->allow_devices) {
            kywc_log(KYWC_DEBUG, "Requested devices do not match");
        } else if (session->ctx->remote_desktop.version != restore_data->version) {
            kywc_log(KYWC_DEBUG, "Requested remote desktop version not match");
        } else if (session->remote_desktop.clipboard_enabled != restore_data->clipboard_enabled) {
            kywc_log(KYWC_DEBUG, "Requested clipboard enabled do not match");
        } else if (session->remote_desktop.screen_share_enabled !=
                   restore_data->screen_share_enabled) {
            kywc_log(KYWC_DEBUG, "Requested screen sharing  do not match");
        } else {
            restore_data_flag = true;
        }
    }

    if (restore_data_flag) {
        sd_bus_call_method_async(ctx->bus, NULL, "org.freedesktop.Notifications",
                                 "/org/freedesktop/Notifications", "org.freedesktop.Notifications",
                                 "Notify", NULL, NULL, "susssasa{sv}i", tr("Remote desktop"), 0,
                                 app_id, tr("Remote desktop session started"), NULL, 0, 0, 5000);
        if (session->remote_desktop.screen_share_enabled) {
            portal_stream_select(session, app_id, NULL, msg);
            return 1;
        } else {
            handle_result_send(msg, session, PORTAL_RESPONSE_SUCCESS);
            return 0;
        }
    }

    json_object *json_req = NULL;
    build_permission_request_json(
        &json_req, app_id, parent_window,
        session->remote_desktop.select_devices & ctx->remote_desktop.devices,
        session->remote_desktop.screen_share_enabled, session->remote_desktop.persist_mode);

    int dialog_socket = spawn_dialog("wldialog");
    if (!dialog_send_request(dialog_socket, json_req)) {
        close(dialog_socket);
        json_object_put(json_req);
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
    }

    json_object_put(json_req);

    struct permission_dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        return -ENOMEM;
    }

    struct portal_request *request = portal_request_create(ctx, handle, dialog_data);
    if (!request) {
        free(dialog_data);
        return -ENOMEM;
    }
    request->destroy = handle_request_destroy;

    dialog_data->session = session;
    dialog_data->socket_fd = dialog_socket;
    dialog_data->event_loop = ctx->event_loop;
    dialog_data->app_id = app_id;
    dialog_data->msg = sd_bus_message_ref(msg);

    if (event_loop_add_source(ctx->event_loop, dialog_data->socket_fd, handle_permission_dialog,
                              request) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
        portal_request_destroy(request);
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
    }

    return 1;
}

static int handle_notify_pointer_motion(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyPointerMotion called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify pointer motion on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    double dx, dy;
    ret = sd_bus_message_read(msg, "dd", &dx, &dy);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyPointerMotion by session_handle %s, dx %f dy %f", session_handle,
             dx, dy);

    ei_sender_pointer_motion(session->remote_desktop.ei_sender, dx, dy);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_pointer_motion_absolute(sd_bus_message *msg, void *data,
                                                 sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyPointerMotionAbsolute called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify pointer motion absolute on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    uint32_t stream;
    double x, y;
    ret = sd_bus_message_read(msg, "udd", &stream, &x, &y);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyPointerMotionAbsolute by session_handle %s, stream %d x %f y %f",
             session_handle, stream, x, y);

    struct portal_stream *portal_stream = portal_stream_get_from_remote_desktop(session, stream);
    if (portal_stream) {
        ei_sender_pointer_motion_absolute(session->remote_desktop.ei_sender,
                                          x + portal_stream->geometry.x,
                                          y + portal_stream->geometry.y);
    } else {
        kywc_log(KYWC_ERROR, "NotifyPointerMotionAbsolute find stream %d failed ", stream);
    }

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_pointer_button(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyPointerButton called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify pointer button on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    int button;
    uint32_t state;
    ret = sd_bus_message_read(msg, "iu", &button, &state);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyPointerButton by session_handle %s, button %d state %d",
             session_handle, button, state);

    ei_sender_button(session->remote_desktop.ei_sender, button, state);

    return sd_bus_reply_method_return(msg, NULL);
}

// The axis movement from a ‘smooth scroll’ device, such as a touchpad.
static int handle_notify_pointer_axis(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyPointerAxis called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify pointer axis on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    int finish = false;

    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        int inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "finish") == 0) {
            sd_bus_message_read(msg, "v", "b", &finish);
            kywc_log(KYWC_DEBUG, "NotifyPointerAxis: option finish: %d", finish);
        } else {
            kywc_log(KYWC_WARN, "NotifyPointerAxis: unknown option %s", key);
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

    double dx, dy;
    ret = sd_bus_message_read(msg, "dd", &dx, &dy);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyPointerAxis by session_handle %s, finish %d dx %f dy %f",
             session_handle, finish, dx, dy);

    ei_sender_axis(session->remote_desktop.ei_sender, dx, dy, finish);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_pointer_axis_discrete(sd_bus_message *msg, void *data,
                                               sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyPointerAxisDiscrete called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify pointer axis discrete on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    uint32_t axis;
    int steps;
    ret = sd_bus_message_read(msg, "ui", &axis, &steps);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyPointerAxisDiscrete by session_handle %s, axis %d steps %d",
             session_handle, axis, steps);

    ei_sender_axis_discrete(session->remote_desktop.ei_sender, axis, steps);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_keyboard_keycode(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyKeyboardKeycode called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify keyboard keycode on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    int keycode;
    uint32_t state;
    ret = sd_bus_message_read(msg, "iu", &keycode, &state);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyKeyboardKeycode by session_handle %s, keycode %d state %d",
             session_handle, keycode, state);

    ei_sender_keycode(session->remote_desktop.ei_sender, keycode, state);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_keyboard_keysym(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyKeyboardKeysym called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify keyboard keysym on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    int keysym;
    uint32_t state;
    ret = sd_bus_message_read(msg, "iu", &keysym, &state);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyKeyboardKeycode by session_handle %s, keysym %d state %d",
             session_handle, keysym, state);

    ei_sender_keysym(session->remote_desktop.ei_sender, keysym, state);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_touch_down(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyTouchDown called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify touch down on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    uint32_t stream, slot;
    double x, y;
    ret = sd_bus_message_read(msg, "uudd", &stream, &slot, &x, &y);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyTouchDown by session_handle %s, stream %d slot %d x %f y %f",
             session_handle, stream, slot, x, y);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_touch_motion(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyTouchMotion called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify touch motion on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    uint32_t stream, slot;
    double x, y;
    ret = sd_bus_message_read(msg, "uudd", &stream, &slot, &x, &y);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyTouchMotion by session_handle %s, stream %d slot %d x %f y %f",
             session_handle, stream, slot, x, y);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_notify_touch_up(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: NotifyTouchUp called");

    char *session_handle;
    int ret = sd_bus_message_read(msg, "o", &session_handle);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to notify touch up on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    uint32_t slot;
    ret = sd_bus_message_read(msg, "u", &slot);
    if (ret < 0) {
        return ret;
    }

    kywc_log(KYWC_DEBUG, "NotifyTouchUp by session_handle %s, slot %d", session_handle, slot);

    return sd_bus_reply_method_return(msg, NULL);
}

static int handle_connect_to_EIS(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "RemoteDesktop: ConnectToEIS called");

    char *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "os", &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->remote_desktop.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to connect to eis on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "ConnectToEIS by session_handle %s app_id %s", session_handle, app_id);

    sd_bus_message *reply = NULL;
    ret =
        sd_bus_call_method(ctx->bus, "com.kylin.Wlcom", "/com/kylin/Wlcom/EIS/RemoteDesktop",
                           "com.kylin.Wlcom.EIS.RemoteDesktop", "ConnectToEIS", NULL, &reply, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to get EIS fd");
        return ret;
    }

    int eis_fd = -1;
    ret = sd_bus_message_read(reply, "h", &eis_fd);
    if (ret >= 0) {
        ret = sd_bus_reply_method_return(msg, "h", eis_fd);
    }
    sd_bus_message_unref(reply);
    kywc_log(KYWC_DEBUG, "Get EIS fd %d from compositor", eis_fd);

    return ret;
}

static const sd_bus_vtable remote_desktop_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("AvailableDeviceTypes", "u", NULL,
                    offsetof(struct portal_context, remote_desktop.devices),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("version", "u", NULL, offsetof(struct portal_context, remote_desktop.version),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_METHOD("CreateSession", "oosa{sv}", "ua{sv}", handle_create_session,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SelectDevices", "oosa{sv}", "ua{sv}", handle_select_devices,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Start", "oossa{sv}", "ua{sv}", handle_start, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyPointerMotion", "oa{sv}dd", "", handle_notify_pointer_motion,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyPointerMotionAbsolute", "oa{sv}udd", "",
                  handle_notify_pointer_motion_absolute, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyPointerButton", "oa{sv}iu", "", handle_notify_pointer_button,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyPointerAxis", "oa{sv}dd", "", handle_notify_pointer_axis,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyPointerAxisDiscrete", "oa{sv}ui", "", handle_notify_pointer_axis_discrete,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyKeyboardKeycode", "oa{sv}iu", "", handle_notify_keyboard_keycode,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyKeyboardKeysym", "oa{sv}iu", "", handle_notify_keyboard_keysym,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyTouchDown", "oa{sv}uudd", "", handle_notify_touch_down,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyTouchMotion", "oa{sv}uudd", "", handle_notify_touch_motion,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("NotifyTouchUp", "oa{sv}u", "", handle_notify_touch_up,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ConnectToEIS", "osa{sv}", "h", handle_connect_to_EIS,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

bool portal_remote_desktop_init(struct portal_context *ctx)
{
    ctx->remote_desktop.version = 2;
    ctx->remote_desktop.devices = 3; // keyboard and pointer
    wl_list_init(&ctx->remote_desktop.sessions);

    int ret = sd_bus_add_object_vtable(ctx->bus, NULL, object_path, interface_name,
                                       remote_desktop_vtable, ctx);
    if (ret < 0) {
        return false;
    }

    return true;
}

void portal_remote_desktop_fini(struct portal_context *ctx)
{
    if (ctx->remote_desktop.version == 0) {
        return;
    }

    struct portal_session *session, *tmp;
    wl_list_for_each_safe(session, tmp, &ctx->remote_desktop.sessions, link) {
        portal_session_destroy(session);
    }
}
