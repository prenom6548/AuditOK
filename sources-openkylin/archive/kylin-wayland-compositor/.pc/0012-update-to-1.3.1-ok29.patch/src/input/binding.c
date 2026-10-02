// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_keyboard.h>
#include <xkbcommon/xkbcommon.h>

#include <kywc/binding.h>
#include <kywc/log.h>

#include "input_p.h"
#include "server.h"
#include "util/macros.h"
#include "util/string.h"

struct key_binding {
    struct wl_list link;

    uint32_t nblocks;
    uint32_t modifiers;
    uint32_t keysym;
    uint32_t options;

    char *keybind;
    char *desc;

    enum function_id func_id;

    void (*action)(struct key_binding *binding, void *data, struct seat *seat,
                   enum key_binding_stage stage);
    void *data;
    bool active;

    struct {
        struct wl_signal cancel;
    } events;
};

struct keysyms_binding {
    struct wl_list bindings; /* key binding */
};

struct gesture_binding {
    struct wl_list link;

    enum gesture_type type;
    enum gesture_device device;
    enum gesture_direction direction;
    enum gesture_edge edge;
    uint8_t fingers;
    uint32_t stages;
    uint32_t follow_directions;
    double follow_threshold;
    char *desc;

    uint32_t id;
    uint32_t func_id;
    bool enabled;

    void (*action)(struct gesture_binding *binding, void *data, struct seat *seat,
                   enum gesture_stage stage, enum gesture_device device, uint32_t follow_directions,
                   double dx, double dy);
    void *data;

    struct {
        struct wl_signal cancel;
    } events;
};

static struct bindings {
    struct keysyms_binding keysyms_binding[KEY_BINDING_TYPE_NUM];
    struct wl_list gesture_bindings;
    struct wl_listener server_destroy;

    size_t keysym_bindings_block;
    size_t type_nlocks[KEY_BINDING_TYPE_NUM];
    uint32_t keysyms_binding_masks; /* binding mask */
} *bindings = NULL;

const struct key_binding_type2string {
    enum key_binding_type type;
    const char *name;
} key_binding_table[] = {
    { KEY_BINDING_TYPE_CUSTOM_DEF, "WLCOM_CUSTOM_DEF" },
    { KEY_BINDING_TYPE_CUSTOM_CONFIG, "WLCOM_CUSTOM_CONFIG" },
    { KEY_BINDING_TYPE_WIN_MENU, "WLCOM_WIN_MENU" },
    { KEY_BINDING_TYPE_SWITCH_WORKSPACE, "WLCOM_SWITCH_WORKSPACE" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_MINIMIZE, "WLCOM_WINDOW_ACTION_MINIMIZE" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_MAXIMIZE, "WLCOM_WINDOW_ACTION_MAXIMIZE" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_CLOSE, "WLCOM_WINDOW_ACTION_CLOSE" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_MENU, "WLCOM_WINDOW_ACTION_MENU" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_TILED, "WLCOM_WINDOW_ACTION_TILED" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_OUTPUT, "WLCOM_WINDOW_ACTION_OUTPUT" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_SEND, "WLCOM_WINDOW_ACTION_SEND" },
    { KEY_BINDING_TYPE_WINDOW_ACTION_CAPTURE, "WLCOM_WINDOW_ACTION_CAPTURE" },
    { KEY_BINDING_TYPE_MAXIMIZED_VIEWS, "WLCOM_MAXIMIZED_VIEWS" },
    { KEY_BINDING_TYPE_TOGGLE_SHOW_DESKTOP, "WLCOM_TOGGLE_SHOW_DESKTOP" },
    { KEY_BINDING_TYPE_SHOW_DESKTOP, "WLCOM_SHOW_DESKTOP" },
    { KEY_BINDING_TYPE_RESTORE_DESKTOP, "WLCOM_RESTORE_DESKTOP" },
    { KEY_BINDING_TYPE_TOGGLE_SHOW_VIEWS, "WLCOM_TOGGLE_SHOW_VIEWS" },
    { KEY_BINDING_TYPE_TOGGLE_SHOW_WINDOWS, "WLCOM_TOGGLE_SHOW_WINDOWS" },
    { KEY_BINDING_TYPE_COLOR_FILTER, "WLCOM_COLOR_FILTER" },
    { KEY_BINDING_TYPE_ZOOM, "WLCOM_ZOOM" },
    { KEY_BINDING_TYPE_NUM, "WLCOM_ALL" },
};

