// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <kywc/identifier.h>
#include <kywc/log.h>

#include "effect/translation.h"
#include "input/cursor.h"
#include "nls.h"
#include "server.h"
#include "util/macros.h"
#include "util/string.h"
#include "view_p.h"

#define TRANSLATION_THRESHOLD 0.35
#define TRANSLATION_EDGE_OFFSET_SCALE 0.2

struct workspace_manager {
    struct view_manager *view_manager;
    struct workspace_translation *translation; // for worskspace translation effect

    /* current activated workspace */
    struct workspace *current;
    struct workspace **workspaces;

    struct {
        struct wl_signal new_workspace;
        struct wl_signal active_workspace;
    } events;

    struct wl_listener display_destroy;
    struct wl_listener server_ready;
    struct wl_listener server_destroy;

    uint32_t count, rows, columns;
};

static struct workspace_manager *workspace_manager = NULL;

static struct shortcut {
    char *keybind;
    char *desc;
    enum function_id func_id;
} shortcuts[] = {
    { "Ctrl+Alt+Left:no", "workspace_left_switch_1", FUNC_ID_SWITCH_LEFT_WORKSPACE },
    { "Win+ctrl+left:no", "workspace_left_switch_2", FUNC_ID_SWITCH_LEFT_WORKSPACE },
    { "Ctrl+Alt+Right:no", "workspace_right_switch_1", FUNC_ID_SWITCH_RIGHT_WORKSPACE },
    { "Win+ctrl+Right:no", "workspace_right_switch_2", FUNC_ID_SWITCH_RIGHT_WORKSPACE },
    { "Ctrl+F1:no", "workspace_01_switch", FUNC_ID_SWITCH_01_WORKSPACE },
    { "Ctrl+F2:no", "workspace_02_switch", FUNC_ID_SWITCH_02_WORKSPACE },
    { "Ctrl+F3:no", "workspace_03_switch", FUNC_ID_SWITCH_03_WORKSPACE },
    { "Ctrl+F4:no", "workspace_04_switch", FUNC_ID_SWITCH_04_WORKSPACE },
};

static bool workspace_manager_can_switch_direction(enum direction direction)
{
    if (!workspace_manager || !workspace_manager->current) {
        return false;
    }

    int32_t current = workspace_manager->current->position;
    int32_t last = workspace_manager->count - 1;

    if (direction == DIRECTION_LEFT) {
        return current > 0;
    }

    if (direction == DIRECTION_RIGHT) {
        return current < last;
    }

    return false;
}

static void workspace_manager_switch_to(int switch_workspace)
{
    int32_t current = workspace_manager->current->position;
    int32_t last = workspace_manager->count - 1;
    int32_t pending = current;

    if (switch_workspace == DIRECTION_LEFT) {
        pending = current - 1;
    } else if (switch_workspace == DIRECTION_RIGHT) {
        pending = current + 1;
    } else {
        /* add ctrl+f1...f4 to workspace */
        pending = switch_workspace - 4;
        if (pending == current || pending > last) {
            return;
        }
        switch_workspace = pending > current ? DIRECTION_RIGHT : DIRECTION_LEFT;
    }

    struct workspace *pending_workspace =
        pending < 0 || pending > last ? NULL : workspace_manager->workspaces[pending];
    if (!workspace_add_automatic_translation_effect(workspace_manager->workspaces[current],
                                                    pending_workspace, switch_workspace)) {
        workspace_activate(pending_workspace);
    }
}

static struct gesture {
    enum gesture_type type;
    uint32_t stages;
    uint8_t fingers;
    enum gesture_device device;
    enum gesture_direction direction;
    enum gesture_edge edge;
    uint32_t follow_direction;
    double follow_threshold;
    char *desc;
    enum function_id func_id;
} gestures[] = {
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        4,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_LEFT,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT,
        0.0,
        "touchpad_(4)fingers_swipe_left",
        FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        4,
        GESTURE_DEVICE_TOUCHPAD,
        GESTURE_DIRECTION_RIGHT,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT,
        0.0,
        "touchpad_(4)fingers_swipe_right",
        FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        4,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_LEFT,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT,
        0.0,
        "touchscreen_(4)fingers_swipe_left",
        FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE,
    },
    {
        GESTURE_TYPE_SWIPE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        4,
        GESTURE_DEVICE_TOUCHSCREEN,
        GESTURE_DIRECTION_RIGHT,
        GESTURE_EDGE_NONE,
        GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT,
        0.0,
        "touchscreen_(4)fingers_swipe_right",
        FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE,
    },
};

