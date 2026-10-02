// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "effect/fade.h"
#include "output.h"
#include "theme.h"
#include "util/macros.h"
#include "view_p.h"

/* 0.15 is trigger window switcher threshold */
#define TRIGGER_SWITCHER_THRESHOLD (0.15)

struct tablet_mode_view {
    struct wl_list link;

    struct view *view;
    struct wl_listener parent;
    struct wl_listener view_update_capabilities;

    bool remove_workspace;
    struct scene_rect *blur_rect;
};

struct task_bar {
    struct view *view;
    int default_y;

    struct wl_listener output_geometry;
};

static struct tablet_mode_manager {
    struct wl_list tablet_views;

    struct task_bar task_bar;
    struct view *pc_desktop;
    struct view *tablet_desktop;
    struct scene_rect *global_blur_rect;
    struct view_manager *view_manager;

    struct wl_listener theme_pre_update;
} *manager = NULL;

/* chase kwin, some special app require special treatment */
#define TASK_BAR "tablet-taskbar"
#define PC_DESKTOP "peony-qt-desktop"
#define TABLET_DESKTOP "ukui-tablet-desktop"

static const char *special_appid[] = { "ukui-bluetooth", "kylin-nm", "ukui-search" };
static const char *special_title[] = { TASK_BAR };

static struct gesture {
    enum gesture_type type;
    enum gesture_stage stage;
    uint8_t fingers;
    uint32_t devices;
    uint32_t directions;
    uint32_t edges;
    uint32_t follow_direction;
    double follow_threshold;
    char *desc;
    enum function_id func_id;
    struct gesture_binding *binding;
} gestures[] = {
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_STOP,
        1,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_UP,
        GESTURE_EDGE_BOTTOM,
        GESTURE_DIRECTION_UP,
        0.0,
        "touch_(1)fingers_swipe_up(bottom)",
        FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE,
        NULL,
    },
};

static void tablet_mode_view_maximized(struct view *view, bool maximized,
                                       struct kywc_output *kywc_output);

static void tablet_mode_view_minimized(struct view *view, bool minimized);

static struct tablet_mode_view *tablet_mode_view_from_view(struct view *view)
{
    return (struct tablet_mode_view *)view->mode_view;
}

static bool view_is_special(struct view *view)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(special_appid); i++) {
        if (strcmp(view->base.app_id, special_appid[i]) == 0) {
            return true;
        }
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(special_title); i++) {
        if (strcmp(view->base.title, special_title[i]) == 0) {
            return true;
        }
    }

    return false;
}

static void task_bar_update_coord(void)
{
    struct view *view = manager->task_bar.view;

    // default y is %4 distance from bottom
    struct output *output = output_from_kywc_output(view->output);
    manager->task_bar.default_y = output->geometry.height - output->geometry.height * 0.04 -
                                  view->base.geometry.height - view->base.margin.off_height;
}

static void handle_output_geometry(struct wl_listener *listener, void *data)
{
    task_bar_update_coord();
}

static void tablet_mode_set_task_bar(struct view *view)
{
    if (!view) {
        manager->task_bar.view = NULL;
        manager->task_bar.default_y = 0;
        wl_list_remove(&manager->task_bar.output_geometry.link);
        wl_list_init(&manager->task_bar.output_geometry.link);
        return;
    }

    manager->task_bar.view = view;
    task_bar_update_coord();
    struct output *output = output_from_kywc_output(view->output);
    wl_signal_add(&output->events.geometry, &manager->task_bar.output_geometry);
}

static void tablet_mode_task_bar_show(bool show)
{
    struct view *view = manager->task_bar.view;
    if (!view) {
        return;
    }
    scene_node_set_enabled(&view->tree->node, show);
}

static void handle_view_parent(struct wl_listener *listener, void *data)
{
    struct tablet_mode_view *tablet_view = wl_container_of(listener, tablet_view, parent);
    if (!tablet_view->view->base.mapped) {
        return;
    }

    view_move_to_center(tablet_view->view);
}

