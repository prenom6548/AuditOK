// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>

#include <kywc/binding.h>
#include <kywc/log.h>

#include "config.h"
#include "input/input.h"
#include "server.h"
#include "util/dbus.h"
#include "util/string.h"

static const char *service_path = "/com/kylin/Wlcom/GlobalGestures";
static const char *service_iface = "com.kylin.Wlcom.GlobalGestures";

struct gesture_optional {
    uint32_t stages;
    uint32_t follow_directions;
    double follow_threshold;
};

struct gesture {
    uint32_t id;

    const char *name;
    struct gesture_binding *binding;

    bool enabled;

    enum function_id default_id;
    enum function_id current_id;

    struct gesture_optional optional;

    struct wl_list link;
};

struct gesture_block {
    uint32_t id;

    struct wl_list link;
};

struct gesture_block_owner {
    const char *owner;
    uint32_t blocked_devices;
    struct sd_bus_slot *watcher;

    struct wl_list gestures; // gesture_block
    struct wl_list link;
};

static struct gestures_manager {
    struct config *config;

    struct wl_list gestures;
    struct wl_list block_owners;

    struct wl_listener server_start;
    struct wl_listener server_destroy;
} *manager = NULL;

static void gesture_destroy(struct gesture *gesture)
{
    wl_list_remove(&gesture->link);
    free((void *)gesture->name);
    free(gesture);
}

static struct gesture *gesture_by_id(uint32_t id)
{
    struct gesture *gesture;
    wl_list_for_each(gesture, &manager->gestures, link) {
        if (gesture->id == id) {
            return gesture;
        }
    }
    return NULL;
}

static enum gesture_device gesture_device_by_id(uint32_t id)
{
    return (id >> 20) & 0x0F;
}

static bool gesture_device_is_valid(enum gesture_device device)
{
    return device != GESTURE_DEVICE_NONE && !(device & (device - 1));
}

static struct gesture_block_owner *gesture_block_owner_get(const char *owner)
{
    struct gesture_block_owner *block_owner;
    wl_list_for_each(block_owner, &manager->block_owners, link) {
        if (strcmp(block_owner->owner, owner) == 0) {
            return block_owner;
        }
    }

    return NULL;
}

static struct gesture_block *gesture_block_find(struct gesture_block_owner *block_owner,
                                                uint32_t id)
{
    struct gesture_block *gesture_block;
    wl_list_for_each(gesture_block, &block_owner->gestures, link) {
        if (gesture_block->id == id) {
            return gesture_block;
        }
    }

    return NULL;
}

static bool gesture_block_exists(struct gesture_block_owner *block_owner, uint32_t id)
{
    return gesture_block_find(block_owner, id) != NULL;
}

static struct gesture_block *gesture_block_get_or_create(struct gesture_block_owner *block_owner,
                                                         uint32_t id)
{
    struct gesture_block *gesture_block = gesture_block_find(block_owner, id);
    if (gesture_block) {
        return gesture_block;
    }

    gesture_block = calloc(1, sizeof(*gesture_block));
    if (!gesture_block) {
        return NULL;
    }

    gesture_block->id = id;
    wl_list_insert(&block_owner->gestures, &gesture_block->link);

    return gesture_block;
}

static bool gesture_blocked_by_owner(struct gesture_block_owner *block_owner, uint32_t gesture_id)
{
    enum gesture_device device = gesture_device_by_id(gesture_id);
    return (gesture_device_is_valid(device) && (block_owner->blocked_devices & device)) ||
           gesture_block_exists(block_owner, gesture_id);
}

static void gesture_binding_update_owner_block(struct gesture *gesture, bool old_blocked,
                                               bool new_blocked)
{
    if (!gesture || !gesture->binding || old_blocked == new_blocked) {
        return;
    }

    kywc_gesture_binding_block(gesture->binding, new_blocked);
}

