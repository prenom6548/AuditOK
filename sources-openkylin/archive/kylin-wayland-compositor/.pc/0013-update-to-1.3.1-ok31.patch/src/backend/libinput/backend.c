// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "backend/libinput.h"
#include "backend/session.h"
#include "libinput_p.h"

static struct libinput_backend *get_libinput_backend_from_backend(struct wlr_backend *wlr_backend)
{
    assert(wlr_backend_is_libinput(wlr_backend));
    struct libinput_backend *backend = wl_container_of(wlr_backend, backend, backend);
    return backend;
}

static int libinput_open_restricted(const char *path, int flags, void *_backend)
{
    struct libinput_backend *backend = _backend;
    struct session_device *dev = session_device_create(backend->session, path);
    return dev ? dev->fd : -1;
}

static void libinput_close_restricted(int fd, void *_backend)
{
    struct libinput_backend *backend = _backend;
    bool found = false;

    struct session_device *dev;
    wl_list_for_each(dev, &backend->session->devices, link) {
        if (dev->fd == fd) {
            found = true;
            break;
        }
    }

    if (found) {
        session_device_destroy(backend->session, dev);
    }
}

static const struct libinput_interface libinput_impl = {
    .open_restricted = libinput_open_restricted,
    .close_restricted = libinput_close_restricted,
};

static void init_property_exts(struct libinput_backend *backend)
{
    struct libinput_property_exts *property_exts = &backend->property_exts;

    property_exts->touch_has_major = dlsym(RTLD_DEFAULT, "libinput_event_touch_has_major");
    property_exts->touch_has_minor = dlsym(RTLD_DEFAULT, "libinput_event_touch_has_minor");

    property_exts->touch_get_major_value =
        dlsym(RTLD_DEFAULT, "libinput_event_touch_get_major_value");
    property_exts->touch_get_minor_value =
        dlsym(RTLD_DEFAULT, "libinput_event_touch_get_minor_value");
    property_exts->device_get_touch_major_minimum =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_major_minimum");
    property_exts->device_get_touch_major_maximum =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_major_maximum");
    property_exts->device_get_touch_major_resolution =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_major_resolution");
    property_exts->device_get_touch_minor_minimum =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_minor_minimum");
    property_exts->device_get_touch_minor_maximum =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_minor_maximum");
    property_exts->device_get_touch_minor_resolution =
        dlsym(RTLD_DEFAULT, "libinput_device_get_touch_minor_resolution");

    property_exts->support_touch_size =
        property_exts->touch_has_major && property_exts->touch_has_minor &&
        property_exts->touch_get_major_value && property_exts->touch_get_minor_value;

    property_exts->support_touch_abs = property_exts->device_get_touch_major_minimum &&
                                       property_exts->device_get_touch_major_maximum &&
                                       property_exts->device_get_touch_major_resolution &&
                                       property_exts->device_get_touch_minor_minimum &&
                                       property_exts->device_get_touch_minor_maximum &&
                                       property_exts->device_get_touch_minor_resolution;

    kywc_log(KYWC_INFO, "libinput touch size extension %s",
             property_exts->support_touch_size ? "supported" : "unsupported");
    kywc_log(KYWC_INFO, "libinput touch size abs extension %s",
             property_exts->support_touch_abs ? "supported" : "unsupported");
}

static enum kywc_log_level libinput_log_priority_to_kywc(enum libinput_log_priority priority)
{
    switch (priority) {
    case LIBINPUT_LOG_PRIORITY_ERROR:
        return KYWC_ERROR;
    case LIBINPUT_LOG_PRIORITY_INFO:
        return KYWC_INFO;
    default:
        return KYWC_DEBUG;
    }
}

static void log_libinput(struct libinput *libinput_context, enum libinput_log_priority priority,
                         const char *fmt, va_list args)
{
    enum kywc_log_level level = libinput_log_priority_to_kywc(priority);
    static char kywc_fmt[1024];
    snprintf(kywc_fmt, sizeof(kywc_fmt), "[libinput] %s", fmt);
    kywc_vlog(level, kywc_fmt, args);
}

