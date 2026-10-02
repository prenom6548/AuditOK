// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <libinput.h>

#include <kywc/log.h>

#include "backend/libinput.h"
#include "input_p.h"

void libinput_get_prop(struct input *input, struct input_prop *prop)
{
    struct libinput_device *device = input->device;
    if (!device) {
        kywc_log(KYWC_DEBUG, "Input %s is not a libinput device", input->name);
        return;
    }

    libinput_backend_lock(device);

    prop->send_events_modes = libinput_device_config_send_events_get_modes(device);
    prop->click_methods = libinput_device_config_click_get_methods(device);
    prop->scroll_methods = libinput_device_config_scroll_get_methods(device);
    prop->accel_profiles = libinput_device_config_accel_get_profiles(device);

    prop->tap_finger_count = libinput_device_config_tap_get_finger_count(device);

    prop->has_calibration_matrix = libinput_device_config_calibration_has_matrix(device);
    prop->has_pointer_accel = libinput_device_config_accel_is_available(device);
    prop->has_natural_scroll = libinput_device_config_scroll_has_natural_scroll(device);
    prop->has_left_handed = libinput_device_config_left_handed_is_available(device);
    prop->has_middle_emulation = libinput_device_config_middle_emulation_is_available(device);
    prop->has_dwt = libinput_device_config_dwt_is_available(device);
    prop->has_dwtp = libinput_device_config_dwtp_is_available(device);
    prop->has_rotation = libinput_device_config_rotation_is_available(device);
#if HAVE_LIBINPUT_ZONE_GESTURE
    int zone_gesture_available = libinput_device_config_zone_gesture_is_available(device);
    prop->has_zone_gesture = !!zone_gesture_available;
    kywc_log(KYWC_DEBUG, "Input %s zone gesture available=%d has=%d", input->name,
             zone_gesture_available, prop->has_zone_gesture);
#endif
#if HAVE_LIBINPUT_GESTURE_PINCH_CONFIG
    int pinch_gesture_available = libinput_device_config_gesture_pinch_is_available(device);
    prop->has_pinch_gesture = !!pinch_gesture_available;
    kywc_log(KYWC_DEBUG, "Input %s pinch gesture available=%d has=%d", input->name,
             pinch_gesture_available, prop->has_pinch_gesture);
#endif

    libinput_backend_unlock(device);
}

void libinput_get_state(struct input *input, struct input_state *state)
{
    struct libinput_device *device = input->device;
    if (!device) {
        kywc_log(KYWC_DEBUG, "Input %s is not a libinput device", input->name);
        return;
    }

    libinput_backend_lock(device);

    struct input_prop *prop = &input->prop;

    state->send_events_mode = libinput_device_config_send_events_get_mode(device);
    state->click_method = libinput_device_config_click_get_method(device);

    if (prop->tap_finger_count > 0) {
        state->tap_to_click = libinput_device_config_tap_get_enabled(device);
        state->tap_button_map = libinput_device_config_tap_get_button_map(device);
        state->tap_and_drag = libinput_device_config_tap_get_drag_enabled(device);
        state->tap_drag_lock = libinput_device_config_tap_get_drag_lock_enabled(device);
    }

    if (prop->has_calibration_matrix) {
        libinput_device_config_calibration_get_matrix(device, state->calibration_matrix);
    }

    if (prop->has_pointer_accel) {
        state->pointer_accel_speed = libinput_device_config_accel_get_speed(device);
        state->accel_profile = libinput_device_config_accel_get_profile(device);
    }

    if (prop->has_natural_scroll) {
        state->natural_scroll = libinput_device_config_scroll_get_natural_scroll_enabled(device);
    }

    if (prop->has_left_handed) {
        state->left_handed = libinput_device_config_left_handed_get(device);
    }

    if (prop->has_middle_emulation) {
        state->middle_emulation = libinput_device_config_middle_emulation_get_enabled(device);
    }

    state->scroll_method = libinput_device_config_scroll_get_method(device);
    if (state->scroll_method == LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) {
        state->scroll_button = libinput_device_config_scroll_get_button(device);
        state->scroll_button_lock = libinput_device_config_scroll_get_button_lock(device);
    }

    if (prop->has_dwt) {
        state->dwt = libinput_device_config_dwt_get_enabled(device);
    }

    if (prop->has_dwtp) {
        state->dwtp = libinput_device_config_dwtp_get_enabled(device);
    }

    if (prop->has_rotation) {
        state->rotation_angle = libinput_device_config_rotation_get_angle(device);
    }

#if HAVE_LIBINPUT_ZONE_GESTURE
    if (prop->has_zone_gesture) {
        state->zone_gesture_enabled = libinput_device_config_zone_gesture_get_enabled(device);
        state->zone_gesture_left_enabled =
            libinput_device_config_zone_gesture_get_left_enabled(device);
        state->zone_gesture_right_enabled =
            libinput_device_config_zone_gesture_get_right_enabled(device);
        state->zone_gesture_top_enabled =
            libinput_device_config_zone_gesture_get_top_enabled(device);
        state->zone_gesture_right_priority =
            libinput_device_config_zone_gesture_get_right_priority(device);
        kywc_log(KYWC_DEBUG,
                 "Input %s zone gesture current enabled=%d left=%d right=%d top=%d "
                 "right_priority=%u",
                 input->name, state->zone_gesture_enabled, state->zone_gesture_left_enabled,
                 state->zone_gesture_right_enabled, state->zone_gesture_top_enabled,
                 state->zone_gesture_right_priority);
    }
#endif
#if HAVE_LIBINPUT_GESTURE_PINCH_CONFIG
    if (prop->has_pinch_gesture) {
        state->pinch_gesture_enabled = libinput_device_config_gesture_get_pinch_enabled(device);
        kywc_log(KYWC_DEBUG, "Input %s pinch gesture current enabled=%d", input->name,
                 state->pinch_gesture_enabled);
    }
#endif

    libinput_backend_unlock(device);
}