static bool keyboard_binding_finish(struct keyboard *keyboard, bool emit_inactive)
{
    struct keyboard_binding_state *state = &keyboard->binding_state;
    struct key_binding *binding = state->binding;
    if (!binding) {
        return false;
    }

    wl_list_remove(&state->binding_cancel.link);
    keyboard->binding_state = (struct keyboard_binding_state){ 0 };
    keyboard_repeat_stop(keyboard);
    binding->active = false;

    if (emit_inactive && (binding->options & KEY_BINDING_TRIGGER_IN_INACTIVE) && binding->action) {
        binding->action(binding, binding->data, keyboard->seat, KEY_BINDING_STAGE_INACTIVE);
    }

    return true;
}

static void keyboard_binding_handle_cancel(struct wl_listener *listener, void *data)
{
    struct keyboard_binding_state *state = wl_container_of(listener, state, binding_cancel);
    struct keyboard *keyboard = wl_container_of(state, keyboard, binding_state);
    bool *emit_inactive = data;

    keyboard_binding_finish(keyboard, *emit_inactive);
}

static void key_binding_activate(struct key_binding *binding, struct keyboard *keyboard,
                                 uint32_t key)
{
    struct keyboard_binding_state *state = &keyboard->binding_state;

    assert(!state->binding);

    *state = (struct keyboard_binding_state){
        .binding = binding,
        .key = key,
    };
    state->binding_cancel.notify = keyboard_binding_handle_cancel;
    wl_signal_add(&binding->events.cancel, &state->binding_cancel);
}

static void key_binding_cancel_states(struct key_binding *binding, bool emit_inactive)
{
    wl_signal_emit_mutable(&binding->events.cancel, &emit_inactive);
}

static void keysyms_binding_cancel_states(struct keysyms_binding *keysyms_binding,
                                          bool emit_inactive)
{
    struct key_binding *binding, *binding_tmp;
    wl_list_for_each_safe(binding, binding_tmp, &keysyms_binding->bindings, link) {
        key_binding_cancel_states(binding, emit_inactive);
    }
}

static bool is_key_binding_desc_unique(const char *desc)
{
    if (!desc) {
        return true;
    }

    for (int i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        struct key_binding *bind;
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        wl_list_for_each(bind, &keysyms_binding->bindings, link) {
            if (!bind->desc) {
                continue;
            }
            if (!strcmp(bind->desc, desc)) {
                kywc_log(KYWC_DEBUG, "%s is already registered", bind->desc);
                return false;
            }
        }
    }

    return true;
}

struct key_binding *kywc_key_binding_create(const char *keybind, const char *desc,
                                            enum function_id func_id, uint32_t options)
{
    if (!is_key_binding_desc_unique(desc)) {
        return NULL;
    }

    struct key_binding *binding = calloc(1, sizeof(struct key_binding));
    if (!binding) {
        return NULL;
    }

    binding->options =
        options ? options : KEY_BINDING_TRIGGER_IN_ACTIVE | KEY_BINDING_TRIGGER_IN_REPEAT;
    wl_signal_init(&binding->events.cancel);

    size_t length = 0;
    char **bind_str = string_split(keybind, ":", &length);
    if (length == 2 && strcmp(bind_str[1], "no") == 0) {
        binding->options &= ~KEY_BINDING_TRIGGER_IN_REPEAT;
    }

    size_t len = 0;
    const char *bind_name = length == 2 ? bind_str[0] : keybind;
    char **split_str = string_split(bind_name, "+", &len);
    binding->keybind = strdup(bind_name);

    string_free_split(bind_str);

    for (size_t i = 0; i < len; i++) {
        uint32_t mod = keyboard_get_modifier_mask_by_name(split_str[i]);
        if (mod) {
            binding->modifiers |= mod;
            continue;
        }

        /* whatever keep the lower keysym */
        xkb_keysym_t sym = xkb_keysym_from_name(split_str[i], XKB_KEYSYM_CASE_INSENSITIVE);
        binding->keysym = xkb_keysym_to_lower(sym);
    }

    string_free_split(split_str);

    if (desc) {
        binding->desc = strdup(desc);
    }

    binding->func_id = func_id;

    wl_list_init(&binding->link);
    kywc_log(KYWC_DEBUG, "Keybind %s: %s", keybind, desc);

    return binding;
}

struct key_binding *kywc_key_binding_create_by_symbol(unsigned int keysym, unsigned int modifiers,
                                                      const char *desc, enum function_id func_id,
                                                      uint32_t options)
{
    if (!is_key_binding_desc_unique(desc)) {
        return NULL;
    }

    struct key_binding *binding = calloc(1, sizeof(struct key_binding));
    if (!binding) {
        return NULL;
    }

    wl_signal_init(&binding->events.cancel);
    binding->modifiers = modifiers;
    binding->keysym = keysym;
    binding->options =
        options ? options : KEY_BINDING_TRIGGER_IN_ACTIVE | KEY_BINDING_TRIGGER_IN_REPEAT;
    binding->func_id = func_id;

    if (desc) {
        binding->desc = strdup(desc);
    }

    char name[64];
    if (xkb_keysym_get_name(keysym, name, sizeof(name)) > 0) {
        const char *mods = keyboard_get_modifier_names(modifiers, '+');
        binding->keybind = string_create("%s+%s", mods, name);
    }

    wl_list_init(&binding->link);

    return binding;
}

