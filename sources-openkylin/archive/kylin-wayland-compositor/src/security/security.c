// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include <kywc/log.h>

#include "input/seat.h"
#include "security/render.h"
#include "security_p.h"
#include "server.h"
#include "view/view.h"

enum security_type {
    SECURITY_TYPE_NODE = 0,
    SECURITY_TYPE_OUTPUT,
    SECURITY_TYPE_SELECTION,
};

struct security {
    struct wl_list link;
    struct security_object *object;

    const struct security_interface *impl;
    void *user_data;

    enum security_decision decision;
};

struct security_object {
    struct wl_list link;

    /* securities with decision */
    struct wl_list securities;

    /* attached object */
    union {
        struct scene_node *node;
        struct scene_output *output;
        struct seat *seat;
        void *data;
    };
    struct wlr_addon addon;          // for node and output
    struct wl_listener seat_destroy; // for selection

    enum security_type type;
    int32_t n_locks;
    bool dropped; // true when attached object is destroyed
};

struct security_buffer_item {
    struct wl_list link;

    struct security_object *object;
    /* region in the security buffer */
    pixman_region32_t region;
};

struct security_buffer {
    struct wlr_buffer *buffer;
    struct wlr_addon buffer_addon;
    struct wl_list items;
};

static struct security_manager {
    struct wl_list node_objects;
    struct wl_list output_objects;
    struct wl_list selection_objects;
    struct wl_listener server_destroy;
} *manager = NULL;

static void security_object_consider_destroy(struct security_object *object)
{
    if (!object->dropped || object->n_locks > 0) {
        return;
    }

    struct security *security, *tmp;
    wl_list_for_each_safe(security, tmp, &object->securities, link) {
        if (security->user_data && security->impl && security->impl->destroy) {
            security->impl->destroy(security, security->user_data);
        }
        wl_list_remove(&security->link);
        free(security);
    }

    wl_list_remove(&object->link);
    free(object);
}

static struct security_object *security_object_lock(struct security_object *object)
{
    object->n_locks++;
    return object;
}

static void security_object_unlock(struct security_object *object)
{
    if (!object) {
        return;
    }

    assert(object->n_locks > 0);
    object->n_locks--;

    security_object_consider_destroy(object);
}

void security_set_user_data(struct security *security, void *data)
{
    if (security) {
        security->user_data = data;
    }
}

void *security_get_user_data(struct security *security)
{
    return security ? security->user_data : NULL;
}

void security_set_decision(struct security *security, enum security_decision decision)
{
    if (!security || security->decision == decision) {
        return;
    }

    security->decision = decision;

    // push damage to schedule the rendering
    struct security_object *object = security->object;
    if (object->type == SECURITY_TYPE_NODE) {
        scene_node_push_damage(object->node, SCENE_DAMAGE_BOTH, NULL);
    } else if (object->type == SECURITY_TYPE_OUTPUT) {
        scene_output_damage_whole(object->output);
    }
}

static void security_object_handle_destroy(struct wlr_addon *addon)
{
    struct security_object *object = wl_container_of(addon, object, addon);
    wlr_addon_finish(&object->addon);
    object->dropped = true;
    security_object_consider_destroy(object);
}

static const struct wlr_addon_interface security_object_addon_impl = {
    .name = "security_object",
    .destroy = security_object_handle_destroy,
};

static void security_object_handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct security_object *object = wl_container_of(listener, object, seat_destroy);
    wl_list_remove(&object->seat_destroy.link);
    wl_list_init(&object->seat_destroy.link);
    object->dropped = true;
    security_object_consider_destroy(object);
}

static struct security_object *security_object_get(struct wlr_addon_set *set)
{
    struct wlr_addon *addon = wlr_addon_find(set, manager, &security_object_addon_impl);
    if (!addon) {
        return NULL;
    }

    struct security_object *object = wl_container_of(addon, object, addon);
    return object;
}

static struct security_object *security_object_get_or_create(enum security_type type, void *data)
{
    struct wlr_addon_set *addons = NULL;
    struct wl_list *list;

    switch (type) {
    case SECURITY_TYPE_NODE:
        list = &manager->node_objects;
        addons = &((struct scene_node *)data)->addons;
        break;
    case SECURITY_TYPE_OUTPUT:
        list = &manager->output_objects;
        addons = &((struct scene_output *)data)->addons;
        break;
    case SECURITY_TYPE_SELECTION:
        list = &manager->selection_objects;
        break;
    default:
        return NULL;
    }

    struct security_object *object = NULL;
    if (addons) {
        object = security_object_get(addons);
    } else {
        wl_list_for_each(object, list, link) {
            if (object->seat == data) {
                return object;
            }
        }
        object = NULL;
    }
    if (object) {
        return object;
    }