static int handle_libinput_readable(int fd, uint32_t mask, void *_backend)
{
    struct libinput_backend *backend = _backend;
    int ret = libinput_dispatch(backend->libinput_context);
    if (ret != 0) {
        kywc_log(KYWC_ERROR, "Failed to dispatch libinput: %s", strerror(-ret));
        wlr_backend_destroy(&backend->backend);
        return 0;
    }
    struct libinput_event *event;
    while ((event = libinput_get_event(backend->libinput_context))) {
        handle_libinput_event(backend, event);
        libinput_event_destroy(event);
    }
    return 0;
}

static bool backend_start(struct wlr_backend *wlr_backend)
{
    struct libinput_backend *backend = get_libinput_backend_from_backend(wlr_backend);
    kywc_log(KYWC_DEBUG, "Starting libinput backend");

    backend->libinput_context =
        libinput_udev_create_context(&libinput_impl, backend, backend->session->udev);
    if (!backend->libinput_context) {
        kywc_log(KYWC_ERROR, "Failed to create libinput context");
        return false;
    }

    if (libinput_udev_assign_seat(backend->libinput_context, backend->session->seat) != 0) {
        kywc_log(KYWC_ERROR, "Failed to assign libinput seat");
        return false;
    }

    libinput_log_set_handler(backend->libinput_context, log_libinput);
    libinput_log_set_priority(backend->libinput_context, LIBINPUT_LOG_PRIORITY_ERROR);

    int libinput_fd = libinput_get_fd(backend->libinput_context);

    handle_libinput_readable(libinput_fd, WL_EVENT_READABLE, backend);
    if (wl_list_empty(&backend->devices)) {
        kywc_log(KYWC_WARN, "No input devices during libinput backend initialization");
    }

    if (backend->input_event) {
        wl_event_source_remove(backend->input_event);
    }
    backend->input_event = wl_event_loop_add_fd(backend->event_loop, libinput_fd, WL_EVENT_READABLE,
                                                handle_libinput_readable, backend);
    if (!backend->input_event) {
        kywc_log(KYWC_ERROR, "Failed to create input event on event loop");
        return false;
    }
    kywc_log(KYWC_DEBUG, "libinput successfully initialized");
    return true;
}

static void backend_destroy(struct wlr_backend *wlr_backend)
{
    if (!wlr_backend) {
        return;
    }

    struct libinput_backend *backend = get_libinput_backend_from_backend(wlr_backend);

    struct libinput_input_device *dev, *tmp;
    wl_list_for_each_safe(dev, tmp, &backend->devices, link) {
        destroy_libinput_input_device(dev);
    }

    wlr_backend_finish(wlr_backend);

    wl_list_remove(&backend->session_destroy.link);
    wl_list_remove(&backend->session_active.link);

    if (backend->input_event) {
        wl_event_source_remove(backend->input_event);
    }
    libinput_unref(backend->libinput_context);
    free(backend);
}

static const struct wlr_backend_impl backend_impl = {
    .start = backend_start,
    .destroy = backend_destroy,
};

bool wlr_backend_is_libinput(struct wlr_backend *backend)
{
    return backend->impl == &backend_impl;
}

static void handle_session_active(struct wl_listener *listener, void *data)
{
    struct libinput_backend *backend = wl_container_of(listener, backend, session_active);
    struct session *session = backend->session;

    if (!backend->libinput_context) {
        return;
    }

    if (session->active) {
        libinput_resume(backend->libinput_context);
    } else {
        libinput_suspend(backend->libinput_context);
    }

    int libinput_fd = libinput_get_fd(backend->libinput_context);
    handle_libinput_readable(libinput_fd, WL_EVENT_READABLE, backend);
}

static void handle_session_destroy(struct wl_listener *listener, void *data)
{
    struct libinput_backend *backend = wl_container_of(listener, backend, session_destroy);
    backend_destroy(&backend->backend);
}

struct wlr_backend *libinput_backend_create(struct wl_display *display, struct session *session)
{
    struct libinput_backend *backend = calloc(1, sizeof(*backend));
    if (!backend) {
        kywc_log(KYWC_ERROR, "Allocation failed: %s", strerror(errno));
        return NULL;
    }

