// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <wlr/interfaces/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

#include <kywc/log.h>

#include "config.h"
#include "input_p.h"
#include "server.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "util/spawn.h"
#include "util/string.h"

enum action_type {
    ACTION_TYPE_NONE = 0,
    ACTION_TYPE_DBUS_ACTION,
    ACTION_TYPE_RUN_COMMAND,
    ACTION_TYPE_SEND_BUTTON,
    ACTION_TYPE_SEND_KEY,
};

enum key_action {
    KEY_ACTION_PRESS = 1 << 0,
    KEY_ACTION_RELEASE = 1 << 1,
    KEY_ACTION_CLICK = (1 << 2) - 1,
};

enum input_type { INPUT_TYPE_NONE = 0, INPUT_TYPE_KEYBOARD, INPUT_TYPE_GESTURE };

enum dbus_type { DBUS_TYPE_NONE = 0, DBUS_TYPE_SESSION, DBUS_TYPE_SYSTEM };

enum control_type { CONTROL_TYPE_DELETE, CONTROL_TYPE_ENABLE, CONTROL_TYPE_DISABLE };

struct action_dbus_data {
    enum dbus_type type;
    const char *service;
    const char *path;
    const char *interface;
    const char *method;

    const char *param_type;
    const char *param_value;
};

struct action_command_data {
    const char *cmd;
};

struct action_button_data {
    uint32_t val;
};

struct keycodes {
    enum key_action action;
    uint32_t *code;
    uint32_t len;
};

struct action_key_data {
    struct keycodes *modifiers;
    struct keycodes *keys;
};

struct action_data {
    enum action_type type;
    enum gesture_stage stage;
    enum gesture_direction follow_direction;
    union {
        struct action_dbus_data dbus;
        struct action_button_data button;
        struct action_key_data key;
        struct action_command_data command;
    } data;
    const char *desc;
    struct wl_list link;
};

struct input_action {
    const char *name;
    enum input_type type;
    enum key_binding_type binding_type;
    const char *bindings;
    struct wl_list actions;

    void *data; /* key or gesture binding */

    bool enable;

    struct wl_list link;
};

static struct input_action_manager {
    struct config *config;
    struct server *server;

    struct wl_listener server_destroy;

    struct wl_list actions;
} *manager = NULL;

static struct keycode_map {
    const char *key;
    uint32_t code;
} keycode_maps[] = {
    { "a", KEY_A },
    { "b", KEY_B },
    { "c", KEY_C },
    { "d", KEY_D },
    { "e", KEY_E },
    { "f", KEY_F },
    { "g", KEY_G },
    { "h", KEY_H },
    { "i", KEY_I },
    { "g", KEY_G },
    { "k", KEY_K },
    { "l", KEY_L },
    { "m", KEY_M },
    { "n", KEY_N },
    { "o", KEY_O },
    { "p", KEY_P },
    { "q", KEY_Q },
    { "r", KEY_R },
    { "s", KEY_S },
    { "t", KEY_T },
    { "u", KEY_U },
    { "v", KEY_V },
    { "w", KEY_W },
    { "x", KEY_X },
    { "y", KEY_Y },
    { "z", KEY_Z },
    { "Tab", KEY_TAB },
    { "Super_L", KEY_LEFTMETA },
    { "Alt_L", KEY_LEFTALT },
    { "Left", KEY_LEFT },
    { "Right", KEY_RIGHT },
    { "Down", KEY_DOWN },
    { "Up", KEY_UP },
    { "Shift_L", KEY_LEFTSHIFT },
    { "Control_R", KEY_RIGHTCTRL },
    { "Control_L", KEY_LEFTCTRL },
    { "Alt_R", KEY_RIGHTALT },
    { "Super_R", KEY_RIGHTMETA },
    { "Shift_R", KEY_RIGHTSHIFT },
};

static struct btncode_map {
    const char *btn;
    uint32_t code;
} btncode_maps[] = {
    { "left", BTN_LEFT }, { "right", BTN_RIGHT },     { "middle", BTN_MIDDLE },
    { "back", BTN_BACK }, { "forward", BTN_FORWARD },
};