    object = calloc(1, sizeof(*object));
    if (!object) {
        return NULL;
    }

    object->type = type;
    wl_list_init(&object->securities);
    object->data = data;
    wl_list_insert(list, &object->link);

    if (addons) {
        wlr_addon_init(&object->addon, addons, manager, &security_object_addon_impl);
    } else {
        object->seat_destroy.notify = security_object_handle_seat_destroy;
        wl_signal_add(&object->seat->events.destroy, &object->seat_destroy);
    }

    return object;
}

static struct security *security_create(enum security_type type, void *data,
                                        enum security_decision decision,
                                        const struct security_interface *impl, void *user_data)
{
    if (decision == SECURITY_DECISION_SOFT_ALLOW && (!impl || !impl->notify)) {
        return NULL;
    } else if (decision == SECURITY_DECISION_SOFT_DENY && (!impl || !impl->access)) {
        return NULL;
    }

    struct security_object *object = security_object_get_or_create(type, data);
    if (!object) {
        return NULL;
    }

    struct security *security = calloc(1, sizeof(*security));
    if (!security) {
        // security object may left empty
        return NULL;
    }

    security->object = object;
    wl_list_insert(&object->securities, &security->link);
    security_object_lock(object);

    security->impl = impl;
    security->user_data = user_data;
    security_set_decision(security, decision);

    return security;
}

struct security *security_create_for_node(struct scene_node *node, enum security_decision decision,
                                          const struct security_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    return security_create(SECURITY_TYPE_NODE, node, decision, impl, data);
}

struct security *security_create_for_output(struct scene_output *output,
                                            enum security_decision decision,
                                            const struct security_interface *impl, void *data)
{
    if (!manager) {
        return NULL;
    }

    return security_create(SECURITY_TYPE_OUTPUT, output, decision, impl, data);
}

struct security *security_create_for_selection(struct seat *seat, enum security_decision decision,
                                               const struct security_interface *impl, void *data)
{
    if (!manager || !seat) {
        return NULL;
    }

    return security_create(SECURITY_TYPE_SELECTION, seat, decision, impl, data);
}

void security_ref(struct security *security)
{
    if (security) {
        security_object_lock(security->object);
    }
}

void security_unref(struct security *security)
{
    if (security) {
        security_object_unlock(security->object);
    }
}

bool security_check_node(struct scene_node *node)
{
    struct security_object *object = security_object_get(&node->addons);
    return object && !wl_list_empty(&object->securities);
}

bool security_check_output(struct scene_output *output)
{
    struct security_object *object = security_object_get(&output->addons);
    return object && !wl_list_empty(&object->securities);
}

static bool security_object_access(struct security_object *object,
                                   const struct security_action_info *info)
{
    bool allowed = true;

    struct security *security;
    wl_list_for_each(security, &object->securities, link) {
        if (security->decision == SECURITY_DECISION_SOFT_DENY) {
            if (security->user_data) {
                allowed &= security->impl->access(security, info, security->user_data);
            } else {
                allowed = false;
            }
        } else if (security->decision == SECURITY_DECISION_DENY) {
            allowed = false;
        }

        if (security->user_data && security->impl && security->impl->notify) {
            security->impl->notify(security, info, security->user_data);
        }
    }

    return allowed;
}

static struct security_object *security_selection_object_get(struct seat *seat)
{
    if (!manager) {
        return NULL;
    }

    struct security_object *object;
    wl_list_for_each(object, &manager->selection_objects, link) {
        if (object->seat == seat) {
            return object;
        }
    }
    return NULL;
}

bool security_check_selection(struct seat *seat)
{
    if (!manager) {
        return false;
    }

    struct security_object *object = security_selection_object_get(seat);
    return object && !wl_list_empty(&object->securities);
}

bool security_access(const struct security_action_info *info)
{
    // get security object
    struct security_object *object = NULL;
    switch (info->action) {
    case SECURITY_ACCESS_NODE:
        object = security_object_get(&info->node.node->addons);
        break;
    case SECURITY_ACCESS_OUTPUT:
        object = security_object_get(&info->output.output->addons);
        break;
    case SECURITY_WRITE_SELECTION:
    case SECURITY_READ_SELECTION:
        object = security_selection_object_get(info->selection.seat);
        break;
    case SECURITY_START_DRAG:
    case SECURITY_DROP:
        object = security_selection_object_get(info->dnd.seat);
        break;
    }

    // return true if no security
    if (!object || wl_list_empty(&object->securities)) {
        return true;
    }

    return security_object_access(object, info);
}

static void security_buffer_item_destroy(struct security_buffer_item *item)
{
    wl_list_remove(&item->link);
    security_object_unlock(item->object);
    pixman_region32_fini(&item->region);
    free(item);
}