void kywc_key_binding_destroy(struct key_binding *binding)
{
    key_binding_cancel_states(binding, false);
    wl_list_remove(&binding->link);

    free(binding->keybind);
    free(binding->desc);
    free(binding);
}

static bool key_binding_is_valid(struct key_binding *binding, uint32_t keysym, uint32_t modifiers)
{
    for (int i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        struct key_binding *bind;
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        wl_list_for_each(bind, &keysyms_binding->bindings, link) {
            /* skip itself */
            if (bind == binding) {
                continue;
            }

            if (!(modifiers ^ bind->modifiers) && keysym == bind->keysym) {
                kywc_log(KYWC_WARN, "Keybind %s(%s) is already registered by %s(%s)",
                         binding->keybind, binding->desc, bind->keybind, bind->desc);
                return false;
            }
        }
    }

    return true;
}

bool kywc_key_binding_register(struct key_binding *binding, enum key_binding_type type,
                               void (*action)(struct key_binding *binding, void *data,
                                              struct seat *seat, enum key_binding_stage stage),
                               void *data)
{
    if (kywc_key_binding_is_registered(binding)) {
        return true;
    }

    if (!key_binding_is_valid(binding, binding->keysym, binding->modifiers)) {
        return false;
    }

    struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[type];
    wl_list_insert(&keysyms_binding->bindings, &binding->link);
    /* Make sure the type is unblocking */
    if (!bindings->type_nlocks[type]) {
        bindings->keysyms_binding_masks |= 1 << type;
    }

    if (action) {
        binding->action = action;
    }
    if (data) {
        binding->data = data;
    }

    return true;
}

bool kywc_key_binding_update(struct key_binding *binding, unsigned int keysym,
                             unsigned int modifiers)
{
    if (kywc_key_binding_is_registered(binding) &&
        !key_binding_is_valid(binding, keysym, modifiers)) {
        return false;
    }

    key_binding_cancel_states(binding, false);
    binding->keysym = keysym;
    binding->modifiers = modifiers;

    return true;
}

bool kywc_key_binding_update_state(struct key_binding *binding, enum function_id func_id,
                                   uint32_t options)
{
    if (!kywc_key_binding_is_registered(binding)) {
        return false;
    }

    if (binding->func_id != func_id) {
        binding->func_id = func_id;
    }

    key_binding_cancel_states(binding, false);
    binding->options =
        options ? options : KEY_BINDING_TRIGGER_IN_ACTIVE | KEY_BINDING_TRIGGER_IN_REPEAT;
    return true;
}

void kywc_key_binding_unregister(struct key_binding *binding)
{
    key_binding_cancel_states(binding, true);
    wl_list_remove(&binding->link);
    wl_list_init(&binding->link);
}

bool kywc_key_binding_is_registered(struct key_binding *binding)
{
    return !wl_list_empty(&binding->link);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&bindings->server_destroy.link);

    for (size_t i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        struct key_binding *key_binding, *key_binding_tmp;
        wl_list_for_each_safe(key_binding, key_binding_tmp, &keysyms_binding->bindings, link) {
            kywc_key_binding_destroy(key_binding);
        }
    }

    struct gesture_binding *gesture_binding, *gesture_binding_tmp;
    wl_list_for_each_safe(gesture_binding, gesture_binding_tmp, &bindings->gesture_bindings, link) {
        kywc_gesture_binding_destroy(gesture_binding);
    }

    free(bindings);
    bindings = NULL;
}

bool bindings_create(struct input_manager *input_manager)
{
    bindings = calloc(1, sizeof(*bindings));
    if (!bindings) {
        return false;
    }

    for (size_t i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        wl_list_init(&keysyms_binding->bindings);
    }
    wl_list_init(&bindings->gesture_bindings);

    bindings->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(input_manager->server, &bindings->server_destroy);
    return true;
}

static bool match_key_binding(struct keyboard_state *keyboard_state, struct key_binding *binding)
{
    for (size_t j = 0; j < keyboard_state->npressed; j++) {
        uint32_t keysym = keyboard_state->pressed_keysyms[j];
        if (binding->keysym == xkb_keysym_to_lower(keysym)) {
            return true;
        }
    }

    return false;
}