static const char *service_path = "/com/kylin/Wlcom/InputAction";
static const char *service_interface = "com.kylin.Wlcom.InputAction";

static uint32_t keycode_map(const char *keystr)
{
    for (size_t i = 0; i < ARRAY_SIZE(keycode_maps); i++) {
        if (strcasecmp(keystr, keycode_maps[i].key) == 0) {
            return keycode_maps[i].code;
        }
    }
    return 0;
}

static uint32_t btncode_map(const char *btnstr)
{
    for (size_t i = 0; i < ARRAY_SIZE(btncode_maps); i++) {
        if (strcasecmp(btnstr, btncode_maps[i].btn) == 0) {
            return btncode_maps[i].code;
        }
    }
    return 0;
}

static struct keycodes *keycodes_create(const char *str)
{
    struct keycodes *keycodes = calloc(1, sizeof(*keycodes));
    if (!keycodes) {
        return NULL;
    }

    size_t action_len = 0;
    char **split_action = string_split(str, ":", &action_len);
    if (action_len > 2) {
        kywc_log(KYWC_ERROR, "Split key action error");
    }
    size_t len = 0;
    char **split_str = string_split(split_action[0], "+", &len);
    for (size_t i = 0, j = 0; i < len; i++) {
        uint32_t keycode = keycode_map(split_str[i]);
        if (!keycode) {
            continue;
        }
        keycodes->code = realloc(keycodes->code, (keycodes->len + 1) * sizeof(uint32_t));
        keycodes->code[j++] = keycode;
        keycodes->len++;
    }
    string_free_split(split_str);
    keycodes->action = KEY_ACTION_CLICK;
    if (action_len == 2) {
        if (strcmp(split_action[1], "press") == 0) {
            keycodes->action = KEY_ACTION_PRESS;
        } else if (strcmp(split_action[1], "release") == 0) {
            keycodes->action = KEY_ACTION_RELEASE;
        }
    }
    string_free_split(split_action);

    return keycodes;
}

static int list_input_actions(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct input_action_manager *manager = userdata;

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "(ss)"));

    struct input_action *action;
    wl_list_for_each(action, &manager->actions, link) {
        json_object *itype_obj = json_object_object_get(
            manager->config->json, action->type == INPUT_TYPE_KEYBOARD ? "keyboards" : "gestures");
        json_object *config = json_object_object_get(itype_obj, action->bindings);
        const char *cfg = json_object_to_json_string(config);
        sd_bus_message_append(reply, "(ss)", action->bindings, cfg);
    }
    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static bool dbus_method_paramter_handle(enum dbus_type type, const char *service, const char *path,
                                        const char *interface, const char *method,
                                        const char *param_type, const char *param_value)
{
    if (!param_type || !param_value) {
        return type == DBUS_TYPE_SESSION
                   ? dbus_call_method(service, path, interface, method, NULL, NULL)
                   : dbus_call_system_method(service, path, interface, method, NULL, NULL);
    }

    if (!strcmp(param_type, "s")) {
        const char *value = param_value;
        return type == DBUS_TYPE_SESSION
                   ? dbus_call_methodv(service, path, interface, method, NULL, NULL, param_type,
                                       value)
                   : dbus_call_system_methodv(service, path, interface, method, NULL, NULL,
                                              param_type, value);
    } else if (!strcmp(param_type, "b")) {
        bool value = strcasecmp(param_value, "true") == 0;
        return type == DBUS_TYPE_SESSION
                   ? dbus_call_methodv(service, path, interface, method, NULL, NULL, param_type,
                                       value)
                   : dbus_call_system_methodv(service, path, interface, method, NULL, NULL,
                                              param_type, value);
    } else if (!strcmp(param_type, "d")) {
        double value = atof(param_value);
        return type == DBUS_TYPE_SESSION
                   ? dbus_call_methodv(service, path, interface, method, NULL, NULL, param_type,
                                       value)
                   : dbus_call_system_methodv(service, path, interface, method, NULL, NULL,
                                              param_type, value);
    } else if (!strcmp(param_type, "i")) {
        int value = atoi(param_value);
        return type == DBUS_TYPE_SESSION
                   ? dbus_call_methodv(service, path, interface, method, NULL, NULL, param_type,
                                       value)
                   : dbus_call_system_methodv(service, path, interface, method, NULL, NULL,
                                              param_type, value);
    }

    return false;
}

