// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_seat.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "effect/capture.h"
#include "input/cursor.h"
#include "input/function.h"
#include "input/seat.h"
#include "nls.h"
#include "output.h"
#include "scene/thumbnail.h"
#include "util/dbus.h"
#include "util/file.h"
#include "util/macros.h"
#include "util/string.h"
#include "view/action.h"
#include "view_p.h"

static struct window_shortcut {
    char *keybind;
    char *desc;
    enum function_id func_id;
} window_shortcuts[] = {
    { "Alt+F10", "window_maximized", FUNC_ID_MAXIMIZE_ACTIVE_WINDOW },
    { "Alt+F9", "window_minimized", FUNC_ID_MINIMIZE_ACTIVE_WINDOW },
    { "Alt+F4", "window_closed", FUNC_ID_CLOSE_ACTIVE_WINDOW },
    { "Alt+F3", "window_menu", FUNC_ID_SHOW_ACTIVE_WINDOW_MENU },
    { "win+up:no", "window_tile_top", FUNC_ID_TILE_ACTIVE_WINDOW_TOP },
    { "win+down:no", "window_tile_bottom", FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM },
    { "win+left:no", "window_tile_left", FUNC_ID_TILE_ACTIVE_WINDOW_LEFT },
    { "win+right:no", "window_tile_right", FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT },
    { "win+alt+up:no", "window_tile_top_half", FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF },
    { "win+alt+down:no", "window_tile_bottom_half", FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF },
    { "win+alt+left:no", "window_tile_left_half", FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF },
    { "win+alt+right:no", "window_tile_right_half", FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF },
    { "win+shift+left:no", "window_send_left", FUNC_ID_SEND_ACTIVE_WINDOW_LEFT },
    { "win+shift+right:no", "window_send_right", FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT },
    { "ctrl+shift+left:no", "window_send_left_maximize", FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE },
    { "ctrl+shift+right:no", "window_send_right_maximize",
      FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE },
};

static void view_manager_show_switcher(bool show)
{
    if (!dbus_call_method("org.kylin.switch", "/MultitaskView", "org.kylin.switch.MultitaskView",
                          show ? "show" : "hide", NULL, NULL)) {
        kywc_log(KYWC_ERROR, "DBUS call Multitaskview failed");
    }
}

#define MIRROR_BUFFER_DEBUG 0

struct view_capture {
    struct thumbnail *thumbnail;
#if MIRROR_BUFFER_DEBUG
    struct scene_buffer *buffer;
    struct wl_event_source *timer;
#endif
};

static void view_capture_destroy(struct view_capture *capture)
{
#if MIRROR_BUFFER_DEBUG
    if (capture->buffer) {
        scene_node_destroy(&capture->buffer->node);
    }
    wl_event_source_remove(capture->timer);
#endif
    if (capture->thumbnail) {
        thumbnail_destroy(capture->thumbnail);
    }

    free(capture);
}

static void capture_handle_thumbnail_cancel(struct thumbnail *thumbnail, void *data)
{
    struct view_capture *capture = data;
    capture->thumbnail = NULL;
    view_capture_destroy(capture);
}

#if MIRROR_BUFFER_DEBUG
static void capture_handle_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                            bool buffer_changed, void *data)
{
    struct view_capture *capture = data;

    scene_buffer_set_opacity(capture->buffer, 0.5);
    scene_buffer_set_dest_size(capture->buffer, buffer->width, buffer->height);
    scene_buffer_set_buffer(capture->buffer, buffer);

    thumbnail_mark_wants_update(capture->thumbnail, false, false);
    wl_event_source_timer_update(capture->timer, 250);
}

static int handle_capture(void *data)
{
    struct view_capture *capture = data;
    thumbnail_mark_wants_update(capture->thumbnail, true, false);
    return 0;
}
#else
static void capture_done(const char *path, void *data)
{
    char href[512];
    snprintf(href, sizeof(href), "<a href=\"file://%s\">%s</a>", path, path);
    dbus_notify(tr("Screenshot"), tr("Capture saved to"), href, "kylin-screenshot");
    free(data);
}