struct key_binding *bindings_get_key_binding(struct keyboard_state *keyboard_state,
                                             uint32_t trigger_options, bool modifier_only_released)
{
    if (bindings->keysym_bindings_block) {
        return NULL;
    }

    for (size_t i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        if (!(bindings->keysyms_binding_masks & (1 << i))) {
            continue;
        }

        struct key_binding *binding;
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        wl_list_for_each(binding, &keysyms_binding->bindings, link) {
            if (binding->nblocks) {
                continue;
            }

            if (!(binding->options & trigger_options)) {
                continue;
            }

            if (keyboard_state->last_modifiers ^ binding->modifiers) {
                continue;
            }

            if (!binding->keysym) {
                if (!modifier_only_released) {
                    continue;
                }

                return binding;
            }

            if (keyboard_state->npressed < 1) {
                continue;
            }

            if (match_key_binding(keyboard_state, binding)) {
                return binding;
            }
        }
    }

    return NULL;
}

enum function_id bindings_get_key_binding_function_id(struct key_binding *binding)
{
    return binding ? binding->func_id : FUNC_ID_NONE;
}

const char *bindings_get_key_binding_unique(struct key_binding *binding)
{
    return binding ? binding->desc : NULL;
}

bool bindings_get_key_binding_bypass_grab(struct key_binding *binding)
{
    if (!binding) {
        return false;
    }

    return binding->options & KEY_BINDING_BYPASS_GRAB;
}

bool bindings_get_key_binding_no_repeat(struct key_binding *binding)
{
    if (!binding) {
        return false;
    }

    return !(binding->options & KEY_BINDING_TRIGGER_IN_REPEAT);
}

bool bindings_handle_key_binding(struct keyboard *keyboard, struct key_binding *binding,
                                 uint32_t key, bool pressed, bool *repeat)
{
    struct keyboard_binding_state *state = &keyboard->binding_state;
    *repeat = false;

    if (!pressed) {
        if (state->binding && state->key == key) {
            keyboard_binding_finish(keyboard, true);
        } else if (binding && binding->active) {
            return true;
        } else if (binding && binding->action) {
            binding->action(binding, binding->data, keyboard->seat, KEY_BINDING_STAGE_INACTIVE);
        }
        return false;
    }

    if (!binding) {
        if (state->binding && !state->active_emitted) {
            keyboard_binding_finish(keyboard, false);
        }
        return false;
    }

    if (state->binding == binding) {
        if (bindings_get_key_binding_no_repeat(binding)) {
            return true;
        }

        *repeat = true;
        if (binding->action) {
            binding->action(binding, binding->data, keyboard->seat, KEY_BINDING_STAGE_REPEAT);
        }
        return true;
    }

    if (state->binding) {
        keyboard_binding_finish(keyboard, state->active_emitted);
    }

    if (binding->active) {
        return true;
    }

    key_binding_activate(binding, keyboard, key);
    binding->active = true;
    if (binding->options & KEY_BINDING_TRIGGER_IN_ACTIVE) {
        if (binding->action) {
            binding->action(binding, binding->data, keyboard->seat, KEY_BINDING_STAGE_ACTIVE);
        }
        if (keyboard->binding_state.binding != binding) {
            return true;
        }
        state->active_emitted = true;
    }

    if (keyboard->binding_state.binding != binding) {
        return true;
    }

    *repeat = !bindings_get_key_binding_no_repeat(binding);

    return true;
}

bool bindings_handle_key_binding_modifiers(struct keyboard *keyboard, uint32_t modifiers)
{
    struct keyboard_binding_state *state = &keyboard->binding_state;
    struct key_binding *binding = state->binding;

    if (!binding || !(binding->modifiers & ~modifiers)) {
        return false;
    }

    keyboard_binding_finish(keyboard, true);

    return true;
}

bool bindings_cancel_key_binding(struct keyboard *keyboard, uint32_t key, bool pressed)
{
    struct keyboard_binding_state *state = &keyboard->binding_state;
    if (!state->binding) {
        return false;
    }

    bool emit_inactive = state->key == key && !pressed;
    if (state->active_emitted) {
        emit_inactive = true;
    }

    return keyboard_binding_finish(keyboard, emit_inactive);
}

void bindings_clear_key_binding(struct keyboard *keyboard)
{
    keyboard_binding_finish(keyboard, false);
}

static void gesture_binding_finish(struct gesture_state *gesture_state)
{
    if (!gesture_state->active_binding) {
        return;
    }

    wl_list_remove(&gesture_state->active_binding_cancel.link);
    wl_list_init(&gesture_state->active_binding_cancel.link);
    gesture_state->active_binding = NULL;
    gesture_state->active_binding_cancel.notify = NULL;
}

static void gesture_binding_handle_cancel(struct wl_listener *listener, void *data)
{
    struct gesture_state *gesture_state =
        wl_container_of(listener, gesture_state, active_binding_cancel);
    struct gesture_binding *binding = data;

    if (gesture_state->active_binding == binding) {
        gesture_binding_finish(gesture_state);
    }
}

static void gesture_binding_activate(struct gesture_binding *binding,
                                     struct gesture_state *gesture_state)
{
    assert(!gesture_state->active_binding);