static bool workspace_gesture_get_offset(enum gesture_stage stage, enum gesture_device device,
                                         enum gesture_direction binding_direction,
                                         enum gesture_direction follow_direction, double dx,
                                         double dy, enum gesture_direction *gesture_direction,
                                         float *offset)
{
    *gesture_direction =
        follow_direction != GESTURE_DIRECTION_NONE ? follow_direction : binding_direction;
    *offset = 0.0f;

    switch (*gesture_direction) {
    case GESTURE_DIRECTION_LEFT:
    case GESTURE_DIRECTION_RIGHT:
        *offset = -dx / (device == GESTURE_DEVICE_TOUCHPAD ? 300.0 : 1.0);
        break;
    case GESTURE_DIRECTION_UP:
    case GESTURE_DIRECTION_DOWN:
        *offset = -dy / (device == GESTURE_DEVICE_TOUCHPAD ? 300.0 : 1.0);
        break;
    case GESTURE_DIRECTION_NONE:
        if (stage != GESTURE_STAGE_STOP) {
            return false;
        }
        break;
    default:
        return false;
    }

    return true;
}

static enum direction workspace_gesture_direction_from_offset(float offset,
                                                              enum direction default_direction)
{
    if (offset < 0.0f) {
        return DIRECTION_LEFT;
    } else if (offset > 0.0f) {
        return DIRECTION_RIGHT;
    }

    return default_direction;
}

static void workspace_manager_switch_by_gesture(enum gesture_stage stage,
                                                enum gesture_device device,
                                                enum gesture_direction binding_direction,
                                                enum gesture_direction follow_direction, double dx,
                                                double dy, enum direction switch_direction)
{
    enum gesture_direction gesture_direction;
    float manual_offset;
    if (!workspace_gesture_get_offset(stage, device, binding_direction, follow_direction, dx, dy,
                                      &gesture_direction, &manual_offset)) {
        return;
    }

    enum direction direct =
        workspace_gesture_direction_from_offset(manual_offset, switch_direction);
    bool can_switch = workspace_manager_can_switch_direction(direct);

    if (!can_switch) {
        manual_offset *= TRANSLATION_EDGE_OFFSET_SCALE;
        if (stage == GESTURE_STAGE_STOP) {
            if (!workspace_manager->translation) {
                return;
            }

            struct workspace *last_show = workspace_translation_destroy(
                workspace_manager->translation, TRANSLATION_THRESHOLD);
            if (last_show) {
                workspace_activate(last_show);
            }
            workspace_manager->translation = NULL;
            return;
        }

        if (stage != GESTURE_STAGE_BEFORE && stage != GESTURE_STAGE_TRIGGER &&
            stage != GESTURE_STAGE_AFTER) {
            return;
        }
    }

    if (stage == GESTURE_STAGE_BEFORE ||
        (!can_switch && (stage == GESTURE_STAGE_TRIGGER || stage == GESTURE_STAGE_AFTER))) {
        if (!workspace_manager->translation) {
            workspace_manager->translation =
                workspace_create_manual_translation_effect(workspace_manager->current);
            if (!workspace_manager->translation) {
                return;
            }
            kywc_log(KYWC_DEBUG,
                     "Workspace manual translation direct: %d, gesture = %d, dx = %f, dy = %f, "
                     "offset = %f",
                     direct, gesture_direction, dx, dy, manual_offset);
        }

        workspace_translation_manual(workspace_manager->translation, direct, manual_offset);
    } else if (stage == GESTURE_STAGE_TRIGGER) {
        if (workspace_manager->translation) {
            workspace_translation_manual(workspace_manager->translation, direct, manual_offset);
        } else {
            workspace_manager_switch_to(direct);
            kywc_log(KYWC_DEBUG,
                     "Workspace auto translation direct: %d, switch = %d, gesture = %d, dx = %f, "
                     "dy = %f, offset = %f",
                     direct, switch_direction, gesture_direction, dx, dy, manual_offset);
        }
    } else if (stage == GESTURE_STAGE_AFTER && workspace_manager->translation) {
        workspace_translation_manual(workspace_manager->translation, direct, manual_offset);
    } else if (stage == GESTURE_STAGE_STOP && workspace_manager->translation) {
        struct workspace *last_show =
            workspace_translation_destroy(workspace_manager->translation, TRANSLATION_THRESHOLD);
        if (last_show) {
            workspace_activate(last_show);
        }
        workspace_manager->translation = NULL;
    }
}

