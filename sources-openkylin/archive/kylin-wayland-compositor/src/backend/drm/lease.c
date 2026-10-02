// SPDX-FileCopyrightText: 2025 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <kywc/log.h>

#include "drm-lease-v1-protocol.h"
#include "drm_p.h"
#include "util/wayland.h"

#define DRM_LEASE_DEVICE_V1_VERSION 1

struct drm_lease_device_v1 {
    struct wl_list resources;
    struct wl_global *global;

    struct drm_backend *backend;

    struct wl_list connectors; // drm_lease_connector_v1.link
    struct wl_list leases;     // drm_lease_v1.link
    struct wl_list requests;   // drm_lease_request_v1.link

    void *data;

    struct wl_listener new_output;
    struct wl_listener backend_destroy;
};

struct drm_lease_v1;

struct drm_lease_connector_v1 {
    struct wl_list resources; // wl_resource_get_link()

    struct wlr_output *output;
    struct drm_lease_device_v1 *device;
    /** NULL if no client is currently leasing this connector */
    struct drm_lease_v1 *active_lease;

    struct wl_list link; // drm_lease_device_v1.connectors

    struct wl_listener destroy;
};

struct drm_lease_request_v1 {
    struct wl_resource *resource;

    struct drm_lease_device_v1 *device;

    struct drm_lease_connector_v1 **connectors;
    size_t n_connectors;

    struct wl_resource *lease_resource;

    bool invalid;

    struct wl_list link; // drm_lease_device_v1.requests
};

struct drm_lease_v1 {
    struct wl_resource *resource;

    struct drm_lease *drm_lease;

    struct drm_lease_device_v1 *device;

    struct drm_lease_connector_v1 **connectors;
    size_t n_connectors;

    struct wl_list link; // drm_lease_device_v1.leases

    void *data;

    struct wl_listener destroy;
};

static struct wp_drm_lease_device_v1_interface lease_device_impl;
static struct wp_drm_lease_connector_v1_interface lease_connector_impl;
static struct wp_drm_lease_request_v1_interface lease_request_impl;
static struct wp_drm_lease_v1_interface lease_impl;

static void drm_lease_request_v1_reject(struct drm_lease_request_v1 *request)
{
    assert(request);
    kywc_log(KYWC_DEBUG, "Rejecting request %p", request);

    request->invalid = true;
    wp_drm_lease_v1_send_finished(request->lease_resource);
}
/*
static void drm_lease_v1_revoke(struct drm_lease_v1 *lease)
{
    assert(lease);
    kywc_log(KYWC_DEBUG, "Revoking lease %" PRIu32, lease->drm_lease->lessee_id);
    drm_lease_terminate(lease->drm_lease);
}
*/

static struct drm_lease_device_v1 *drm_lease_device_v1_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &wp_drm_lease_device_v1_interface, &lease_device_impl));
    return wl_resource_get_user_data(resource);
}

static struct drm_lease_connector_v1 *
drm_lease_connector_v1_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wp_drm_lease_connector_v1_interface,
                                   &lease_connector_impl));
    return wl_resource_get_user_data(resource);
}

static struct drm_lease_request_v1 *drm_lease_request_v1_from_resource(struct wl_resource *resource)
{
    assert(
        wl_resource_instance_of(resource, &wp_drm_lease_request_v1_interface, &lease_request_impl));
    return wl_resource_get_user_data(resource);
}

static void drm_connector_v1_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static struct drm_lease_v1 *drm_lease_v1_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wp_drm_lease_v1_interface, &lease_impl));
    return wl_resource_get_user_data(resource);
}

static void drm_lease_request_v1_destroy(struct drm_lease_request_v1 *request)
{
    if (!request) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Destroying request %p", request);

    wl_list_remove(&request->link);
    wl_resource_set_user_data(request->resource, NULL);

    free(request->connectors);
    free(request);
}

static void drm_lease_request_v1_handle_resource_destroy(struct wl_resource *resource)
{
    struct drm_lease_request_v1 *request = drm_lease_request_v1_from_resource(resource);
    drm_lease_request_v1_destroy(request);
}

static void lease_handle_destroy(struct wl_listener *listener, void *data)
{
    struct drm_lease_v1 *lease = wl_container_of(listener, lease, destroy);

    kywc_log(KYWC_DEBUG, "Destroying lease %" PRIu32, lease->drm_lease->lessee_id);

    wp_drm_lease_v1_send_finished(lease->resource);

    for (size_t i = 0; i < lease->n_connectors; ++i) {
        lease->connectors[i]->active_lease = NULL;
    }

    wl_list_remove(&lease->destroy.link);
    wl_list_remove(&lease->link);
    wl_resource_set_user_data(lease->resource, NULL);

    free(lease->connectors);
    free(lease);
}

