// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "portal.h"

static const char object_path[] = "/org/freedesktop/portal/desktop";
static const char interface_name[] = "org.freedesktop.impl.portal.RemoteDesktop";

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

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
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

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
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
            sd_bus_message_skip(msg, "v");
            kywc_log(KYWC_DEBUG, "SelectDevices: option restore_data skipped");
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

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
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

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
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

    return sd_bus_reply_method_return(msg, "ua{sv}", PORTAL_RESPONSE_SUCCESS, 0);
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

    int inner_ret = 0;
    char *key;

    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        inner_ret = sd_bus_message_read(msg, "s", &key);
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
