// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "portal.h"

static const char interface_name[] = "org.freedesktop.impl.portal.Request";

static int handle_close(sd_bus_message *msg, void *data, sd_bus_error *ret_error)
{
    kywc_log(KYWC_DEBUG, "Request: Close called");

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

    struct portal_request *request = data;
    portal_request_destroy(request);

    return 0;
}

static const sd_bus_vtable request_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Close", "", "", handle_close, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

struct portal_request *portal_request_create(struct portal_context *ctx, const char *object_path,
                                             void *data)
{
    struct portal_request *request = calloc(1, sizeof(*request));
    if (!request) {
        return NULL;
    }

    if (sd_bus_add_object_vtable(ctx->bus, &request->slot, object_path, interface_name,
                                 request_vtable, request) < 0) {
        free(request);
        kywc_log(KYWC_ERROR, "DBus: sd_bus_add_object_vtable failed: %s", strerror(-errno));
        return NULL;
    }

    request->ctx = ctx;
    request->handle = strdup(object_path);
    request->data = data;
    wl_list_init(&request->link);

    return request;
}

void portal_request_destroy(struct portal_request *request)
{
    if (!request) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Request: %s destroy", request->handle);

    if (request->destroy) {
        request->destroy(request->data);
    }
    sd_bus_slot_unref(request->slot);
    wl_list_remove(&request->link);

    free(request->handle);
    free(request);
}