void workspace_manager_handle_function(enum function_id id, enum gesture_stage stage,
                                       enum gesture_device device,
                                       enum gesture_direction binding_direction,
                                       enum gesture_direction follow_direction, double dx,
                                       double dy)
{
    switch (id) {
    case FUNC_ID_NEW_WORKSPACE:
        workspace_create(NULL, workspace_manager_get_count());
        break;
    case FUNC_ID_DEL_WORKSPACE:
        if (workspace_manager_get_count() > 1) {
            workspace_destroy(workspace_by_position(workspace_manager_get_count() - 1));
        }
        break;
    case FUNC_ID_SWITCH_LEFT_WORKSPACE:
        workspace_manager_switch_to(DIRECTION_LEFT);
        break;
    case FUNC_ID_SWITCH_RIGHT_WORKSPACE:
        workspace_manager_switch_to(DIRECTION_RIGHT);
        break;
    case FUNC_ID_SWITCH_01_WORKSPACE:
        workspace_manager_switch_to(4);
        break;
    case FUNC_ID_SWITCH_02_WORKSPACE:
        workspace_manager_switch_to(5);
        break;
    case FUNC_ID_SWITCH_03_WORKSPACE:
        workspace_manager_switch_to(6);
        break;
    case FUNC_ID_SWITCH_04_WORKSPACE:
        workspace_manager_switch_to(7);
        break;
    case FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE:
        workspace_manager_switch_by_gesture(stage, device, binding_direction, follow_direction, dx,
                                            dy, DIRECTION_LEFT);
        break;
    case FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE:
        workspace_manager_switch_by_gesture(stage, device, binding_direction, follow_direction, dx,
                                            dy, DIRECTION_RIGHT);
        break;
    default:
        break;
    }
}

static void workspace_register_shortcut(void)
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
    }
}

static void workspace_manager_update_count(uint32_t count)
{
    uint32_t column = count / workspace_manager->rows;
    if (count % workspace_manager->rows > 0) {
        column++;
    }

    workspace_manager->count = count;
    workspace_manager->columns = column;
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    assert(wl_list_empty(&workspace_manager->events.new_workspace.listener_list));
    assert(wl_list_empty(&workspace_manager->events.active_workspace.listener_list));
    wl_list_remove(&workspace_manager->server_destroy.link);
    wl_list_remove(&workspace_manager->server_ready.link);

    free(workspace_manager->workspaces);
    free(workspace_manager);
    workspace_manager = NULL;
}

static void handle_display_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&workspace_manager->display_destroy.link);

    workspace_manager->current = NULL;
    /* workspace_destroy will update count */
    while (workspace_manager->count) {
        workspace_destroy(workspace_manager->workspaces[0]);
    }
}

static void handle_server_ready(struct wl_listener *listener, void *data)
{
    /* create the default workspace and activate it */
    workspace_activate(
        workspace_create(workspace_manager->view_manager->state.workspace_names[0], 0));
    /* create workspaces according to configuration */
    for (uint32_t i = 1; i < workspace_manager->view_manager->state.num_workspaces; i++) {
        workspace_create(workspace_manager->view_manager->state.workspace_names[i], i);
    }
}