static void capture_handle_thumbnail_update(struct thumbnail *thumbnail, struct wlr_buffer *buffer,
                                            bool buffer_changed, void *data)
{
    const char *dir = file_get_xdg_pictures_dir();
    const char *file = kywc_identifier_time_generate("", ".png");
    const char *path = string_create("%s/%s", dir, file);
    free((void *)dir);
    free((void *)file);

    capture_write_file(buffer, path, capture_done, (void *)path);

    struct view_capture *capture = data;
    view_capture_destroy(capture);
}
#endif

static const struct thumbnail_interface window_capture_impl = {
    .update = capture_handle_thumbnail_update,
    .cancel = capture_handle_thumbnail_cancel,
};

static void window_capture_create(struct view *view, struct seat *seat)
{
    if (!view->base.mapped) {
        return;
    }

    struct view_capture *capture = calloc(1, sizeof(*capture));
    if (!capture) {
        return;
    }

    // struct ky_scene_buffer *buffer = scene_buffer_try_from_surface(view->surface);
    capture->thumbnail = thumbnail_create_from_view(view, THUMBNAIL_ENABLE_SECURITY, 1.0,
                                                    &window_capture_impl, capture);
    if (!capture->thumbnail) {
        free(capture);
        return;
    }

#if MIRROR_BUFFER_DEBUG
    struct view_layer *layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    capture->buffer = scene_buffer_create(layer->tree, NULL);
    scene_node_set_input_bypassed(&capture->buffer->node, true);
    struct wl_event_loop *loop = wl_display_get_event_loop(seat->wlr_seat->display);
    capture->timer = wl_event_loop_add_timer(loop, handle_capture, capture);
#endif
}

static struct output *target_output_by_view(struct view *view, enum window_action action)
{
    if (output_state_is_mirror_mode()) {
        return NULL;
    }

    struct output *output = output_from_kywc_output(view->output);
    enum layout_edge edge = LAYOUT_EDGE_RIGHT;
    if (action == WINDOW_ACTION_SEND_LEFT_OUTPUT ||
        action == WINDOW_ACTION_SEND_LEFT_OUTPUT_MAXIMIZE) {
        edge = LAYOUT_EDGE_LEFT;
    }

    return output_find_specified_output(output, edge);
}

/* Move the view directly to the next output in equal proportions */
static void view_send_to_output(struct view *view, enum window_action action)
{
    if (view->base.fullscreen || view_has_modal_property(view)) {
        return;
    }

    struct output *output = target_output_by_view(view, action);
    if (output) {
        view_move_to_output(view, NULL, NULL, &output->base);
        sub_view_move_to_output(view, &output->base);
    }
}

/**
 * Move the view to the next output and maximize it,
 * but restore the original view size when the output state
 * of the original view is not the maximized state
 */
static void view_send_to_output_and_maximize(struct view *view, enum window_action action)
{
    if (view->base.fullscreen || view_has_modal_property(view) ||
        !KYWC_VIEW_IS_MAXIMIZABLE(&view->base)) {
        return;
    }

    struct output *output = target_output_by_view(view, action);
    if (!output) {
        return;
    }

    if (!view->saved.output || !view->base.maximized) {
        view->saved.tiled = view->base.maximized ? KYWC_TILE_ALL : view->base.tiled;
        view->saved.output = view->output;
    }

    view_move_to_output(view, NULL, NULL, &output->base);
    sub_view_move_to_output(view, &output->base);

    if (output_from_kywc_output(view->saved.output) != output) {
        kywc_view_set_maximized(&view->base, true, &output->base);
        return;
    }

    if (view->saved.tiled == KYWC_TILE_ALL || !view->saved.tiled) {
        kywc_view_set_maximized(&view->base, view->saved.tiled ? true : false, &output->base);
    } else {
        kywc_view_set_tiled(&view->base, view->saved.tiled, &output->base);
    }

    view->saved.output = NULL;
}