static void tablet_mode_view_blur_rect_show(struct tablet_mode_view *tablet_view, bool show)
{
    if (!tablet_view->blur_rect) {
        return;
    }

    if (!show) {
        scene_node_set_enabled(&tablet_view->blur_rect->node, false);
        return;
    }

    scene_node_place_below(&tablet_view->blur_rect->node,
                           &tablet_view->view->current_proxy->tree->node);
    scene_node_set_enabled(&tablet_view->blur_rect->node, true);
}

static void tablet_mode_handle_view_update_capabilities(struct wl_listener *listener, void *data)
{
    struct tablet_mode_view *tablet_view =
        wl_container_of(listener, tablet_view, view_update_capabilities);
    struct view *view = tablet_view->view;
    struct view_update_capabilities_event *event = data;

    if (event->mask & KYWC_VIEW_MAXIMIZABLE) {
        if (view->parent) {
            event->state &= ~KYWC_VIEW_MAXIMIZABLE;
        }
    }
    if (event->mask & KYWC_VIEW_MAXIMIZE_BUTTON) {
        event->state &= ~KYWC_VIEW_MAXIMIZE_BUTTON;
    }
}

static struct view *tablet_mode_ancestor_from_view(struct view *view)
{
    struct view *ancestor = view;
    while (ancestor->parent) {
        ancestor = ancestor->parent;
    }

    return ancestor;
}

static void tablet_mode_view_activate(struct view *view)
{
    if (!KYWC_VIEW_IS_ACTIVATABLE(&view->base)) {
        return;
    }

    if (view != manager->task_bar.view) {
        tablet_mode_task_bar_show(false);
    }

    /* minimize the old activated view */
    struct tablet_mode_view *tablet_view = tablet_mode_view_from_view(view);
    struct view *activated_view = view_manager_get_activated();
    if (tablet_view && activated_view) {
        if (view != activated_view && !view_has_ancestor(view, activated_view) &&
            !view_has_descendant(view, activated_view)) {
            tablet_mode_view_minimized(tablet_mode_ancestor_from_view(activated_view), true);
        }
    }

    /* activate the view */
    struct view *descendant = view_find_descendant_modal(view);
    view_do_activate(descendant ? descendant : view);
    view_raise_to_top(view, true);

    if (tablet_view) {
        tablet_mode_view_blur_rect_show(tablet_view, true);
    }
}

static void tablet_mode_view_blur_rect_create(struct tablet_mode_view *tablet_view)
{
    if (tablet_view->blur_rect) {
        return;
    }

    if (!tablet_view->view->current_proxy) {
        return;
    }

    float color[4] = { 0.5, 0.5, 0.5, 0.5 };
    struct output *output = output_from_kywc_output(tablet_view->view->output);
    enum layer layer = tablet_view->view->base.kept_above
                           ? LAYER_ABOVE
                           : (tablet_view->view->base.kept_below ? LAYER_BELOW : LAYER_NORMAL);
    struct view_layer *view_layer =
        workspace_layer(tablet_view->view->current_proxy->workspace, layer);
    tablet_view->blur_rect =
        scene_rect_create(view_layer->tree, output->geometry.width, output->geometry.height, color);
    if (!tablet_view->blur_rect) {
        return;
    }

    pixman_region32_t region;
    pixman_region32_init(&region);
    scene_node_set_blur_region(&tablet_view->blur_rect->node, &region);
    pixman_region32_fini(&region);

    scene_node_place_below(&tablet_view->blur_rect->node,
                           &tablet_view->view->current_proxy->tree->node);
}

static void tablet_mode_view_blur_rect_destroy(struct tablet_mode_view *tablet_view)
{
    if (!tablet_view->blur_rect) {
        return;
    }

    scene_node_destroy(&tablet_view->blur_rect->node);
    tablet_view->blur_rect = NULL;
}