bool workspace_manager_create(struct view_manager *view_manager)
{
    workspace_manager = calloc(1, sizeof(*workspace_manager));
    if (!workspace_manager) {
        return false;
    }

    workspace_manager->view_manager = view_manager;
    workspace_manager->workspaces = calloc(MAX_WORKSPACES, sizeof(struct workspace *));
    workspace_manager->rows = 2;

    wl_signal_init(&workspace_manager->events.new_workspace);
    wl_signal_init(&workspace_manager->events.active_workspace);

    workspace_manager->server_ready.notify = handle_server_ready;
    wl_signal_add(&view_manager->server->events.ready, &workspace_manager->server_ready);
    workspace_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(view_manager->server, &workspace_manager->server_destroy);
    workspace_manager->display_destroy.notify = handle_display_destroy;
    wl_display_add_destroy_listener(view_manager->server->display,
                                    &workspace_manager->display_destroy);

    ky_workspace_manager_create(view_manager->server);
    /* kde-plasma-virtual-desktop support */
    kde_virtual_desktop_management_create(view_manager->server);

    workspace_register_shortcut();

    return true;
}

void workspace_manager_add_new_listener(struct wl_listener *listener)
{
    wl_signal_add(&workspace_manager->events.new_workspace, listener);
}

void workspace_manager_add_active_listener(struct wl_listener *listener)
{
    wl_signal_add(&workspace_manager->events.active_workspace, listener);
}

void workspace_manager_set_rows(uint32_t rows)
{
    if (rows == workspace_manager->rows) {
        return;
    }

    workspace_manager->rows = rows;
    /* update columns */
    workspace_manager_update_count(workspace_manager->count);
}

uint32_t workspace_manager_get_rows(void)
{
    return workspace_manager->rows;
}

struct workspace *workspace_manager_get_current(void)
{
    return workspace_manager->current;
}

uint32_t workspace_manager_get_count(void)
{
    return workspace_manager->count;
}

void workspace_manager_for_each_workspace(workspace_iterator_func_t iterator, void *data)
{
    for (uint32_t i = 0; i < workspace_manager->count; i++) {
        if (iterator(workspace_manager->workspaces[i], data)) {
            break;
        }
    }
}

static void workspace_set_enabled(struct workspace *workspace, bool enabled)
{
    for (int i = 0; i < 3; i++) {
        scene_node_set_enabled(&workspace->layers[i].tree->node, enabled);
    }
}

void workspace_update_name(struct workspace *workspace, const char *name)
{
    if (workspace->name && name && strcmp(workspace->name, name) == 0) {
        return;
    }

    free((void *)workspace->name);
    workspace->name = STRING_VALID(name)
                          ? strdup(name)
                          : string_create("%s %d", tr("Desktop"), workspace->position + 1);
    workspace->has_custom_name = STRING_VALID(name);

    wl_signal_emit_mutable(&workspace->events.name, NULL);
}

/* auto add views in all workspaces */
static void workspace_auto_add_views(struct workspace *workspace)
{
    if (!workspace_manager->view_manager->server->ready) {
        return;
    }
    struct workspace *first_workspace = workspace_manager->workspaces[0];
    struct view_proxy *view_proxy;
    wl_list_for_each(view_proxy, &first_workspace->view_proxies, workspace_link) {
        struct view *view = view_proxy->view;
        if (view->base.sticky) {
            view_add_workspace(view, workspace);
        }
    }
}

static const char *workspace_handle_desc(const struct scene_node *node, void *data)
{
    struct view_layer *layer = data;
    int index = MIN(layer->layer - LAYER_BELOW, 2);
    struct workspace *workspace = wl_container_of(layer, workspace, layers[index]);
    return workspace->name;
}

static const struct scene_node_role_interface role_impl = {
    .desc = workspace_handle_desc,
};

struct workspace *workspace_create(const char *name, uint32_t position)
{
    /* too many workspaces, reject it */
    if (workspace_manager->count == MAX_WORKSPACES) {
        return NULL;
    }

    struct workspace *workspace = calloc(1, sizeof(*workspace));
    if (!workspace) {
        return NULL;
    };

    if (position > workspace_manager->count) {
        position = workspace_manager->count;
    }