void libinput_get_default_state(struct input *input, struct input_state *state)
{
    struct libinput_device *device = input->device;
    if (!device) {
        kywc_log(KYWC_DEBUG, "Input %s is not a libinput device", input->name);
        return;
    }

    libinput_backend_lock(device);

    struct input_prop *prop = &input->prop;

    state->send_events_mode = libinput_device_config_send_events_get_default_mode(device);
    state->click_method = libinput_device_config_click_get_default_method(device);

    if (prop->tap_finger_count > 0) {
        state->tap_to_click = libinput_device_config_tap_get_default_enabled(device);
        state->tap_button_map = libinput_device_config_tap_get_default_button_map(device);
        state->tap_and_drag = libinput_device_config_tap_get_default_drag_enabled(device);
        state->tap_drag_lock = libinput_device_config_tap_get_default_drag_lock_enabled(device);
    }

    if (prop->has_calibration_matrix) {
        libinput_device_config_calibration_get_default_matrix(device, state->calibration_matrix);
    }

    if (prop->has_pointer_accel) {
        state->pointer_accel_speed = libinput_device_config_accel_get_default_speed(device);
        state->accel_profile = libinput_device_config_accel_get_default_profile(device);
    }

    if (prop->has_natural_scroll) {
        state->natural_scroll =
            libinput_device_config_scroll_get_default_natural_scroll_enabled(device);
    }

    if (prop->has_left_handed) {
        state->left_handed = libinput_device_config_left_handed_get_default(device);
    }

    if (prop->has_middle_emulation) {
        state->middle_emulation =
            libinput_device_config_middle_emulation_get_default_enabled(device);
    }

    state->scroll_method = libinput_device_config_scroll_get_default_method(device);
    if (state->scroll_method == LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) {
        state->scroll_button = libinput_device_config_scroll_get_default_button(device);
        state->scroll_button_lock = libinput_device_config_scroll_get_default_button_lock(device);
    }

    if (prop->has_dwt) {
        state->dwt = libinput_device_config_dwt_get_default_enabled(device);
    }

    if (prop->has_dwtp) {
        state->dwtp = libinput_device_config_dwtp_get_default_enabled(device);
    }

    if (prop->has_rotation) {
        state->rotation_angle = libinput_device_config_rotation_get_default_angle(device);
    }

#if HAVE_LIBINPUT_ZONE_GESTURE
    if (prop->has_zone_gesture) {
        state->zone_gesture_enabled =
            libinput_device_config_zone_gesture_get_default_enabled(device);
        state->zone_gesture_left_enabled =
            libinput_device_config_zone_gesture_get_default_left_enabled(device);
        state->zone_gesture_right_enabled =
            libinput_device_config_zone_gesture_get_default_right_enabled(device);
        state->zone_gesture_top_enabled =
            libinput_device_config_zone_gesture_get_default_top_enabled(device);
        state->zone_gesture_right_priority =
            libinput_device_config_zone_gesture_get_right_priority(device);
        kywc_log(KYWC_DEBUG,
                 "Input %s zone gesture default enabled=%d left=%d right=%d top=%d "
                 "right_priority=%u",
                 input->name, state->zone_gesture_enabled, state->zone_gesture_left_enabled,
                 state->zone_gesture_right_enabled, state->zone_gesture_top_enabled,
                 state->zone_gesture_right_priority);
    }
#endif
#if HAVE_LIBINPUT_GESTURE_PINCH_CONFIG
    if (prop->has_pinch_gesture) {
        state->pinch_gesture_enabled =
            libinput_device_config_gesture_get_pinch_default_enabled(device);
        kywc_log(KYWC_DEBUG, "Input %s pinch gesture default enabled=%d", input->name,
                 state->pinch_gesture_enabled);
    }
#endif

    libinput_backend_unlock(device);
}

