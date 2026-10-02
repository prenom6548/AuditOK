// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _INPUT_P_H_
#define _INPUT_P_H_

#include "input/cursor.h"
#include "input/gesture.h"
#include "input/keyboard.h"
#include "input/seat.h"
#include "util/queue.h"

enum input_lock_key {
    INPUT_KEY_LOCK_NONE = 0,
    INPUT_KEY_CAPSLOCK = 1 << 0,
    INPUT_KEY_NUMLOCK = 1 << 1,
    INPUT_KEY_SCROLLLOCK = 1 << 2,
    INPUT_KEY_LOCK_ALL = (1 << 3) - 1,
};

enum keyboard_lock_mode {
    LOCK_MODE_KEY_NONE = 0,
    LOCK_MODE_KEY_ALL,
    LOCK_MODE_KEY_CUSTOM,
};

struct input_keymap {
    struct wl_list link;
    struct xkb_keymap *keymap;
    struct keymap_rules rules;
};

struct input_manager {
    struct server *server;

    struct wl_list seats;
    struct wl_list inputs;

    struct wl_list keymaps;
    struct queue_fence fence;

    struct {
        struct wl_signal new_input;
        struct wl_signal new_seat;
    } events;

    struct seat *default_seat;
    void (*bind_seat)(struct input *input, const char *seat);

    event_filter_func_t event_filter;
    void *event_filter_data;

    struct config *config;
    struct config *seat_config;

    struct wl_listener new_input;
    struct wl_listener backend_destroy;
    struct wl_listener server_destroy;

    struct wlr_pointer_gestures_v1 *pointer_gestures;
    struct wlr_relative_pointer_manager_v1 *relative_pointer;

    struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard;
    struct wl_listener new_virtual_keyboard;

    struct wlr_virtual_pointer_manager_v1 *virtual_pointer;
    struct wl_listener new_virtual_pointer;

    struct wlr_keyboard_shortcuts_inhibit_manager_v1 *shortcuts_inhibit;
    struct wl_listener new_shortcuts_inhibit;

    struct wlr_pointer_constraints_v1 *pointer_constraints;
    struct wl_listener new_pointer_constraint;

    struct wlr_cursor_shape_manager_v1 *cursor_shape;
    struct wl_listener request_set_cursor_shape;
};

bool input_manager_config_init(struct input_manager *input_manager);

bool input_read_config(struct input *input, struct input_state *state);

void input_write_config(struct input *input);

void input_prop_and_state_debug(struct input *input);

void input_manager_switch_vt(unsigned vt);

void input_notify_destroy(struct input *input);

void input_notify_create(struct input *input);

bool input_event_filter(struct seat *seat, struct input *input, enum input_event_type type,
                        void *event);

struct xkb_keymap *input_get_or_create_keymap(struct keymap_rules *rules, bool wait);

struct seat *seat_by_name(const char *seat_name);

void cursor_set_xcursor_manager(struct cursor *cursor, const char *theme, uint32_t size,
                                bool saved);

void cursor_set_surface(struct cursor *cursor, struct wlr_surface *surface, int32_t hotspot_x,
                        int32_t hotspot_y, struct wl_client *client);

bool seat_read_config(struct seat *seat);

void seat_write_config(struct seat *seat);

void seat_feed_keyboard_key(struct seat *seat, uint32_t key, bool pressed);

/**
 * libinput helper functions
 */
void libinput_get_prop(struct input *input, struct input_prop *prop);

void libinput_get_state(struct input *input, struct input_state *state);

void libinput_get_default_state(struct input *input, struct input_state *state);

bool libinput_set_state(struct input *input, struct input_state *state);

/**
 * monitor for input cursor and others
 */
struct input_monitor *input_monitor_create(struct input_manager *input_manager);

/**
 * idle manager
 */

bool idle_manager_create(struct server *server);

void idle_manager_notify_activity(struct seat *seat);

/**
 * idle inhibitor manager
 */

bool idle_inhibit_manager_create(struct server *server);

/**
 * input method and text input
 */

bool input_method_manager_create(struct input_manager *input_manager);

void input_method_set_focus(struct seat *seat, struct wlr_surface *wlr_surface);

bool input_method_handle_key(struct keyboard *keyboard, uint32_t time, uint32_t key,
                             uint32_t state);

bool input_method_handle_modifiers(struct keyboard *keyboard);

bool keyboard_is_from_input_method(struct keyboard *keyboard);

void keyboard_repeat_stop(struct keyboard *keyboard);

/**
 * selection drag icon
 */

bool selection_manager_create(struct input_manager *input_manager);

