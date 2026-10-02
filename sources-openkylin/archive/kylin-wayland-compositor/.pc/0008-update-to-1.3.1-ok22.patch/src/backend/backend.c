// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <libudev.h>
#include <string.h>
#include <xf86drmMode.h>

#include <kywc/log.h>

#include <wlr/backend/multi.h>
#include <wlr/backend/session.h>

#include "backend/backend.h"
#include "backend/drm.h"
#include "backend/fbdev.h"
#include "backend/libinput.h"
#include "util/time.h"

#define WAIT_SESSION_TIMEOUT 10000 // ms
#define WAIT_GPU_TIMEOUT 5000      // ms
#define MAX_NUM_GPUS 8

static struct wlr_session *session_create_and_wait(struct wl_display *disp)
{
    struct wlr_session *session = wlr_session_create(disp);
    if (!session) {
        kywc_log(KYWC_ERROR, "Failed to start a session");
        return NULL;
    }

    if (!session->active) {
        kywc_log(KYWC_INFO, "Waiting for a session to become active");

        int64_t started_at = current_time_msec();
        int64_t timeout = WAIT_SESSION_TIMEOUT;
        struct wl_event_loop *event_loop = wl_display_get_event_loop(session->display);

        while (!session->active) {
            int ret = wl_event_loop_dispatch(event_loop, (int)timeout);
            if (ret < 0) {
                kywc_log_errno(KYWC_ERROR, "Failed to wait for session active: "
                                           "wl_event_loop_dispatch failed");
                return NULL;
            }

            int64_t now = current_time_msec();
            if (now >= started_at + WAIT_SESSION_TIMEOUT) {
                break;
            }
            timeout = started_at + WAIT_SESSION_TIMEOUT - now;
        }

        if (!session->active) {
            kywc_log(KYWC_ERROR, "Timeout waiting session to become active");
            return NULL;
        }
    }

    return session;
}

static int explicit_find_fbs(struct wlr_session *session, int dev_len, char *dev[static dev_len],
                             const char *str)
{
    char *fbs = strdup(str);
    if (!fbs) {
        kywc_log(KYWC_ERROR, "Allocation failed");
        return -1;
    }

    int i = 0;
    char *save;
    char *ptr = strtok_r(fbs, ":", &save);
    do {
        if (i >= dev_len) {
            break;
        }

        dev[i] = strdup(ptr);
        if (!dev[i]) {
            kywc_log(KYWC_ERROR, "Allocation failed");
            break;
        }

        ++i;
    } while ((ptr = strtok_r(NULL, ":", &save)));

    free(fbs);
    return i;
}

static int session_find_framebuffer_device(struct wlr_session *session, char **devices,
                                           int devices_len)
{
    const char *explicit = getenv("KYWC_FB_DEVICES");
    if (explicit) {
        return explicit_find_fbs(session, devices_len, devices, explicit);
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(session->udev);
    if (!enumerate) {
        return -1;
    }

    udev_enumerate_add_match_sysname(enumerate, "fb[0-9]*");
    if (udev_enumerate_add_match_subsystem(enumerate, "graphics") < 0) {
        udev_enumerate_unref(enumerate);
        kywc_log(KYWC_ERROR, "Failed to add match subsystem");
        return -1;
    }

    if (udev_enumerate_scan_devices(enumerate) < 0) {
        udev_enumerate_unref(enumerate);
        kywc_log(KYWC_ERROR, "Failed to scan devices");
        return -1;
    }

    int i = 0;
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(enumerate)) {
        if (i >= devices_len) {
            break;
        }

        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *device = udev_device_new_from_syspath(session->udev, path);
        if (!device) {
            continue;
        }

        const char *seat = udev_device_get_property_value(device, "ID_SEAT");
        if (!seat) {
            seat = "seat0";
        }
        if (session->seat[0] && strcmp(session->seat, seat) != 0) {
            udev_device_unref(device);
            continue;
        }

        /* a framebuffer device was found */
        char *fbdev_path = strdup(udev_device_get_devnode(device));
        udev_device_unref(device);
        if (!fbdev_path) {
            kywc_log(KYWC_ERROR, "Allocation failed");
            break;
        }
        devices[i++] = fbdev_path;
    }

    udev_enumerate_unref(enumerate);

    return i;
}

