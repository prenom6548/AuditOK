// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _INPUT_H_
#define _INPUT_H_

#include <linux/input-event-codes.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>

#include <kywc/binding.h>

#define DEFAULT_DOUBLE_CLICK_TIME (500)

struct server;

enum input_event_type {
    INPUT_EVENT_NONE = 0,

    INPUT_EVENT_KEYBOARD_KEY = 100, // struct wlr_keyboard_key_event
    INPUT_EVENT_KEYBOARD_MODIFIERS,

    INPUT_EVENT_POINTER_MOTION = 200,    // struct wlr_pointer_motion_event
    INPUT_EVENT_POINTER_MOTION_ABSOLUTE, // struct wlr_pointer_motion_absolute_event
    INPUT_EVENT_POINTER_BUTTON,          // struct wlr_pointer_button_event
    INPUT_EVENT_POINTER_AXIS,            // struct wlr_pointer_axis_event
    INPUT_EVENT_POINTER_FRAME,

    INPUT_EVENT_TOUCH_DOWN = 300, // struct wlr_touch_down_event
    INPUT_EVENT_TOUCH_UP,         // struct wlr_touch_up_event
    INPUT_EVENT_TOUCH_MOTION,     // struct wlr_touch_motion_event
    INPUT_EVENT_TOUCH_CANCEL,     // struct wlr_touch_cancel_event
    INPUT_EVENT_TOUCH_FRAME,

    INPUT_EVENT_TABLET_TOOL_AXIS = 400, // struct wlr_tablet_tool_axis_event
    INPUT_EVENT_TABLET_TOOL_PROXIMITY,  // struct wlr_tablet_tool_proximity_event
    INPUT_EVENT_TABLET_TOOL_TIP,        // struct wlr_tablet_tool_tip_event
    INPUT_EVENT_TABLET_TOOL_BUTTON,     // struct wlr_tablet_tool_button_event

    INPUT_EVENT_TABLET_PAD_BUTTON = 500, // struct wlr_tablet_pad_button_event
    INPUT_EVENT_TABLET_PAD_RING,         //  struct wlr_tablet_pad_ring_event
    INPUT_EVENT_TABLET_PAD_STRIP,        // struct wlr_tablet_pad_strip_event

    INPUT_EVENT_GESTURE_SWIPE_BEGIN = 600, // struct wlr_pointer_swipe_begin_event
    INPUT_EVENT_GESTURE_SWIPE_UPDATE,      // struct wlr_pointer_swipe_update_event
    INPUT_EVENT_GESTURE_SWIPE_END,         // struct wlr_pointer_swipe_end_event
    INPUT_EVENT_GESTURE_PINCH_BEGIN,       // struct wlr_pointer_pinch_begin_event
    INPUT_EVENT_GESTURE_PINCH_UPDATE,      // struct wlr_pointer_pinch_update_event
    INPUT_EVENT_GESTURE_PINCH_END,         // struct wlr_pointer_pinch_end_event
    INPUT_EVENT_GESTURE_HOLD_BEGIN,        // struct wlr_pointer_hold_begin_event
    INPUT_EVENT_GESTURE_HOLD_END,          // struct wlr_pointer_hold_end_event

    INPUT_EVENT_SWITCH_TOGGLE = 700, // struct wlr_switch_toggle_event
};

struct keymap_rules {
    const char *xkb_layout;
    const char *xkb_model;
    const char *xkb_options;
    const char *xkb_rules;
    const char *xkb_variant;
};

struct input_state {
    const char *mapped_to_output;
    const char *seat;

    uint32_t send_events_mode;
    uint32_t click_method;

    /* useful when prop.tap_finger_count > 0 */
    bool tap_to_click;
    bool tap_and_drag;
    bool tap_drag_lock;
    uint32_t tap_button_map; // LRM or LMR

    /* useful when prop.has_pointer_accel is true */
    float pointer_accel_speed; // [-1, 1]
    uint32_t accel_profile;

    bool natural_scroll;
    bool left_handed;
    /* When enabled, a simultaneous press of the left and right button generates a middle mouse
     * button event. Releasing the buttons generates a middle mouse button release, the left and
     * right button events are discarded otherwise. */
    bool middle_emulation;

    uint32_t scroll_method;
    /* when scroll_method is LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN */
    uint32_t scroll_button;
    bool scroll_button_lock;

    bool dwt;
    bool dwtp;

    uint32_t rotation_angle; // CW

    float calibration_matrix[6];

    double scroll_factor;
    uint32_t double_click_time;

