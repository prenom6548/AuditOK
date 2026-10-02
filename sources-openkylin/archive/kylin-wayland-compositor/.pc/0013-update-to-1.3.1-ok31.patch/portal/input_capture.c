// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "portal.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.InputCapture";

static const char wlcom_interface[] = "com.kylin.Wlcom";
static const char wlcom_eis_interface[] = "com.kylin.Wlcom.EIS.InputCapture";

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

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
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

    sd_bus_message *reply = NULL;
    ret = sd_bus_call_method(ctx->bus, wlcom_interface, "/com/kylin/Wlcom/EIS/InputCapture",
                             "com.kylin.Wlcom.EIS.InputCaptureManager", "AddInputCapture", NULL,
                             &reply, "u", capabilities);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Failed to add input capture");
        return ret;
    }

    char *input_capture_path = NULL;
    ret = sd_bus_message_read(reply, "o", &input_capture_path);
    if (ret >= 0) {
        session->input_capture_path = strdup(input_capture_path);
    }
    sd_bus_message_unref(reply);
    kywc_log(KYWC_DEBUG, "Get input capture %s from compositor", session->input_capture_path);

    // connect signals
    if (session->input_capture_path) {
        sd_bus_match_signal(ctx->bus, NULL, wlcom_interface, session->input_capture_path,
                            wlcom_eis_interface, "Activated", handle_session_activated, session);
        sd_bus_match_signal(ctx->bus, NULL, wlcom_interface, session->input_capture_path,
                            wlcom_eis_interface, "Deactivated", handle_session_deactivated,
                            session);
        sd_bus_match_signal(ctx->bus, NULL, wlcom_interface, session->input_capture_path,
                            wlcom_eis_interface, "Disabled", handle_session_disable, session);
    }

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 1, "capabilities",
                                      "u", capabilities);
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

    while ((ret == sd_bus_message_peek_type(msg, NULL, NULL)) > 0) {
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
    sd_bus_message_new_method_call(ctx->bus, &request, wlcom_interface, session->input_capture_path,
                                   wlcom_eis_interface, "Enable");
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
    SD_BUS_METHOD("CreateSession", "oossa{sv}", "ua{sv}", handle_create_session, 0),
    SD_BUS_METHOD("GetZones", "oosa{sv}", "ua{sv}", handle_get_zones, 0),
    SD_BUS_METHOD("SetPointerBarriers", "oosa{sv}aa{sv}u", "ua{sv}", handle_set_pointer_barriers,
                  0),
    SD_BUS_METHOD("Enable", "osa{sv}", "ua{sv}", handle_enable, 0),
    SD_BUS_METHOD("Disable", "osa{sv}", "ua{sv}", handle_disable, 0),
    SD_BUS_METHOD("Release", "osa{sv}", "ua{sv}", handle_release, 0),
    SD_BUS_METHOD("ConnectToEIS", "osa{sv}", "h", handle_connect_to_EIS, 0),
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
