// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "ext-foreign-toplevel-list-v1-protocol.h"

#include "view_p.h"

struct ext_foreign_toplevel_list {
    struct wl_global *global;
    struct wl_list resources;
    struct wl_list toplevels;

    struct wl_listener new_mapped_toplevel;
    struct wl_listener display_destroy;
    struct wl_listener server_destroy;
};

struct ext_foreign_toplevel {
    struct ext_foreign_toplevel_list *list;
    struct wl_list link;
    struct wl_list resources;

    struct kywc_view *view;
    struct wl_listener unmap;
    struct wl_listener title;
    struct wl_listener app_id;
};

static void handle_toplevel_title(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel *toplevel = wl_container_of(listener, toplevel, title);

    struct wl_resource *resource;
    wl_resource_for_each(resource, &toplevel->resources) {
        ext_foreign_toplevel_handle_v1_send_title(
            resource, toplevel->view->title ? toplevel->view->title : "");
        ext_foreign_toplevel_handle_v1_send_done(resource);
    }
}

static void handle_toplevel_app_id(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel *toplevel = wl_container_of(listener, toplevel, app_id);

    struct wl_resource *resource;
    wl_resource_for_each(resource, &toplevel->resources) {
        ext_foreign_toplevel_handle_v1_send_app_id(
            resource, toplevel->view->app_id ? toplevel->view->app_id : "");
        ext_foreign_toplevel_handle_v1_send_done(resource);
    }
}

static void handle_toplevel_unmap(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &toplevel->resources) {
        ext_foreign_toplevel_handle_v1_send_closed(resource);
        // make the resource inert
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
        wl_resource_set_user_data(resource, NULL);
    }

    wl_list_remove(&toplevel->title.link);
    wl_list_remove(&toplevel->app_id.link);
    wl_list_remove(&toplevel->unmap.link);
    wl_list_remove(&toplevel->link);
    free(toplevel);
}

static void toplevel_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ext_foreign_toplevel_handle_v1_interface toplevel_handle_impl = {
    .destroy = toplevel_handle_destroy,
};

static void toplevel_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static struct wl_resource *
create_toplevel_resource_for_resource(struct ext_foreign_toplevel *toplevel,
                                      struct wl_resource *list_resource)
{
    struct wl_client *client = wl_resource_get_client(list_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &ext_foreign_toplevel_handle_v1_interface,
                           wl_resource_get_version(list_resource), 0);
    if (!resource) {
        wl_client_post_no_memory(client);
        return NULL;
    }

    wl_resource_set_implementation(resource, &toplevel_handle_impl, toplevel,
                                   toplevel_resource_destroy);

    wl_list_insert(&toplevel->resources, wl_resource_get_link(resource));
    ext_foreign_toplevel_list_v1_send_toplevel(list_resource, resource);
    return resource;
}

static void toplevel_send_details_to_toplevel_resource(struct ext_foreign_toplevel *toplevel,
                                                       struct wl_resource *resource)
{
    struct kywc_view *kywc_view = toplevel->view;
    if (kywc_view->title) {
        ext_foreign_toplevel_handle_v1_send_title(resource, kywc_view->title);
    }
    if (kywc_view->app_id) {
        ext_foreign_toplevel_handle_v1_send_app_id(resource, kywc_view->app_id);
    }
    ext_foreign_toplevel_handle_v1_send_identifier(resource, kywc_view->uuid);
    ext_foreign_toplevel_handle_v1_send_done(resource);
}

static void handle_new_mapped_toplevel(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel *toplevel = calloc(1, sizeof(*toplevel));
    if (!toplevel) {
        return;
    }

    struct ext_foreign_toplevel_list *list = wl_container_of(listener, list, new_mapped_toplevel);
    toplevel->list = list;
    wl_list_init(&toplevel->resources);
    wl_list_insert(&list->toplevels, &toplevel->link);

    struct kywc_view *kywc_view = data;
    toplevel->view = kywc_view;

    toplevel->unmap.notify = handle_toplevel_unmap;
    wl_signal_add(&kywc_view->events.unmap, &toplevel->unmap);
    toplevel->title.notify = handle_toplevel_title;
    wl_signal_add(&kywc_view->events.title, &toplevel->title);
    toplevel->app_id.notify = handle_toplevel_app_id;
    wl_signal_add(&kywc_view->events.app_id, &toplevel->app_id);

    /* send toplevel and detail */
    struct wl_resource *resource, *toplevel_resource;
    wl_resource_for_each(resource, &toplevel->list->resources) {
        toplevel_resource = create_toplevel_resource_for_resource(toplevel, resource);
        toplevel_send_details_to_toplevel_resource(toplevel, toplevel_resource);
    }
}

static void toplevel_list_handle_stop(struct wl_client *client, struct wl_resource *resource)
{
    ext_foreign_toplevel_list_v1_send_finished(resource);
    wl_list_remove(wl_resource_get_link(resource));
    wl_list_init(wl_resource_get_link(resource));
}

static void toplevel_list_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct ext_foreign_toplevel_list_v1_interface toplevel_list_impl = {
    .stop = toplevel_list_handle_stop,
    .destroy = toplevel_list_handle_destroy,
};

static void toplevel_list_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void toplevel_list_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct ext_foreign_toplevel_list *list = data;
    struct wl_resource *resource =
        wl_resource_create(client, &ext_foreign_toplevel_list_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &toplevel_list_impl, list,
                                   toplevel_list_resource_destroy);
    wl_list_insert(&list->resources, wl_resource_get_link(resource));

    /* send all toplevels and details */
    struct ext_foreign_toplevel *toplevel;
    wl_list_for_each(toplevel, &list->toplevels, link) {
        struct wl_resource *toplevel_resource =
            create_toplevel_resource_for_resource(toplevel, resource);
        toplevel_send_details_to_toplevel_resource(toplevel, toplevel_resource);
    }
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel_list *list = wl_container_of(listener, list, server_destroy);
    wl_list_remove(&list->server_destroy.link);
    free(list);
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    struct ext_foreign_toplevel_list *list = wl_container_of(listener, list, display_destroy);
    wl_list_remove(&list->new_mapped_toplevel.link);
    wl_list_remove(&list->display_destroy.link);
    wl_global_destroy(list->global);
}

bool ext_foreign_toplevel_create(struct server *server)
{
    struct ext_foreign_toplevel_list *list = calloc(1, sizeof(*list));
    if (!list) {
        return false;
    }

    list->global = wl_global_create(server->display, &ext_foreign_toplevel_list_v1_interface, 1,
                                    list, toplevel_list_bind);
    if (!list->global) {
        kywc_log(KYWC_WARN, "Ext_foreign_toplevel_list_v1 create failed");
        free(list);
        return false;
    }

    wl_list_init(&list->resources);
    wl_list_init(&list->toplevels);

    list->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &list->server_destroy);
    list->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(server->display, &list->display_destroy);

    list->new_mapped_toplevel.notify = handle_new_mapped_toplevel;
    kywc_view_add_new_mapped_listener(&list->new_mapped_toplevel);

    return true;
}