static void security_buffer_handle_destroy(struct wlr_addon *addon)
{
    struct security_buffer *buffer = wl_container_of(addon, buffer, buffer_addon);
    wlr_addon_finish(addon);

    struct security_buffer_item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &buffer->items, link) {
        security_buffer_item_destroy(item);
    }

    free(buffer);
}

static const struct wlr_addon_interface security_buffer_addon_impl = {
    .name = "security_buffer",
    .destroy = security_buffer_handle_destroy,
};

static struct security_buffer *security_buffer_get(struct wlr_buffer *wlr_buffer)
{
    struct wlr_addon *addon =
        wlr_addon_find(&wlr_buffer->addons, manager, &security_buffer_addon_impl);
    if (!addon) {
        return NULL;
    }

    struct security_buffer *buffer = wl_container_of(addon, buffer, buffer_addon);
    return buffer;
}

bool security_check_buffer(struct wlr_buffer *wlr_buffer)
{
    struct security_buffer *buffer = security_buffer_get(wlr_buffer);
    return buffer && !wl_list_empty(&buffer->items);
}

static void security_node_calc_visible(struct scene_node *node, int lx, int ly,
                                       pixman_region32_t *visible, bool with_visibility)
{
    if (!node->enabled) {
        return;
    }

    if (node->type == SCENE_NODE_TREE) {
        struct scene_tree *tree = scene_tree_from_node(node);
        struct scene_node *child;
        wl_list_for_each_reverse(child, &tree->children, link) {
            security_node_calc_visible(child, lx + child->x, ly + child->y, visible,
                                       with_visibility);
        }
    } else {
        pixman_region32_t visible_region;
        if (with_visibility) {
            pixman_region32_init(&visible_region);
            pixman_region32_copy(&visible_region, &node->visible_region);
        } else {
            struct wlr_box box;
            node->impl.get_bounding_box(node, &box);
            pixman_region32_init_rect(&visible_region, box.x, box.y, box.width, box.height);
            if (pixman_region32_not_empty(&node->clip_region)) {
                pixman_region32_intersect(&visible_region, &visible_region, &node->clip_region);
            }
            pixman_region32_translate(&visible_region, lx, ly);
        }
        // special for decoration
        if (node->type == SCENE_NODE_RECT) {
            struct scene_rect *rect = scene_rect_from_node(node);
            pixman_region32_intersect_rect(&visible_region, &visible_region, lx, ly, rect->width,
                                           rect->height);
        }
        pixman_region32_union(visible, visible, &visible_region);
        pixman_region32_fini(&visible_region);
    }
}

static struct security_buffer *security_buffer_get_or_create(struct wlr_buffer *wlr_buffer)
{
    struct security_buffer *buffer = security_buffer_get(wlr_buffer);
    if (buffer) {
        return buffer;
    }