    wl_list_init(&workspace->view_proxies);
    wl_signal_init(&workspace->events.name);
    wl_signal_init(&workspace->events.position);
    wl_signal_init(&workspace->events.activate);
    wl_signal_init(&workspace->events.destroy);
    wl_signal_init(&workspace->events.view_enter);
    wl_signal_init(&workspace->events.view_leave);

    struct view_layer *layers = workspace_manager->view_manager->layers;
    int layer[3] = { LAYER_BELOW, LAYER_NORMAL, LAYER_ABOVE };
    /* create 3 tree per workspace and disabled default */
    for (size_t i = 0; i < ARRAY_SIZE(layer); i++) {
        struct view_layer *workspace_layer = &workspace->layers[i];
        workspace_layer->layer = layer[i];
        workspace_layer->tree = scene_tree_create(layers[layer[i]].tree);
        scene_node_set_role(&workspace_layer->tree->node, SCENE_ROLE_WORKSPACE, &role_impl,
                            workspace_layer);
        wl_list_init(&workspace_layer->views);
    }
    workspace_set_enabled(workspace, false);

    /* insert to workspace manager workspaces */
    for (uint32_t i = workspace_manager->count; i > position; i--) {
        workspace_manager->workspaces[i] = workspace_manager->workspaces[i - 1];
        workspace_manager->workspaces[i]->position = i;
    }
    workspace->position = position;
    workspace_manager->workspaces[position] = workspace;
    workspace_manager_update_count(workspace_manager->count + 1);

    workspace->uuid = kywc_identifier_uuid_generate();
    workspace_update_name(workspace, name);
    wl_signal_emit_mutable(&workspace_manager->events.new_workspace, workspace);
    /* after new_workspace */
    workspace_auto_add_views(workspace);

    return workspace;
}

static void fix_workspace(struct workspace *workspace)
{
    /* fixup workspace position */
    for (uint32_t i = workspace->position; i < workspace_manager->count - 1; i++) {
        workspace_manager->workspaces[i] = workspace_manager->workspaces[i + 1];
        workspace_manager->workspaces[i]->position = i;
        wl_signal_emit_mutable(&workspace_manager->workspaces[i]->events.position, NULL);
        if (!workspace_manager->workspaces[i]->has_custom_name) {
            workspace_update_name(workspace_manager->workspaces[i], NULL);
        }
    }
    workspace_manager_update_count(workspace_manager->count - 1);

    if (workspace_manager->count == 0) {
        return;
    }

    /* fixup activated workspace */
    struct workspace *activate_workspace =
        workspace_manager->workspaces[workspace->position ? workspace->position - 1 : 0];
    if (workspace_manager->current == workspace) {
        workspace_activate(activate_workspace);
    }

    /* move all views to activate workspace */
    struct view_proxy *view_proxy, *tmp;
    wl_list_for_each_reverse_safe(view_proxy, tmp, &workspace->view_proxies, workspace_link) {
        struct view_proxy *new_proxy = view_add_workspace(view_proxy->view, activate_workspace);
        if (new_proxy) {
            view_set_current_proxy(view_proxy->view, new_proxy);
        }
        view_proxy_destroy(view_proxy);
    }
}

void workspace_destroy(struct workspace *workspace)
{
    kywc_log(KYWC_INFO, "Workspace %s destroy", workspace->name);

    fix_workspace(workspace);

    wl_signal_emit_mutable(&workspace->events.destroy, NULL);
    assert(wl_list_empty(&workspace->events.view_enter.listener_list));
    assert(wl_list_empty(&workspace->events.view_leave.listener_list));
    assert(wl_list_empty(&workspace->events.name.listener_list));
    assert(wl_list_empty(&workspace->events.position.listener_list));
    assert(wl_list_empty(&workspace->events.activate.listener_list));
    assert(wl_list_empty(&workspace->events.destroy.listener_list));

    /* destroy trees, trees must be empty */
    for (int i = 0; i < 3; i++) {
        scene_node_destroy(&workspace->layers[i].tree->node);
        struct view *view, *tmp;
        wl_list_for_each_safe(view, tmp, &workspace->layers[i].views, layer_link) {
            wl_list_remove(&view->layer_link);
            wl_list_init(&view->layer_link);
        }
    }

    free((void *)workspace->uuid);
    free((void *)workspace->name);
    free(workspace);
}