static void drm_lease_connector_v1_destroy(struct drm_lease_connector_v1 *connector)
{
    if (!connector) {
        return;
    }
    kywc_log(KYWC_DEBUG, "Destroying connector %s", connector->output->name);

    if (connector->active_lease) {
        drm_lease_terminate(connector->active_lease->drm_lease);
    }

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &connector->resources) {
        wp_drm_lease_connector_v1_send_withdrawn(resource);
        wl_resource_set_user_data(resource, NULL);
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
    }

    struct wl_resource *device_resource;
    wl_resource_for_each(device_resource, &connector->device->resources) {
        wp_drm_lease_device_v1_send_done(device_resource);
    }

    wl_list_remove(&connector->link);
    wl_list_remove(&connector->destroy.link);
    free(connector);
}

static void drm_connector_v1_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct wp_drm_lease_connector_v1_interface lease_connector_impl = {
    .destroy = drm_connector_v1_handle_destroy,
};

static void drm_lease_connector_v1_send_to_client(struct drm_lease_connector_v1 *connector,
                                                  struct wl_resource *resource)
{
    if (connector->active_lease) {
        return;
    }

    struct wl_client *client = wl_resource_get_client(resource);

    uint32_t version = wl_resource_get_version(resource);
    struct wl_resource *connector_resource =
        wl_resource_create(client, &wp_drm_lease_connector_v1_interface, version, 0);
    if (!connector_resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(connector_resource, &lease_connector_impl, connector,
                                   drm_connector_v1_handle_resource_destroy);
    wp_drm_lease_device_v1_send_connector(resource, connector_resource);

    struct wlr_output *output = connector->output;
    wp_drm_lease_connector_v1_send_name(connector_resource, output->name);
    // TODO: re-send the description when it's updated
    wp_drm_lease_connector_v1_send_description(connector_resource, output->description);
    wp_drm_lease_connector_v1_send_connector_id(connector_resource,
                                                drm_connector_from_output(output)->id);
    wp_drm_lease_connector_v1_send_done(connector_resource);

    wl_list_insert(&connector->resources, wl_resource_get_link(connector_resource));
}

static void drm_lease_request_v1_handle_request_connector(struct wl_client *client,
                                                          struct wl_resource *request_resource,
                                                          struct wl_resource *connector_resource)
{
    struct drm_lease_request_v1 *request = drm_lease_request_v1_from_resource(request_resource);
    if (!request) {
        kywc_log(KYWC_ERROR, "Request has been destroyed");
        return;
    }

    struct drm_lease_connector_v1 *connector =
        drm_lease_connector_v1_from_resource(connector_resource);

    if (!connector) {
        /* This connector offer has been withdrawn or is leased */
        kywc_log(KYWC_ERROR, "Failed to request connector");
        request->invalid = true;
        return;
    }

    kywc_log(KYWC_DEBUG, "Requesting connector %s", connector->output->name);

    if (request->device != connector->device) {
        kywc_log(KYWC_ERROR, "The connector belongs to another device");
        wl_resource_post_error(request_resource, WP_DRM_LEASE_REQUEST_V1_ERROR_WRONG_DEVICE,
                               "The requested connector belongs to another device");
        return;
    }

    for (size_t i = 0; i < request->n_connectors; ++i) {
        struct drm_lease_connector_v1 *tmp = request->connectors[i];
        if (connector == tmp) {
            kywc_log(KYWC_ERROR, "The connector has already been requested");
            wl_resource_post_error(request_resource,
                                   WP_DRM_LEASE_REQUEST_V1_ERROR_DUPLICATE_CONNECTOR,
                                   "The connector has already been requested");
            return;
        }
    }

    size_t n_connectors = request->n_connectors + 1;

    struct drm_lease_connector_v1 **tmp_connectors =
        realloc(request->connectors, n_connectors * sizeof(struct drm_lease_connector_v1 *));
    if (!tmp_connectors) {
        kywc_log(KYWC_ERROR, "Failed to grow connectors request array");
        return;
    }