static void action_call_dbus_method(struct action_dbus_data *dbus_data)
{
    bool ret = dbus_method_paramter_handle(dbus_data->type, dbus_data->service, dbus_data->path,
                                           dbus_data->interface, dbus_data->method,
                                           dbus_data->param_type, dbus_data->param_value);
    if (!ret) {
        kywc_log(KYWC_ERROR, "Dbus call failed: %s %s %s %s", dbus_data->service, dbus_data->path,
                 dbus_data->interface, dbus_data->method);
    }
}

static void action_call_send_button(struct seat *seat, struct action_button_data *data)
{
    struct cursor *cursor = seat->cursor;
    seat_feed_pointer_motion(seat, cursor->lx, cursor->ly, true);

    if (cursor->touch_simulation_pointer && cursor->last_click_button == BTN_LEFT &&
        data->val == BTN_RIGHT) {
        seat_feed_pointer_button(seat, BTN_LEFT, false);
        cursor->touch_simulation_pointer = false;
    }

    seat_feed_pointer_button(seat, data->val, true);
    seat_feed_pointer_button(seat, data->val, false);
}

static void action_call_send_key(struct seat *seat, struct action_key_data *data)
{
    for (uint32_t i = 0; data->modifiers && i < data->modifiers->len; ++i) {
        if (data->modifiers->action & KEY_ACTION_PRESS) {
            seat_feed_keyboard_key(seat, data->modifiers->code[i], true);
        }
    }
    for (uint32_t i = 0; data->keys && i < data->keys->len; ++i) {
        if (data->keys->action & KEY_ACTION_PRESS) {
            seat_feed_keyboard_key(seat, data->keys->code[i], true);
        }
    }
    for (uint32_t i = 0; data->modifiers && i < data->modifiers->len; ++i) {
        if (data->modifiers->action & KEY_ACTION_RELEASE) {
            seat_feed_keyboard_key(seat, data->modifiers->code[i], false);
        }
    }
    for (uint32_t i = 0; data->keys && i < data->keys->len; ++i) {
        if (data->keys->action & KEY_ACTION_RELEASE) {
            seat_feed_keyboard_key(seat, data->keys->code[i], false);
        }
    }
}

static void handle_input_action(struct input_action *input_action, struct seat *seat,
                                enum gesture_stage stage, enum gesture_direction direction)
{
    if (!input_action) {
        return;
    }

    if (!seat) {
        seat = input_manager_get_default_seat();
        if (!seat) {
            return;
        }
    }

    struct action_data *action_data = NULL;
    wl_list_for_each(action_data, &input_action->actions, link) {
        if (action_data->stage != stage) {
            continue;
        }

        if ((action_data->stage == GESTURE_STAGE_AFTER &&
             action_data->follow_direction != direction)) {
            continue;
        }

        switch (action_data->type) {
        case ACTION_TYPE_DBUS_ACTION:
            action_call_dbus_method(&action_data->data.dbus);
            break;
        case ACTION_TYPE_RUN_COMMAND:
            spawn_invoke(action_data->data.command.cmd);
            break;
        case ACTION_TYPE_SEND_BUTTON:
            action_call_send_button(seat, &action_data->data.button);
            break;
        case ACTION_TYPE_SEND_KEY:
            action_call_send_key(seat, &action_data->data.key);
            break;
        default:
            break;
        }

        kywc_log(KYWC_DEBUG, "Input_action: %s, type: %d", input_action->bindings,
                 action_data->type);
    }
}

static const char *input_action_get_unique(struct input_action *input_action)
{
    if (input_action->name) {
        return input_action->name;
    }

    struct action_data *action_data;
    wl_list_for_each(action_data, &input_action->actions, link) {
        if (action_data->desc) {
            return action_data->desc;
        }
    }

    return NULL;
}