    gesture_state->active_binding = binding;
    gesture_state->active_binding_cancel.notify = gesture_binding_handle_cancel;
    wl_signal_add(&binding->events.cancel, &gesture_state->active_binding_cancel);
}

void bindings_clear_gesture_binding(struct gesture_state *gesture_state)
{
    gesture_binding_finish(gesture_state);
}

void kywc_gesture_binding_destroy(struct gesture_binding *binding)
{
    wl_signal_emit_mutable(&binding->events.cancel, binding);
    wl_list_remove(&binding->link);
    free(binding->desc);
    free(binding);
}

static bool gesture_binding_is_valid(struct gesture_binding *binding, uint32_t gesture_id)
{
    struct gesture_binding *bind;
    wl_list_for_each(bind, &bindings->gesture_bindings, link) {
        /* skip itself */
        if (bind == binding) {
            continue;
        }
        if (bind->id == gesture_id) {
            return false;
        }
    }

    return true;
}

static uint32_t gesture_string_parse_directions(const char *str)
{
    uint32_t directions = GESTURE_DIRECTION_NONE;

    size_t len = 0;
    char **split_str = string_split(str, "+", &len);

    for (size_t i = 0; i < len; i++) {
        if (strcmp(split_str[i], "none") == 0) {
            directions |= GESTURE_DIRECTION_NONE;
        } else if (strcmp(split_str[i], "up") == 0) {
            directions |= GESTURE_DIRECTION_UP;
        } else if (strcmp(split_str[i], "down") == 0) {
            directions |= GESTURE_DIRECTION_DOWN;
        } else if (strcmp(split_str[i], "left") == 0) {
            directions |= GESTURE_DIRECTION_LEFT;
        } else if (strcmp(split_str[i], "right") == 0) {
            directions |= GESTURE_DIRECTION_RIGHT;
        } else if (strcmp(split_str[i], "inward") == 0) {
            directions |= GESTURE_DIRECTION_INWARD;
        } else if (strcmp(split_str[i], "outward") == 0) {
            directions |= GESTURE_DIRECTION_OUTWARD;
        } else if (strcmp(split_str[i], "clockwise") == 0) {
            directions |= GESTURE_DIRECTION_CLOCKWISE;
        } else if (strcmp(split_str[i], "counterclockwise") == 0) {
            directions |= GESTURE_DIRECTION_COUNTERCLOCKWISE;
        } else {
            kywc_log(KYWC_WARN, "Expected directions, got %s", str);
        }
    }

    string_free_split(split_str);
    return directions;
}

static uint32_t gesture_string_parse_edge(const char *str)
{
    uint32_t edge = GESTURE_EDGE_NONE;

    if (strcmp(str, "none") == 0) {
        edge = GESTURE_EDGE_NONE;
    } else if (strcmp(str, "top") == 0) {
        edge = GESTURE_EDGE_TOP;
    } else if (strcmp(str, "bottom") == 0) {
        edge = GESTURE_EDGE_BOTTOM;
    } else if (strcmp(str, "left") == 0) {
        edge = GESTURE_EDGE_LEFT;
    } else if (strcmp(str, "right") == 0) {
        edge = GESTURE_EDGE_RIGHT;
    } else {
        kywc_log(KYWC_WARN, "Expected edge, got %s", str);
    }

    return edge;
}

static uint32_t gesture_string_parse_stages(const char *str)
{
    uint32_t stages = GESTURE_STAGE_NONE;

    size_t len = 0;
    char **split_str = string_split(str, "+", &len);

    for (size_t i = 0; i < len; i++) {
        if (strcmp(split_str[i], "none") == 0) {
            stages |= GESTURE_STAGE_NONE;
        } else if (strcmp(split_str[i], "before") == 0) {
            stages |= GESTURE_STAGE_BEFORE;
        } else if (strcmp(split_str[i], "trigger") == 0) {
            stages |= GESTURE_STAGE_TRIGGER;
        } else if (strcmp(split_str[i], "after") == 0) {
            stages |= GESTURE_STAGE_AFTER;
        } else if (strcmp(split_str[i], "stop") == 0) {
            stages |= GESTURE_STAGE_STOP;
        } else {
            kywc_log(KYWC_WARN, "Expected stages, got %s", str);
        }
    }

    string_free_split(split_str);
    return stages;
}

struct gesture_binding *kywc_gesture_binding_create_by_string(const char *gesture, const char *desc,
                                                              enum function_id func_id)
{
    enum gesture_type type;
    uint8_t fingers;
    enum gesture_device device;
    enum gesture_direction direction;
    enum gesture_edge edge = GESTURE_EDGE_NONE;
    uint32_t stages = GESTURE_STAGE_TRIGGER;
    double follow_threshold = 0.0;
    uint32_t follow_directions = GESTURE_DIRECTION_NONE;