void selection_handle_cursor_move(struct seat *seat, int lx, int ly);

/**
 * tablet manager
 */

struct wlr_tablet_tool_axis_event;
struct wlr_tablet_tool_proximity_event;
struct wlr_tablet_tool_tip_event;
struct wlr_tablet_tool_button_event;

bool tablet_manager_create(struct input_manager *input_manager);

void tablet_set_focus(struct seat *seat, struct wlr_surface *surface);

void tablet_handle_tool_axis(struct wlr_tablet_tool_axis_event *event);

bool tablet_handle_tool_proximity(struct wlr_tablet_tool_proximity_event *event);

bool tablet_handle_tool_tip(struct wlr_tablet_tool_tip_event *event);

bool tablet_handle_tool_button(struct wlr_tablet_tool_button_event *event);

bool tablet_has_implicit_grab(struct seat *seat);

/**
 * touchscreen manager
 */

struct wlr_touch_up_event;
struct wlr_touch_down_event;
struct wlr_touch_motion_event;
struct wlr_touch_cancel_event;

bool touch_manager_create(struct input_manager *input_manager);

bool touch_handle_down(struct wlr_touch_down_event *event);

void touch_handle_up(struct wlr_touch_up_event *event, bool handle);

void touch_handle_motion(struct wlr_touch_motion_event *event, bool handle);

void touch_handle_cancel(struct wlr_touch_cancel_event *event, bool handle);

void touch_reset_gesture(struct input_manager *input_manager);

void seat_cancel_touch_points(struct seat *seat, uint32_t retain_serial);

/**
 * binding manager for keysym, gesture
 */

bool bindings_create(struct input_manager *input_manager);

struct key_binding *bindings_get_key_binding(struct keyboard_state *keyboard_state,
                                             uint32_t trigger_options, bool modifier_only_released);

bool bindings_get_key_binding_bypass_grab(struct key_binding *binding);

bool bindings_get_key_binding_no_repeat(struct key_binding *binding);

enum function_id bindings_get_key_binding_function_id(struct key_binding *binding);

const char *bindings_get_key_binding_unique(struct key_binding *binding);

bool bindings_handle_key_binding(struct keyboard *keyboard, struct key_binding *binding,
                                 uint32_t key, bool pressed, bool *repeat);

bool bindings_handle_key_binding_modifiers(struct keyboard *keyboard, uint32_t modifiers);

bool bindings_cancel_key_binding(struct keyboard *keyboard, uint32_t key, bool pressed);

void bindings_clear_key_binding(struct keyboard *keyboard);

bool bindings_handle_gesture_binding(struct gesture_state *gesture_state);

void bindings_clear_gesture_binding(struct gesture_state *gesture_state);

/**
 * seat pointer and keyboard feed event
 */

void cursor_feed_motion(struct cursor *cursor, uint32_t time, struct wlr_input_device *device,
                        double dx, double dy, double dx_unaccel, double dy_unaccel);

void cursor_feed_button(struct cursor *cursor, uint32_t button, bool pressed, uint32_t time,
                        uint32_t double_click_time);

void cursor_feed_axis(struct cursor *cursor, uint32_t orientation, uint32_t source, double delta,
                      int32_t delta_discrete, uint32_t time);

/**
 * seat pointer constraint
 */

struct cursor_constraint *cursor_constraint_create(struct cursor *cursor,
                                                   struct wlr_pointer_constraint_v1 *constraint);

void cursor_constraint_set_focus(struct seat *seat, struct wlr_surface *surface);

/**
 * keeps track of the states of lock and modifier keys
 */

#if HAVE_KDE_KEYSTATE
bool kde_keystate_manager_create(struct input_manager *input_manager);
#else
static __attribute__((unused)) inline bool
kde_keystate_manager_create(struct input_manager *input_manager)
{
    return false;
}
#endif

#if HAVE_KDE_CLIPBOARD
bool kde_clipboard_manager_create(struct input_manager *input_manager);
#else
static __attribute__((unused)) inline bool
kde_clipboard_manager_create(struct input_manager *input_manager)
{
    return false;
}
#endif

bool transient_seat_manager_create(struct input_manager *input_manager);

/**
 * xdg toplevel drag support
 */

bool toplevel_drag_manager_create(struct server *server);

bool toplevel_drag_move(struct data_source *source, int lx, int ly);

bool data_device_manager_create(struct wl_display *display);

bool primary_selection_device_manager_create(struct wl_display *display);

bool wlr_data_control_manager_create(struct wl_display *display);

#endif /* _INPUT_P_H_ */