static struct tablet_mode_view *tablet_mode_view_create(struct view *view)
{
    struct tablet_mode_view *tablet_view = calloc(1, sizeof(*tablet_view));
    if (!tablet_view) {
        return NULL;
    }

    wl_list_insert(&manager->tablet_views, &tablet_view->link);

    tablet_view->view = view;
    tablet_view->parent.notify = handle_view_parent;
    wl_signal_add(&view->events.parent, &tablet_view->parent);
    tablet_view->view_update_capabilities.notify = tablet_mode_handle_view_update_capabilities;
    view_add_update_capabilities_listener(view, &tablet_view->view_update_capabilities);

    return tablet_view;
}

static void tablet_mode_view_destroy(struct tablet_mode_view *tablet_view)
{
    wl_list_remove(&tablet_view->link);
    wl_list_remove(&tablet_view->parent.link);
    wl_list_remove(&tablet_view->view_update_capabilities.link);
    tablet_mode_view_blur_rect_destroy(tablet_view);

    free(tablet_view);
}

static void tablet_mode_view_apply_minimized(struct view *view, bool minimized)
{
    view_do_minimized(view, minimized);
}

static void tablet_mode_view_minimized(struct view *view, bool minimized)
{
    struct tablet_mode_view *tablet_view = tablet_mode_view_from_view(view);
    if (!tablet_view || !KYWC_VIEW_IS_MINIMIZABLE(&view->base)) {
        return;
    }

    struct view *ancestor = tablet_mode_ancestor_from_view(view);
    if (!minimized && !view->base.maximized) {
        // restore minimized view and then maximize the view
        tablet_mode_view_apply_minimized(ancestor, minimized);
        tablet_mode_view_maximized(ancestor, true, ancestor->output);
        return;
    }

    tablet_mode_view_blur_rect_show(tablet_view, !minimized);
    tablet_mode_view_apply_minimized(minimized ? view : ancestor, minimized);
}

static void tablet_mode_view_maximized(struct view *view, bool maximized,
                                       struct kywc_output *kywc_output)
{
    struct tablet_mode_view *tablet_view = tablet_mode_view_from_view(view);
    if (!tablet_view || !maximized) {
        return;
    }

    if (KYWC_VIEW_IS_MAXIMIZABLE(&view->base)) {
        view_do_maximized(view, maximized, kywc_output);
    } else {
        view_move_to_center(view);
        if (!view->parent) {
            /* need blur if view can not maximized*/
            tablet_mode_view_blur_rect_create(tablet_view);
        }
    }
}

static void tablet_mode_view_map(struct view *view)
{
    positioner_add_new_view(view);

    if (!manager->task_bar.view) {
        if (strcmp(view->base.title, TASK_BAR) == 0) {
            tablet_mode_set_task_bar(view);
            tablet_mode_task_bar_show(false);
        }
    }
    if (!manager->tablet_desktop) {
        if (strcmp(view->base.title, TABLET_DESKTOP) == 0) {
            manager->tablet_desktop = view;
        }
    }
    if (!manager->pc_desktop) {
        if (strcmp(view->base.app_id, PC_DESKTOP) == 0) {
            manager->pc_desktop = view;
        }
    }

    if (view->base.role != KYWC_VIEW_ROLE_NORMAL || view_is_special(view)) {
        return;
    }

    view->mode_view = tablet_mode_view_create(view);
    tablet_mode_view_maximized(view, true, view->output);
}

static void tablet_mode_view_unmap(struct view *view)
{
    if (view == manager->task_bar.view) {
        tablet_mode_set_task_bar(NULL);
    } else if (view == manager->tablet_desktop) {
        manager->tablet_desktop = NULL;
    } else if (view == manager->pc_desktop) {
        manager->pc_desktop = NULL;
    }

    struct tablet_mode_view *tablet_view = tablet_mode_view_from_view(view);
    if (!tablet_view) {
        return;
    }

    tablet_mode_view_destroy(tablet_view);
    view->mode_view = NULL;
}