void input_action_handle_key_binding(struct key_binding *binding, struct seat *seat)
{
    const char *unique = bindings_get_key_binding_unique(binding);
    if (!unique) {
        return;
    }

    struct input_action *input_action;
    wl_list_for_each(input_action, &manager->actions, link) {
        if (input_action->type != INPUT_TYPE_KEYBOARD) {
            continue;
        }
        if (!input_action->name || strcmp(input_action->name, unique)) {
            continue;
        }

        handle_input_action(input_action, seat, GESTURE_STAGE_NONE, GESTURE_DIRECTION_NONE);
    }
}

void input_action_handle_gesture_binding(struct gesture_binding *binding, struct seat *seat,
                                         enum gesture_stage stage,
                                         enum gesture_direction follow_direction)
{
    const char *name = kywc_gesture_binding_desc(binding);
    if (!name) {
        return;
    }

    struct input_action *input_action;
    wl_list_for_each(input_action, &manager->actions, link) {
        if (input_action->type != INPUT_TYPE_GESTURE) {
            continue;
        }
        if (!input_action->name || strcmp(input_action->name, name)) {
            continue;
        }
        handle_input_action(input_action, seat, stage, follow_direction);
    }
}

static void action_data_destroy(struct action_data *action_data)
{
    if (action_data->type == ACTION_TYPE_SEND_KEY) {
        struct keycodes *modifiers = action_data->data.key.modifiers;
        struct keycodes *keys = action_data->data.key.keys;
        if (modifiers) {
            free(modifiers->code);
        }
        if (keys) {
            free(keys->code);
        }
        free(modifiers);
        free(keys);
    }

    wl_list_remove(&action_data->link);
    free(action_data);
}

static void input_action_destroy(struct input_action *input_action)
{
    struct action_data *action_data, *tmp;
    wl_list_for_each_safe(action_data, tmp, &input_action->actions, link) {
        action_data_destroy(action_data);
    }

    wl_list_remove(&input_action->link);
    free(input_action);
}

static void action_data_create_from_config(struct input_action *input_action,
                                           enum gesture_stage stage,
                                           enum gesture_direction follow_direct,
                                           json_object *action_config)
{
    struct action_data *action_data = calloc(1, sizeof(*action_data));
    if (!action_data) {
        return;
    }
    action_data->stage = stage;
    action_data->follow_direction = follow_direct;

    json_object *data;

    if (json_object_object_get_ex(action_config, "desc", &data)) {
        action_data->desc = json_object_get_string(data);
    }

    enum action_type type = ACTION_TYPE_NONE;
    if (json_object_object_get_ex(action_config, "actiontype", &data)) {
        const char *type_str = json_object_get_string(data);
        if (strcmp(type_str, "dbus") == 0) {
            type = ACTION_TYPE_DBUS_ACTION;
        } else if (strcmp(type_str, "command") == 0) {
            type = ACTION_TYPE_RUN_COMMAND;
        } else if (strcmp(type_str, "button") == 0) {
            type = ACTION_TYPE_SEND_BUTTON;
        } else if (strcmp(type_str, "key") == 0) {
            type = ACTION_TYPE_SEND_KEY;
        }
        action_data->type = type;
    }

    switch (type) {
    case ACTION_TYPE_DBUS_ACTION:
        if (json_object_object_get_ex(action_config, "bustype", &data)) {
            const char *bustype = json_object_get_string(data);
            if (strcmp(bustype, "session") == 0) {
                action_data->data.dbus.type = DBUS_TYPE_SESSION;
            } else {
                action_data->data.dbus.type = DBUS_TYPE_SYSTEM;
            }
        }
        if (json_object_object_get_ex(action_config, "service", &data)) {
            action_data->data.dbus.service = json_object_get_string(data);
        }
        if (json_object_object_get_ex(action_config, "path", &data)) {
            action_data->data.dbus.path = json_object_get_string(data);
        }
        if (json_object_object_get_ex(action_config, "interface", &data)) {
            action_data->data.dbus.interface = json_object_get_string(data);
        }
        if (json_object_object_get_ex(action_config, "method", &data)) {
            action_data->data.dbus.method = json_object_get_string(data);
        }
        if (json_object_object_get_ex(action_config, "param_type", &data)) {
            action_data->data.dbus.param_type = json_object_get_string(data);
        }
        if (json_object_object_get_ex(action_config, "param_value", &data)) {
            action_data->data.dbus.param_value = json_object_get_string(data);
        }
        break;
    case ACTION_TYPE_RUN_COMMAND:
        if (json_object_object_get_ex(action_config, "command", &data)) {
            action_data->data.command.cmd = json_object_get_string(data);
        }
        break;
    case ACTION_TYPE_SEND_BUTTON:
        if (json_object_object_get_ex(action_config, "button", &data)) {
            const char *button = json_object_get_string(data);
            action_data->data.button.val = btncode_map(button);
        }
        break;
    case ACTION_TYPE_SEND_KEY:
        if (json_object_object_get_ex(action_config, "modifiers", &data)) {
            const char *modifiers = json_object_get_string(data);
            action_data->data.key.modifiers = keycodes_create(modifiers);
        }
        if (json_object_object_get_ex(action_config, "keys", &data)) {
            const char *keys = json_object_get_string(data);
            action_data->data.key.keys = keycodes_create(keys);
        }
        break;
    default:
        break;
    }

    wl_list_insert(&input_action->actions, &action_data->link);
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListAllActions", "", "a(ss)", list_input_actions, 0),
    SD_BUS_VTABLE_END,
};

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    struct input_action_manager *manager = wl_container_of(listener, manager, server_destroy);

    struct input_action *action, *temp;
    wl_list_for_each_safe(action, temp, &manager->actions, link) {
        input_action_destroy(action);
    }

    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->actions);
    free(manager);
}