static void gesture_block_owner_destroy(struct gesture_block_owner *block_owner)
{
    struct gesture *gesture;
    wl_list_for_each(gesture, &manager->gestures, link) {
        bool old_blocked = gesture_blocked_by_owner(block_owner, gesture->id);
        gesture_binding_update_owner_block(gesture, old_blocked, false);
    }

    struct gesture_block *gesture_block, *tmp;
    wl_list_for_each_safe(gesture_block, tmp, &block_owner->gestures, link) {
        wl_list_remove(&gesture_block->link);
        free(gesture_block);
    }

    if (block_owner->watcher) {
        sd_bus_slot_unref(block_owner->watcher);
    }

    wl_list_remove(&block_owner->link);
    free((void *)block_owner->owner);
    free(block_owner);
}

static int gesture_block_owner_handle_lost(sd_bus_message *m, void *userdata,
                                           sd_bus_error *ret_error)
{
    const char *new_owner = NULL;
    CK(sd_bus_message_skip(m, "ss"));
    CK(sd_bus_message_read(m, "s", &new_owner));
    if (!new_owner || *new_owner) {
        return 0;
    }

    struct gesture_block_owner *block_owner = userdata;
    gesture_block_owner_destroy(block_owner);

    return 0;
}

static struct gesture_block_owner *gesture_block_owner_get_or_create(const char *owner)
{
    struct gesture_block_owner *block_owner = gesture_block_owner_get(owner);
    if (block_owner) {
        return block_owner;
    }

    block_owner = calloc(1, sizeof(*block_owner));
    if (!block_owner) {
        return NULL;
    }

    block_owner->owner = strdup(owner);
    if (!block_owner->owner) {
        free(block_owner);
        return NULL;
    }

    char *match_rule = string_create("sender='org.freedesktop.DBus',"
                                     "type='signal',"
                                     "interface='org.freedesktop.DBus',"
                                     "member='NameOwnerChanged',"
                                     "path='/org/freedesktop/DBus',"
                                     "arg0='%s'",
                                     block_owner->owner);
    if (!match_rule) {
        free((void *)block_owner->owner);
        free(block_owner);
        return NULL;
    }

    if (!dbus_add_match_slot(&block_owner->watcher, match_rule, gesture_block_owner_handle_lost,
                             block_owner)) {
        free(match_rule);
        free((void *)block_owner->owner);
        free(block_owner);
        return NULL;
    }
    free(match_rule);

    wl_list_init(&block_owner->gestures);
    wl_list_insert(&manager->block_owners, &block_owner->link);

    return block_owner;
}

static bool gesture_block_owner_is_unused(struct gesture_block_owner *block_owner)
{
    return block_owner && !block_owner->blocked_devices && wl_list_empty(&block_owner->gestures);
}

static bool system_gesture_write_config(struct gesture *gesture)
{
    if (!manager || !manager->config || !manager->config->json || !gesture) {
        return false;
    }

    char gesture_id[12] = { 0 };
    snprintf(gesture_id, 11, "0x%08x", gesture->id);

    json_object *config = json_object_object_get(manager->config->json, gesture_id);
    if (!config) {
        config = json_object_new_object();
        json_object_object_add(manager->config->json, gesture_id, config);
    }

    json_object_object_add(config, "name", json_object_new_string(gesture->name));
    json_object_object_add(config, "default", json_object_new_uint64(gesture->default_id));

    if (gesture->current_id != gesture->default_id) {
        json_object_object_add(config, "current", json_object_new_uint64(gesture->current_id));
    } else {
        json_object_object_del(config, "current");
    }

    if (!gesture->enabled) {
        json_object_object_add(config, "enabled", json_object_new_boolean(gesture->enabled));
    } else {
        json_object_object_del(config, "enabled");
    }

    return true;
}