    size_t len = 0;
    char **split_str = string_split(gesture, ":", &len);
    if (len != 4 && len != 5 && len != 6 && len != 7 && len != 8) {
        kywc_log(KYWC_ERROR,
                 "Expected <gesture>:<device>:<fingers>:<direction>"
                 "[:edge][:stages][:follow_threshold][:follow_directions],"
                 " got %s",
                 gesture);
        goto err;
    }

    // type
    if (strcmp(split_str[0], "tap") == 0) {
#if HAVE_LIBINPUT_RAW_TAP
        type = GESTURE_TYPE_TAP;
#else
        type = GESTURE_TYPE_HOLD;
#endif
    } else if (strcmp(split_str[0], "hold") == 0) {
        type = GESTURE_TYPE_HOLD;
    } else if (strcmp(split_str[0], "pinch") == 0) {
        type = GESTURE_TYPE_PINCH;
    } else if (strcmp(split_str[0], "swipe") == 0) {
        type = GESTURE_TYPE_SWIPE;
    } else {
        kywc_log(KYWC_ERROR, "Expected hold|pinch|swipe, got %s", gesture);
        goto err;
    }

    // device
    if (strcmp(split_str[1], "touch") == 0) {
        device = GESTURE_DEVICE_TOUCHSCREEN;
    } else if (strcmp(split_str[1], "touchpad") == 0) {
        device = GESTURE_DEVICE_TOUCHPAD;
    } else {
        kywc_log(KYWC_ERROR, "Expected touch|touchpad, got %s", gesture);
        goto err;
    }

    /* fingers: 1 - 9 */
    /* directions: up down left right inward outward clockwise counterclockwise */
    if ('1' <= split_str[2][0] && split_str[2][0] <= '9') {
        fingers = atoi(split_str[2]);
        direction = gesture_string_parse_directions(split_str[3]);
    } else {
        kywc_log(KYWC_ERROR, "Expected 1 - 9, got %s", gesture);
        goto err;
    }

    /* edge: top bottom left right */
    /* stage: before trigger after stop */
    switch (len) {
    case 8:
        follow_directions = gesture_string_parse_directions(split_str[7]);
        // fallthrough
    case 7:
        follow_threshold = atof(split_str[6]);
        // fallthrough
    case 6:
        stages = gesture_string_parse_stages(split_str[5]);
        // fallthrough
    case 5:
        edge = gesture_string_parse_edge(split_str[4]);
        break;
    }

    string_free_split(split_str);

    kywc_log(KYWC_DEBUG, "Gesture binding: %s", gesture);
    return kywc_gesture_binding_create(type, stages, device, direction, edge, fingers,
                                       follow_directions, follow_threshold, desc, func_id);

err:
    string_free_split(split_str);
    return NULL;
}

static bool gesture_checked(enum gesture_type type, enum gesture_device device,
                            enum gesture_direction direction, enum gesture_edge edge,
                            uint8_t fingers)
{
    /* gesture type、devices and fingers cannot be 0 */
    if (type == GESTURE_TYPE_NONE || device == GESTURE_DEVICE_NONE || fingers == 0) {
        return false;
    }

    /* pinch gesture fingers require no less than 2 */
    if (type == GESTURE_TYPE_PINCH && fingers < 2) {
        return false;
    }

    /* pinch gestures edges need to be 0 */
    if (type == GESTURE_TYPE_PINCH && edge != GESTURE_EDGE_NONE) {
        return false;
    }

    /* the number of fingers for touchpad gestures need to be greater than 2 */
    if (type == GESTURE_TYPE_TAP && device & GESTURE_DEVICE_TOUCHPAD && fingers < 3) {
        return false;
    }

    /* ordinary touchpad swipe gestures need to be greater than 2 */
    if (type == GESTURE_TYPE_SWIPE && device == GESTURE_DEVICE_TOUCHPAD &&
        edge == GESTURE_EDGE_NONE && fingers < 3) {
        return false;
    }

    /* the edge of the swipe gesture for one finger of the touchscreen cannot be 0 */
    if (type == GESTURE_TYPE_SWIPE && edge == GESTURE_EDGE_NONE && fingers == 1) {
        return false;
    }

    /* touchscreen swipe gesture from edge, the start edges and directions need to be set properly
     */
    if (device == GESTURE_DEVICE_TOUCHSCREEN && edge != GESTURE_EDGE_NONE &&
        ((direction == GESTURE_DIRECTION_LEFT && (edge != GESTURE_EDGE_RIGHT)) ||
         (direction == GESTURE_DIRECTION_RIGHT && (edge != GESTURE_EDGE_LEFT)) ||
         (direction == GESTURE_DIRECTION_UP && (edge != GESTURE_EDGE_BOTTOM)) ||
         (direction == GESTURE_DIRECTION_DOWN && (edge != GESTURE_EDGE_TOP)))) {
        return false;
    }

    return true;
}

