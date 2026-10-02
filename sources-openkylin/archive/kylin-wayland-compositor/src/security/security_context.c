// SPDX-FileCopyrightText: 2023 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>

#include <kywc/log.h>

#include "security-context-v1-protocol.h"
#include "security_p.h"

struct security_context_client {
    struct security_context_state state;
    struct wl_listener destroy;
};

struct security_context {
    struct wl_list link;

    struct security_context_state state;
    struct wl_event_source *listen_source, *close_source;

    int listen_fd, close_fd;
};

struct security_context_manager {
    struct wl_global *global;
    struct wl_listener display_destroy;

    struct wl_list contexts;
};

static struct security_context_manager *manager = NULL;

static const struct wp_security_context_v1_interface security_context_impl;

static void resource_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct security_context *security_context_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource, &wp_security_context_v1_interface,
                                   &security_context_impl));
    return wl_resource_get_user_data(resource);
}

static void security_context_state_finish(struct security_context_state *state)
{
    free(state->app_id);
    free(state->sandbox_engine);
    free(state->instance_id);
}

static bool copy_state_field(char **dst, const char *src)
{
    if (!src) {
        return true;
    }
    *dst = strdup(src);
    return *dst != NULL;
}

static bool security_context_state_copy(struct security_context_state *dst,
                                        const struct security_context_state *src)
{
    bool ok = copy_state_field(&dst->app_id, src->app_id) &&
              copy_state_field(&dst->sandbox_engine, src->sandbox_engine) &&
              copy_state_field(&dst->instance_id, src->instance_id);
    if (!ok) {
        security_context_state_finish(dst);
    }
    return ok;
}

static void security_context_destroy(struct security_context *security_context)
{
    if (!security_context) {
        return;
    }

    if (security_context->listen_source) {
        wl_event_source_remove(security_context->listen_source);
    }
    if (security_context->close_source) {
        wl_event_source_remove(security_context->close_source);
    }

    close(security_context->listen_fd);
    close(security_context->close_fd);

    security_context_state_finish(&security_context->state);
    wl_list_remove(&security_context->link);
    free(security_context);
}

static void security_context_client_destroy(struct security_context_client *security_context_client)
{
    wl_list_remove(&security_context_client->destroy.link);
    security_context_state_finish(&security_context_client->state);
    free(security_context_client);
}

static void security_context_client_handle_destroy(struct wl_listener *listener, void *data)
{
    struct security_context_client *security_context_client =
        wl_container_of(listener, security_context_client, destroy);
    security_context_client_destroy(security_context_client);
}

const struct security_context_state *
security_context_manager_lookup_client(const struct wl_client *client)
{
    if (!manager) {
        return NULL;
    }

    struct wl_listener *listener = wl_client_get_destroy_listener(
        (struct wl_client *)client, security_context_client_handle_destroy);
    if (!listener) {
        return NULL;
    }

    struct security_context_client *security_context_client =
        wl_container_of(listener, security_context_client, destroy);
    return &security_context_client->state;
}

static int security_context_handle_listen_fd_event(int listen_fd, uint32_t mask, void *data)
{
    struct security_context *security_context = data;

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
        security_context_destroy(security_context);
        return 0;
    }

    if (mask & WL_EVENT_READABLE) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            kywc_log_errno(KYWC_ERROR, "accept failed");
            return 0;
        }

        struct security_context_client *security_context_client =
            calloc(1, sizeof(*security_context_client));
        if (!security_context_client) {
            kywc_log_errno(KYWC_ERROR, "Allocation failed");
            close(client_fd);
            return 0;
        }

        struct wl_display *display = wl_global_get_display(manager->global);
        struct wl_client *client = wl_client_create(display, client_fd);
        if (!client) {
            kywc_log(KYWC_ERROR, "wl_client_create failed");
            close(client_fd);
            free(security_context_client);
            return 0;
        }

        security_context_client->destroy.notify = security_context_client_handle_destroy;
        wl_client_add_destroy_listener(client, &security_context_client->destroy);

        if (!security_context_state_copy(&security_context_client->state,
                                         &security_context->state)) {
            security_context_client_destroy(security_context_client);
            wl_client_post_no_memory(client);
            return 0;
        }
    }

    return 0;
}

