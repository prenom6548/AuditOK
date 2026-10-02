// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "portal.h"
#include "util/dialog.h"
#include "util/event.h"
#include "util/macros.h"
#include "util/nls.h"
#include "util/spawn.h"
#include "util/string.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.InputCapture";

static const char wlcom_interface[] = "com.kylin.Wlcom";
static const char wlcom_eis_interface[] = "com.kylin.Wlcom.EIS.InputCapture";

struct permission_dialog_data {
    sd_bus_message *msg;
    struct portal_session *session;
    uint32_t requested_caps;
    int socket_fd;
    struct event_context *event_loop;
};

static int handle_session_activated(sd_bus_message *msg, void *data, sd_bus_error *error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Activated received");

    uint32_t activation_id;
    double x, y;
    int ret = sd_bus_message_read(msg, "u(dd)", &activation_id, &x, &y);
    if (ret < 0) {
        return ret;
    }

    // send Activated signal
    struct portal_session *session = data;
    return sd_bus_emit_signal(sd_bus_slot_get_bus(session->slot), object_path, interface_name,
                              "Activated", "oa{sv}", session->session_handle, 3, "activation_id",
                              "u", activation_id, "cursor_position", "(dd)", x, y, "barrier_id",
                              "u", 0);
}

static int handle_session_deactivated(sd_bus_message *msg, void *data, sd_bus_error *error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Deactivated received");

    uint32_t activation_id;
    int ret = sd_bus_message_read(msg, "u", &activation_id);
    if (ret < 0) {
        return ret;
    }

    // send Deactivated signal
    struct portal_session *session = data;
    return sd_bus_emit_signal(sd_bus_slot_get_bus(session->slot), object_path, interface_name,
                              "Deactivated", "oa{sv}", session->session_handle, 2, "activation_id",
                              "u", activation_id, "cursor_position", "(dd)", 0, 0);
}

static int handle_session_disable(sd_bus_message *msg, void *data, sd_bus_error *error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Disabled received");

    // send Disabled signal
    struct portal_session *session = data;
    return sd_bus_emit_signal(sd_bus_slot_get_bus(session->slot), object_path, interface_name,
                              "Disabled", "oa{sv}", session->session_handle, 0);
}

static char *build_capabilities_text(uint32_t caps)
{
    struct {
        uint32_t flag;
        const char *name;
        const char *desc;
    } infos[] = { { 1, tr("Keyboard"), tr("Capture keyboard input") },
                  { 2, tr("Pointer"), tr("Capture mouse/trackpad input") },
                  { 4, tr("Touch"), tr("Capture touch screen input") } };

    char *full_content = NULL;
    for (size_t i = 0; i < ARRAY_SIZE(infos); i++) {
        if (caps & infos[i].flag) {
            const char *prefix = full_content ? full_content : "";
            char *new_content = string_create("%s• %s: %s\n", prefix, infos[i].name, infos[i].desc);
            free(full_content);
            if (new_content == NULL) {
                return NULL;
            }
            full_content = new_content;
        }
    }

    return full_content;
}

