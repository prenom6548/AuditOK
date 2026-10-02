// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "portal.h"

static const char interface_name[] = "org.freedesktop.impl.portal.Session";

static int handle_close(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "Session: Close called");

    sd_bus_message *reply = NULL;
    int ret = sd_bus_message_new_method_return(msg, &reply);
    if (ret < 0) {
        return ret;
    }

    ret = sd_bus_send(NULL, reply, NULL);
    if (ret < 0) {
        return ret;
    }

    sd_bus_message_unref(reply);

    struct portal_session *session = data;
    session->closed = true;
    portal_session_destroy(session);

    return 0;
}

static const sd_bus_vtable session_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Close", "", "", handle_close, 0),
    SD_BUS_VTABLE_END,
};

struct portal_session *portal_session_create(struct portal_context *ctx,
                                             enum portal_session_type type, char *object_path)
{
    struct portal_session *session = calloc(1, sizeof(*session));
    if (!session) {
        return NULL;
    }

    if (sd_bus_add_object_vtable(ctx->bus, &session->slot, object_path, interface_name,
                                 session_vtable, session) < 0) {
        free(session);
        kywc_log(KYWC_ERROR, "DBus: sd_bus_add_object_vtable failed: %s", strerror(-errno));
        return NULL;
    }

    session->session_handle = strdup(object_path);
    session->type = type;

    if (type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        wl_list_insert(&ctx->remote_desktop.sessions, &session->link);
    } else if (type == PORTAL_SESSION_TYPE_INPUT_CAPTURE) {
        wl_list_insert(&ctx->input_capture.sessions, &session->link);
    }

    wl_array_init(&session->barriers);

    return session;
}

struct portal_session *portal_session_get(struct wl_list *sessions, const char *object_path)
{
    if (!sessions || !object_path) {
        return NULL;
    }

    struct portal_session *session;
    wl_list_for_each(session, sessions, link) {
        if (strcmp(session->session_handle, object_path) == 0) {
            return session;
        }
    }

    return NULL;
}

void portal_session_destroy(struct portal_session *session)
{
    if (!session) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Session: %s destroy", session->session_handle);

    if (!session->closed) {
        sd_bus_emit_signal(sd_bus_slot_get_bus(session->slot), session->session_handle,
                           "org.freedesktop.impl.portal.Session", "Closed", "");
    }

    if (session->input_capture_path) {
        sd_bus_call_method(sd_bus_slot_get_bus(session->slot), "com.kylin.Wlcom",
                           "/com/kylin/Wlcom/EIS/InputCapture",
                           "com.kylin.Wlcom.EIS.InputCaptureManager", "RemoveInputCapture", NULL,
                           NULL, "o", session->input_capture_path);
    }

    sd_bus_slot_unref(session->slot);
    wl_list_remove(&session->link);

    wl_array_release(&session->barriers);

    free(session->session_handle);
    free(session->input_capture_path);
    free(session);
}