static void workspace_set_activated(struct workspace *workspace, bool activated)
{
    if (workspace->activated == activated) {
        return;
    }

    workspace_set_enabled(workspace, activated);

    /* reparent view_tree which in multi workspace */
    struct view_proxy *view_proxy;
    wl_list_for_each(view_proxy, &workspace->view_proxies, workspace_link) {
        if (view_proxy->view->current_proxy != view_proxy) {
            view_set_current_proxy(view_proxy->view, view_proxy);
        }
    }

    workspace->activated = activated;
    wl_signal_emit_mutable(&workspace->events.activate, NULL);
}

void workspace_activate(struct workspace *workspace)
{
    struct workspace *old = workspace_manager->current;
    if (!workspace || old == workspace) {
        return;
    }

    if (old) {
        workspace_set_activated(old, false);
    }

    /* disable show desktop when switching between workspaces */
    view_manager_show_desktop(false, true);

    workspace_manager->current = workspace;
    workspace_set_activated(workspace, true);

    /* auto activate topmost enabled view */
    view_activate_topmost(true);

    cursor_rebase_all(false);
    wl_signal_emit_mutable(&workspace_manager->events.active_workspace, workspace);
    kywc_log(KYWC_INFO, "Workspace %s(%d) is activated", workspace->name, workspace->position);
}

void workspace_activate_with_effect(struct workspace *workspace)
{
    struct workspace *current = workspace_manager->current;
    enum direction direction =
        current->position < workspace->position ? DIRECTION_RIGHT : DIRECTION_LEFT;

    if (!workspace_add_automatic_translation_effect(current, workspace, direction)) {
        workspace_activate(workspace);
    }
}

struct view_layer *workspace_layer(struct workspace *workspace, enum layer layer)
{
    switch (layer) {
    case LAYER_BELOW:
        return &workspace->layers[0];
    case LAYER_NORMAL:
        return &workspace->layers[1];
    case LAYER_ABOVE:
        return &workspace->layers[2];
    default:
        return NULL;
    }
}

struct workspace *workspace_by_position(uint32_t position)
{
    if (position >= workspace_manager->count) {
        return NULL;
    }

    return workspace_manager->workspaces[position];
}

struct workspace *workspace_by_uuid(const char *uuid)
{
    struct workspace *workspace;
    for (uint32_t i = 0; i < workspace_manager->count; i++) {
        workspace = workspace_manager->workspaces[i];
        if (strcmp(workspace->uuid, uuid) == 0) {
            return workspace;
        }
    }
    return NULL;
}

void workspace_set_position(struct workspace *workspace, uint32_t position)
{
    /* insert to last if position is too bigger */
    if (position >= workspace_manager->count) {
        position = workspace_manager->count - 1;
    }
    if (position == workspace->position) {
        return;
    }

    /* move workspaces */
    for (uint32_t i = workspace->position; i > position; i--) {
        workspace_manager->workspaces[i] = workspace_manager->workspaces[i - 1];
        workspace_manager->workspaces[i]->position = i;
        wl_signal_emit_mutable(&workspace_manager->workspaces[i]->events.position, NULL);
        if (!workspace_manager->workspaces[i]->has_custom_name) {
            workspace_update_name(workspace_manager->workspaces[i], NULL);
        }
    }
    for (uint32_t i = workspace->position; i < position; i++) {
        workspace_manager->workspaces[i] = workspace_manager->workspaces[i + 1];
        workspace_manager->workspaces[i]->position = i;
        wl_signal_emit_mutable(&workspace_manager->workspaces[i]->events.position, NULL);
        if (!workspace_manager->workspaces[i]->has_custom_name) {
            workspace_update_name(workspace_manager->workspaces[i], NULL);
        }
    }

    workspace->position = position;
    workspace_manager->workspaces[position] = workspace;
    wl_signal_emit_mutable(&workspace->events.position, NULL);
    if (!workspace->has_custom_name) {
        workspace_update_name(workspace, NULL);
    }
}