void window_action(struct view *view, struct seat *seat, enum window_action action)
{
    struct kywc_view *kywc_view = &view->base;
    int lx, ly;

    switch (action) {
    case WINDOW_ACTION_NONE:
        break;
    case WINDOW_ACTION_MINIMIZE:
        kywc_view_set_minimized(kywc_view, true);
        break;
    case WINDOW_ACTION_MAXIMIZE:
        kywc_view_toggle_maximized(kywc_view);
        break;
    case WINDOW_ACTION_CLOSE:
        kywc_view_close(kywc_view);
        break;
    case WINDOW_ACTION_MOVE:
        /* seat cursor to middle of the view */
        lx = kywc_view->geometry.x + kywc_view->geometry.width / 2;
        ly = kywc_view->geometry.y + kywc_view->geometry.height / 2;
        cursor_move(seat->cursor, NULL, lx, ly, false, false);
        window_begin_move(view, seat);
        break;
    case WINDOW_ACTION_RESIZE:
        lx = kywc_view->geometry.x + kywc_view->geometry.width;
        ly = kywc_view->geometry.y + kywc_view->geometry.height;
        cursor_move(seat->cursor, NULL, lx + 8, ly + 8, false, false);
        window_begin_resize(view, KYWC_EDGE_RIGHT | KYWC_EDGE_BOTTOM, seat);
        break;
    case WINDOW_ACTION_KEEP_ABOVE:
        kywc_view_toggle_kept_above(kywc_view);
        break;
    case WINDOW_ACTION_KEEP_BELOW:
        kywc_view_toggle_kept_below(kywc_view);
        break;
    case WINDOW_ACTION_MENU:
        if (kywc_view->ssd & KYWC_SSD_TITLE && !kywc_view->fullscreen) {
            lx = kywc_view->geometry.x;
            ly = kywc_view->geometry.y;
            view_show_window_menu(view, seat, lx, ly);
        }
        break;
    case WINDOW_ACTION_TILE_TOP:
    case WINDOW_ACTION_TILE_BOTTOM:
    case WINDOW_ACTION_TILE_LEFT:
    case WINDOW_ACTION_TILE_RIGHT:
        window_begin_tile(view, action, seat);
        break;
    case WINDOW_ACTION_TILE_TOP_HALF_SCREEN:
    case WINDOW_ACTION_TILE_BOTTOM_HALF_SCREEN:
    case WINDOW_ACTION_TILE_LEFT_HALF_SCREEN:
    case WINDOW_ACTION_TILE_RIGHT_HALF_SCREEN:
        window_begin_tile_half_screen(view, action, seat);
        break;
    case WINDOW_ACTION_SEND_LEFT_OUTPUT:
    case WINDOW_ACTION_SEND_RIGHT_OUTPUT:
        view_send_to_output(view, action);
        break;
    case WINDOW_ACTION_SEND_LEFT_OUTPUT_MAXIMIZE:
    case WINDOW_ACTION_SEND_RIGHT_OUTPUT_MAXIMIZE:
        view_send_to_output_and_maximize(view, action);
        break;
    case WINDOW_ACTION_CAPTURE:
        window_capture_create(view, seat);
        break;
    }
}

static void view_manager_handle_window_action(enum window_action action)
{
    struct view *view = view_manager_get_activated();
    if (!view || view->base.role != KYWC_VIEW_ROLE_NORMAL) {
        return;
    }

    window_action(view, input_manager_get_default_seat(), action);
}