static void input_action_create_with_keyboard(struct input_action_manager *manager,
                                              json_object *keyboard_obj, bool user)
{
    json_object_object_foreach(keyboard_obj, keybind, action_config) {
        struct input_action *input_action = calloc(1, sizeof(*input_action));
        if (!input_action) {
            continue;
        }

        wl_list_init(&input_action->actions);

        input_action->bindings = keybind;
        input_action->type = INPUT_TYPE_KEYBOARD;

        json_object *data;
        if (json_object_object_get_ex(action_config, "enable", &data)) {
            input_action->enable = json_object_get_boolean(data);
        }

        if (json_object_object_get_ex(action_config, "type", &data)) {
            const char *type_str = json_object_get_string(data);
            input_action->binding_type = kywc_key_binding_type_by_name(type_str, NULL);
        }

        if (json_object_object_get_ex(action_config, "name", &data)) {
            input_action->name = json_object_get_string(data);
        }

        if (json_object_object_get_ex(action_config, "none", &data)) {
            action_data_create_from_config(input_action, GESTURE_STAGE_NONE, GESTURE_DIRECTION_NONE,
                                           data);
        } else {
            action_data_create_from_config(input_action, GESTURE_STAGE_NONE, GESTURE_DIRECTION_NONE,
                                           action_config);
        }
        input_action->name = input_action_get_unique(input_action);

        kywc_log(KYWC_DEBUG, "Input_action keybind: %s", keybind);

        wl_list_insert(&manager->actions, &input_action->link);

        if (!input_action->enable) {
            continue;
        }

        if (!input_action->name) {
            kywc_log(KYWC_WARN, "Input_action keybind %s missing unique name", keybind);
            continue;
        }

        struct key_binding *binding =
            kywc_key_binding_create(keybind, input_action->name, FUNC_ID_CONFIG);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, input_action->binding_type,
                                       function_manager_handle_key_binding, NULL)) {
            kywc_log(KYWC_DEBUG, "Key_binding registion failed");
            if (user) {
                json_object_object_del(keyboard_obj, keybind);
            }
            kywc_key_binding_destroy(binding);
            binding = NULL;
        }
        input_action->data = binding;
    }
}

