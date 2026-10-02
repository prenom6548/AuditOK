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
    sd_bus_message_unref(reply);
    if (ret < 0) {
        return ret;
    }

    struct portal_session *session = data;
    session->closed = true;
    portal_session_destroy(session);

    return 0;
}

static const sd_bus_vtable session_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Close", "", "", handle_close, SD_BUS_VTABLE_UNPRIVILEGED),
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
    session->ctx = ctx;

    if (type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        wl_list_insert(&ctx->remote_desktop.sessions, &session->link);
        session->remote_desktop.screen_share_enabled = false;
        session->remote_desktop.clipboard_enabled = false;
        session->remote_desktop.ei_sender = NULL;
    } else if (type == PORTAL_SESSION_TYPE_INPUT_CAPTURE) {
        wl_list_insert(&ctx->input_capture.sessions, &session->link);
        wl_array_init(&session->barriers);
    } else if (type == PORTAL_SESSION_TYPE_SCREENCAST) {
        wl_list_insert(&ctx->screencast.sessions, &session->link);
    }

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

int portal_get_session_type(struct portal_context *ctx, const char *object_path)
{
    if (!ctx || !object_path) {
        return -1;
    }

    struct portal_session *session;
    wl_list_for_each(session, &ctx->remote_desktop.sessions, link) {
        if (strcmp(session->session_handle, object_path) == 0) {
            return PORTAL_SESSION_TYPE_REMOTE_DESKTOP;
        }
    }

    wl_list_for_each(session, &ctx->input_capture.sessions, link) {
        if (strcmp(session->session_handle, object_path) == 0) {
            return PORTAL_SESSION_TYPE_INPUT_CAPTURE;
        }
    }

    wl_list_for_each(session, &ctx->screencast.sessions, link) {
        if (strcmp(session->session_handle, object_path) == 0) {
            return PORTAL_SESSION_TYPE_SCREENCAST;
        }
    }

    return -1;
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

    if (session->type == PORTAL_SESSION_TYPE_INPUT_CAPTURE) {
        if (session->input_capture_path) {
            sd_bus_call_method(sd_bus_slot_get_bus(session->slot), "com.kylin.Wlcom",
                               "/com/kylin/Wlcom/EIS/InputCapture",
                               "com.kylin.Wlcom.EIS.InputCaptureManager", "RemoveInputCapture",
                               NULL, NULL, "o", session->input_capture_path);
        }
        wl_array_release(&session->barriers);
        free(session->input_capture_path);
    } else if (session->type == PORTAL_SESSION_TYPE_REMOTE_DESKTOP) {
        if (session->remote_desktop.restore_data_destroy) {
            session->remote_desktop.restore_data_destroy(session->remote_desktop.restore_data);
        }
        if (session->remote_desktop.screen_share_enabled) {
            portal_stream_destroy(session);
        }
        ei_sender_close(session->remote_desktop.ei_sender);
    } else if (session->type == PORTAL_SESSION_TYPE_SCREENCAST) {
        if (session->screencast.restore_data) {
            session->screencast.restore_data_destroy(session->screencast.restore_data);
        }
        portal_stream_destroy(session);
    }

    if (session->destroy) {
        session->destroy(session->data);
    }

    sd_bus_slot_unref(session->slot);
    wl_list_remove(&session->link);

    free(session->session_handle);
    free(session);
}