static enum window_action view_manager_get_window_action(enum function_id id)
{
    switch (id) {
    case FUNC_ID_MAXIMIZE_ACTIVE_WINDOW:
        return WINDOW_ACTION_MAXIMIZE;
    case FUNC_ID_MINIMIZE_ACTIVE_WINDOW:
        return WINDOW_ACTION_MINIMIZE;
    case FUNC_ID_CLOSE_ACTIVE_WINDOW:
        return WINDOW_ACTION_CLOSE;
    case FUNC_ID_SHOW_ACTIVE_WINDOW_MENU:
        return WINDOW_ACTION_MENU;
    case FUNC_ID_CAPTURE_ACTIVE_WINDOW:
        return WINDOW_ACTION_CAPTURE;
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP:
        return WINDOW_ACTION_TILE_TOP;
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM:
        return WINDOW_ACTION_TILE_BOTTOM;
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT:
        return WINDOW_ACTION_TILE_LEFT;
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT:
        return WINDOW_ACTION_TILE_RIGHT;
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF:
        return WINDOW_ACTION_TILE_TOP_HALF_SCREEN;
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF:
        return WINDOW_ACTION_TILE_BOTTOM_HALF_SCREEN;
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF:
        return WINDOW_ACTION_TILE_LEFT_HALF_SCREEN;
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF:
        return WINDOW_ACTION_TILE_RIGHT_HALF_SCREEN;
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT:
        return WINDOW_ACTION_SEND_LEFT_OUTPUT;
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT:
        return WINDOW_ACTION_SEND_RIGHT_OUTPUT;
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE:
        return WINDOW_ACTION_SEND_LEFT_OUTPUT_MAXIMIZE;
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE:
        return WINDOW_ACTION_SEND_RIGHT_OUTPUT_MAXIMIZE;
    default:
        return WINDOW_ACTION_NONE;
    }
}

static void window_action_to_tiled(enum kywc_tile tiled)
{
    struct view *view = view_manager_get_activated();
    if (!view || view->base.role != KYWC_VIEW_ROLE_NORMAL) {
        return;
    }

    struct output *output = output_from_kywc_output(view->output);
    kywc_view_set_tiled(&view->base, tiled, &output->base);
    view_manager_show_tile_assist(view, input_manager_get_default_seat(), &output->base);
}

static bool view_manager_handle_special_function(enum function_id id, enum gesture_stage stage,
                                                 double dx, double dy)
{
    switch (id) {
    case FUNC_ID_TOGGLE_DESKTOP_VISIBILITY:
        view_manager_show_desktop(!view_manager_get_show_desktop(), true);
        return true;
    case FUNC_ID_SHOW_DESKTOP:
        if (stage != GESTURE_STAGE_NONE && view_manager_get_show_switcher()) {
            view_manager_show_switcher(false);
        } else {
            view_manager_show_desktop(true, true);
        }
        return true;
    case FUNC_ID_RESTORE_DESKTOP_WINDOWS:
        view_manager_show_desktop(false, true);
        return true;
    case FUNC_ID_HIDE_INACTIVE_WINDOWS:
        view_manager_show_activated_only(true, true);
        return true;
    case FUNC_ID_RESTORE_INACTIVE_WINDOWS:
        view_manager_show_activated_only(false, true);
        return true;
    case FUNC_ID_TOGGLE_INACTIVE_WINDOWS:
        view_manager_show_activated_only(!view_manager_get_show_activte_only(), true);
        return true;
    case FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS:
        maximize_switcher_show();
        return true;
    case FUNC_ID_MULTI_TASKVIEW:
        if (!view_manager_get_show_desktop()) {
            view_manager_show_switcher(true);
        } else {
            view_manager_show_desktop(false, true);
        }
        return true;
    case FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE:
        tablet_mode_handle_task_bar_gesture(stage, dx, dy);
        return true;
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT:
        if (stage == GESTURE_STAGE_NONE) {
            return false;
        }
        if (view_manager_get_show_desktop()) {
            view_manager_show_desktop(false, true);
            return true;
        }
        if (view_manager_get_show_activte_only()) {
            view_manager_show_activated_only(false, true);
            return true;
        }
        window_action_to_tiled(KYWC_TILE_LEFT);
        return true;
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT:
        if (stage == GESTURE_STAGE_NONE) {
            return false;
        }
        if (view_manager_get_show_desktop()) {
            view_manager_show_desktop(false, true);
            return true;
        }
        if (view_manager_get_show_activte_only()) {
            view_manager_show_activated_only(false, true);
            return true;
        }
        window_action_to_tiled(KYWC_TILE_RIGHT);
        return true;
    default:
        return false;
    }
}

