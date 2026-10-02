// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/types/wlr_buffer.h>

#include <kywc/log.h>

#include "renderer_p.h"
#include "single-pixel-buffer-v1-protocol.h"

#define SINGLE_PIXEL_MANAGER_VERSION 1

struct single_pixel_buffer_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;
};

struct single_pixel_buffer {
    struct wlr_buffer base;
    struct wl_resource *resource;
    struct wl_listener release;
    // Full-scale for each component is UINT32_MAX
    uint32_t r, g, b, a;
    // Packed little-endian DRM_FORMAT_ARGB8888. Used for data_ptr_access
    uint8_t argb8888[4];
};

static void handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface wl_buffer_impl = {
    .destroy = handle_destroy,
};

static bool buffer_resource_is_instance(struct wl_resource *resource)
{
    return wl_resource_instance_of(resource, &wl_buffer_interface, &wl_buffer_impl);
}

static struct single_pixel_buffer *single_pixel_buffer_from_resource(struct wl_resource *resource)
{
    assert(buffer_resource_is_instance(resource));
    return wl_resource_get_user_data(resource);
}

static struct wlr_buffer *buffer_from_resource(struct wl_resource *resource)
{
    return &single_pixel_buffer_from_resource(resource)->base;
}

static const struct wlr_buffer_resource_interface buffer_resource_interface = {
    .name = "single_pixel_buffer",
    .is_instance = buffer_resource_is_instance,
    .from_resource = buffer_from_resource,
};

static void buffer_destroy(struct wlr_buffer *wlr_buffer)
{
    struct single_pixel_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    wl_list_remove(&buffer->release.link);
    if (buffer->resource) {
        wl_resource_set_user_data(buffer->resource, NULL);
    }
    free(buffer);
}

static bool buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer, uint32_t flags, void **data,
                                         uint32_t *format, size_t *stride)
{
    struct single_pixel_buffer *buffer = wl_container_of(wlr_buffer, buffer, base);
    if (flags & ~WLR_BUFFER_DATA_PTR_ACCESS_READ) {
        return false; // the buffer is read-only
    }
    *data = &buffer->argb8888;
    *format = DRM_FORMAT_ARGB8888;
    *stride = sizeof(buffer->argb8888);
    return true;
}

static void buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer)
{
    // This space is intentionally left blank
}

static const struct wlr_buffer_impl buffer_impl = {
    .destroy = buffer_destroy,
    .begin_data_ptr_access = buffer_begin_data_ptr_access,
    .end_data_ptr_access = buffer_end_data_ptr_access,
};

static void buffer_handle_resource_destroy(struct wl_resource *resource)
{
    struct single_pixel_buffer *buffer = single_pixel_buffer_from_resource(resource);
    buffer->resource = NULL;
    wlr_buffer_drop(&buffer->base);
}

static void buffer_handle_release(struct wl_listener *listener, void *data)
{
    struct single_pixel_buffer *buffer = wl_container_of(listener, buffer, release);
    if (buffer->resource) {
        wl_buffer_send_release(buffer->resource);
    }
}

static void handle_create_u32_rgba_buffer(struct wl_client *client, struct wl_resource *resource,
                                          uint32_t id, uint32_t r, uint32_t g, uint32_t b,
                                          uint32_t a)
{
    struct single_pixel_buffer *buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        wl_client_post_no_memory(client);
        return;
    }

    buffer->resource = wl_resource_create(client, &wl_buffer_interface, 1, id);
    if (!buffer->resource) {
        wl_client_post_no_memory(client);
        free(buffer);
        return;
    }

    wlr_buffer_init(&buffer->base, &buffer_impl, 1, 1);
    wl_resource_set_implementation(buffer->resource, &wl_buffer_impl, buffer,
                                   buffer_handle_resource_destroy);

    buffer->r = r;
    buffer->g = g;
    buffer->b = b;
    buffer->a = a;

    double f = (double)0xFF / 0xFFFFFFFF;
    buffer->argb8888[0] = (uint8_t)((double)buffer->b * f);
    buffer->argb8888[1] = (uint8_t)((double)buffer->g * f);
    buffer->argb8888[2] = (uint8_t)((double)buffer->r * f);
    buffer->argb8888[3] = (uint8_t)((double)buffer->a * f);

    buffer->release.notify = buffer_handle_release;
    wl_signal_add(&buffer->base.events.release, &buffer->release);
}

static const struct wp_single_pixel_buffer_manager_v1_interface manager_impl = {
    .destroy = handle_destroy,
    .create_u32_rgba_buffer = handle_create_u32_rgba_buffer,
};

static void manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wp_single_pixel_buffer_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &manager_impl, NULL, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct single_pixel_buffer_manager *manager =
        wl_container_of(listener, manager, display_destroy);
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
    free(manager);
}

bool single_pixel_buffer_manager_create(struct wl_display *display)
{
    struct single_pixel_buffer_manager *manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(display, &wp_single_pixel_buffer_manager_v1_interface,
                                       SINGLE_PIXEL_MANAGER_VERSION, NULL, manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "Failed to create %s global",
                 wp_single_pixel_buffer_manager_v1_interface.name);
        free(manager);
        return false;
    }

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    wlr_buffer_register_resource_interface(&buffer_resource_interface);

    return manager;
}