static uint32_t generate_gesture_id(enum gesture_device device, uint8_t fingers,
                                    enum gesture_type type, enum gesture_direction direction,
                                    enum gesture_edge edge)
{
    uint32_t gesture_id = (device << 20) & 0x0F00000;
    gesture_id |= (fingers << 16) & 0xF0000;
    gesture_id |= (type << 12) & 0x0F000;
    gesture_id |= (direction << 4) & 0x0FF0;
    gesture_id |= edge & 0x0F;

    return gesture_id;
}

enum function_id kywc_gesture_binding_function_id(struct gesture_binding *binding)
{
    return binding->func_id;
}

enum gesture_direction kywc_gesture_binding_direction(struct gesture_binding *binding)
{
    return binding->direction;
}

const char *kywc_gesture_binding_desc(struct gesture_binding *binding)
{
    return binding->desc;
}

struct gesture_binding *
kywc_gesture_binding_create(enum gesture_type type, uint32_t stages, enum gesture_device device,
                            enum gesture_direction direction, enum gesture_edge edge,
                            uint8_t fingers, uint32_t follow_directions, double follow_threshold,
                            const char *desc, enum function_id func_id)
{
    if (!gesture_checked(type, device, direction, edge, fingers)) {
        kywc_log(KYWC_ERROR, "Gesture checks are illega");
        return NULL;
    }

    struct gesture_binding *binding = calloc(1, sizeof(*binding));
    if (!binding) {
        return NULL;
    }

    binding->enabled = true;
    binding->func_id = func_id;
    binding->id = generate_gesture_id(device, fingers, type, direction, edge);
    binding->type = type;
    binding->stages = stages;
    binding->device = device;
    binding->direction = direction;
    binding->fingers = fingers;
    binding->edge = edge;
    binding->follow_threshold = follow_threshold > 0.0 ? follow_threshold : 0.0;
    binding->follow_directions = follow_directions;
    if (desc) {
        binding->desc = strdup(desc);
    }
    wl_list_init(&binding->link);
    wl_signal_init(&binding->events.cancel);

    return binding;
}

bool kywc_gesture_binding_register(struct gesture_binding *binding,
                                   void (*action)(struct gesture_binding *binding, void *data,
                                                  struct seat *seat, enum gesture_stage stage,
                                                  enum gesture_device device,
                                                  uint32_t follow_directions, double dx, double dy),
                                   void *data)
{
    if (!wl_list_empty(&binding->link)) {
        return true;
    }
    if (!gesture_binding_is_valid(binding, binding->id)) {
        return false;
    }

    wl_list_insert(&bindings->gesture_bindings, &binding->link);
    binding->action = action;
    binding->data = data;

    return true;
}

bool bindings_handle_gesture_binding(struct gesture_state *gesture_state)
{
    struct gesture_binding *binding;
    wl_list_for_each(binding, &bindings->gesture_bindings, link) {
        if (gesture_state->active_binding && binding != gesture_state->active_binding) {
            continue;
        }
        if (!binding->enabled) {
            continue;
        }
        if (gesture_state->device != binding->device) {
            continue;
        }
        if (gesture_state->type != binding->type) {
            continue;
        }
        if (gesture_state->fingers != binding->fingers) {
            continue;
        }
        if (!(gesture_state->stage & binding->stages)) {
            continue;
        }
        if (binding->edge != GESTURE_EDGE_NONE && binding->edge != gesture_state->edge) {
            continue;
        }

        if (gesture_state->stage == GESTURE_STAGE_BEFORE &&
            gesture_state->follow_direction != binding->direction) {
            continue;
        }
        if (!gesture_state->active_binding && gesture_state->stage != GESTURE_STAGE_BEFORE &&
            binding->direction != GESTURE_DIRECTION_NONE && gesture_state->directions &&
            !(gesture_state->directions & binding->direction)) {
            continue;
        }
        if (gesture_state->stage == GESTURE_STAGE_AFTER &&
            !(binding->follow_directions
                  ? (gesture_state->follow_direction & binding->follow_directions)
                  : (gesture_state->follow_direction == binding->direction))) {
            continue;
        }

        /* do not trigger follow if interval < follow_threshold */
        if (gesture_state->stage == GESTURE_STAGE_BEFORE ||
            gesture_state->stage == GESTURE_STAGE_AFTER) {
            if (gesture_state->follow_dx > -binding->follow_threshold &&
                gesture_state->follow_dx < binding->follow_threshold &&
                gesture_state->follow_dy > -binding->follow_threshold &&
                gesture_state->follow_dy < binding->follow_threshold) {
                continue;
            }
        }

        kywc_log(KYWC_DEBUG,
                 "Start gesture [type:%d] [fingers:%d] [stage:%d] [direction:%d] binding: %s",
                 gesture_state->type, gesture_state->fingers, gesture_state->stage,
                 gesture_state->directions, binding->desc);
        if (!gesture_state->active_binding && (gesture_state->stage == GESTURE_STAGE_BEFORE ||
                                               gesture_state->stage == GESTURE_STAGE_TRIGGER)) {
            gesture_binding_activate(binding, gesture_state);
        }
        if (binding->action) {
            binding->action(binding, binding->data, gesture_state->seat, gesture_state->stage,
                            gesture_state->device, gesture_state->follow_direction,
                            gesture_state->dx, gesture_state->dy);
        }
        return true;
    }
    return false;
}