static int handle_permission_dialog(uint32_t mask, void *data)
{
    if (!data) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: data is NULL");
        return 0;
    }

    struct portal_request *request = data;
    struct permission_dialog_data *dialog_data = request->data;
    struct portal_context *ctx = request->ctx;

    if (!dialog_data || !ctx || !dialog_data->session || !dialog_data->msg) {
        kywc_log(KYWC_ERROR, "handle_permission_dialog: invalid fields in dialog_data");
        portal_request_destroy(request);
        return 0;
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

    int result_value = PORTAL_RESPONSE_ENDED;
    json_object *obj = dialog_get_reply(dialog_data->socket_fd);
    if (obj) {
        json_object *result_obj;
        if (json_object_object_get_ex(obj, "user_action", &result_obj))
            result_value = json_object_get_int(result_obj);
        json_object_put(obj);
    } else {
        result_value = PORTAL_RESPONSE_CANCELLED;
    }

    int ret = 0;
    if (result_value != PORTAL_RESPONSE_SUCCESS) {
        ret = sd_bus_reply_method_return(msg, "ua{sv}", result_value, 0);
        goto cleanup;
    }

    sd_bus_message *reply = NULL;
    ret = sd_bus_call_method(ctx->bus, wlcom_interface, "/com/kylin/Wlcom/EIS/InputCapture",
                             "com.kylin.Wlcom.EIS.InputCaptureManager", "AddInputCapture", NULL,
                             &reply, "u", dialog_data->requested_caps);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to add input capture: %s", strerror(-ret));
        ret = sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
        goto cleanup;
    }

    const char *input_capture_path = NULL;
    ret = sd_bus_message_read(reply, "o", &input_capture_path);
    if (ret >= 0 && input_capture_path) {
        session->input_capture_path = strdup(input_capture_path);
        // connect signals
        sd_bus_match_signal(ctx->bus, NULL, NULL, session->input_capture_path, wlcom_eis_interface,
                            "Activated", handle_session_activated, session);
        sd_bus_match_signal(ctx->bus, NULL, NULL, session->input_capture_path, wlcom_eis_interface,
                            "Deactivated", handle_session_deactivated, session);
        sd_bus_match_signal(ctx->bus, NULL, NULL, session->input_capture_path, wlcom_eis_interface,
                            "Disabled", handle_session_disable, session);
    }
    sd_bus_message_unref(reply);

    ret = sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 1, "capabilities", "u",
                                     dialog_data->requested_caps);
    if (ret < 0) {
        kywc_log_errno(KYWC_ERROR, "Failed to send D-Bus reply");
        sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_CANCELLED, 0);
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

static void build_permission_request_json(json_object **out_obj, const char *app_id,
                                          const char *parent_window, uint32_t capabilities,
                                          struct portal_context *ctx)
{
    json_object *obj = json_object_new_object();
    if (parent_window) {
        json_object_object_add(obj, "parent_window", json_object_new_string(parent_window));
    }
    json_object_object_add(obj, "title_bar", json_object_new_string("no_maximizable"));
    json_object_object_add(obj, "window_title",
                           json_object_new_string(tr("Input Capture Permission")));
    json_object_object_add(obj, "window_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));
    json_object_object_add(obj, "title_icon",
                           json_object_new_string(app_id ? app_id : "dialog-warning"));

    json_object *title_obj = json_object_new_object();
    json_object_object_add(title_obj, "text", json_object_new_string(tr("Input Capture Request")));
    json_object_object_add(obj, "title", title_obj);

    char temp_str[256];
    snprintf(temp_str, sizeof(temp_str), tr("This action requested by %s"), app_id);
    json_object *subtitle_obj = json_object_new_object();
    json_object_object_add(subtitle_obj, "text", json_object_new_string(temp_str));
    json_object_object_add(obj, "sub_title", subtitle_obj);

    snprintf(temp_str, sizeof(temp_str), tr("%s wants to capture input device."), app_id);
    json_object_object_add(obj, "message", json_object_new_string(temp_str));

    char *cap_text = build_capabilities_text(capabilities & ctx->input_capture.capabilities);
    if (cap_text) {
        json_object_object_add(obj, "details", json_object_new_string(cap_text));
        free(cap_text);
    }

    json_object_object_add(obj, "button_mode", json_object_new_int64(0));

    json_object *dialog = json_object_new_object();
    json_object_object_add(dialog, "dialog", obj);
    *out_obj = dialog;
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
        event_loop_remove_source(dialog_data->event_loop, dialog_data->socket_fd);
    }
    if (dialog_data->socket_fd >= 0) {
        dialog_close(dialog_data->socket_fd);
        close(dialog_data->socket_fd);
        dialog_data->socket_fd = -1;
    }

    free(dialog_data);
}

