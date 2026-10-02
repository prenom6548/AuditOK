// SPDX-FileCopyrightText: 2024 The wlroots contributors
// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <libudev.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <kywc/log.h>

#include <wlr/backend/multi.h>

#include "backend/backend.h"
#include "backend/drm.h"
#include "backend/fbdev.h"
#include "backend/libinput.h"
#include "backend/session.h"
#include "util/string.h"
#include "util/time.h"

#define WAIT_SESSION_TIMEOUT 10000 // ms
#define WAIT_GPU_TIMEOUT 5000      // ms
#define MAX_NUM_GPUS 8

static struct session *session_create_and_wait(struct wl_display *disp)
{
    struct session *session = session_create(disp);
    if (!session) {
        kywc_log(KYWC_ERROR, "Failed to start a session");
        return NULL;
    }

    if (!session->active) {
        kywc_log(KYWC_INFO, "Waiting for a session to become active");

        int64_t started_at = current_time_msec();
        int64_t timeout = WAIT_SESSION_TIMEOUT;
        struct wl_event_loop *event_loop = wl_display_get_event_loop(disp);

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

static ssize_t find_framebuffer_devices(struct session *session, size_t devices_len, char **devices)
{
    const char *explicit = getenv("KYWC_FB_DEVICES");
    if (explicit) {
        size_t split_len, i;
        char **devs = string_split(explicit, ":", &split_len);
        for (i = 0; i < devices_len && i < split_len; i++) {
            devices[i] = strdup(devs[i]);
        }
        string_free_split(devs);
        return i;
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

    size_t i = 0;
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
        const char *devnode = udev_device_get_devnode(device);
        if (!devnode) {
            udev_device_unref(device);
            continue;
        }
        char *fbdev_path = strdup(devnode);
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

static void framebuffer_devices_release(char **devices, ssize_t n)
{
    for (ssize_t i = 0; i < n; i++) {
        free((void *)devices[i]);
    }
}

static bool attempt_fbdev_backend(struct wl_display *display, struct wlr_backend *backend,
                                  struct session *session)
{
    char *devices[MAX_NUM_GPUS];
    ssize_t n = find_framebuffer_devices(session, MAX_NUM_GPUS, devices);
    if (n <= 0) {
        kywc_log(KYWC_ERROR, "Failed to find framebuffer device, can not create backend");
        return false;
    }

    kywc_log(KYWC_INFO, "Found %ld framebuffer devices", n);

    struct wlr_backend *fbdev = fbdev_backend_create(display, session, (const char **)devices, n);
    if (!fbdev) {
        kywc_log(KYWC_ERROR, "Failed to create fbdev backend");
        framebuffer_devices_release(devices, n);
        return false;
    }

    wlr_multi_backend_add(backend, fbdev);
    framebuffer_devices_release(devices, n);

    return true;
}

static struct udev_enumerate *enumerate_drm_cards(struct udev *udev)
{
    struct udev_enumerate *en = udev_enumerate_new(udev);
    if (!en) {
        kywc_log(KYWC_ERROR, "udev_enumerate_new failed");
        return NULL;
    }

    udev_enumerate_add_match_subsystem(en, "drm");
    udev_enumerate_add_match_sysname(en, DRM_PRIMARY_MINOR_NAME "[0-9]*");

    if (udev_enumerate_scan_devices(en) != 0) {
        kywc_log(KYWC_ERROR, "udev_enumerate_scan_devices failed");
        udev_enumerate_unref(en);
        return NULL;
    }

    return en;
}

struct new_dev_handler {
    bool found;
    struct wl_listener listener;
    struct session *session;
};

static void handle_session_new_dev(struct wl_listener *listener, void *data)
{
    struct session_add_event *event = data;
    struct new_dev_handler *handler = wl_container_of(listener, handler, listener);
    struct session_device *dev = session_device_create(handler->session, event->path);
    if (dev && drmIsKMS(dev->fd)) {
        handler->found = true;
    }
    session_device_destroy(handler->session, dev);
}

enum gpu_prop_map {
    GPU_IS_BOOT_DISPLAY = 0,
    GPU_IS_BOOT_VGA = 1,
    GPU_IS_PCI = 2,
    GPU_IS_USB = 3,
};

/**
 * main gpu priority
 * 1. boot_display
 * 2. boot_vga
 * 3. the first pci dev
 * 4. the first usb dev
 * */
static void pick_main_gpu(struct session_device **ret, uint32_t *props, size_t len)
{
    struct session_device *tmp = NULL;
    size_t i;

    for (i = 0; i < len; i++) {
        if (props[i] & 1 << GPU_IS_BOOT_DISPLAY) {
            tmp = ret[i];
            goto out;
        }
    }

    for (i = 0; i < len; i++) {
        if (props[i] & 1 << GPU_IS_BOOT_VGA) {
            tmp = ret[i];
            goto out;
        }
    }

    for (i = 0; i < len; i++) {
        if (props[i] & 1 << GPU_IS_PCI) {
            tmp = ret[i];
            goto out;
        }
    }

    // D3000M: boot_display 0, boot_vga 0, pci 0, usb 0
    // UDL: boot_display 0, boot_vga 0, pci 0, usb 1
    if (props[0] & 1 << GPU_IS_USB) {
        for (i = 1; i < len; i++) {
            if (!(props[i] & 1 << GPU_IS_USB)) {
                tmp = ret[i];
                goto out;
            }
        }
    }

    // all usb, do nothing

out:
    if (i && tmp) {
        ret[i] = ret[0];
        ret[0] = tmp;
    }
}

static void init_device_property(struct udev_device *dev, uint32_t *prop)
{
    const char *boot_display = udev_device_get_sysattr_value(dev, "boot_display");
    *prop |= ((boot_display && strcmp(boot_display, "1") == 0) << GPU_IS_BOOT_DISPLAY);

    // This is owned by 'dev', so we don't need to free it
    struct udev_device *pci = udev_device_get_parent_with_subsystem_devtype(dev, "pci", NULL);
    if (pci) {
        const char *id = udev_device_get_sysattr_value(pci, "boot_vga");
        *prop |= ((id && strcmp(id, "1") == 0) << GPU_IS_BOOT_VGA);
        *prop |= (1 << GPU_IS_PCI);
    } else {
        *prop |= (!!udev_device_get_parent_with_subsystem_devtype(dev, "usb", NULL) << GPU_IS_USB);
    }

    kywc_log(KYWC_INFO, "Found %s(%s): boot_display %d, boot_vga %d, pci %d, usb %d",
             udev_device_get_devnode(dev), udev_device_get_subsystem(dev),
             !!(*prop & 1 << GPU_IS_BOOT_DISPLAY), !!(*prop & 1 << GPU_IS_BOOT_VGA),
             !!(*prop & 1 << GPU_IS_PCI), !!(*prop & 1 << GPU_IS_USB));
}

static ssize_t find_gpus(struct session *session, size_t ret_len, struct session_device **ret)
{
    const char *explicit = getenv("KYWC_DRM_DEVICES");
    if (explicit) {
        kywc_log(KYWC_INFO, "Opening devices: %s", explicit);
        size_t split_len, j = 0;
        char **devs = string_split(explicit, ":", &split_len);
        for (size_t i = 0; i < split_len && j < ret_len; i++) {
            ret[j] = session_device_create(session, devs[i]);
            if (ret[j] && drmIsKMS(ret[j]->fd)) {
                j++;
            } else {
                session_device_destroy(session, ret[j]);
                kywc_log(KYWC_ERROR, "Unable to open %s as KMS device", devs[i]);
            }
        }
        string_free_split(devs);
        return j;
    }

    struct udev_enumerate *en = enumerate_drm_cards(session->udev);
    if (!en) {
        return -1;
    }

    if (udev_enumerate_get_list_entry(en) == NULL) {
        udev_enumerate_unref(en);
        en = NULL;
        kywc_log(KYWC_INFO, "Waiting for a KMS device");

        struct new_dev_handler handler = { 0 };
        handler.listener.notify = handle_session_new_dev;
        wl_signal_add(&session->events.new_dev, &handler.listener);

        int64_t started_at = current_time_msec();
        int64_t timeout = WAIT_GPU_TIMEOUT;
        struct wl_event_loop *event_loop = wl_display_get_event_loop(session->display);
        while (!handler.found) {
            if (wl_event_loop_dispatch(event_loop, (int)timeout) < 0) {
                kywc_log_errno(KYWC_ERROR, "Failed to wait for KMS device");
                return -1;
            }

            int64_t now = current_time_msec();
            if (now >= started_at + WAIT_GPU_TIMEOUT) {
                break;
            }
            timeout = started_at + WAIT_GPU_TIMEOUT - now;
        }

        wl_list_remove(&handler.listener.link);

        en = enumerate_drm_cards(session->udev);
        if (!en) {
            return -1;
        }
    }

    size_t i = 0;
    uint32_t props[ret_len];
    memset(props, 0, sizeof(props));
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(en)) {
        if (i == ret_len) {
            break;
        }

        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(session->udev, path);
        if (!dev) {
            continue;
        }

        const char *seat = udev_device_get_property_value(dev, "ID_SEAT");
        if (!seat) {
            seat = "seat0";
        }
        if (session->seat[0] && strcmp(session->seat, seat) != 0) {
            udev_device_unref(dev);
            continue;
        }

        struct session_device *sdev = session_device_create(session, udev_device_get_devnode(dev));
        if (!sdev || !drmIsKMS(sdev->fd)) {
            session_device_destroy(session, sdev);
            udev_device_unref(dev);
            continue;
        }

        ret[i] = sdev;

        init_device_property(dev, &props[i]);
        udev_device_unref(dev);
        i++;
    }

    udev_enumerate_unref(en);
    pick_main_gpu(ret, props, i);

    return i;
}

static struct wlr_backend *attempt_drm_backend(struct wlr_backend *backend, struct session *session)
{
    struct session_device *devices[MAX_NUM_GPUS];
    ssize_t n_gpus = find_gpus(session, MAX_NUM_GPUS, devices);
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
            session_device_destroy(session, devices[i]);
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

    if (!getenv("KYWC_DRM_DEVICES")) {
        drm_backend_monitor_create(backend, primary_drm, session);
    }
    return primary_drm;
}

struct wlr_backend *backend_autocreate(struct wl_display *display, struct session **session_ptr)
{
    /* try nested backend first */
    if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET") || getenv("DISPLAY")) {
        return wlr_backend_autocreate(display, NULL); // wayland/X11 backend don't need session
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

    struct session *session = session_create_and_wait(display);
    if (!session) {
        kywc_log(KYWC_ERROR, "Failed to start a valid session");
        wlr_backend_destroy(multi);
        return NULL;
    }

    const char *env = getenv("KYWC_BACKEND");
    bool use_fbdev_backend = env && strcmp(env, "fbdev") == 0;

    /* try drm backend if fbdev is not specified */
    if (!use_fbdev_backend && !attempt_drm_backend(multi, session)) {
        kywc_log(KYWC_ERROR, "Failed to open any drm device, fallback to fbdev");
        use_fbdev_backend = true;
    }

    if (use_fbdev_backend && !attempt_fbdev_backend(display, multi, session)) {
        kywc_log(KYWC_ERROR, "Failed to open any framebuffer device");
        wlr_backend_destroy(multi);
        session_destroy(session);
        return NULL;
    }

    struct wlr_backend *libinput = libinput_backend_create(display, session);
    if (libinput) {
        wlr_multi_backend_add(multi, libinput);
    } else {
        kywc_log(KYWC_ERROR, "Failed to start libinput backend");
        wlr_backend_destroy(multi);
        session_destroy(session);
        return NULL;
    }

    if (session_ptr != NULL) {
        *session_ptr = session;
    }

    return multi;
}