static void input_action_create_with_gesture(struct input_action_manager *manager,
                                             json_object *gesture_obj, bool user)
{
    json_object_object_foreach(gesture_obj, gesture, action_config) {
        struct input_action *input_action = calloc(1, sizeof(*input_action));
        if (!input_action) {
            continue;
        }
        wl_list_init(&input_action->actions);
        input_action->bindings = gesture;
        input_action->type = INPUT_TYPE_GESTURE;

        json_object *data;
        if (json_object_object_get_ex(action_config, "name", &data)) {
            input_action->name = json_object_get_string(data);
        }

        if (json_object_object_get_ex(action_config, "enable", &data)) {
            input_action->enable = json_object_get_boolean(data);
        }

        if (json_object_object_get_ex(action_config, "before", &data)) {
            action_data_create_from_config(input_action, GESTURE_STAGE_BEFORE,
                                           GESTURE_DIRECTION_NONE, data);
        }

        if (json_object_object_get_ex(action_config, "trigger", &data)) {
            action_data_create_from_config(input_action, GESTURE_STAGE_TRIGGER,
                                           GESTURE_DIRECTION_NONE, data);
        }

        if (json_object_object_get_ex(action_config, "after", &data)) {
            json_object *dir_data;
            if (json_object_object_get_ex(data, "up", &dir_data)) {
                action_data_create_from_config(input_action, GESTURE_STAGE_AFTER,
                                               GESTURE_DIRECTION_UP, dir_data);
            }
            if (json_object_object_get_ex(data, "down", &dir_data)) {
                action_data_create_from_config(input_action, GESTURE_STAGE_AFTER,
                                               GESTURE_DIRECTION_DOWN, dir_data);
            }
            if (json_object_object_get_ex(data, "left", &dir_data)) {
                action_data_create_from_config(input_action, GESTURE_STAGE_AFTER,
                                               GESTURE_DIRECTION_LEFT, dir_data);
            }
            if (json_object_object_get_ex(data, "right", &dir_data)) {
                action_data_create_from_config(input_action, GESTURE_STAGE_AFTER,
                                               GESTURE_DIRECTION_RIGHT, dir_data);
            }
        }

        if (json_object_object_get_ex(action_config, "stop", &data)) {
            action_data_create_from_config(input_action, GESTURE_STAGE_STOP, GESTURE_DIRECTION_NONE,
                                           data);
        }

        kywc_log(KYWC_DEBUG, "Input_action gesture bind: %s", gesture);

        wl_list_insert(&manager->actions, &input_action->link);

        if (!input_action->enable) {
            continue;
        }

        struct gesture_binding *binding =
            kywc_gesture_binding_create_by_string(gesture, input_action->name, FUNC_ID_CONFIG);
        if (!binding) {
            continue;
        }

        if (!kywc_gesture_binding_register(binding, function_manager_handle_gesture_binding,
                                           input_action)) {
            kywc_log(KYWC_DEBUG, "Gesture_binding registion failed");
            if (user) {
                json_object_object_del(gesture_obj, gesture);
            }
            kywc_gesture_binding_destroy(binding);
            binding = NULL;
        }
        input_action->data = binding;
    }
}

static bool input_action_manager_read_config(struct input_action_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return false;
    }

    json_object *data;
    /* get system default config */
    if (manager->config->sys_json &&
        json_object_object_get_ex(manager->config->sys_json, "keyboards", &data)) {
        input_action_create_with_keyboard(manager, data, false);
    }

    if (manager->config->sys_json &&
        json_object_object_get_ex(manager->config->sys_json, "gestures", &data)) {
        input_action_create_with_gesture(manager, data, false);
    }

    /* get user config */
    if (manager->config->json &&
        json_object_object_get_ex(manager->config->json, "keyboards", &data)) {
        input_action_create_with_keyboard(manager, data, true);
    }

    if (json_object_object_get_ex(manager->config->json, "gestures", &data)) {
        input_action_create_with_gesture(manager, data, true);
    }
    return true;
}

static bool input_action_manager_config_init(struct input_action_manager *manager)
{
    manager->config = config_manager_add_config("InputAction");
    if (!manager->config) {
        return false;
    }
    return dbus_register_object(NULL, service_path, service_interface, service_vtable, manager);
}

static struct gesture {
    enum gesture_device device;
    uint8_t fingers;
    enum gesture_type type;
    enum gesture_direction direction;
    enum gesture_edge edge;

    uint32_t stages;
    uint32_t follow_directions;
    double follow_threshold;

