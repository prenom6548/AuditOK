// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _KYWC_BINDING_H_
#define _KYWC_BINDING_H_

#include <stdbool.h>
#include <stdint.h>

#include "input/function.h"

struct seat;
struct wl_listener;

enum gesture_type {
    GESTURE_TYPE_NONE = 0,
    GESTURE_TYPE_TAP,
    GESTURE_TYPE_PINCH,
    GESTURE_TYPE_SWIPE,
    GESTURE_TYPE_HOLD,
};

enum gesture_device {
    GESTURE_DEVICE_NONE = -1,
    GESTURE_DEVICE_TOUCHPAD = 0,
    GESTURE_DEVICE_TOUCHSCREEN,
    GESTURE_DEVICE_COUNT,
};

#define GESTURE_DEVICE_BIT(device) (1u << (device))
#define GESTURE_ID_EDGE_SHIFT 0
#define GESTURE_ID_EDGE_MASK 0x0F
#define GESTURE_ID_DIRECTION_SHIFT 4
#define GESTURE_ID_DIRECTION_MASK 0xFF
#define GESTURE_ID_TYPE_SHIFT 12
#define GESTURE_ID_TYPE_MASK 0x0F
#define GESTURE_ID_FINGERS_SHIFT 16
#define GESTURE_ID_FINGERS_MASK 0x0F
#define GESTURE_ID_DEVICE_SHIFT 20
#define GESTURE_ID_DEVICE_MASK 0x0F

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

struct gesture_binding_info {
    uint32_t id;
    const char *desc;
    enum function_id func_id;

    enum gesture_type type;
    enum gesture_device device;
    enum gesture_direction direction;
    enum gesture_edge edge;
    uint8_t fingers;

    uint32_t stages;
    uint32_t follow_directions;
    double follow_threshold;
};

struct gesture_binding *kywc_gesture_binding_create_by_string(const char *gestures,
                                                              const char *desc,
                                                              enum function_id func_id);

struct gesture_binding *
kywc_gesture_binding_create(enum gesture_type type, uint32_t stages, enum gesture_device device,
                            enum gesture_direction direction, enum gesture_edge edge,
                            uint8_t fingers, uint32_t follow_directions, double follow_threshold,
                            const char *desc, enum function_id func_id);

void kywc_gesture_binding_destroy(struct gesture_binding *binding);

void kywc_gesture_binding_add_destroy_listener(struct gesture_binding *binding,
                                               struct wl_listener *listener);

void kywc_gesture_binding_add_registered_listener(struct wl_listener *listener);

bool kywc_gesture_binding_register(struct gesture_binding *binding,
                                   void (*action)(struct gesture_binding *binding, void *data,
                                                  struct seat *seat, enum gesture_stage stage,
                                                  enum gesture_device device,
                                                  uint32_t follow_directions, double dx, double dy),
                                   void *data);

/* Info belongs to binding and may change. Do not use it after binding is destroyed. */
const struct gesture_binding_info *
kywc_gesture_binding_get_info(const struct gesture_binding *binding);

void kywc_gesture_binding_update(struct gesture_binding *binding, bool enabled,
                                 enum function_id func_id, uint32_t stages,
                                 uint32_t follow_directions, double follow_threshold);

bool kywc_gesture_binding_check(enum gesture_type type, enum gesture_device device,
                                uint8_t fingers);

enum gesture_edge kywc_gesture_binding_check_edge(enum gesture_device device);

bool kywc_gesture_binding_device_available(enum gesture_device device);

void kywc_gesture_binding_block(struct gesture_binding *binding, bool block);

void kywc_gesture_binding_block_device(enum gesture_device device, bool block);

/**
 * keysym bindings
 */

struct key_binding_info {
    const char *keybind;
    const char *desc;
    enum function_id func_id;
    uint32_t modifiers;
    uint32_t keysym;
    uint32_t options;
};

struct key_binding *kywc_key_binding_create(const char *keybind, const char *desc,
                                            enum function_id func_id, uint32_t options);

struct key_binding *kywc_key_binding_create_by_symbol(unsigned int keysym, unsigned int modifiers,
                                                      const char *desc, enum function_id func_id,
                                                      uint32_t options);

void kywc_key_binding_destroy(struct key_binding *binding);

void kywc_key_binding_add_destroy_listener(struct key_binding *binding,
                                           struct wl_listener *listener);

void kywc_key_binding_add_registered_listener(struct wl_listener *listener);

bool kywc_key_binding_register(struct key_binding *binding, enum key_binding_type type,
                               void (*action)(struct key_binding *binding, void *data,
                                              struct seat *seat, enum key_binding_stage stage),
                               void *data);

bool kywc_key_binding_update(struct key_binding *binding, unsigned int keysym,
                             unsigned int modifiers);

bool kywc_key_binding_update_state(struct key_binding *binding, enum function_id func_id,
                                   uint32_t options);

void kywc_key_binding_unregister(struct key_binding *binding);

/* Info belongs to binding and may change. Do not use it after binding is destroyed. */
const struct key_binding_info *kywc_key_binding_get_info(const struct key_binding *binding);

typedef bool (*key_binding_iterator_func_t)(struct key_binding *binding);

void kywc_key_binding_for_each(key_binding_iterator_func_t iterator);

void kywc_key_binding_block_all(bool block);

void kywc_key_binding_block_type(enum key_binding_type type, bool block);

void kywc_key_binding_block(struct key_binding *binding, bool block);

enum key_binding_type kywc_key_binding_type_by_name(const char *name, bool *found);

#endif /* _KYWC_BINDING_H_ */