static void frambffer_devices_release(char **devices, int n)
{
    for (int i = 0; i < n; i++) {
        free((void *)devices[i]);
    }
}

static bool attempt_fbdev_backend(struct wl_display *display, struct wlr_backend *backend,
                                  struct wlr_session *session)
{
    char *devices[MAX_NUM_GPUS];
    int n = session_find_framebuffer_device(session, devices, MAX_NUM_GPUS);
    if (n <= 0) {
        kywc_log(KYWC_ERROR, "Failed to find framebuffer device, can not create backend");
        return false;
    }

    kywc_log(KYWC_INFO, "Found %d framebuffer devices", n);

    struct wlr_backend *fbdev = fbdev_backend_create(display, session, (const char **)devices, n);
    if (!fbdev) {
        kywc_log(KYWC_ERROR, "Failed to create fbdev backend");
        frambffer_devices_release(devices, n);
        return false;
    }

    wlr_multi_backend_add(backend, fbdev);
    frambffer_devices_release(devices, n);

    return true;
}

static struct wlr_device *session_open_if_kms(struct wlr_session *restrict session,
                                              const char *restrict path)
{
    if (!path) {
        return NULL;
    }

    struct wlr_device *dev = wlr_session_open_file(session, path);
    if (!dev) {
        return NULL;
    }

    if (!drmIsKMS(dev->fd)) {
        kywc_log(KYWC_WARN, "Ignoring '%s': not a KMS device", path);
        wlr_session_close_file(session, dev);
        return NULL;
    }

    return dev;
}

struct add_drm_card_handler {
    bool found;
    struct wl_listener listener;
    struct wlr_session *session;
};

static void handle_session_add_drm_card(struct wl_listener *listener, void *data)
{
    struct wlr_session_add_event *event = data;
    struct add_drm_card_handler *handler = wl_container_of(listener, handler, listener);
    struct wlr_device *dev = session_open_if_kms(handler->session, event->path);
    if (dev) {
        wlr_session_close_file(handler->session, dev);
        handler->found = true;
    }
}

static bool find_drm_cards(struct wl_display *display, struct wlr_session *session)
{
    bool found = false;
    struct udev_enumerate *en = udev_enumerate_new(session->udev);
    if (en) {
        udev_enumerate_add_match_subsystem(en, "drm");
        udev_enumerate_add_match_sysname(en, "card[0-9]*");
        if (udev_enumerate_scan_devices(en) == 0) {
            struct udev_list_entry *entry;
            udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(en)) {
                const char *path = udev_list_entry_get_name(entry);
                struct udev_device *dev = udev_device_new_from_syspath(session->udev, path);

                const char *seat = udev_device_get_property_value(dev, "ID_SEAT");
                seat = seat ? seat : "seat0";
                if (session->seat[0] && strcmp(session->seat, seat) != 0) {
                    udev_device_unref(dev);
                    continue;
                }

                struct wlr_device *wlr_dev =
                    session_open_if_kms(session, udev_device_get_devnode(dev));
                udev_device_unref(dev);
                if (wlr_dev) {
                    wlr_session_close_file(session, wlr_dev);
                    found = true;
                    break;
                }
            }
        }
        udev_enumerate_unref(en);
    }

    if (found) {
        return found;
    }

    kywc_log(KYWC_WARN, "Waiting for a KMS device");

    struct add_drm_card_handler handler = { 0 };
    handler.session = session;
    handler.listener.notify = handle_session_add_drm_card;
    wl_signal_add(&session->events.add_drm_card, &handler.listener);

    int64_t started_at = current_time_msec();
    struct wl_event_loop *event_loop = wl_display_get_event_loop(display);
    do {
        int64_t now = current_time_msec();
        if (now >= started_at + WAIT_GPU_TIMEOUT) {
            break;
        }
        int64_t timeout = started_at + WAIT_GPU_TIMEOUT - now;
        int ret = wl_event_loop_dispatch(event_loop, (int)timeout);
        if (ret < 0) {
            kywc_log_errno(KYWC_ERROR, "Failed with wl_event_loop_dispatch");
            break;
        }
    } while (!handler.found);
    wl_list_remove(&handler.listener.link);

    return handler.found;
}