static int handle_create_session(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: CreateSession called");

    char *handle, *session_handle, *app_id, *parent_window;
    int ret = sd_bus_message_read(msg, "ooss", &handle, &session_handle, &app_id, &parent_window);
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    uint32_t capabilities = 0;

    char *key;
    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        int inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "capabilities") == 0) {
            sd_bus_message_read(msg, "v", "u", &capabilities);
            kywc_log(KYWC_DEBUG, "CreateSession: option capabilities: %d", capabilities);
        } else {
            kywc_log(KYWC_WARN, "CreateSession: unknown option %s", key);
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

    kywc_log(
        KYWC_DEBUG,
        "CreateSession by handle %s session_handle %s app_id %s parent_window %s, capabilities %d",
        handle, session_handle, app_id, parent_window, capabilities);

    if (capabilities == 0) { // This value is required
        kywc_log(KYWC_ERROR, "No capabilities requested");
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_create(ctx, PORTAL_SESSION_TYPE_INPUT_CAPTURE, session_handle);
    if (!session) {
        return -ENOMEM;
    }

    json_object *json_req = NULL;
    build_permission_request_json(&json_req, app_id, parent_window, capabilities, ctx);

    int dialog_socket = spawn_dialog("wldialog");
    if (!dialog_send_request(dialog_socket, json_req)) {
        close(dialog_socket);
        json_object_put(json_req);
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
    }

    json_object_put(json_req);

    struct permission_dialog_data *dialog_data = calloc(1, sizeof(*dialog_data));
    if (!dialog_data) {
        kywc_log(KYWC_ERROR, "Failed to allocate dialog data");
        close(dialog_socket);
        return -ENOMEM;
    }
    struct portal_request *request = portal_request_create(ctx, handle, dialog_data);
    if (!request) {
        free(dialog_data);
        close(dialog_socket);
        return -ENOMEM;
    }
    request->destroy = handle_request_destroy;

    dialog_data->msg = sd_bus_message_ref(msg);
    dialog_data->requested_caps = capabilities;
    dialog_data->session = session;
    dialog_data->socket_fd = dialog_socket;
    dialog_data->event_loop = ctx->event_loop;

    if (event_loop_add_source(ctx->event_loop, dialog_data->socket_fd, handle_permission_dialog,
                              request) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add dialog socket to event loop");
        portal_request_destroy(request);
        return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_ENDED, 0);
    }

    return 1;
}

static int handle_get_zones(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: GetZones called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to get zones on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "GetZones by handle %s session_handle %s app_id %s", handle,
             session_handle, app_id);

    sd_bus_message *reply = NULL;
    ret = sd_bus_message_new_method_return(msg, &reply);
    if (ret < 0) {
        return ret;
    }

    sd_bus_message_append(reply, "u", PORTAL_RESPONSE_SUCCESS);
    sd_bus_message_open_container(reply, 'a', "{sv}");
    sd_bus_message_open_container(reply, 'e', "sv");
    sd_bus_message_append_basic(reply, 's', "zones");
    sd_bus_message_open_container(reply, 'v', "a(uuii)");
    sd_bus_message_open_container(reply, 'a', "(uuii)");

    struct portal_output *output;
    wl_list_for_each(output, &ctx->wayland.outputs, link) {
        if (!output->output->enabled) {
            continue;
        }
        sd_bus_message_append(reply, "(uuii)", output->geometry.width, output->geometry.height,
                              output->geometry.x, output->geometry.y);
    }

    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);
    sd_bus_message_append(reply, "{sv}", "zone_set", "u", session->zone_set);
    sd_bus_message_close_container(reply);

    ret = sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);

    return ret;
}

#define SWAP(a, b)                                                                                 \
    do {                                                                                           \
        int _tmp = (a);                                                                            \
        (a) = (b);                                                                                 \
        (b) = _tmp;                                                                                \
    } while (0)