static bool gesture_update(struct gesture *gesture, struct gesture *pending)
{
    if (!gesture || !pending || !gesture->binding) {
        return false;
    }

    struct gesture_optional *optional = &gesture->optional;

    if (gesture->current_id != pending->current_id) {
        enum gesture_device device = (gesture->id >> 20) & 0x0F;
        enum gesture_direction direction = (gesture->id >> 4) & 0xFF;
        function_entry_get_gesture_optional(pending->current_id, device, direction,
                                            &optional->stages, &optional->follow_directions,
                                            &optional->follow_threshold);
    }

    kywc_gesture_binding_update(gesture->binding, pending->enabled, pending->current_id,
                                optional->stages, optional->follow_directions,
                                optional->follow_threshold);

    gesture->default_id = pending->default_id;
    gesture->enabled = pending->enabled;
    gesture->current_id = pending->current_id;

    return true;
}

static void gesture_reset_default_function(void)
{
    struct gesture *gesture;
    wl_list_for_each(gesture, &manager->gestures, link) {
        if (gesture->default_id == gesture->current_id && gesture->enabled) {
            continue;
        }

        struct gesture pending = {
            .enabled = true,
            .current_id = gesture->default_id,
            .default_id = gesture->default_id,
        };
        if (gesture_update(gesture, &pending)) {
            system_gesture_write_config(gesture);
        }
    }
}

static int list_gestures(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    sd_bus_message *reply = NULL;
    struct gestures_manager *manager = userdata;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "{uv}"));

    struct gesture *gesture;
    wl_list_for_each(gesture, &manager->gestures, link) {
        if (!gesture->binding) {
            continue;
        }

        CK(sd_bus_message_open_container(reply, 'e', "uv"));
        CK(sd_bus_message_append_basic(reply, 'u', &gesture->id));
        CK(sd_bus_message_open_container(reply, 'v', "a{sv}"));
        CK(sd_bus_message_open_container(reply, 'a', "{sv}"));

        CK(sd_bus_message_append(reply, "{sv}", "name", "s", gesture->name));
        CK(sd_bus_message_append(reply, "{sv}", "default", "u", gesture->default_id));
        CK(sd_bus_message_append(reply, "{sv}", "current", "u", gesture->current_id));
        CK(sd_bus_message_append(reply, "{sv}", "enabled", "b", gesture->enabled));

        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
    }

    CK(sd_bus_message_close_container(reply));
    sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);

    return 1;
}

static int configure_gestures(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_count = 0;

    CK(sd_bus_message_enter_container(m, 'a', "{uv}"));

    while (!sd_bus_message_at_end(m, 0)) {
        uint32_t gesture_id;

        CK(sd_bus_message_enter_container(m, 'e', "uv"));
        CK(sd_bus_message_read(m, "u", &gesture_id));
        struct gesture *gesture = gesture_by_id(gesture_id);
        if (!gesture) {
            CK(sd_bus_message_skip(m, "v"));
            CK(sd_bus_message_exit_container(m));
            continue;
        }

        CK(sd_bus_message_enter_container(m, 'v', "a{sv}"));
        CK(sd_bus_message_enter_container(m, 'a', "{sv}"));
        struct gesture pending = {
            .enabled = gesture->enabled,
            .binding = gesture->binding,
            .default_id = gesture->default_id,
            .current_id = gesture->current_id,
        };

        while (!sd_bus_message_at_end(m, 0)) {
            const char *key;
            CK(sd_bus_message_enter_container(m, 'e', "sv"));
            CK(sd_bus_message_read(m, "s", &key));

            CK(sd_bus_message_enter_container(m, 'v', NULL));
            if (strcmp(key, "current") == 0) {
                CK(sd_bus_message_read(m, "u", &pending.current_id));
            } else if (strcmp(key, "default") == 0) {
                CK(sd_bus_message_read(m, "u", &pending.default_id));
            } else if (strcmp(key, "enabled") == 0) {
                int enabled;
                CK(sd_bus_message_read(m, "b", &enabled));
                pending.enabled = enabled;
            } else {
                CK(sd_bus_message_skip(m, NULL));
            }
            CK(sd_bus_message_exit_container(m));
            CK(sd_bus_message_exit_container(m));
        }

        if (gesture_update(gesture, &pending)) {
            system_gesture_write_config(gesture);
            ret_count++;
        }

        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
    }

    CK(sd_bus_message_exit_container(m));

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_append(reply, "u", ret_count));

    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static int reset_gestures(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    gesture_reset_default_function();

    return sd_bus_reply_method_return(m, NULL);
}