    char *desc;
    enum function_id func_id;
} gestures[] = {
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        1,
        GESTURE_TYPE_HOLD,
        GESTURE_DIRECTION_NONE,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(1)fingers_hold",
        FUNC_ID_SIMULATE_RIGHTBTN,
    },
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        1,
        GESTURE_TYPE_SWIPE,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_BOTTOM,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(1)fingers_swipe_up(bottom)",
        FUNC_ID_MULTI_TASKVIEW,
    },
    {
        GESTURE_DEVICE_TOUCHPAD,
        3,
        GESTURE_TYPE_TAP,
        GESTURE_DIRECTION_NONE,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(3)fingers_tap",
        FUNC_ID_UKUI_GLOBAL_SEARCH,
    },
    {
        GESTURE_DEVICE_TOUCHPAD,
        3,
        GESTURE_TYPE_SWIPE,
        GESTURE_DIRECTION_LEFT,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        50.0,
        "touchpad_(3)fingers_swipe_left",
        FUNC_ID_WINDOWS_SWITCH_BACKWARD,
    },
    {
        GESTURE_DEVICE_TOUCHPAD,
        3,
        GESTURE_TYPE_SWIPE,
        GESTURE_DIRECTION_RIGHT,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        50.0,
        "touchpad_(3)fingers_swipe_right",
        FUNC_ID_WINDOWS_SWITCH,
    },
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        3,
        GESTURE_TYPE_SWIPE,
        GESTURE_DIRECTION_LEFT,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        0.15,
        "touchscreen_(3)fingers_swipe_left",
        FUNC_ID_WINDOWS_SWITCH_BACKWARD,
    },
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        3,
        GESTURE_TYPE_SWIPE,
        GESTURE_DIRECTION_RIGHT,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        0.15,
        "touchscreen_(3)fingers_swipe_right",
        FUNC_ID_WINDOWS_SWITCH,
    },
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        3,
        GESTURE_TYPE_HOLD,
        GESTURE_DIRECTION_NONE,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(3)fingers_hold",
        FUNC_ID_UKUI_GLOBAL_SEARCH,
    },
    {
        GESTURE_DEVICE_TOUCHPAD,
        4,
        GESTURE_TYPE_TAP,
        GESTURE_DIRECTION_NONE,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(4)fingers_tap",
        FUNC_ID_SHOW_UKUI_SIDEBAR,
    },
    {
        GESTURE_DEVICE_TOUCHSCREEN,
        4,
        GESTURE_TYPE_HOLD,
        GESTURE_DIRECTION_NONE,
        GESTURE_EDGE_NONE,
        GESTURE_STAGE_TRIGGER,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(4)fingers_hold",
        FUNC_ID_SHOW_UKUI_SIDEBAR,
    },
};

static struct shortcut {
    char *keybind;
    char *desc;
    enum function_id id;
} shortcuts[] = {
    { "Win:no", "ukui_menu_show", FUNC_ID_SHOW_UKUI_MENU },
};

static bool builtin_actions_create(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(shortcuts); i++) {
        struct shortcut *shortcut = &shortcuts[i];
        struct key_binding *binding =
            kywc_key_binding_create(shortcut->keybind, shortcut->desc, shortcut->id);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->id),
                                       function_manager_handle_key_binding, NULL)) {
            kywc_key_binding_destroy(binding);
            continue;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(gestures); i++) {
        struct gesture *gesture = &gestures[i];
        struct gesture_binding *binding = kywc_gesture_binding_create(
            gesture->type, gesture->stages, gesture->device, gesture->direction, gesture->edge,
            gesture->fingers, gesture->follow_directions, gesture->follow_threshold, gesture->desc,
            gesture->func_id);
        if (!binding) {
            continue;
        }

        if (!kywc_gesture_binding_register(binding, function_manager_handle_gesture_binding,
                                           gesture)) {
            kywc_gesture_binding_destroy(binding);
            continue;
        }
    }

    return true;
}

bool input_action_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    manager->server = server;
    wl_list_init(&manager->actions);

    builtin_actions_create();
    input_action_manager_config_init(manager);
    input_action_manager_read_config(manager);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);

    return true;
}