    buffer = calloc(1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    buffer->buffer = wlr_buffer;
    wlr_addon_init(&buffer->buffer_addon, &wlr_buffer->addons, manager,
                   &security_buffer_addon_impl);
    wl_list_init(&buffer->items);

    return buffer;
}

void security_target_add_node(struct scene_render_target *target, struct scene_node *node, int lx,
                              int ly)
{
    struct security_object *object = security_object_get(&node->addons);
    if (!object || wl_list_empty(&object->securities)) {
        return;
    }

    struct security_buffer *buffer = security_buffer_get_or_create(target->buffer);
    if (!buffer) {
        return;
    }

    struct security_buffer_item *item = calloc(1, sizeof(*item));
    if (!item) {
        return;
    }

    item->object = object;
    wl_list_insert(&buffer->items, &item->link);
    security_object_lock(object);

    pixman_region32_init(&item->region);
    security_node_calc_visible(node, lx, ly, &item->region,
                               !(target->options & SCENE_RENDER_DISABLE_VISIBILITY));
    /* translate region to target buffer */
    pixman_region32_intersect_rect(&item->region, &item->region, target->logical.x,
                                   target->logical.y, target->logical.width,
                                   target->logical.height);
    scene_render_target_region(target, &item->region);
}

void security_target_add_output(struct scene_render_target *target, struct scene_output *output)
{
    struct security_object *object = security_object_get(&output->addons);
    if (!object || wl_list_empty(&object->securities)) {
        return;
    }

    struct security_buffer *buffer = security_buffer_get_or_create(target->buffer);
    if (!buffer) {
        return;
    }

    struct security_buffer_item *item = calloc(1, sizeof(*item));
    if (!item) {
        return;
    }

    item->object = object;
    wl_list_insert(&buffer->items, &item->link);
    security_object_lock(object);

    struct kywc_box *logical =
        kywc_box_not_empty(&output->viewport.src) ? &output->viewport.src : &output->geometry;

    pixman_region32_init(&item->region);
    pixman_region32_init_rect(&item->region, logical->x, logical->y, logical->width,
                              logical->height);
    /* translate region to target buffer */
    pixman_region32_intersect_rect(&item->region, &item->region, target->logical.x,
                                   target->logical.y, target->logical.width,
                                   target->logical.height);
    scene_render_target_region(target, &item->region);
}

void security_target_add_buffer(struct scene_render_target *target, struct wlr_buffer *wlr_buffer,
                                const struct kywc_fbox *src_box, enum wl_output_transform transform,
                                const struct kywc_box *dst_box)
{
    struct security_buffer *src_buffer = security_buffer_get(wlr_buffer);
    if (!src_buffer || wl_list_empty(&src_buffer->items)) {
        return;
    }

    struct security_buffer *buffer = security_buffer_get_or_create(target->buffer);
    if (!buffer) {
        return;
    }

    /* copy the items in src buffer to target buffer */
    struct security_buffer_item *src_item;
    wl_list_for_each(src_item, &src_buffer->items, link) {
        struct security_buffer_item *item = calloc(1, sizeof(*item));
        if (!item) {
            continue;
        }

        item->object = src_item->object;
        wl_list_insert(&buffer->items, &item->link);
        security_object_lock(item->object);

        pixman_region32_init(&item->region);
        scene_render_buffer_region(src_box, &src_item->region, transform, wlr_buffer->width,
                                   wlr_buffer->height, target->transform, dst_box, &item->region);
    }
}

void security_target_add_buffer_with_region(struct scene_render_target *target,
                                            struct wlr_buffer *wlr_buffer,
                                            const pixman_region32_t *region)
{
    struct security_buffer *src_buffer = security_buffer_get(wlr_buffer);
    if (!src_buffer || wl_list_empty(&src_buffer->items)) {
        return;
    }

    struct security_buffer *buffer = security_buffer_get_or_create(target->buffer);
    if (!buffer) {
        return;
    }

    /* copy the items in src buffer to target buffer */
    struct security_buffer_item *src_item;
    wl_list_for_each(src_item, &src_buffer->items, link) {
        struct security_buffer_item *item = calloc(1, sizeof(*item));
        if (!item) {
            continue;
        }

        item->object = src_item->object;
        wl_list_insert(&buffer->items, &item->link);
        security_object_lock(item->object);

        pixman_region32_init(&item->region);
        /* replace the secure region with region */
        pixman_region32_copy(&item->region, region);
    }
}

void security_target_reset(struct scene_render_target *target)
{
    if (!target || !target->buffer) {
        return;
    }

    struct security_buffer *buffer = security_buffer_get(target->buffer);
    if (!buffer) {
        return;
    }

    struct security_buffer_item *item, *tmp;
    wl_list_for_each_safe(item, tmp, &buffer->items, link) {
        security_buffer_item_destroy(item);
    }
}

bool security_get_region(struct wlr_buffer *wlr_buffer, struct client *client,
                         pixman_region32_t *region)
{
    if (!wlr_buffer || !region) {
        return false;
    }

    struct security_buffer *buffer = security_buffer_get(wlr_buffer);
    if (!buffer || wl_list_empty(&buffer->items)) {
        return false;
    }

    /* calc the secure region in this buffer */
    struct security_buffer_item *item;
    wl_list_for_each(item, &buffer->items, link) {
        struct security_action_info info;
        switch (item->object->type) {
        case SECURITY_TYPE_NODE:
            info = (struct security_action_info){
                .action = SECURITY_ACCESS_NODE,
                .node = {
                    .node = item->object->node,
                    .client = client,
                },
            };
            break;
        case SECURITY_TYPE_OUTPUT:
            info = (struct security_action_info){
                .action = SECURITY_ACCESS_OUTPUT,
                .output = {
                    .output = item->object->output,
                    .client = client,
                },
            };
            break;
        default:
            /* selection/dnd objects never appear as buffer items */
            assert(false);
            continue;
        }

        if (security_object_access(item->object, &info)) {
            continue;
        }
        pixman_region32_union(region, region, &item->region);
    }

    return pixman_region32_not_empty(region);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);

    assert(wl_list_empty(&manager->node_objects));
    assert(wl_list_empty(&manager->output_objects));
    assert(wl_list_empty(&manager->selection_objects));

    free(manager);
    manager = NULL;
}

bool security_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->node_objects);
    wl_list_init(&manager->output_objects);
    wl_list_init(&manager->selection_objects);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    global_filter_manager_create(server);
    ky_security_manager_create(server);
    security_protector_create(server);
    security_context_manager_create(server->display);

    return true;
}