void view_manager_handle_function(enum function_id id, enum gesture_stage stage, double dx,
                                  double dy)
{
    if (view_manager_handle_special_function(id, stage, dx, dy)) {
        return;
    }

    enum window_action action = view_manager_get_window_action(id);
    if (action != WINDOW_ACTION_NONE) {
        view_manager_handle_window_action(action);
    }
}

bool window_actions_create(struct view_manager *view_manager)
{
    for (size_t i = 0; i < ARRAY_SIZE(window_shortcuts); i++) {
        struct window_shortcut *shortcut = &window_shortcuts[i];
        struct key_binding *binding =
            kywc_key_binding_create(shortcut->keybind, shortcut->desc, shortcut->func_id, 0);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->func_id),
                                       function_manager_handle_key_binding, NULL)) {
            kywc_key_binding_destroy(binding);
            continue;
        }
    }
    return true;
}

static struct shortcut {
    char *keybind;
    char *desc;
    enum function_id func_id;
} shortcuts[] = {
    { "win+d:no", "desktop_toggle_visibility", FUNC_ID_TOGGLE_DESKTOP_VISIBILITY },
    { "win+h", "desktop_show_1", FUNC_ID_SHOW_DESKTOP },
    { "win+m", "desktop_show_2", FUNC_ID_SHOW_DESKTOP },
    { "win+g", "desktop_windows_restore_1", FUNC_ID_RESTORE_DESKTOP_WINDOWS },
    { "win+shift+m", "desktop_windows_restore_2", FUNC_ID_RESTORE_DESKTOP_WINDOWS },
    { "win+home", "inactive_windows_minimize_restore", FUNC_ID_TOGGLE_INACTIVE_WINDOWS },
};

static struct gesture {
    enum gesture_type type;
    uint32_t stages;
    uint8_t fingers;
    enum gesture_device device;
    enum gesture_direction direction;
    enum gesture_edge edge;
    uint32_t follow_directions;
    double follow_threshold;
    char *desc;
    enum function_id func_id;
} gestures[] = {
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        3,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(3)fingers_swipe_up",
        FUNC_ID_MULTI_TASKVIEW,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        3,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_DOWN,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(3)fingers_swipe_down",
        FUNC_ID_SHOW_DESKTOP,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        4,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(4)fingers_swipe_up",
        FUNC_ID_MULTI_TASKVIEW,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        4,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_DOWN,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchpad_(4)fingers_swipe_down",
        FUNC_ID_SHOW_DESKTOP,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        3,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(3)fingers_swipe_up",
        FUNC_ID_MULTI_TASKVIEW,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        3,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_DOWN,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(3)fingers_swipe_down",
        FUNC_ID_SHOW_DESKTOP,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        4,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(4)fingers_swipe_up",
        FUNC_ID_MULTI_TASKVIEW,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_TRIGGER,
        4,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_DOWN,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_NONE,
        0.0,
        "touchscreen_(4)fingers_swipe_down",
        FUNC_ID_SHOW_DESKTOP,
    },
};

bool view_manager_actions_create(struct view_manager *view_manager)
{
    for (size_t i = 0; i < ARRAY_SIZE(shortcuts); i++) {
        struct shortcut *shortcut = &shortcuts[i];
        struct key_binding *binding =
            kywc_key_binding_create(shortcut->keybind, shortcut->desc, shortcut->func_id, 0);
        if (!binding) {
            continue;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->func_id),
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