void kywc_gesture_binding_for_each(gesture_binding_iterator_func_t iterator)
{
    struct gesture_binding *binding;
    wl_list_for_each(binding, &bindings->gesture_bindings, link) {
        if (iterator(binding, binding->id, binding->desc)) {
            return;
        }
    }
}

void kywc_gesture_binding_update(struct gesture_binding *binding, bool enabled,
                                 enum function_id func_id, uint32_t stages,
                                 uint32_t follow_directions, double follow_threshold)
{
    if (binding->enabled != enabled) {
        binding->enabled = enabled;
    }

    if (!binding->enabled) {
        return;
    }

    if (binding->func_id != func_id) {
        binding->func_id = func_id;
    }

    if (binding->stages != stages) {
        binding->stages = stages;
    }

    if (binding->follow_directions != follow_directions) {
        binding->follow_directions = follow_directions;
    }

    if (binding->follow_threshold != follow_threshold) {
        binding->follow_threshold = follow_threshold;
    }
}

bool kywc_gesture_binding_check(enum gesture_type type, enum gesture_device device, uint8_t fingers)
{
    struct gesture_binding *binding;
    wl_list_for_each(binding, &bindings->gesture_bindings, link) {
        if (binding->type == type && binding->device == device && binding->fingers == fingers) {
            return binding->enabled;
        }
    }
    return false;
}

enum gesture_edge kywc_gesture_binding_check_edge(enum gesture_device device)
{
    enum gesture_edge edges = GESTURE_EDGE_NONE;
    struct gesture_binding *binding;
    wl_list_for_each(binding, &bindings->gesture_bindings, link) {
        if (binding->enabled && binding->device == device) {
            edges |= binding->edge;
        }
    }
    return edges;
}

void kywc_key_binding_for_each(binding_iterator_func_t iterator)
{
    for (size_t i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
        struct key_binding *binding;
        struct keysyms_binding *keysyms_binding = &bindings->keysyms_binding[i];
        wl_list_for_each(binding, &keysyms_binding->bindings, link) {
            if (iterator(binding, binding->keybind, binding->desc, binding->modifiers,
                         binding->keysym)) {
                return;
            }
        }
    }
}

void kywc_key_binding_block_all(bool block)
{
    if (block) {
        bindings->keysym_bindings_block++;
        if (bindings->keysym_bindings_block == 1) {
            for (size_t i = 0; i < KEY_BINDING_TYPE_NUM; ++i) {
                keysyms_binding_cancel_states(&bindings->keysyms_binding[i], true);
            }
        }
    } else {
        assert(bindings->keysym_bindings_block > 0);
        bindings->keysym_bindings_block--;
    }
}

void kywc_key_binding_block_type(enum key_binding_type type, bool block)
{
    if (block) {
        bindings->keysyms_binding_masks &= ~(1 << type);
        bindings->type_nlocks[type]++;
        if (bindings->type_nlocks[type] == 1) {
            keysyms_binding_cancel_states(&bindings->keysyms_binding[type], true);
        }
    } else {
        assert(bindings->type_nlocks[type] > 0);
        bindings->type_nlocks[type]--;
        if (!bindings->type_nlocks[type]) {
            bindings->keysyms_binding_masks |= (1 << type);
        }
    }
}

void kywc_key_binding_block(struct key_binding *binding, bool block)
{
    if (block) {
        binding->nblocks++;
        if (binding->nblocks == 1) {
            key_binding_cancel_states(binding, true);
        }
    } else {
        assert(binding->nblocks > 0);
        binding->nblocks--;
    }
}

enum key_binding_type kywc_key_binding_type_by_name(const char *name, bool *found)
{
    for (size_t i = 0; i < ARRAY_SIZE(key_binding_table); ++i) {
        if (strcmp(name, key_binding_table[i].name)) {
            continue;
        }

        if (found) {
            *found = true;
        }
        return key_binding_table[i].type;
    }

    if (found) {
        *found = false;
    }
    return KEY_BINDING_TYPE_CUSTOM_DEF;
}