static void global_blur_rect_create(void)
{
    if (manager->global_blur_rect) {
        return;
    }

    float color[4] = { 0.1, 0.1, 0.1, 0.1 };
    struct output *output = input_current_output(input_manager_get_default_seat());
    struct view_layer *layer = view_manager_get_layer(LAYER_ON_SCREEN_DISPLAY, false);
    manager->global_blur_rect =
        scene_rect_create(layer->tree, output->geometry.width, output->geometry.height, color);
    if (!manager->global_blur_rect) {
        return;
    }

    pixman_region32_t region;
    pixman_region32_init_rect(&region, output->geometry.x, output->geometry.y,
                              output->geometry.width, output->geometry.height);
    scene_node_set_blur_region(&manager->global_blur_rect->node, &region);
    pixman_region32_fini(&region);
}

static void global_blur_rect_destroy(void)
{
    if (!manager->global_blur_rect) {
        return;
    }

    scene_node_destroy(&manager->global_blur_rect->node);
    manager->global_blur_rect = NULL;
}

static void global_blur_rect_set_blur(float percent)
{
    if (!manager->global_blur_rect) {
        global_blur_rect_create();
    }
    if (!manager->global_blur_rect) {
        return;
    }

    if (percent == 0.0) {
        scene_node_set_enabled(&manager->global_blur_rect->node, false);
        return;
    }

    /* offset 0-10 */
    float offset = percent * 10.0 / 1.0;
    scene_node_set_blur_level(&manager->global_blur_rect->node, 2, offset);
    scene_node_set_enabled(&manager->global_blur_rect->node, true);
}

void tablet_mode_handle_task_bar_gesture(enum gesture_stage stage, double dx, double dy)
{
    struct output *output = input_current_output(input_manager_get_default_seat());

    struct view *view = view_manager_get_activated();
    struct view *task_bar = manager->task_bar.view;
    if (!task_bar) {
        return;
    }

    // task bar follow finger if activated view is normal view
    if (tablet_mode_view_from_view(view) || view == task_bar) {
        if (stage == GESTURE_STAGE_STOP) {
            int threshold = output->geometry.height - task_bar->base.geometry.height / 2;
            if (task_bar->base.geometry.y > threshold) {
                tablet_mode_task_bar_show(false);
            } else {
                view_do_move(task_bar, task_bar->base.geometry.x, manager->task_bar.default_y);
            }
            /* hide task bar if trigger switcher */
            if (dy <= -TRIGGER_SWITCHER_THRESHOLD) {
                tablet_mode_task_bar_show(false);
            }
            return;
        }

        if (!task_bar->base.activated) {
            view_do_activate(task_bar);
            view_raise_to_top(task_bar, false);
        }
        // calculate follow y
        int y = output->geometry.y + output->geometry.height + dy * output->geometry.height;
        y = y < manager->task_bar.default_y ? manager->task_bar.default_y : y;
        tablet_mode_task_bar_show(true);
        view_do_move(task_bar, task_bar->base.geometry.x, y);
        return;
    }

    /* set blur */
    if (stage == GESTURE_STAGE_STOP) {
        global_blur_rect_set_blur(0.0);
        return;
    }
    dy = CLAMP(dy, -TRIGGER_SWITCHER_THRESHOLD, 0.0);
    global_blur_rect_set_blur(-dy / TRIGGER_SWITCHER_THRESHOLD);
}

static void tablet_mode_update_or_restore_theme(bool update)
{
    struct theme *theme = theme_manager_get_theme();
    if (update) {
        theme->icon_size = 32;
        theme->button_width = 40;
        theme->title_height = 64;
        theme->subtitle_height = 48;
        return;
    }

    theme->icon_size = 24;
    theme->button_width = 32;
    theme->title_height = 38;
    theme->subtitle_height = 38;
}