static bool gesture_block_owner_set_device_block(struct gesture_block_owner *block_owner,
                                                 enum gesture_device device, bool block)
{
    if (!gesture_device_is_valid(device)) {
        return false;
    }

    bool old_device_blocked = block_owner->blocked_devices & device;
    if (old_device_blocked == block) {
        return false;
    }

    if (block) {
        block_owner->blocked_devices |= device;
    } else {
        block_owner->blocked_devices &= ~device;
    }

    struct gesture *gesture;
    wl_list_for_each(gesture, &manager->gestures, link) {
        if (gesture_device_by_id(gesture->id) != device) {
            continue;
        }

        bool has_gesture_block = gesture_block_exists(block_owner, gesture->id);
        bool old_blocked = old_device_blocked || has_gesture_block;
        bool new_blocked = block || has_gesture_block;
        gesture_binding_update_owner_block(gesture, old_blocked, new_blocked);
    }

    return true;
}

static bool gesture_block_owner_set_gesture_block(struct gesture_block_owner *block_owner,
                                                  uint32_t gesture_id, bool block)
{
    struct gesture *gesture = gesture_by_id(gesture_id);
    if (!gesture || !gesture->binding) {
        return false;
    }

    enum gesture_device device = gesture_device_by_id(gesture_id);
    bool device_blocked =
        gesture_device_is_valid(device) && (block_owner->blocked_devices & device);
    struct gesture_block *gesture_block = gesture_block_find(block_owner, gesture_id);
    bool old_gesture_blocked = gesture_block != NULL;
    if (old_gesture_blocked == block) {
        return false;
    }

    bool old_blocked = device_blocked || old_gesture_blocked;
    if (block) {
        gesture_block = gesture_block_get_or_create(block_owner, gesture_id);
        if (!gesture_block) {
            return false;
        }
    } else {
        wl_list_remove(&gesture_block->link);
        free(gesture_block);
    }

    bool new_blocked = device_blocked || block;
    gesture_binding_update_owner_block(gesture, old_blocked, new_blocked);

    return true;
}

static int block_gestures_by_device(sd_bus_message *m, struct gesture_block_owner *block_owner,
                                    uint32_t *ret_count)
{
    CK(sd_bus_message_enter_container(m, 'a', "{ub}"));
    while (!sd_bus_message_at_end(m, 0)) {
        uint32_t device_id;
        int block;

        CK(sd_bus_message_enter_container(m, 'e', "ub"));
        CK(sd_bus_message_read(m, "ub", &device_id, &block));
        enum gesture_device device = device_id;
        if (gesture_block_owner_set_device_block(block_owner, device, block)) {
            (*ret_count)++;
        }
        CK(sd_bus_message_exit_container(m));
    }
    CK(sd_bus_message_exit_container(m));

    return 0;
}

static int block_gestures_by_gesture(sd_bus_message *m, struct gesture_block_owner *block_owner,
                                     uint32_t *ret_count)
{
    CK(sd_bus_message_enter_container(m, 'a', "{ub}"));
    while (!sd_bus_message_at_end(m, 0)) {
        uint32_t gesture_id;
        int block;

        CK(sd_bus_message_enter_container(m, 'e', "ub"));
        CK(sd_bus_message_read(m, "ub", &gesture_id, &block));
        if (gesture_block_owner_set_gesture_block(block_owner, gesture_id, block)) {
            (*ret_count)++;
        }
        CK(sd_bus_message_exit_container(m));
    }
    CK(sd_bus_message_exit_container(m));

    return 0;
}