static bool check_barrier(struct portal_barrier *barrier, struct portal_context *ctx)
{
    int x1 = barrier->x1, y1 = barrier->y1;
    int x2 = barrier->x2, y2 = barrier->y2;

    if (x1 != x2 && y1 != y2) {
        kywc_log(KYWC_WARN, "Disallowed Diagonal barrier %d", barrier->id);
        return false;
    }

    if (x1 == x2 && y1 > y2) {
        SWAP(y1, y2);
    } else if (y1 == y2 && x1 > x2) {
        SWAP(x1, x2);
    }

    bool is_vertical = (x1 == x2);
    int adjacent_count = 0;
    struct kywc_box geo = { 0 };

    struct portal_output *output;
    wl_list_for_each(output, &ctx->wayland.outputs, link) {
        struct kywc_box *r = &output->geometry;

        if (is_vertical) {
            if (x1 == r->x || x1 == r->x + r->width) {
                adjacent_count++;
                geo = *r;
            }
        } else {
            if (y1 == r->y || y1 == r->y + r->height) {
                adjacent_count++;
                geo = *r;
            }
        }
    }

    if (adjacent_count != 1) {
        if (adjacent_count == 0) {
            kywc_log(KYWC_WARN, "Barrier %d not on any screen edge", barrier->id);
        } else {
            kywc_log(KYWC_WARN, "Barrier %d doesnt fill or on edge to another screen", barrier->id);
        }
        return false;
    }

    int left = geo.x, right = geo.x + geo.width;
    int top = geo.y, bottom = geo.y + geo.height;
    bool fully_covers = false;
    bool on_right_or_bottom = false;

    if (is_vertical) {
        if ((x1 == left || x1 == right) && y1 == top && y2 == bottom - 1) {
            fully_covers = true;
            on_right_or_bottom = (x1 == right);
        }
    } else {
        if ((y1 == top || y1 == bottom) && x1 == left && x2 == right - 1) {
            fully_covers = true;
            on_right_or_bottom = (y1 == bottom);
        }
    }

    if (!fully_covers) {
        kywc_log(KYWC_WARN, "Barrier %d doesnt fill or on edge to another screen", barrier->id);
        return false;
    }

    if (on_right_or_bottom) {
        if (x1 == x2) {
            x1--, x2--;
        } else {
            y1--, y2--;
        }
    }

    barrier->x1 = x1;
    barrier->y1 = y1;
    barrier->x2 = x2;
    barrier->y2 = y2;

    return true;
}

static int handle_set_pointer_barriers(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: SetPointerBarriers called");

    char *handle, *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "oos", &handle, &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to set pointer barriers on non-existing session %s",
                 session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    // barriers
    ret = sd_bus_message_enter_container(msg, 'a', "a{sv}");
    if (ret < 0) {
        return ret;
    }

    int inner_ret = 0;
    char *key;
    // clear the barriers
    session->barriers.size = 0;

    while ((ret = sd_bus_message_peek_type(msg, NULL, NULL)) > 0) {
        inner_ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
        if (inner_ret < 0) {
            return inner_ret;
        }

        struct portal_barrier *barrier = wl_array_add(&session->barriers, sizeof(*barrier));

        while ((inner_ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
            inner_ret = sd_bus_message_read(msg, "s", &key);
            if (inner_ret < 0) {
                return inner_ret;
            }

            if (strcmp(key, "barrier_id") == 0) {
                sd_bus_message_read(msg, "v", "u", &barrier->id);
                kywc_log(KYWC_DEBUG, "SetPointerBarriers: option barrier_id: %d", barrier->id);
            } else if (strcmp(key, "position") == 0) {
                sd_bus_message_read(msg, "v", "(iiii)", &barrier->x1, &barrier->y1, &barrier->x2,
                                    &barrier->y2);
                kywc_log(KYWC_DEBUG, "SetPointerBarriers: option position: %d/%d %d/%d",
                         barrier->x1, barrier->y1, barrier->x2, barrier->y2);
            } else {
                kywc_log(KYWC_WARN, "SetPointerBarriers: unknown option %s", key);
                sd_bus_message_skip(msg, "v");
            }

            inner_ret = sd_bus_message_exit_container(msg);
            if (inner_ret < 0) {
                return inner_ret;
            }
        }
        sd_bus_message_exit_container(msg);
    }
    if (ret < 0) {
        return ret;
    }
    ret = sd_bus_message_exit_container(msg);
    if (ret < 0) {
        return ret;
    }

    uint32_t zone_set;
    sd_bus_message_read(msg, "u", &zone_set);
    if (zone_set != session->zone_set) {
        kywc_log(KYWC_ERROR, "Invalid zone_set %d", zone_set);
        session->barriers.size = 0;
        return -PORTAL_RESPONSE_ENDED;
    }

    kywc_log(KYWC_DEBUG, "SetPointerBarriers by handle %s session_handle %s app_id %s zone_set %d",
             handle, session_handle, app_id, zone_set);

    sd_bus_message *reply = NULL;
    ret = sd_bus_message_new_method_return(msg, &reply);
    if (ret < 0) {
        return ret;
    }

    sd_bus_message_append(reply, "u", PORTAL_RESPONSE_SUCCESS);
    sd_bus_message_open_container(reply, 'a', "{sv}");
    sd_bus_message_open_container(reply, 'e', "sv");
    sd_bus_message_append_basic(reply, 's', "failed_barriers");
    sd_bus_message_open_container(reply, 'v', "au");
    sd_bus_message_open_container(reply, 'a', "u");

    struct portal_barrier *barrier;
    wl_array_for_each(barrier, &session->barriers) {
        if (barrier->id == 0) {
            kywc_log(KYWC_WARN, "Invalid barrier id %d", barrier->id);
            sd_bus_message_append(reply, "u", barrier->id);
            continue;
        }
        if (!check_barrier(barrier, ctx)) {
            sd_bus_message_append(reply, "u", barrier->id);
            // mark barrier is invalid
            barrier->id = 0;
            continue;
        }
    }

    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);
    sd_bus_message_close_container(reply);

    ret = sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);

    return ret;
}