static void tablet_mode_hanlde_theme_pre_update(struct wl_listener *listener, void *data)
{
    struct theme_update_event *event = data;
    if (event->update_mask & THEME_UPDATE_MASK_DECORATION_SIZE) {
        event->update_mask &= ~THEME_UPDATE_MASK_DECORATION_SIZE;
        tablet_mode_update_or_restore_theme(true);
    }
}

static void tablet_mode_input_actions_block(bool enable)
{
    kywc_key_binding_block_type(KEY_BINDING_TYPE_WIN_MENU, enable);
    kywc_key_binding_block_type(KEY_BINDING_TYPE_SWITCH_WORKSPACE, enable);
}

static void input_actions_register(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(gestures); i++) {
        struct gesture *gesture = &gestures[i];
        if (gesture->binding) {
            kywc_gesture_binding_destroy(gesture->binding);
        }
        struct gesture_binding *binding = kywc_gesture_binding_create(
            gesture->type, gesture->stage, gesture->devices, gesture->directions, gesture->edges,
            gesture->fingers, gesture->follow_direction, gesture->follow_threshold, gesture->desc,
            gesture->func_id);
        if (!binding) {
            continue;
        }

        if (!kywc_gesture_binding_register(binding, function_manager_handle_gesture_binding,
                                           gesture)) {
            kywc_gesture_binding_destroy(binding);
            continue;
        }

        gesture->binding = binding;
    }
}

static void input_actions_unregister(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(gestures); i++) {
        struct gesture *gesture = &gestures[i];
        if (gesture->binding) {
            kywc_gesture_binding_destroy(gesture->binding);
        }
        gesture->binding = NULL;
    }
}

static void tablet_mode_enter(void)
{
    /* update theme */
    tablet_mode_update_or_restore_theme(true);
    theme_manager_update_theme(THEME_UPDATE_MASK_DECORATION_SIZE);
    theme_manager_add_update_listener(&manager->theme_pre_update, true);

    /* input actions */
    tablet_mode_input_actions_block(true);
    input_actions_register();

    struct view *view, *maximize_view = NULL;
    struct view_manager *view_manager = manager->view_manager;
    struct view *activated_view = view_manager_get_activated();
    wl_list_for_each(view, &view_manager->views, link) {
        if (!view->base.mapped) {
            continue;
        }

        if (strcmp(view->base.title, TASK_BAR) == 0) {
            tablet_mode_set_task_bar(view);
            tablet_mode_task_bar_show(false);
        } else if (strcmp(view->base.title, TABLET_DESKTOP) == 0) {
            manager->tablet_desktop = view;
        } else if (strcmp(view->base.app_id, PC_DESKTOP) == 0) {
            manager->pc_desktop = view;
        }

        if (view->base.role != KYWC_VIEW_ROLE_NORMAL || view_is_special(view)) {
            continue;
        }

        view->mode_view = tablet_mode_view_create(view);

        /* workspace */
        struct workspace *workspace = workspace_manager_get_current();
        struct tablet_mode_view *tablet_view = tablet_mode_view_from_view(view);
        tablet_view->remove_workspace = workspace != view->current_proxy->workspace;
        view_add_workspace(view, workspace);

        if (view == activated_view) {
            maximize_view = view;
            continue;
        }
        if (view_has_ancestor(view, activated_view)) {
            view_move_to_center(view);
            continue;
        }
        if (view_has_descendant(view, activated_view)) {
            tablet_mode_view_maximized(view, true, view->output);
            continue;
        }
        if (view_has_modal_property(view)) {
            tablet_mode_view_maximized(view, true, view->output);
            continue;
        }

        tablet_mode_view_minimized(view, true);
    }

    if (manager->pc_desktop) {
        view_add_fade_effect(manager->pc_desktop, FADE_OUT);
        scene_node_set_enabled(&manager->pc_desktop->tree->node, false);
    }

    if (manager->tablet_desktop) {
        scene_node_set_enabled(&manager->tablet_desktop->tree->node, true);
        view_add_fade_effect(manager->tablet_desktop, FADE_IN);
    }

    if (maximize_view) {
        view_raise_to_top(maximize_view, true);
        tablet_mode_view_maximized(maximize_view, true, maximize_view->output);
    }
}