    /* for keyboard */
    struct keymap_rules rules;
    int repeat_delay;
    int repeat_rate;
};

struct input_prop {
    enum wlr_input_device_type type;
    unsigned int vendor, product;
    bool is_virtual;

    union {
        struct {
            /* how many fingers can be used for tapping,
             * 0 if the device does not support tapping  */
            uint32_t tap_finger_count : 3;
            /* https://wayland.freedesktop.org/libinput/doc/latest/clickpad-softbuttons.html */
            uint32_t click_methods : 3;
            uint32_t scroll_methods : 3;
            uint32_t accel_profiles : 3;
            /* https://wayland.freedesktop.org/libinput/doc/latest/configuration.html */
            /* bitmask */
            uint32_t send_events_modes : 3;
            /* whether a device can have a custom rotation applied */
            uint32_t has_rotation : 1;
            /* if this device supports configurable disable-while-trackpointing feature */
            uint32_t has_dwtp : 1;
            /* if this device supports configurable disable-while-typing feature */
            uint32_t has_dwt : 1;
            /* if middle mouse button emulation configuration is available */
            uint32_t has_middle_emulation : 1;
            /* if a device has a configuration that supports left-handed usage */
            uint32_t has_left_handed : 1;
            /* if the device supports "natural scrolling" */
            uint32_t has_natural_scroll : 1;
            /* if the device can be calibrated via a calibration matrix */
            uint32_t has_calibration_matrix : 1;
            /* if a device uses libinput-internal pointer-acceleration */
            uint32_t has_pointer_accel : 1;
            uint32_t support_mapped_to_output : 1;
            uint32_t : 8;
        };
        uint32_t prop;
    };
};

struct input {
    struct wlr_input_device *wlr_input;
    struct wl_list link;

    const char *name;
    struct input_manager *manager;
    struct libinput_device *device;

    /* seat that input device attached */
    struct seat *seat;
    struct wl_list seat_link;

    struct gesture_state *tap_gestures;
    /* output that mapped to */
    struct kywc_output *mapped_output;
    struct wl_listener mapped_output_disable;
    struct wl_listener primary_output;
    struct wl_listener viewport;
    struct wl_listener raw_tap;

    /* input device prop and state per device */
    struct input_prop prop;
    struct input_state state;
    struct input_state default_state;

    struct {
        struct wl_signal destroy;
    } events;

    /* only use when add or remove keyboard group */
    uint32_t scroll_lock;

    struct wl_listener destroy;
};

struct input_event {
    struct seat *seat;
    struct input *input;
    enum input_event_type type;
    void *event;
};

typedef bool (*event_filter_func_t)(struct input_event *event, void *data);

typedef bool (*seat_iterator_func_t)(struct seat *seat, int index, void *data);

uint32_t input_manager_for_each_seat(seat_iterator_func_t iterator, void *data);

struct input_manager *input_manager_create(struct server *server);

bool input_action_manager_create(struct server *server);

bool global_shortcuts_manager_create(struct server *server);

bool global_gestures_manager_create(struct server *server);

void global_shortcuts_manager_action(void *data, enum key_binding_stage stage);

void input_action_handle_key_binding(struct key_binding *binding, struct seat *seat);

void input_action_handle_gesture_binding(struct gesture_binding *binding, struct seat *seat,
                                         enum gesture_stage stage,
                                         enum gesture_direction follow_direction);

struct seat *input_manager_get_default_seat(void);

void input_manager_set_event_filter(event_filter_func_t filter, void *data);

struct input *input_create(const char *name, struct wlr_input_device *wlr_input);

struct output *input_current_output(struct seat *seat);

void input_add_new_listener(struct wl_listener *listener);

bool input_set_state(struct input *input, struct input_state *state);

struct input *input_by_name(const char *name);

struct input *input_from_wlr_input(struct wlr_input_device *wlr_input);

void input_set_seat(struct input *input, const char *seat);

void input_set_all_cursor(const char *cursor_theme, uint32_t cursor_size);

void seat_add_new_listener(struct wl_listener *listener);

/* destroy_func can be NULL */
struct idle *idle_manager_add_idle(struct seat *seat, bool support_inhibit, uint32_t timeout,
                                   void (*idle_func)(struct idle *idle, void *data),
                                   void (*resume_func)(struct idle *idle, void *data),
                                   void (*destroy_func)(struct idle *idle, void *data), void *data);

void idle_destroy(struct idle *idle);

void idle_manager_set_inhibited(bool inhibited);

#endif /* _INPUT_H_ */
