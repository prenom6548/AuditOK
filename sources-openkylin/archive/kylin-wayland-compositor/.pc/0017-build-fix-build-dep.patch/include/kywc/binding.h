// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _KYWC_BINDING_H_
#define _KYWC_BINDING_H_

#include <stdbool.h>
#include <stdint.h>

#include "input/function.h"

struct seat;

enum gesture_type {
    GESTURE_TYPE_NONE = 0,
    GESTURE_TYPE_TAP,
    GESTURE_TYPE_PINCH,
    GESTURE_TYPE_SWIPE,
    GESTURE_TYPE_HOLD,
};

enum gesture_device {
    GESTURE_DEVICE_NONE = 0,
    GESTURE_DEVICE_TOUCHPAD = 1 << 0,
    GESTURE_DEVICE_TOUCHSCREEN = 1 << 1,
};

enum gesture_direction {
    GESTURE_DIRECTION_NONE = 0,
    // Directions based on delta x and y
    GESTURE_DIRECTION_UP = 1 << 0,
    GESTURE_DIRECTION_DOWN = 1 << 1,
    GESTURE_DIRECTION_LEFT = 1 << 2,
    GESTURE_DIRECTION_RIGHT = 1 << 3,
    // Directions based on scale
    GESTURE_DIRECTION_INWARD = 1 << 4,
    GESTURE_DIRECTION_OUTWARD = 1 << 5,
    // Directions based on rotation
    GESTURE_DIRECTION_CLOCKWISE = 1 << 6,
    GESTURE_DIRECTION_COUNTERCLOCKWISE = 1 << 7,
};

enum gesture_edge {
    GESTURE_EDGE_NONE = 0,
    GESTURE_EDGE_TOP = 1 << 0,
    GESTURE_EDGE_BOTTOM = 1 << 1,
    GESTURE_EDGE_LEFT = 1 << 2,
    GESTURE_EDGE_RIGHT = 1 << 3,
};

/**
 * 4 stages:
 * 1、before triggered
 * 2、trigger
 * 3、after triggered
 * 4、stop
 */
enum gesture_stage {
    GESTURE_STAGE_NONE = 0,
    GESTURE_STAGE_BEFORE = 1 << 0,
    GESTURE_STAGE_TRIGGER = 1 << 1,
    GESTURE_STAGE_AFTER = 1 << 2,
    GESTURE_STAGE_STOP = 1 << 3,
};

/* key binding type */
enum key_binding_type {
    KEY_BINDING_TYPE_CUSTOM_DEF = 0,
    KEY_BINDING_TYPE_CUSTOM_CONFIG,
    KEY_BINDING_TYPE_WIN_MENU,
    KEY_BINDING_TYPE_SWITCH_WORKSPACE,
    KEY_BINDING_TYPE_WINDOW_ACTION_MINIMIZE,
    KEY_BINDING_TYPE_WINDOW_ACTION_MAXIMIZE,
    KEY_BINDING_TYPE_WINDOW_ACTION_CLOSE,
    KEY_BINDING_TYPE_WINDOW_ACTION_MENU,
    KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
    KEY_BINDING_TYPE_WINDOW_ACTION_OUTPUT,
    KEY_BINDING_TYPE_WINDOW_ACTION_SEND,
    KEY_BINDING_TYPE_WINDOW_ACTION_CAPTURE,
    KEY_BINDING_TYPE_MAXIMIZED_VIEWS,
    KEY_BINDING_TYPE_TOGGLE_SHOW_DESKTOP,
    KEY_BINDING_TYPE_SHOW_DESKTOP,
    KEY_BINDING_TYPE_RESTORE_DESKTOP,
    KEY_BINDING_TYPE_TOGGLE_SHOW_VIEWS,
    KEY_BINDING_TYPE_TOGGLE_SHOW_WINDOWS,
    KEY_BINDING_TYPE_COLOR_FILTER,
    KEY_BINDING_TYPE_ZOOM,
    KEY_BINDING_TYPE_NUM,
};

