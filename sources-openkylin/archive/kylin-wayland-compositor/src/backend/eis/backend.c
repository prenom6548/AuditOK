// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include "backend/eis.h"
#include "eis_p.h"
#include "util/dbus.h"

static int add_input_capture(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    uint32_t capabilities = 0;
    CK(sd_bus_message_read(msg, "u", &capabilities));

    uint32_t eis_capabilities = 0;
    if (capabilities & 1) {
        eis_capabilities |= EIS_DEVICE_CAP_KEYBOARD;
    }
    if (capabilities & 2) {
        eis_capabilities |= EIS_DEVICE_CAP_POINTER;
        eis_capabilities |= EIS_DEVICE_CAP_POINTER_ABSOLUTE;
        eis_capabilities |= EIS_DEVICE_CAP_BUTTON;
        eis_capabilities |= EIS_DEVICE_CAP_SCROLL;
    }
    if (capabilities & 4) {
        eis_capabilities |= EIS_DEVICE_CAP_TOUCH;
    }

    struct eis_backend *backend = userdata;
    eis_capabilities &= backend->capabilities;

    const char *sender = sd_bus_message_get_sender(msg);
    struct eis_context *context =
        eis_context_create(backend, EIS_CONTEXT_TYPE_INPUT_CAPTURE, eis_capabilities, sender);
    if (!context) {
        return -ENOMEM;
    }

    return sd_bus_reply_method_return(msg, "o", context->dbus_path);
}

static int remove_input_capture(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    const char *path = NULL;
    CK(sd_bus_message_read(msg, "o", &path));

    struct eis_backend *backend = userdata;
    struct eis_context *context, *tmp;
    wl_list_for_each_safe(context, tmp, &backend->contexts, link) {
        if (context->type != EIS_CONTEXT_TYPE_INPUT_CAPTURE) {
            continue;
        }
        if (context->dbus_path && strcmp(context->dbus_path, path) == 0) {
            eis_context_destroy(context);
            break;
        }
    }

    return sd_bus_reply_method_return(msg, NULL);
}

static const sd_bus_vtable capture_manager_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("AddInputCapture", "u", "o", add_input_capture, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("RemoveInputCapture", "o", "", remove_input_capture, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

static struct eis_backend *get_eis_backend_from_backend(struct wlr_backend *wlr_backend)
{
    assert(wlr_backend_is_eis(wlr_backend));
    struct eis_backend *backend = wl_container_of(wlr_backend, backend, backend);
    return backend;
}

static bool backend_start(struct wlr_backend *wlr_backend)
{
    struct eis_backend *backend = get_eis_backend_from_backend(wlr_backend);
    kywc_log(KYWC_DEBUG, "Starting eis backend");

    /* input capture manager dbus */
    backend->capture_manager = dbus_register_object(NULL, "/com/kylin/Wlcom/EIS/InputCapture",
                                                    "com.kylin.Wlcom.EIS.InputCaptureManager",
                                                    capture_manager_vtable, backend);
    if (!backend->capture_manager) {
        kywc_log(KYWC_ERROR, "Failed to register capture manager, skip eis backend");
        return true;
    }

    /* create socket context for debug */
    const char *name = getenv("KYWC_EIS_SOCKET");
    if (name) {
        kywc_log(KYWC_INFO, "Create eis context with socket %s", name);
        eis_context_create(backend, EIS_CONTEXT_TYPE_NORMAL, backend->capabilities, name);
    }

    kywc_log(KYWC_INFO, "Create eis context for remote desktop");
    eis_context_create(backend, EIS_CONTEXT_TYPE_REMOTE_DESKTOP, backend->capabilities, NULL);

    kywc_log(KYWC_DEBUG, "Eis successfully initialized");
    return true;
}

static void backend_destroy(struct wlr_backend *wlr_backend)
{
    if (!wlr_backend) {
        return;
    }

    struct eis_backend *backend = get_eis_backend_from_backend(wlr_backend);
    dbus_unregister_object(backend->capture_manager);

    // destroy all contexts if compsitor is destroyed
    struct eis_context *context, *tmp;
    wl_list_for_each_safe(context, tmp, &backend->contexts, link) {
        eis_context_destroy(context);
    }

    wlr_backend_finish(wlr_backend);
    free(backend);
}

static const struct wlr_backend_impl backend_impl = {
    .start = backend_start,
    .destroy = backend_destroy,
};

bool wlr_backend_is_eis(struct wlr_backend *backend)
{
    return backend->impl == &backend_impl;
}

struct wlr_backend *eis_backend_create(struct wl_display *display, struct wlr_output_layout *layout)
{
    struct eis_backend *backend = calloc(1, sizeof(*backend));
    if (!backend) {
        kywc_log(KYWC_ERROR, "Failed to allocate eis_backend");
        return NULL;
    }

    // TODO: pause the process when session inactive
    wlr_backend_init(&backend->backend, &backend_impl);

    wl_list_init(&backend->contexts);
    backend->event_loop = wl_display_get_event_loop(display);
    backend->layout = layout;

    // touch is not support current
    backend->capabilities =
        (EIS_DEVICE_CAP_POINTER | EIS_DEVICE_CAP_POINTER_ABSOLUTE | EIS_DEVICE_CAP_KEYBOARD |
         EIS_DEVICE_CAP_SCROLL | EIS_DEVICE_CAP_BUTTON);

    return &backend->backend;
}
