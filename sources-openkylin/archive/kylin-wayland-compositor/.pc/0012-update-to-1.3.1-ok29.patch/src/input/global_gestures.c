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

static struct gestures_manager {
    struct config *config;

    struct wl_list gestures;

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

static const sd_bus_vtable gesture_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListGestures", "", "a{uv}", list_gestures, 0),
    SD_BUS_METHOD("ConfigureGestures", "a{uv}", "u", configure_gestures, 0),
    SD_BUS_METHOD("ResetGestures", "", "", reset_gestures, 0),
    SD_BUS_VTABLE_END,
};

static bool gestures_manager_config_init(struct gestures_manager *manager)
{
    manager->config = config_manager_add_config("Gestures");
    if (!manager->config) {
        return false;
    }

    return dbus_register_object(NULL, service_path, service_iface, gesture_vtable, manager);
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

    gestures_manager_config_init(manager);
    setup_system_gestures_from_config(manager);

    manager->server_start.notify = handle_server_start;
    wl_signal_add(&server->events.start, &manager->server_start);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
