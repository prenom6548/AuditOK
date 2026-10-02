// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/util/region.h>

#include <kywc/log.h>

#include "output.h"
#include "scene/surface.h"
#include "security_p.h"
#include "server.h"

#include "kywc-security-v1-protocol.h"

enum ky_security_type {
    KY_SECURITY_TYPE_SURFACE = 0,
    KY_SECURITY_TYPE_OUTPUT,
};

struct ky_security_manager {
    struct wl_global *global;
    struct wl_list surfaces;
    struct wl_list outputs;

    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct ky_security_output {
    struct wl_list link;
    struct wl_list securities;

    struct output *output;
    struct wl_listener disable;

    uint32_t enabled_ref_count;
};

struct ky_security {
    struct wl_resource *resource;
    struct wl_list link;

    enum ky_security_type type;
    union {
        struct {
            struct wlr_surface *surface;
            struct wl_listener destroy;
        } surface;
        struct {
            struct ky_security_output *output;
        } output;
    };

    bool enabled;
};

static struct ky_security_manager *manager = NULL;

static void ky_security_output_update_state(struct ky_security_output *output, bool enabled)
{
    bool old = output->enabled_ref_count > 0;

    if (enabled) {
        output->enabled_ref_count++;
    } else if (old) {
        output->enabled_ref_count--;
    }

    /* send enabled event if actually changed */
    bool now = output->enabled_ref_count > 0;
    if (old == now) {
        return;
    }

    struct ky_security *security;
    wl_list_for_each(security, &output->securities, link) {
        kywc_security_v1_send_enabled(security->resource, now);
    }
}

static void ky_security_destroy(struct ky_security *security)
{
    wl_resource_set_user_data(security->resource, NULL);
    wl_list_remove(&security->link);

    if (security->type == KY_SECURITY_TYPE_SURFACE) {
        wl_list_remove(&security->surface.destroy.link);
    } else {
        ky_security_output_update_state(security->output.output, false);
    }

    free(security);
}

static struct ky_security *security_surface_get(struct wlr_surface *wlr_surface)
{
    struct ky_security *security;
    wl_list_for_each(security, &manager->surfaces, link) {
        if (security->surface.surface == wlr_surface) {
            return security;
        }
    }
    return NULL;
}

static void security_handle_output_disable(struct wl_listener *listener, void *data)
{
    struct ky_security_output *output = wl_container_of(listener, output, disable);
    wl_list_remove(&output->disable.link);
    wl_list_remove(&output->link);
    /* tricks to skip enabled event */
    output->enabled_ref_count = 0;

    struct ky_security *security, *tmp;
    wl_list_for_each_safe(security, tmp, &output->securities, link) {
        kywc_security_v1_send_cancelled(security->resource);
        ky_security_destroy(security);
    }

    free(output);
}

static struct ky_security_output *security_output_get(struct output *output)
{
    struct ky_security_output *security_output;
    wl_list_for_each(security_output, &manager->outputs, link) {
        if (security_output->output == output) {
            return security_output;
        }
    }
    return NULL;
}

static struct ky_security_output *security_output_get_or_create(struct output *output)
{
    struct ky_security_output *security_output = security_output_get(output);
    if (security_output) {
        return security_output;
    }

    security_output = calloc(1, sizeof(*security_output));
    if (!security_output) {
        return NULL;
    }

    wl_list_init(&security_output->securities);
    wl_list_insert(&manager->outputs, &security_output->link);

    security_output->enabled_ref_count = 0;
    security_output->output = output;
    security_output->disable.notify = security_handle_output_disable;
    wl_signal_add(&output->events.disable, &security_output->disable);

    return security_output;
}

static void security_set_enabled(struct ky_security *security, bool enabled)
{
    if (security->enabled == enabled) {
        return;
    }

    security->enabled = enabled;

    if (security->type == KY_SECURITY_TYPE_SURFACE) {
        kywc_security_v1_send_enabled(security->resource, enabled);
    } else {
        ky_security_output_update_state(security->output.output, enabled);
    }
}

static void security_handle_enable(struct wl_client *client, struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (!security) {
        return;
    }
    security_set_enabled(security, true);
}

static void security_handle_disable(struct wl_client *client, struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (!security) {
        return;
    }
    security_set_enabled(security, false);
}

static void security_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_security_v1_interface security_impl = {
    .destroy = security_handle_destroy,
    .enable = security_handle_enable,
    .disable = security_handle_disable,
};

static void security_handle_resource_destroy(struct wl_resource *resource)
{
    struct ky_security *security = wl_resource_get_user_data(resource);
    if (security) {
        ky_security_destroy(security);
    }
}

static void security_handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct ky_security *security = wl_container_of(listener, security, surface.destroy);
    kywc_security_v1_send_cancelled(security->resource);
    ky_security_destroy(security);
}

static void manager_handle_get_security_for_surface(struct wl_client *client,
                                                    struct wl_resource *resource, uint32_t id,
                                                    struct wl_resource *surface_resource)
{
    struct ky_security *security = calloc(1, sizeof(*security));
    if (!security) {
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t version = wl_resource_get_version(resource);
    security->resource = wl_resource_create(client, &kywc_security_v1_interface, version, id);
    if (!security->resource) {
        wl_client_post_no_memory(client);
        free(security);
        return;
    }

    wl_resource_set_implementation(security->resource, &security_impl, security,
                                   security_handle_resource_destroy);

    /* check if surface already has a security object */
    struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface_resource);
    if (!wlr_surface || security_surface_get(wlr_surface)) {
        kywc_security_v1_send_failed(security->resource);
        wl_resource_set_user_data(security->resource, NULL);
        free(security);
        return;
    }

    security->type = KY_SECURITY_TYPE_SURFACE;
    wl_list_insert(&manager->surfaces, &security->link);