static int handle_enable(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Enable called");

    char *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "os", &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to enable on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "Enable by session_handle %s app_id %s", session_handle, app_id);

    sd_bus_message *request = NULL;
    ret =
        sd_bus_message_new_method_call(ctx->bus, &request, wlcom_interface,
                                       session->input_capture_path, wlcom_eis_interface, "Enable");
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to create enable request");
        return ret;
    }

    sd_bus_message_open_container(request, 'a', "(iiii)");
    struct portal_barrier *barrier;
    wl_array_for_each(barrier, &session->barriers) {
        if (barrier->id == 0) {
            continue;
        }
        sd_bus_message_append(request, "(iiii)", barrier->x1, barrier->y1, barrier->x2,
                              barrier->y2);
    }
    sd_bus_message_close_container(request);

    ret = sd_bus_call(ctx->bus, request, 0, NULL, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to enable input capture");
    }
    sd_bus_message_unref(request);

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static int handle_disable(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Disable called");

    char *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "os", &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to disable on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "Disable by session_handle %s app_id %s", session_handle, app_id);

    ret = sd_bus_call_method(ctx->bus, wlcom_interface, session->input_capture_path,
                             wlcom_eis_interface, "Disable", NULL, NULL, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to disable input capture");
        return ret;
    }

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static int handle_release(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: Release called");

    char *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "os", &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to release on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        return ret;
    }

    /* options */
    uint32_t activation_id = 0;
    double cursor_x = 0, cursor_y = 0;

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
        if (inner_ret < 0) {
            return inner_ret;
        }

        if (strcmp(key, "activation_id") == 0) {
            sd_bus_message_read(msg, "v", "u", &activation_id);
            kywc_log(KYWC_DEBUG, "Release: option activation_id: %d", activation_id);
        } else if (strcmp(key, "cursor_position") == 0) {
            sd_bus_message_read(msg, "v", "(dd)", &cursor_x, &cursor_y);
            kywc_log(KYWC_DEBUG, "Release: option cursor_position: %f, %f", cursor_x, cursor_y);
        } else {
            kywc_log(KYWC_WARN, "Release: unknown option %s", key);
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

    kywc_log(KYWC_DEBUG, "Release by session_handle %s app_id %s", session_handle, app_id);

    ret = sd_bus_call_method(ctx->bus, wlcom_interface, session->input_capture_path,
                             wlcom_eis_interface, "Release", NULL, NULL, "(dd)b", cursor_x,
                             cursor_y, 1);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to release input capture");
        return ret;
    }

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
}