static bool log_status(enum libinput_config_status status)
{
    if (status != LIBINPUT_CONFIG_STATUS_SUCCESS) {
        kywc_log(KYWC_ERROR, "Failed to apply libinput config: %s",
                 libinput_config_status_to_str(status));
        return false;
    }

    return true;
}

bool libinput_set_state(struct input *input, struct input_state *state)
{
    struct libinput_device *device = input->device;
    if (!device) {
        kywc_log(KYWC_DEBUG, "Input %s is not a libinput device", input->name);
        return false;
    }

    libinput_backend_lock(device);

    struct input_state *current = &input->state;
    bool success = true;

    if (current->send_events_mode != state->send_events_mode) {
        kywc_log(KYWC_DEBUG, "Do send_events_set_mode(%d)", state->send_events_mode);
        success &= log_status(
            libinput_device_config_send_events_set_mode(device, state->send_events_mode));
    }

    if (current->click_method != state->click_method) {
        kywc_log(KYWC_DEBUG, "Do click_set_method(%d)", state->click_method);
        success &= log_status(libinput_device_config_click_set_method(device, state->click_method));
    }

    if (input->prop.tap_finger_count > 0) {
        if (current->tap_to_click != state->tap_to_click) {
            kywc_log(KYWC_DEBUG, "Do tap_set_enabled(%d)", state->tap_to_click);
            success &=
                log_status(libinput_device_config_tap_set_enabled(device, state->tap_to_click));
        }

        // XXX: skip others ?
        // if (!state->tap_to_click) {
        //  }

        if (current->tap_and_drag != state->tap_and_drag) {
            kywc_log(KYWC_DEBUG, "Do tap_set_drag_enabled(%d)", state->tap_and_drag);
            success &= log_status(
                libinput_device_config_tap_set_drag_enabled(device, state->tap_and_drag));
        }
        if (current->tap_drag_lock != state->tap_drag_lock) {
            kywc_log(KYWC_DEBUG, "Do tap_set_drag_lock_enabled(%d)", state->tap_drag_lock);
            success &= log_status(
                libinput_device_config_tap_set_drag_lock_enabled(device, state->tap_drag_lock));
        }
        if (current->tap_button_map != state->tap_button_map) {
            kywc_log(KYWC_DEBUG, "Do tap_set_button_map(%d)", state->tap_button_map);
            success &= log_status(
                libinput_device_config_tap_set_button_map(device, state->tap_button_map));
        }
    }

    if (input->prop.has_calibration_matrix) {
        bool changed = false;
        for (int i = 0; i < 6; i++) {
            if (current->calibration_matrix[i] != state->calibration_matrix[i]) {
                changed = true;
                break;
            }
        }
        if (changed) {
            kywc_log(KYWC_DEBUG, "Do calibration_set_matrix(%f, %f, %f, %f, %f, %f)",
                     state->calibration_matrix[0], state->calibration_matrix[1],
                     state->calibration_matrix[2], state->calibration_matrix[3],
                     state->calibration_matrix[4], state->calibration_matrix[5]);
            success &= log_status(
                libinput_device_config_calibration_set_matrix(device, state->calibration_matrix));
        }
    }

    if (input->prop.has_pointer_accel) {
        if (current->pointer_accel_speed != state->pointer_accel_speed) {
            kywc_log(KYWC_DEBUG, "Do accel_set_speed(%f)", state->pointer_accel_speed);
            success &= log_status(
                libinput_device_config_accel_set_speed(device, state->pointer_accel_speed));
        }
        if (current->accel_profile != state->accel_profile) {
            kywc_log(KYWC_DEBUG, "Do accel_set_profile(%d)", state->accel_profile);
            success &=
                log_status(libinput_device_config_accel_set_profile(device, state->accel_profile));
        }
    }

    if (input->prop.has_natural_scroll && current->natural_scroll != state->natural_scroll) {
        kywc_log(KYWC_DEBUG, "Do scroll_set_natural_scroll_enabled(%d)", state->natural_scroll);
        success &= log_status(libinput_device_config_scroll_set_natural_scroll_enabled(
            device, state->natural_scroll));
    }

    if (input->prop.has_left_handed && current->left_handed != state->left_handed) {
        kywc_log(KYWC_DEBUG, "Do left_handed_set(%d)", state->left_handed);
        success &= log_status(libinput_device_config_left_handed_set(device, state->left_handed));
    }

    if (input->prop.has_middle_emulation && current->middle_emulation != state->middle_emulation) {
        kywc_log(KYWC_DEBUG, "Do middle_emulation_set_enabled(%d)", state->middle_emulation);
        success &= log_status(
            libinput_device_config_middle_emulation_set_enabled(device, state->middle_emulation));
    }

    if (current->scroll_method != state->scroll_method) {
        kywc_log(KYWC_DEBUG, "Do scroll_set_method(%d)", state->scroll_method);
        success &=
            log_status(libinput_device_config_scroll_set_method(device, state->scroll_method));
    }
    if (state->scroll_method == LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN) {
        if (current->scroll_button != state->scroll_button) {
            kywc_log(KYWC_DEBUG, "Do scroll_set_button(%d)", state->scroll_button);
            success &=
                log_status(libinput_device_config_scroll_set_button(device, state->scroll_button));
        }
        if (current->scroll_button_lock != state->scroll_button_lock) {
            kywc_log(KYWC_DEBUG, "Do scroll_set_button_lock(%d)", state->scroll_button_lock);
            success &= log_status(
                libinput_device_config_scroll_set_button_lock(device, state->scroll_button_lock));
        }
    }

    if (input->prop.has_dwt && current->dwt != state->dwt) {
        kywc_log(KYWC_DEBUG, "Do dwt_set_enabled(%d)", state->dwt);
        success &= log_status(libinput_device_config_dwt_set_enabled(device, state->dwt));
    }

    if (input->prop.has_dwtp && current->dwtp != state->dwtp) {
        kywc_log(KYWC_DEBUG, "Do dwtp_set_enabled(%d)", state->dwtp);
        success &= log_status(libinput_device_config_dwtp_set_enabled(device, state->dwtp));
    }

    if (input->prop.has_rotation && current->rotation_angle != state->rotation_angle) {
        kywc_log(KYWC_DEBUG, "Do rotation_set_angle(%d)", state->rotation_angle);
        success &=
            log_status(libinput_device_config_rotation_set_angle(device, state->rotation_angle));
    }

#if HAVE_LIBINPUT_ZONE_GESTURE
    if (input->prop.has_zone_gesture) {
        if (current->zone_gesture_enabled != state->zone_gesture_enabled) {
            kywc_log(KYWC_DEBUG, "Do zone_gesture_set_enabled(%d)", state->zone_gesture_enabled);
            success &= log_status(libinput_device_config_zone_gesture_set_enabled(
                device, state->zone_gesture_enabled));
        }
        if (current->zone_gesture_left_enabled != state->zone_gesture_left_enabled) {
            kywc_log(KYWC_DEBUG, "Do zone_gesture_set_left_enabled(%d)",
                     state->zone_gesture_left_enabled);
            success &= log_status(libinput_device_config_zone_gesture_set_left_enabled(
                device, state->zone_gesture_left_enabled));
        }
        if (current->zone_gesture_right_enabled != state->zone_gesture_right_enabled) {
            kywc_log(KYWC_DEBUG, "Do zone_gesture_set_right_enabled(%d)",
                     state->zone_gesture_right_enabled);
            success &= log_status(libinput_device_config_zone_gesture_set_right_enabled(
                device, state->zone_gesture_right_enabled));
        }
        if (current->zone_gesture_top_enabled != state->zone_gesture_top_enabled) {
            kywc_log(KYWC_DEBUG, "Do zone_gesture_set_top_enabled(%d)",
                     state->zone_gesture_top_enabled);
            success &= log_status(libinput_device_config_zone_gesture_set_top_enabled(
                device, state->zone_gesture_top_enabled));
        }
        if (current->zone_gesture_right_priority != state->zone_gesture_right_priority) {
            kywc_log(KYWC_DEBUG, "Do zone_gesture_set_right_priority(%d)",
                     state->zone_gesture_right_priority);
            success &= log_status(libinput_device_config_zone_gesture_set_right_priority(
                device, state->zone_gesture_right_priority));
        }
    }
#endif
#if HAVE_LIBINPUT_GESTURE_PINCH_CONFIG
    if (input->prop.has_pinch_gesture &&
        current->pinch_gesture_enabled != state->pinch_gesture_enabled) {
        kywc_log(KYWC_DEBUG, "Do gesture_set_pinch_enabled(%d)", state->pinch_gesture_enabled);
        success &= log_status(
            libinput_device_config_gesture_set_pinch_enabled(device, state->pinch_gesture_enabled));
    }
#endif

    libinput_backend_unlock(device);

    return success;
}