enum key_binding_stage {
    KEY_BINDING_STAGE_ACTIVE,
    KEY_BINDING_STAGE_REPEAT,
    KEY_BINDING_STAGE_INACTIVE,
};

enum key_binding_option {
    KEY_BINDING_TRIGGER_IN_ACTIVE = 1 << 0,
    KEY_BINDING_TRIGGER_IN_INACTIVE = 1 << 1,
    KEY_BINDING_TRIGGER_IN_REPEAT = 1 << 2,
    KEY_BINDING_BYPASS_GRAB = 1 << 3,
};

/**
 * gesture bindings
 */

struct gesture_binding *kywc_gesture_binding_create_by_string(const char *gestures,
                                                              const char *desc,
                                                              enum function_id func_id);

struct gesture_binding *
kywc_gesture_binding_create(enum gesture_type type, uint32_t stages, enum gesture_device device,
                            enum gesture_direction direction, enum gesture_edge edge,
                            uint8_t fingers, uint32_t follow_directions, double follow_threshold,
                            const char *desc, enum function_id func_id);

void kywc_gesture_binding_destroy(struct gesture_binding *binding);

bool kywc_gesture_binding_register(struct gesture_binding *binding,
                                   void (*action)(struct gesture_binding *binding, void *data,
                                                  struct seat *seat, enum gesture_stage stage,
                                                  enum gesture_device device,
                                                  uint32_t follow_directions, double dx, double dy),
                                   void *data);

enum function_id kywc_gesture_binding_function_id(struct gesture_binding *binding);

enum gesture_direction kywc_gesture_binding_direction(struct gesture_binding *binding);

const char *kywc_gesture_binding_desc(struct gesture_binding *binding);

typedef bool (*gesture_binding_iterator_func_t)(struct gesture_binding *binding,
                                                uint32_t gesture_id, const char *desc);

void kywc_gesture_binding_for_each(gesture_binding_iterator_func_t iterator);

void kywc_gesture_binding_update(struct gesture_binding *binding, bool enabled,
                                 enum function_id func_id, uint32_t stages,
                                 uint32_t follow_directions, double follow_threshold);

bool kywc_gesture_binding_check(enum gesture_type type, enum gesture_device device,
                                uint8_t fingers);

enum gesture_edge kywc_gesture_binding_check_edge(enum gesture_device device);

void kywc_gesture_binding_block(struct gesture_binding *binding, bool block);

/**
 * keysym bindings
 */

struct key_binding *kywc_key_binding_create(const char *keybind, const char *desc,
                                            enum function_id func_id, uint32_t options);

struct key_binding *kywc_key_binding_create_by_symbol(unsigned int keysym, unsigned int modifiers,
                                                      const char *desc, enum function_id func_id,
                                                      uint32_t options);

void kywc_key_binding_destroy(struct key_binding *binding);

bool kywc_key_binding_register(struct key_binding *binding, enum key_binding_type type,
                               void (*action)(struct key_binding *binding, void *data,
                                              struct seat *seat, enum key_binding_stage stage),
                               void *data);

bool kywc_key_binding_update(struct key_binding *binding, unsigned int keysym,
                             unsigned int modifiers);

bool kywc_key_binding_update_state(struct key_binding *binding, enum function_id func_id,
                                   uint32_t options);

void kywc_key_binding_unregister(struct key_binding *binding);

bool kywc_key_binding_is_registered(struct key_binding *binding);

typedef bool (*binding_iterator_func_t)(struct key_binding *binding, char *keybind, char *desc,
                                        int32_t modifiers, int32_t key);

void kywc_key_binding_for_each(binding_iterator_func_t iterator);

void kywc_key_binding_block_all(bool block);

void kywc_key_binding_block_type(enum key_binding_type type, bool block);

void kywc_key_binding_block(struct key_binding *binding, bool block);

enum key_binding_type kywc_key_binding_type_by_name(const char *name, bool *found);

#endif /* _KYWC_BINDING_H_ */
