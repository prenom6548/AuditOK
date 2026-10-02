// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _FUNCTION_H
#define _FUNCTION_H

#include <stdbool.h>
#include <stdint.h>

enum function_id {
    FUNC_ID_NONE,
    FUNC_ID_CUSTOM,
    FUNC_ID_CONFIG,
    FUNC_ID_NEW_WORKSPACE,
    FUNC_ID_DEL_WORKSPACE,
    FUNC_ID_SWITCH_LEFT_WORKSPACE,
    FUNC_ID_SWITCH_RIGHT_WORKSPACE,
    FUNC_ID_SWITCH_01_WORKSPACE,
    FUNC_ID_SWITCH_02_WORKSPACE,
    FUNC_ID_SWITCH_03_WORKSPACE,
    FUNC_ID_SWITCH_04_WORKSPACE,
    FUNC_ID_TOGGLE_DESKTOP_VISIBILITY,
    FUNC_ID_SHOW_DESKTOP,
    FUNC_ID_RESTORE_DESKTOP_WINDOWS,
    FUNC_ID_HIDE_INACTIVE_WINDOWS,
    FUNC_ID_RESTORE_INACTIVE_WINDOWS,
    FUNC_ID_TOGGLE_INACTIVE_WINDOWS,
    FUNC_ID_MAXIMIZE_ACTIVE_WINDOW,
    FUNC_ID_MINIMIZE_ACTIVE_WINDOW,
    FUNC_ID_CLOSE_ACTIVE_WINDOW,
    FUNC_ID_SHOW_ACTIVE_WINDOW_MENU,
    FUNC_ID_CAPTURE_ACTIVE_WINDOW,
    FUNC_ID_TILE_ACTIVE_WINDOW_TOP,
    FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM,
    FUNC_ID_TILE_ACTIVE_WINDOW_LEFT,
    FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT,
    FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF,
    FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF,
    FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF,
    FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF,
    FUNC_ID_SEND_ACTIVE_WINDOW_LEFT,
    FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT,
    FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE,
    FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE,
    FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS,
    FUNC_ID_TOGGLE_COLOR_FILTER,
    FUNC_ID_ZOOM_IN,
    FUNC_ID_ZOOM_OUT,
    FUNC_ID_ZOOM_STOP,
    FUNC_ID_SHOW_UKUI_MENU,
    FUNC_ID_SHOW_UKUI_SIDEBAR,
    FUNC_ID_SIMULATE_LEFTBTN,
    FUNC_ID_SIMULATE_RIGHTBTN,
    FUNC_ID_SIMULATE_MIDDLEBTN,
    FUNC_ID_WINDOWS_SWITCH,
    FUNC_ID_WINDOWS_SWITCH_BACKWARD,
    FUNC_ID_MULTI_TASKVIEW,
    FUNC_ID_UKUI_GLOBAL_SEARCH,
    FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE,
    FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE,
    FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE,
    FUNC_ID_MEDIA_PLAYPAUSE,
    FUNC_ID_MEDIA_PAUSE,
    FUNC_ID_MEDIA_NEXT,
    FUNC_ID_MEDIA_PREV,
    FUNC_ID_MEDIA_VOL_UP,
    FUNC_ID_MEDIA_VOL_DOWN,
    FUNC_ID_MEDIA_VOL_MUTE,
    FUNC_ID_MEDIA_VOL_CONTROL,
    FUNC_ID_MEDIA_REWIND,
    FUNC_ID_MEDIA_FAST_FORWARD,
    FUNC_ID_MEDIA_PROGRESS_CONTROL,
    FUNC_ID_BRIGHTNESS_UP,
    FUNC_ID_BRIGHTNESS_DOWN,
    FUNC_ID_BRIGHTNESS_CONTROL,
    FUNC_ID_COUNT,
};

enum key_binding_type;
enum key_binding_stage;
enum gesture_device;
enum gesture_direction;
enum gesture_stage;
struct seat;
struct key_binding;
struct gesture_binding;

bool function_manager_init(void);

enum key_binding_type function_entry_get_type(enum function_id id);

const char *function_entry_get_desc(enum function_id id);

const char *function_entry_get_l10n_name(enum function_id id);

uint32_t function_entry_get_key_binding_options(enum function_id id);

bool function_entry_get_gesture_optional(enum function_id id, enum gesture_device device,
                                         enum gesture_direction binding_direction, uint32_t *stages,
                                         uint32_t *follow_directions, double *follow_threshold);

void function_manager_handle_key_binding(struct key_binding *binding, void *data, struct seat *seat,
                                         enum key_binding_stage stage);

void function_manager_handle_gesture_binding(struct gesture_binding *binding, void *data,
                                             struct seat *seat, enum gesture_stage stage,
                                             enum gesture_device device, uint32_t follow_direction,
                                             double dx, double dy);

#endif