static int security_context_handle_close_fd_event(int fd, uint32_t mask, void *data)
{
    struct security_context *security_context = data;

    if (mask & (WL_EVENT_HANGUP | WL_EVENT_ERROR)) {
        security_context_destroy(security_context);
    }

    return 0;
}

static void security_context_handle_commit(struct wl_client *client, struct wl_resource *resource)
{
    struct security_context *security_context = security_context_from_resource(resource);
    if (!security_context) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_USED,
                               "Security context has already been committed");
        return;
    }

    // In theory the compositor should prevent this with a global filter, but
    // let's make sure it doesn't happen.
    if (security_context_manager_lookup_client(client)) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_MANAGER_V1_ERROR_NESTED,
                               "Nested security contexts are forbidden");
        return;
    }

    struct wl_display *display = wl_client_get_display(client);
    struct wl_event_loop *loop = wl_display_get_event_loop(display);

    security_context->listen_source =
        wl_event_loop_add_fd(loop, security_context->listen_fd, WL_EVENT_READABLE,
                             security_context_handle_listen_fd_event, security_context);
    if (security_context->listen_source == NULL) {
        wl_resource_post_no_memory(resource);
        return;
    }

    security_context->close_source =
        wl_event_loop_add_fd(loop, security_context->close_fd, 0,
                             security_context_handle_close_fd_event, security_context);
    if (security_context->close_source == NULL) {
        wl_resource_post_no_memory(resource);
        wl_event_source_remove(security_context->listen_source);
        security_context->listen_source = NULL;
        return;
    }

    wl_resource_set_user_data(resource, NULL);
}

static void security_context_handle_set_sandbox_engine(struct wl_client *client,
                                                       struct wl_resource *resource,
                                                       const char *sandbox_engine)
{
    struct security_context *security_context = security_context_from_resource(resource);
    if (!security_context) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_USED,
                               "Security context has already been committed");
        return;
    }

    if (security_context->state.sandbox_engine) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_SET,
                               "Sandbox engine has already been set");
        return;
    }

    security_context->state.sandbox_engine = strdup(sandbox_engine);
    if (!security_context->state.sandbox_engine) {
        wl_resource_post_no_memory(resource);
    }
}

static void security_context_handle_set_app_id(struct wl_client *client,
                                               struct wl_resource *resource, const char *app_id)
{
    struct security_context *security_context = security_context_from_resource(resource);
    if (!security_context) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_USED,
                               "Security context has already been committed");
        return;
    }

    if (security_context->state.app_id) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_SET,
                               "App ID has already been set");
        return;
    }

    security_context->state.app_id = strdup(app_id);
    if (!security_context->state.app_id) {
        wl_resource_post_no_memory(resource);
    }
}

static void security_context_handle_set_instance_id(struct wl_client *client,
                                                    struct wl_resource *resource,
                                                    const char *instance_id)
{
    struct security_context *security_context = security_context_from_resource(resource);
    if (!security_context) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_USED,
                               "Security context has already been committed");
        return;
    }

    if (security_context->state.instance_id) {
        wl_resource_post_error(resource, WP_SECURITY_CONTEXT_V1_ERROR_ALREADY_SET,
                               "Instance ID has already been set");
        return;
    }

    security_context->state.instance_id = strdup(instance_id);
    if (!security_context->state.instance_id) {
        wl_resource_post_no_memory(resource);
    }
}