    request->connectors = tmp_connectors;
    request->connectors[request->n_connectors] = connector;
    request->n_connectors = n_connectors;
}

static void drm_lease_v1_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct wp_drm_lease_v1_interface lease_impl = {
    .destroy = drm_lease_v1_handle_destroy,
};

static void drm_lease_v1_handle_resource_destroy(struct wl_resource *resource)
{
    struct drm_lease_v1 *lease = drm_lease_v1_from_resource(resource);
    if (lease != NULL) {
        drm_lease_terminate(lease->drm_lease);
    }
}

static struct drm_lease_v1 *grant_drm_lease_request_v1(struct drm_lease_request_v1 *request)
{
    assert(!request->invalid);
    kywc_log(KYWC_DEBUG, "Attempting to grant request %p", request);

    struct drm_lease_v1 *lease = calloc(1, sizeof(*lease));
    if (!lease) {
        wl_resource_post_no_memory(request->resource);
        return NULL;
    }

    lease->device = request->device;
    lease->resource = request->lease_resource;

    /* Transform connectors list into wlr_output for leasing */
    struct wlr_output *outputs[request->n_connectors + 1];
    for (size_t i = 0; i < request->n_connectors; ++i) {
        outputs[i] = request->connectors[i]->output;
    }

    int fd;
    lease->drm_lease = drm_create_lease(outputs, request->n_connectors, &fd);
    if (!lease->drm_lease) {
        kywc_log(KYWC_ERROR, "Failed with drm_create_lease");
        wp_drm_lease_v1_send_finished(lease->resource);
        free(lease);
        return NULL;
    }

    lease->connectors = calloc(request->n_connectors, sizeof(struct drm_lease_connector_v1 *));
    if (!lease->connectors) {
        kywc_log(KYWC_ERROR, "Failed to allocate lease connectors list");
        close(fd);
        wp_drm_lease_v1_send_finished(lease->resource);
        free(lease);
        return NULL;
    }

    lease->n_connectors = request->n_connectors;
    for (size_t i = 0; i < request->n_connectors; ++i) {
        lease->connectors[i] = request->connectors[i];
        lease->connectors[i]->active_lease = lease;
    }

    lease->destroy.notify = lease_handle_destroy;
    wl_signal_add(&lease->drm_lease->events.destroy, &lease->destroy);

    wl_list_insert(&lease->device->leases, &lease->link);
    wl_resource_set_user_data(lease->resource, lease);
    kywc_log(KYWC_DEBUG, "Granting request %p", request);
    wp_drm_lease_v1_send_lease_fd(lease->resource, fd);
    close(fd);

    return lease;
}

static void drm_lease_request_v1_handle_submit(struct wl_client *client,
                                               struct wl_resource *resource, uint32_t id)
{
    uint32_t version = wl_resource_get_version(resource);
    struct wl_resource *lease_resource =
        wl_resource_create(client, &wp_drm_lease_v1_interface, version, id);
    if (!lease_resource) {
        kywc_log(KYWC_ERROR, "Failed to allocate wl_resource");
        wl_resource_post_no_memory(resource);
        return;
    }

    wl_resource_set_implementation(lease_resource, &lease_impl, NULL,
                                   drm_lease_v1_handle_resource_destroy);

    struct drm_lease_request_v1 *request = drm_lease_request_v1_from_resource(resource);
    if (!request) {
        kywc_log(KYWC_DEBUG, "Request has been destroyed");
        wp_drm_lease_v1_send_finished(lease_resource);
        return;
    }

    /* Pre-emptively reject invalid lease requests */
    if (request->invalid) {
        kywc_log(KYWC_ERROR, "Invalid request");
        wp_drm_lease_v1_send_finished(lease_resource);
        return;
    } else if (request->n_connectors == 0) {
        wl_resource_post_error(lease_resource, WP_DRM_LEASE_REQUEST_V1_ERROR_EMPTY_LEASE,
                               "Lease request has no connectors");
        return;
    }

    for (size_t i = 0; i < request->n_connectors; ++i) {
        struct drm_lease_connector_v1 *conn = request->connectors[i];
        if (conn->active_lease) {
            kywc_log(KYWC_ERROR,
                     "Failed to create lease, connector %s has "
                     "already been leased",
                     conn->output->name);
            wp_drm_lease_v1_send_finished(lease_resource);
            return;
        }
    }

    request->lease_resource = lease_resource;

    grant_drm_lease_request_v1(request);

    /* If the compositor didn't act upon the request, reject it */
    if (!request->invalid && wl_resource_get_user_data(lease_resource) == NULL) {
        drm_lease_request_v1_reject(request);
    }