    wlr_backend_init(&backend->backend, &backend_impl);
    backend->event_loop = wl_display_get_event_loop(display);
    wl_list_init(&backend->devices);
    init_property_exts(backend);
    kywc_log(KYWC_INFO, "libinput gesture raw tap %s",
             HAVE_LIBINPUT_RAW_TAP ? "supported" : "unsupported");

    backend->session = session;
    backend->session_active.notify = handle_session_active;
    wl_signal_add(&session->events.active, &backend->session_active);
    backend->session_destroy.notify = handle_session_destroy;
    wl_signal_add(&session->events.destroy, &backend->session_destroy);

    return &backend->backend;
}

bool wlr_input_device_is_libinput(struct wlr_input_device *wlr_dev)
{
    switch (wlr_dev->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        return wlr_keyboard_from_input_device(wlr_dev)->impl == &libinput_keyboard_impl;
    case WLR_INPUT_DEVICE_POINTER:
        return wlr_pointer_from_input_device(wlr_dev)->impl == &libinput_pointer_impl;
    case WLR_INPUT_DEVICE_TOUCH:
        return wlr_touch_from_input_device(wlr_dev)->impl == &libinput_touch_impl;
    case WLR_INPUT_DEVICE_TABLET_TOOL:
        return wlr_tablet_from_input_device(wlr_dev)->impl == &libinput_tablet_impl;
    case WLR_INPUT_DEVICE_TABLET_PAD:
        return wlr_tablet_pad_from_input_device(wlr_dev)->impl == &libinput_tablet_pad_impl;
    case WLR_INPUT_DEVICE_SWITCH:
        return wlr_switch_from_input_device(wlr_dev)->impl == &libinput_switch_impl;
    default:
        return false;
    }
}

struct libinput_device *wlr_libinput_get_device_handle(struct wlr_input_device *wlr_dev)
{
    struct libinput_input_device *dev = NULL;
    switch (wlr_dev->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        dev = device_from_keyboard(wlr_keyboard_from_input_device(wlr_dev));
        break;
    case WLR_INPUT_DEVICE_POINTER:
        dev = device_from_pointer(wlr_pointer_from_input_device(wlr_dev));
        break;
    case WLR_INPUT_DEVICE_SWITCH:
        dev = device_from_switch(wlr_switch_from_input_device(wlr_dev));
        break;
    case WLR_INPUT_DEVICE_TOUCH:
        dev = device_from_touch(wlr_touch_from_input_device(wlr_dev));
        break;
    case WLR_INPUT_DEVICE_TABLET_TOOL:
        dev = device_from_tablet(wlr_tablet_from_input_device(wlr_dev));
        break;
    case WLR_INPUT_DEVICE_TABLET_PAD:
        dev = device_from_tablet_pad(wlr_tablet_pad_from_input_device(wlr_dev));
        break;
    }
    assert(dev);
    return dev->handle;
}

void wlr_libinput_add_raw_tap_listener(struct wlr_input_device *wlr_dev,
                                       struct wl_listener *listener)
{
    struct libinput_input_device *dev = device_from_pointer(wlr_pointer_from_input_device(wlr_dev));
    wl_signal_add(&dev->events.raw_tap, listener);
}

void wlr_libinput_add_zone_gesture_begin_listener(struct wlr_input_device *wlr_dev,
                                                  struct wl_listener *listener)
{
    struct libinput_input_device *dev = device_from_pointer(wlr_pointer_from_input_device(wlr_dev));
    wl_signal_add(&dev->events.zone_begin, listener);
}

void wlr_libinput_add_zone_gesture_update_listener(struct wlr_input_device *wlr_dev,
                                                   struct wl_listener *listener)
{
    struct libinput_input_device *dev = device_from_pointer(wlr_pointer_from_input_device(wlr_dev));
    wl_signal_add(&dev->events.zone_update, listener);
}

void wlr_libinput_add_zone_gesture_end_listener(struct wlr_input_device *wlr_dev,
                                                struct wl_listener *listener)
{
    struct libinput_input_device *dev = device_from_pointer(wlr_pointer_from_input_device(wlr_dev));
    wl_signal_add(&dev->events.zone_end, listener);
}