static int handle_connect_to_EIS(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "InputCapture: ConnectToEIS called");

    char *session_handle, *app_id;
    int ret = sd_bus_message_read(msg, "os", &session_handle, &app_id);
    if (ret < 0) {
        return ret;
    }

    struct portal_context *ctx = data;
    struct portal_session *session =
        portal_session_get(&ctx->input_capture.sessions, session_handle);
    if (!session) {
        kywc_log(KYWC_ERROR, "Tried to connect to eis on non-existing session %s", session_handle);
        return -PORTAL_RESPONSE_ENDED;
    }

    // options is unused, skip it
    sd_bus_message_skip(msg, "a{sv}");

    kywc_log(KYWC_DEBUG, "ConnectToEIS by session_handle %s app_id %s", session_handle, app_id);

    sd_bus_message *reply = NULL;
    ret = sd_bus_call_method(ctx->bus, wlcom_interface, session->input_capture_path,
                             wlcom_eis_interface, "ConnectToEIS", NULL, &reply, NULL);
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

static const sd_bus_vtable input_capture_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("SupportedCapabilities", "u", NULL,
                    offsetof(struct portal_context, input_capture.capabilities),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_PROPERTY("version", "u", NULL, offsetof(struct portal_context, input_capture.version),
                    SD_BUS_VTABLE_PROPERTY_CONST),
    SD_BUS_METHOD("CreateSession", "oossa{sv}", "ua{sv}", handle_create_session,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("GetZones", "oosa{sv}", "ua{sv}", handle_get_zones, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("SetPointerBarriers", "oosa{sv}aa{sv}u", "ua{sv}", handle_set_pointer_barriers,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Enable", "osa{sv}", "ua{sv}", handle_enable, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Disable", "osa{sv}", "ua{sv}", handle_disable, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Release", "osa{sv}", "ua{sv}", handle_release, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ConnectToEIS", "osa{sv}", "h", handle_connect_to_EIS,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_SIGNAL("Disabled", "oa{sv}", 0),
    SD_BUS_SIGNAL("Activated", "oa{sv}", 0),
    SD_BUS_SIGNAL("Deactivated", "oa{sv}", 0),
    SD_BUS_SIGNAL("ZonesChanged", "oa{sv}", 0),
    SD_BUS_VTABLE_END,
};

bool portal_input_capture_init(struct portal_context *ctx)
{
    ctx->input_capture.version = 1;
    ctx->input_capture.capabilities = 3; // keyboard and pointer
    wl_list_init(&ctx->input_capture.sessions);

    int ret = sd_bus_add_object_vtable(ctx->bus, NULL, object_path, interface_name,
                                       input_capture_vtable, ctx);
    if (ret < 0) {
        return false;
    }

    return true;
}

void portal_input_capture_emit_zones_changed(struct portal_context *ctx)
{
    if (ctx->input_capture.version == 0) {
        return;
    }

    struct portal_session *session;
    wl_list_for_each(session, &ctx->input_capture.sessions, link) {
        sd_bus_call_method(ctx->bus, wlcom_interface, session->input_capture_path,
                           wlcom_eis_interface, "Disable", NULL, NULL, NULL);

        session->barriers.size = 0;
        session->zone_set++;

        kywc_log(KYWC_DEBUG, "InputCapture: emit ZonesChanged to %s", session->session_handle);
        sd_bus_emit_signal(sd_bus_slot_get_bus(session->slot), object_path, interface_name,
                           "ZonesChanged", "oa{sv}", session->session_handle, 1, "zone_set", "u",
                           session->zone_set);
    }
}

void portal_input_capture_fini(struct portal_context *ctx)
{
    if (ctx->input_capture.version == 0) {
        return;
    }

    struct portal_session *session, *tmp;
    wl_list_for_each_safe(session, tmp, &ctx->input_capture.sessions, link) {
        portal_session_destroy(session);
    }
}