    /* Request is done */
    wl_resource_destroy(resource);
}

static struct wp_drm_lease_request_v1_interface lease_request_impl = {
    .request_connector = drm_lease_request_v1_handle_request_connector,
    .submit = drm_lease_request_v1_handle_submit,
};

static void drm_lease_device_v1_handle_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void drm_lease_device_v1_handle_release(struct wl_client *client,
                                               struct wl_resource *resource)
{
    wp_drm_lease_device_v1_send_released(resource);
    wl_resource_destroy(resource);
}

static void drm_lease_device_v1_handle_create_lease_request(struct wl_client *client,
                                                            struct wl_resource *resource,
                                                            uint32_t id)
{
    uint32_t version = wl_resource_get_version(resource);
    struct wl_resource *request_resource =
        wl_resource_create(client, &wp_drm_lease_request_v1_interface, version, id);
    if (!request_resource) {
        kywc_log(KYWC_ERROR, "Failed to allocate wl_resource");
        return;
    }

    wl_resource_set_implementation(request_resource, &lease_request_impl, NULL,
                                   drm_lease_request_v1_handle_resource_destroy);

    struct drm_lease_device_v1 *device = drm_lease_device_v1_from_resource(resource);
    if (!device) {
        kywc_log(KYWC_DEBUG, "Failed to create lease request, "
                             "drm_lease_device_v1 has been destroyed");
        return;
    }

    struct drm_lease_request_v1 *req = calloc(1, sizeof(*req));
    if (!req) {
        kywc_log(KYWC_ERROR, "Failed to allocate wlr_drm_lease_request_v1");
        wl_resource_post_no_memory(resource);
        return;
    }

    kywc_log(KYWC_DEBUG, "Created request %p", req);

    req->device = device;
    req->resource = request_resource;
    req->connectors = NULL;
    req->n_connectors = 0;

    wl_resource_set_user_data(request_resource, req);
    wl_list_insert(&device->requests, &req->link);
}

static struct wp_drm_lease_device_v1_interface lease_device_impl = {
    .release = drm_lease_device_v1_handle_release,
    .create_lease_request = drm_lease_device_v1_handle_create_lease_request,
};

static void lease_device_bind(struct wl_client *wl_client, void *data, uint32_t version,
                              uint32_t id)
{
    struct wl_resource *device_resource =
        wl_resource_create(wl_client, &wp_drm_lease_device_v1_interface, version, id);
    if (!device_resource) {
        wl_client_post_no_memory(wl_client);
        return;
    }

    wl_list_init(wl_resource_get_link(device_resource));
    wl_resource_set_implementation(device_resource, &lease_device_impl, NULL,
                                   drm_lease_device_v1_handle_resource_destroy);

    struct drm_lease_device_v1 *device = data;
    if (!device) {
        kywc_log(KYWC_DEBUG, "Failed to bind lease device, "
                             "the drm_lease_device_v1 has been destroyed");
        return;
    }

    int fd = drm_backend_get_non_master_fd(&device->backend->wlr_backend);
    if (fd < 0) {
        kywc_log(KYWC_ERROR, "Unable to get read only DRM fd for leasing");
        return;
    }

    wp_drm_lease_device_v1_send_drm_fd(device_resource, fd);
    close(fd);

    wl_resource_set_user_data(device_resource, device);
    wl_list_insert(&device->resources, wl_resource_get_link(device_resource));

    struct drm_lease_connector_v1 *connector;
    wl_list_for_each(connector, &device->connectors, link) {
        drm_lease_connector_v1_send_to_client(connector, device_resource);
    }

    wp_drm_lease_device_v1_send_done(device_resource);
}

static void drm_lease_device_v1_destroy(struct drm_lease_device_v1 *device)
{
    if (!device) {
        return;
    }

    struct drm_backend *backend = device->backend;
    kywc_log(KYWC_DEBUG, "Destroying wlr_drm_lease_device_v1 for %s", backend->drm->name);

    struct wl_resource *resource, *tmp_resource;
    wl_resource_for_each_safe(resource, tmp_resource, &device->resources) {
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
        wl_resource_set_user_data(resource, NULL);
    }

    struct drm_lease_request_v1 *request, *tmp_request;
    wl_list_for_each_safe(request, tmp_request, &device->requests, link) {
        drm_lease_request_v1_destroy(request);
    }

    struct drm_lease_v1 *lease, *tmp_lease;
    wl_list_for_each_safe(lease, tmp_lease, &device->leases, link) {
        drm_lease_terminate(lease->drm_lease);
    }

    struct drm_lease_connector_v1 *connector, *tmp_connector;
    wl_list_for_each_safe(connector, tmp_connector, &device->connectors, link) {
        drm_lease_connector_v1_destroy(connector);
    }

    wl_list_remove(&device->backend_destroy.link);
    wl_list_remove(&device->new_output.link);
    wl_global_destroy_safe(device->global);

    free(device);
}

static void handle_backend_destroy(struct wl_listener *listener, void *data)
{
    struct drm_lease_device_v1 *device = wl_container_of(listener, device, backend_destroy);
    drm_lease_device_v1_destroy(device);
}

static void handle_output_destroy(struct wl_listener *listener, void *data)
{
    struct drm_lease_connector_v1 *conn = wl_container_of(listener, conn, destroy);
    if (!conn->active_lease) {
        wl_list_remove(&conn->destroy.link);
        wl_list_init(&conn->destroy.link);
        return;
    }

    kywc_log(KYWC_DEBUG, "Withdrawing output %s", conn->output->name);

    drm_lease_connector_v1_destroy(conn);
}

static void handle_new_output(struct wl_listener *listener, void *data)
{
    struct drm_lease_device_v1 *device = wl_container_of(listener, device, new_output);
    struct wlr_output *output = data;

    if (!output->non_desktop) {
        return;
    }

    kywc_log(KYWC_DEBUG, "Drm lease get offering output %s", output->name);

    struct drm_lease_connector_v1 *tmp_connector;
    wl_list_for_each(tmp_connector, &device->connectors, link) {
        if (tmp_connector->output == output) {
            kywc_log(KYWC_ERROR, "Output %s has already been offered", output->name);

            wl_list_remove(&tmp_connector->destroy.link);
            wl_signal_add(&output->events.destroy, &tmp_connector->destroy);
            return;
        }
    }

    struct drm_lease_connector_v1 *connector = calloc(1, sizeof(*connector));
    if (!connector) {
        kywc_log(KYWC_ERROR, "Failed to allocate wlr_drm_lease_connector_v1");
        return;
    }

    connector->output = output;
    connector->device = device;

    connector->destroy.notify = handle_output_destroy;
    wl_signal_add(&output->events.destroy, &connector->destroy);

    wl_list_init(&connector->resources);
    wl_list_insert(&device->connectors, &connector->link);

    struct wl_resource *resource;
    wl_resource_for_each(resource, &device->resources) {
        drm_lease_connector_v1_send_to_client(connector, resource);
        wp_drm_lease_device_v1_send_done(resource);
    }
}

bool drm_lease_device_v1_create(struct drm_backend *drm_backend)
{
    // Make sure we can get a non-master FD for the DRM backend. On some setups
    // we don't have the permission for this.
    struct drm_device *drm = drm_backend->drm;
    int fd = drm_backend_get_non_master_fd(&drm_backend->wlr_backend);
    if (fd < 0) {
        kywc_log(KYWC_INFO, "Skipping %s: failed to get read-only DRM FD", drm->name);
        return false;
    }
    close(fd);

    kywc_log(KYWC_DEBUG, "Creating drm_lease_device_v1 for %s", drm->name);

    struct drm_lease_device_v1 *lease_device = calloc(1, sizeof(*lease_device));
    if (!lease_device) {
        kywc_log(KYWC_ERROR, "Failed to allocate drm_lease_device_v1");
        return false;
    }

    lease_device->backend = drm_backend;

    wl_list_init(&lease_device->resources);
    wl_list_init(&lease_device->connectors);
    wl_list_init(&lease_device->requests);
    wl_list_init(&lease_device->leases);

    lease_device->global =
        wl_global_create(drm->display, &wp_drm_lease_device_v1_interface,
                         DRM_LEASE_DEVICE_V1_VERSION, lease_device, lease_device_bind);

    if (!lease_device->global) {
        kywc_log(KYWC_ERROR, "Failed to allocate drm_lease_device_v1 global");
        free(lease_device);
        return false;
    }

    struct wlr_backend *backend = drm->wlr_backend;
    lease_device->backend_destroy.notify = handle_backend_destroy;
    wl_signal_add(&backend->events.destroy, &lease_device->backend_destroy);

    lease_device->new_output.notify = handle_new_output;
    wl_signal_add(&backend->events.new_output, &lease_device->new_output);

    return true;
}