static int block_gestures(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_count = 0;
    const char *sender = sd_bus_message_get_sender(m);
    if (!sender) {
        return sd_bus_reply_method_return(m, "u", ret_count);
    }

    struct gesture_block_owner *block_owner = gesture_block_owner_get_or_create(sender);
    if (!block_owner) {
        return sd_bus_reply_method_return(m, "u", ret_count);
    }

    CK(sd_bus_message_enter_container(m, 'a', "{sv}"));
    while (!sd_bus_message_at_end(m, 0)) {
        const char *key = NULL;

        CK(sd_bus_message_enter_container(m, 'e', "sv"));
        CK(sd_bus_message_read(m, "s", &key));

        if (strcmp(key, "devices") == 0) {
            CK(sd_bus_message_enter_container(m, 'v', "a{ub}"));
            CK(block_gestures_by_device(m, block_owner, &ret_count));
            CK(sd_bus_message_exit_container(m));
        } else if (strcmp(key, "gestures") == 0) {
            CK(sd_bus_message_enter_container(m, 'v', "a{ub}"));
            CK(block_gestures_by_gesture(m, block_owner, &ret_count));
            CK(sd_bus_message_exit_container(m));
        } else {
            CK(sd_bus_message_skip(m, "v"));
        }

        CK(sd_bus_message_exit_container(m));
    }
    CK(sd_bus_message_exit_container(m));

    if (gesture_block_owner_is_unused(block_owner)) {
        gesture_block_owner_destroy(block_owner);
    }
    return sd_bus_reply_method_return(m, "u", ret_count);
}

static const sd_bus_vtable gesture_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListGestures", "", "a{uv}", list_gestures, 0),
    SD_BUS_METHOD("ConfigureGestures", "a{uv}", "u", configure_gestures, 0),
    SD_BUS_METHOD("BlockGestures", "a{sv}", "u", block_gestures, 0),
    SD_BUS_METHOD("ResetGestures", "", "", reset_gestures, 0),
    SD_BUS_VTABLE_END,
};

static bool gestures_manager_config_init(struct gestures_manager *manager)
{
    manager->config = config_manager_add_config("Gestures");
    if (!manager->config) {
        return false;
    }

    if (!dbus_register_object(NULL, service_path, service_iface, gesture_vtable, manager)) {
        kywc_log(KYWC_ERROR, "Gestures manager register dbus object failed");
        return false;
    }

    return true;
}

static struct gesture *gesture_create_by_id(uint32_t id)
{
    struct gesture *gesture = calloc(1, sizeof(*gesture));
    if (!gesture) {
        return NULL;
    }
    gesture->id = id;
    gesture->enabled = true;
    gesture->current_id = FUNC_ID_NONE;
    gesture->default_id = FUNC_ID_NONE;
    gesture->optional.stages = GESTURE_STAGE_TRIGGER;

    wl_list_insert(&manager->gestures, &gesture->link);

    return gesture;
}

static struct gesture *gesture_get_or_create_by_id(uint32_t id)
{
    struct gesture *gesture = gesture_by_id(id);
    if (!gesture) {
        return gesture_create_by_id(id);
    }

    return gesture;
}