    security->surface.surface = wlr_surface;
    security->surface.destroy.notify = security_handle_surface_destroy;
    wl_signal_add(&wlr_surface->events.destroy, &security->surface.destroy);

    /* security is enabled default */
    security_set_enabled(security, true);
}

static void manager_handle_get_security_for_output(struct wl_client *client,
                                                   struct wl_resource *resource, uint32_t id,
                                                   struct wl_resource *output_resource)
{
    struct ky_security *security = calloc(1, sizeof(*security));
    if (!security) {
        wl_client_post_no_memory(client);
        return;
    }

    uint32_t version = wl_resource_get_version(resource);
    security->resource = wl_resource_create(client, &kywc_security_v1_interface, version, id);
    if (!security->resource) {
        wl_client_post_no_memory(client);
        free(security);
        return;
    }

    wl_resource_set_implementation(security->resource, &security_impl, security,
                                   security_handle_resource_destroy);

    struct output *output = output_from_resource(output_resource);
    if (!output) {
        kywc_security_v1_send_failed(security->resource);
        wl_resource_set_user_data(security->resource, NULL);
        free(security);
        return;
    }

    struct ky_security_output *security_output = security_output_get_or_create(output);
    if (!security_output) {
        kywc_security_v1_send_failed(security->resource);
        wl_resource_set_user_data(security->resource, NULL);
        free(security);
        return;
    }

    security->type = KY_SECURITY_TYPE_OUTPUT;
    security->output.output = security_output;
    security_set_enabled(security, true);

    wl_list_insert(&security_output->securities, &security->link);
    kywc_security_v1_send_enabled(security->resource, true);
}

static void manager_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct kywc_security_manager_v1_interface ky_security_manager_impl = {
    .destroy = manager_handle_destroy,
    .get_security_for_surface = manager_handle_get_security_for_surface,
    .get_security_for_output = manager_handle_get_security_for_output,
};

static void ky_security_manager_bind(struct wl_client *client, void *data, uint32_t version,
                                     uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &kywc_security_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &ky_security_manager_impl, NULL, NULL);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    free(manager);
    manager = NULL;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->display_destroy.link);
    wl_global_destroy(manager->global);
}

bool ky_security_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(server->display, &kywc_security_manager_v1_interface, 1,
                                       NULL, ky_security_manager_bind);
    if (!manager->global) {
        kywc_log(KYWC_WARN, "kywc security manager create failed");
        free(manager);
        manager = NULL;
        return false;
    }

    wl_list_init(&manager->surfaces);
    wl_list_init(&manager->outputs);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &manager->display_destroy);

    return true;
}

bool security_check_node(struct scene_node *node)
{
    assert(node->type == SCENE_NODE_BUFFER);
    if (!manager || wl_list_empty(&manager->surfaces)) {
        return false;
    }

    struct wlr_surface *surface = wlr_surface_try_from_node(node);
    if (!surface) {
        return false;
    }

    struct ky_security *security = security_surface_get(surface);
    if (!security) {
        return false;
    }

    bool has_security = security->enabled;
    if (has_security) {
        kywc_security_v1_send_touched(security->resource, "thumbnail");
    }

    if (wl_list_empty(&manager->outputs)) {
        return has_security;
    }

    /* check surface current outputs */
    struct wlr_surface_output *surface_output;
    wl_list_for_each(surface_output, &surface->current_outputs, link) {
        struct ky_security_output *security_output =
            security_output_get(output_from_wlr_output(surface_output->output));
        if (security_output && security_output->enabled_ref_count > 0) {
            struct ky_security *security;
            wl_list_for_each(security, &security_output->securities, link) {
                kywc_security_v1_send_touched(security->resource, "thumbnail");
            }
            has_security = true;
        }
    }

    return has_security;
}

/* clip is in global layout region */
bool security_check_output(struct output *output, pixman_region32_t *clip)
{
    if (!manager) {
        return false;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);
    pixman_region32_copy(&region, clip);

    bool has_security = false;
    /* check output first */
    struct ky_security_output *security_output = security_output_get(output);
    if (security_output && security_output->enabled_ref_count > 0) {
        struct ky_security *security;
        wl_list_for_each(security, &security_output->securities, link) {
            kywc_security_v1_send_touched(security->resource, "capture");
        }
        pixman_region32_clear(clip);
        has_security = true;
    }

    /* check surface visible region */
    if (wl_list_empty(&manager->surfaces)) {
        pixman_region32_fini(&region);
        return has_security;
    }

    pixman_region32_t check;
    pixman_region32_init(&check);
    pixman_region32_t visible;
    pixman_region32_init(&visible);

    float scale = output->wlr_output->scale;
    bool need_expand = floor(scale) != scale;

    struct scene_buffer *buffer;
    struct ky_security *security;
    wl_list_for_each(security, &manager->surfaces, link) {
        if (!security->enabled) {
            continue;
        }
        buffer = scene_buffer_try_from_surface(security->surface.surface);
        if (!buffer) {
            continue;
        }
        /* skip if buffer is not enabled */
        if (!scene_node_is_enabled(&buffer->node)) {
            continue;
        }

        wlr_region_expand(&visible, &buffer->node.visible_region, need_expand ? 1 : 0);
        pixman_region32_intersect(&check, &region, &visible);

        if (pixman_region32_not_empty(&check)) {
            if (pixman_region32_not_empty(clip)) {
                pixman_region32_subtract(clip, clip, &visible);
            }
            has_security = true;
            kywc_security_v1_send_touched(security->resource, "capture");
        }

        pixman_region32_clear(&check);
    }

    pixman_region32_fini(&check);
    pixman_region32_fini(&visible);
    pixman_region32_fini(&region);

    return has_security;
}