static struct wlr_backend *attempt_drm_backend(struct wlr_backend *backend,
                                               struct wlr_session *session)
{
    struct wlr_device *devices[MAX_NUM_GPUS];
    ssize_t n_gpus = wlr_session_find_gpus(session, MAX_NUM_GPUS, devices);
    if (n_gpus <= 0) {
        kywc_log(KYWC_ERROR, "Failed to find GPUS, can not create backend");
        return NULL;
    }

    kywc_log(KYWC_INFO, "Found %zu GPUS", n_gpus);

    struct wlr_backend *primary_drm = NULL;
    for (size_t i = 0; i < (size_t)n_gpus; ++i) {
        struct wlr_backend *drm = drm_backend_create(session, devices[i], primary_drm);
        if (!drm) {
            kywc_log(KYWC_ERROR, "Failed to create DRM backend");
            continue;
        }
        if (!primary_drm) {
            primary_drm = drm;
        }

        wlr_multi_backend_add(backend, drm);
    }
    if (!primary_drm) {
        kywc_log(KYWC_ERROR, "Could not successfully create backend on any GPU");
        return NULL;
    }

    if (!getenv("WLR_DRM_DEVICES")) {
        drm_backend_monitor_create(backend, primary_drm, session);
    }
    return primary_drm;
}

struct wlr_backend *ky_backend_autocreate(struct wl_display *display,
                                          struct wlr_session **session_ptr)
{
    /* try nested backend first */
    if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET") || getenv("DISPLAY")) {
        return wlr_backend_autocreate(display, session_ptr);
    }

    /* now fbdev or drm is used, session is needed */
    if (session_ptr != NULL) {
        *session_ptr = NULL;
    }

    /* create multi backend for fbdev/drm + libinput */
    struct wlr_backend *multi = wlr_multi_backend_create(display);
    if (!multi) {
        kywc_log(KYWC_ERROR, "Could not allocate multibackend");
        return NULL;
    }

    struct wlr_session *session = session_create_and_wait(display);
    if (!session) {
        kywc_log(KYWC_ERROR, "Failed to start a valid session");
        wlr_backend_destroy(multi);
        return NULL;
    }

    const char *env = getenv("KYWC_BACKEND");
    bool use_fbdev_backend = env && strcmp(env, "fbdev") == 0;

    /* try drm backend if fbdev is not specified */
    if (!use_fbdev_backend) {
        if (!find_drm_cards(display, session) || !attempt_drm_backend(multi, session)) {
            kywc_log(KYWC_ERROR, "Failed to open any drm device, fallback to fbdev");
            use_fbdev_backend = true;
        }
    }

    if (use_fbdev_backend && !attempt_fbdev_backend(display, multi, session)) {
        kywc_log(KYWC_ERROR, "Failed to open any framebuffer device");
        wlr_backend_destroy(multi);
        wlr_session_destroy(session);
        return NULL;
    }

    struct wlr_backend *libinput = libinput_backend_create(display, session);
    if (libinput) {
        wlr_multi_backend_add(multi, libinput);
    } else {
        kywc_log(KYWC_ERROR, "Failed to start libinput backend");
        wlr_backend_destroy(multi);
        wlr_session_destroy(session);
        return NULL;
    }

    if (session_ptr != NULL) {
        *session_ptr = session;
    }

    return multi;
}