static bool gesture_create_binding_and_register(struct gesture *gesture)
{
    enum gesture_device device = (gesture->id >> 20) & 0x0F;
    uint8_t fingers = (gesture->id >> 16) & 0x0F;
    enum gesture_type type = (gesture->id >> 12) & 0x0F;
    enum gesture_direction direction = (gesture->id >> 4) & 0xFF;
    enum gesture_edge edge = gesture->id & 0x0F;

    struct gesture_optional *optional = &gesture->optional;
    function_entry_get_gesture_optional(gesture->current_id, device, direction, &optional->stages,
                                        &optional->follow_directions, &optional->follow_threshold);
    struct gesture_binding *binding = kywc_gesture_binding_create(
        type, optional->stages, device, direction, edge, fingers, optional->follow_directions,
        optional->follow_threshold, gesture->name, gesture->current_id);

    if (!binding) {
        return false;
    }
    kywc_log(KYWC_DEBUG, "Gesture %s binding create", gesture->name);
    if (!gesture->enabled) {
        kywc_gesture_binding_update(binding, gesture->enabled, gesture->current_id,
                                    optional->stages, optional->follow_directions,
                                    optional->follow_threshold);
    }

    if (!kywc_gesture_binding_register(binding, function_manager_handle_gesture_binding, NULL)) {
        kywc_gesture_binding_destroy(binding);
        return false;
    }

    gesture->binding = binding;

    return true;
}

static bool setup_system_gestures_from_config(struct gestures_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return false;
    }

    json_object_object_foreach(manager->config->json, gesture_id, gesture_data) {
        uint32_t id;
        sscanf(gesture_id, "0x%08x", &id);
        if (!id) {
            continue;
        }

        struct gesture *gesture = gesture_get_or_create_by_id(id);
        if (!gesture) {
            continue;
        }

        json_object *data;
        bool has_current = false;
        if (json_object_object_get_ex(gesture_data, "enabled", &data)) {
            gesture->enabled = json_object_get_boolean(data);
        }

        if (json_object_object_get_ex(gesture_data, "name", &data)) {
            gesture->name = strdup(json_object_get_string(data));
        }

        if (json_object_object_get_ex(gesture_data, "default", &data)) {
            gesture->default_id = json_object_get_uint64(data);
        }

        if (json_object_object_get_ex(gesture_data, "current", &data)) {
            gesture->current_id = json_object_get_uint64(data);
            has_current = true;
        }

        if (!has_current && gesture->default_id != FUNC_ID_NONE) {
            gesture->current_id = gesture->default_id;
        }
        struct gesture_optional *optional = &gesture->optional;
        function_entry_get_gesture_optional(
            gesture->current_id, (gesture->id >> 20) & 0x0F, (gesture->id >> 4) & 0xFF,
            &optional->stages, &optional->follow_directions, &optional->follow_threshold);

        if (!gesture_create_binding_and_register(gesture)) {
            gesture_destroy(gesture);
        }
    }

    return true;
}

static bool system_builtin_gestures(struct gesture_binding *binding, uint32_t gesture_id,
                                    const char *desc)
{
    struct gesture *gesture = gesture_by_id(gesture_id);
    if (gesture) {
        return false;
    }

    gesture = gesture_create_by_id(gesture_id);
    if (!gesture) {
        return false;
    }

    gesture->binding = binding;
    gesture->default_id = kywc_gesture_binding_function_id(binding);
    gesture->current_id = gesture->default_id;
    gesture->name = strdup(desc);
    system_gesture_write_config(gesture);

    return false;
}

static void handle_server_start(struct wl_listener *listener, void *data)
{
    kywc_gesture_binding_for_each(system_builtin_gestures);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->server_start.link);

    struct gesture_block_owner *block_owner, *block_owner_tmp;
    wl_list_for_each_safe(block_owner, block_owner_tmp, &manager->block_owners, link) {
        gesture_block_owner_destroy(block_owner);
    }

    /* free gesture */
    struct gesture *gesture, *tmp;
    wl_list_for_each_safe(gesture, tmp, &manager->gestures, link) {
        gesture_destroy(gesture);
    }
}

bool global_gestures_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->gestures);
    wl_list_init(&manager->block_owners);

    if (!gestures_manager_config_init(manager)) {
        free(manager);
        manager = NULL;
        return false;
    }

    setup_system_gestures_from_config(manager);

    manager->server_start.notify = handle_server_start;
    wl_signal_add(&server->events.start, &manager->server_start);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