static void tablet_mode_leave(void)
{
    /* desktop and task bar */
    tablet_mode_task_bar_show(false);
    tablet_mode_set_task_bar(NULL);
    if (manager->tablet_desktop) {
        view_add_fade_effect(manager->tablet_desktop, FADE_OUT);
        scene_node_set_enabled(&manager->tablet_desktop->tree->node, false);
        manager->tablet_desktop = NULL;
    }
    if (manager->pc_desktop) {
        scene_node_set_enabled(&manager->pc_desktop->tree->node, true);
        view_add_fade_effect(manager->pc_desktop, FADE_IN);
        manager->pc_desktop = NULL;
    }

    /* input actions */
    tablet_mode_input_actions_block(false);
    input_actions_unregister();

    /* global blur rect */
    global_blur_rect_destroy();

    struct tablet_mode_view *tablet_view, *tmp;
    wl_list_for_each_safe(tablet_view, tmp, &manager->tablet_views, link) {
        if (tablet_view->remove_workspace) {
            view_remove_workspace(tablet_view->view, tablet_view->view->current_proxy->workspace);
        }

        tablet_view->view->mode_view = NULL;
        tablet_mode_view_destroy(tablet_view);
    }

    wl_list_remove(&manager->theme_pre_update.link);
    tablet_mode_update_or_restore_theme(false);
    theme_manager_update_theme(THEME_UPDATE_MASK_DECORATION_SIZE);
}

static void tablet_mode_view_click(struct seat *seat, struct view *view, uint32_t button,
                                   bool pressed, enum click_state state)
{
    /* active current view */
    kywc_view_activate(&view->base);
    view_set_focus(view, seat);
}

static void tablet_mode_destroy(void)
{
    if (!manager) {
        return;
    }

    struct tablet_mode_view *tablet_view, *tmp;
    wl_list_for_each_safe(tablet_view, tmp, &manager->tablet_views, link) {
        tablet_mode_view_destroy(tablet_view);
    }

    free(manager);
    manager = NULL;
}

static const struct view_mode_interface tablet_mode_impl = {
    .name = "tablet_mode",

    .view_map = tablet_mode_view_map,
    .view_unmap = tablet_mode_view_unmap,

    .view_request_move = NULL,
    .view_request_resize = NULL,
    .view_request_minimized = tablet_mode_view_minimized,
    .view_request_maximized = tablet_mode_view_maximized,
    .view_request_fullscreen = NULL,
    .view_request_tiled = NULL,
    .view_request_activate = tablet_mode_view_activate,

    .view_request_show_menu = NULL,

    .view_click = tablet_mode_view_click,
    .view_hover = NULL,

    .view_mode_enter = tablet_mode_enter,
    .view_mode_leave = tablet_mode_leave,

    .mode_destroy = tablet_mode_destroy,
};

void tablet_mode_register(struct view_manager *view_manager)
{
    struct view_mode *mode = view_manager_mode_register(&tablet_mode_impl);
    if (!mode) {
        return;
    }

    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        view_manager_mode_unregister(mode);
        return;
    }
    wl_list_init(&manager->tablet_views);
    wl_list_init(&manager->task_bar.output_geometry.link);
    manager->view_manager = view_manager;
    manager->theme_pre_update.notify = tablet_mode_hanlde_theme_pre_update;
    manager->task_bar.output_geometry.notify = handle_output_geometry;
    tablet_mode_set_task_bar(NULL);
}