static const struct wp_security_context_v1_interface security_context_impl = {
    .destroy = resource_handle_destroy,
    .commit = security_context_handle_commit,
    .set_sandbox_engine = security_context_handle_set_sandbox_engine,
    .set_app_id = security_context_handle_set_app_id,
    .set_instance_id = security_context_handle_set_instance_id,
};

static void security_context_resource_destroy(struct wl_resource *resource)
{
    struct security_context *security_context = security_context_from_resource(resource);
    security_context_destroy(security_context);
}

static void manager_handle_create_listener(struct wl_client *client,
                                           struct wl_resource *manager_resource, uint32_t id,
                                           int listen_fd, int close_fd)
{
    struct stat stat_buf = { 0 };
    if (fstat(listen_fd, &stat_buf) != 0) {
        kywc_log_errno(KYWC_ERROR, "fstat failed on listen FD");
        wl_resource_post_error(manager_resource,
                               WP_SECURITY_CONTEXT_MANAGER_V1_ERROR_INVALID_LISTEN_FD,
                               "Invalid listen_fd");
        goto error;
    } else if (!S_ISSOCK(stat_buf.st_mode)) {
        wl_resource_post_error(manager_resource,
                               WP_SECURITY_CONTEXT_MANAGER_V1_ERROR_INVALID_LISTEN_FD,
                               "listen_fd is not a socket");
        goto error;
    }

    int accept_conn = 0;
    socklen_t accept_conn_size = sizeof(accept_conn);
    if (getsockopt(listen_fd, SOL_SOCKET, SO_ACCEPTCONN, &accept_conn, &accept_conn_size) != 0) {
        kywc_log_errno(KYWC_ERROR, "getsockopt failed on listen FD");
        wl_resource_post_error(manager_resource,
                               WP_SECURITY_CONTEXT_MANAGER_V1_ERROR_INVALID_LISTEN_FD,
                               "Invalid listen_fd");
        goto error;
    } else if (accept_conn == 0) {
        wl_resource_post_error(manager_resource,
                               WP_SECURITY_CONTEXT_MANAGER_V1_ERROR_INVALID_LISTEN_FD,
                               "listen_fd is not a listening socket");
        goto error;
    }

    struct security_context *security_context = calloc(1, sizeof(*security_context));
    if (!security_context) {
        wl_resource_post_no_memory(manager_resource);
        goto error;
    }

    security_context->listen_fd = listen_fd;
    security_context->close_fd = close_fd;

    uint32_t version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &wp_security_context_v1_interface, version, id);
    if (!resource) {
        free(security_context);
        wl_resource_post_no_memory(manager_resource);
        goto error;
    }

    wl_resource_set_implementation(resource, &security_context_impl, security_context,
                                   security_context_resource_destroy);
    wl_list_insert(&manager->contexts, &security_context->link);
    return;

error:
    close(listen_fd);
    close(close_fd);
}

static const struct wp_security_context_manager_v1_interface manager_impl = {
    .destroy = resource_handle_destroy,
    .create_listener = manager_handle_create_listener,
};

static void manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource =
        wl_resource_create(client, &wp_security_context_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct security_context *context, *tmp;
    wl_list_for_each_safe(context, tmp, &manager->contexts, link) {
        security_context_destroy(context);
    }

    security_remove_global_filter(manager->global);
    wl_global_destroy(manager->global);
    wl_list_remove(&manager->display_destroy.link);
    free(manager);
    manager = NULL;
}

static bool security_context_filter_global(const struct wl_client *client, void *data)
{
    const struct security_context_state *state = security_context_manager_lookup_client(client);
    return state == NULL;
}

bool security_context_manager_create(struct wl_display *display)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->global = wl_global_create(display, &wp_security_context_manager_v1_interface, 1,
                                       manager, manager_bind);
    if (!manager->global) {
        free(manager);
        manager = NULL;
        return false;
    }

    wl_list_init(&manager->contexts);
    security_add_global_filter(manager->global, security_context_filter_global, NULL);

    manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(display, &manager->display_destroy);

    return true;
}
