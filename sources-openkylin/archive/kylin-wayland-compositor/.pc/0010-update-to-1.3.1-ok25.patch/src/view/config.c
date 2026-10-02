// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "config.h"
#include "util/dbus.h"
#include "view_p.h"

static const char *service_path = "/com/kylin/Wlcom/View";
static const char *service_interface = "com.kylin.Wlcom.View";

struct close_window {
    struct wl_list link;
    struct view *view;
    struct wl_listener unmap; /* use unmap, wps is not destroyed */
};

static struct close_windows_context {
    struct wl_list windows;
    struct wl_event_source *timer;
    sd_bus_message *msg;
} *context = NULL;

static void close_windows_context_destroy(bool ok)
{
    if (context->timer) {
        wl_event_source_remove(context->timer);
    }

    struct close_window *window, *tmp;
    wl_list_for_each_safe(window, tmp, &context->windows, link) {
        wl_list_remove(&window->unmap.link);
        wl_list_remove(&window->link);
        free(window);
    }

    if (context->msg) {
        sd_bus_reply_method_return(context->msg, "b", ok);
        sd_bus_message_unref(context->msg);
    }

    free(context);
    context = NULL;
}

static void handle_view_unmap(struct wl_listener *listener, void *data)
{
    struct close_window *window = wl_container_of(listener, window, unmap);
    wl_list_remove(&window->unmap.link);
    wl_list_remove(&window->link);
    free(window);

    if (wl_list_empty(&context->windows)) {
        close_windows_context_destroy(true);
    }
}

static int handle_timeout(void *data)
{
    close_windows_context_destroy(false);
    return 0;
}

static int close_windows(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    if (context) {
        close_windows_context_destroy(false);
    }

    context = calloc(1, sizeof(*context));
    if (!context) {
        return sd_bus_reply_method_return(m, "b", false);
    }

    struct view_manager *manager = userdata;
    struct workspace *workspace = workspace_manager_get_current();
    wl_list_init(&context->windows);

    struct view *view;
    wl_list_for_each(view, &manager->views, link) {
        if (!view->base.mapped || !view->current_proxy) {
            continue;
        }
        /* move all views to current workspace */
        view_set_workspace(view, workspace);
    }

    struct view_proxy *view_proxy;
    wl_list_for_each(view_proxy, &workspace->view_proxies, workspace_link) {
        view = view_proxy->view;
        if (!view->base.mapped || !view->impl->close) {
            continue;
        }
        struct close_window *window = calloc(1, sizeof(*window));
        if (!window) {
            continue;
        }
        window->view = view;
        wl_list_insert(&context->windows, &window->link);
        window->unmap.notify = handle_view_unmap;
        wl_signal_add(&view->base.events.unmap, &window->unmap);
        /* call close callback to skip view_is_closeable */
        view->impl->close(view);
    }

    /* no window needs to close, return true */
    if (wl_list_empty(&context->windows)) {
        close_windows_context_destroy(false);
        return sd_bus_reply_method_return(m, "b", true);
    }

    context->timer = wl_event_loop_add_timer(manager->server->event_loop, handle_timeout, context);
    if (!context->timer) {
        close_windows_context_destroy(false);
        return sd_bus_reply_method_return(m, "b", false);
    }

    wl_event_source_timer_update(context->timer, 10000);
    context->msg = sd_bus_message_ref(m);

    return 1;
}

static int have_active_fullscreen_view(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    struct view *view = manager->activated.view;
    if (view && view->base.fullscreen && view->base.role == KYWC_VIEW_ROLE_NORMAL) {
        struct view_layer *view_layer = view_manager_get_layer_by_node(&view->tree->node, false);
        if (view_layer->layer == LAYER_ACTIVE) {
            return sd_bus_reply_method_return(m, "b", true);
        }
    }
    return sd_bus_reply_method_return(m, "b", false);
}

static int get_view_adsorption(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    return sd_bus_reply_method_return(m, "u", manager->state.view_adsorption);
}

static int set_view_adsorption(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    uint32_t adsorption;
    CK(sd_bus_message_read(m, "u", &adsorption));

    if (adsorption > VIEW_ADSORPTION_ALL) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid adsorption.");
        return sd_bus_reply_method_error(m, &error);
    }

    manager->state.view_adsorption = adsorption;

    return sd_bus_reply_method_return(m, NULL);
}

static int set_csd_round_corner(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    uint32_t enabled;
    CK(sd_bus_message_read(m, "b", &enabled));

    if (manager->state.csd_round_corner != enabled) {
        manager->state.csd_round_corner = enabled;

        struct view *view;
        wl_list_for_each(view, &manager->views, link) {
            if (view->base.ssd == KYWC_SSD_NONE) {
                view_update_round_corner(view);
            }
        }
    }

    return sd_bus_reply_method_return(m, NULL);
}

static int get_minimize_effect(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    return sd_bus_reply_method_return(m, "u", manager->state.minimize_effect_type);
}

static int set_minimize_effect(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    uint32_t minimize_effect;
    CK(sd_bus_message_read(m, "u", &minimize_effect));

    if (minimize_effect < MINIMIZE_EFFECT_TYPE_NUM) {
        manager->state.minimize_effect_type = minimize_effect;
    }

    return sd_bus_reply_method_return(m, NULL);
}

static int set_resize_filter(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    uint32_t resize_filter[2];
    CK(sd_bus_message_read(m, "uu", &resize_filter[0], &resize_filter[1]));

    for (int i = 0; i < 2; i++) {
        if (resize_filter[i] <= 100) {
            manager->state.resize_filter[i] = resize_filter[i];
        }
    }

    return sd_bus_reply_method_return(m, NULL);
}

static int list_all_views(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "(ssi)"));

    struct view *view;
    wl_list_for_each(view, &manager->views, link) {
        if (!view->base.mapped) {
            continue;
        }
        const char *app_id = view->base.app_id;
        const char *uuid = view->base.uuid;
        int pid = view->pid;
        sd_bus_message_append(reply, "(ssi)", app_id, uuid, pid);
    }

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static int list_view_states(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *uuid;
    CK(sd_bus_message_read(m, "s", &uuid));

    struct kywc_view *kywc_view = kywc_view_by_uuid(uuid);
    if (!kywc_view) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid uuid.");
        return sd_bus_reply_method_error(m, &error);
    }

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "(sai)"));

    sd_bus_message_append(reply, "(sai)", "role", 1, kywc_view->role);
    sd_bus_message_append(reply, "(sai)", "geometry", 4, kywc_view->geometry.x,
                          kywc_view->geometry.y, kywc_view->geometry.width,
                          kywc_view->geometry.height);
    sd_bus_message_append(reply, "(sai)", "margin", 4, kywc_view->margin.off_x,
                          kywc_view->margin.off_y, kywc_view->margin.off_width,
                          kywc_view->margin.off_height);
    sd_bus_message_append(reply, "(sai)", "padding", 4, kywc_view->padding.top,
                          kywc_view->padding.bottom, kywc_view->padding.left,
                          kywc_view->padding.right);
    sd_bus_message_append(reply, "(sai)", "ssd", 1, kywc_view->ssd);
    sd_bus_message_append(reply, "(sai)", "minimize size", 2, kywc_view->min_width,
                          kywc_view->min_height);
    sd_bus_message_append(reply, "(sai)", "maximize size", 2, kywc_view->max_width,
                          kywc_view->max_height);
    sd_bus_message_append(reply, "(sai)", "kept_above", 1, kywc_view->kept_above);
    sd_bus_message_append(reply, "(sai)", "kept_below", 1, kywc_view->kept_below);
    sd_bus_message_append(reply, "(sai)", "minimized", 1, kywc_view->minimized);
    sd_bus_message_append(reply, "(sai)", "maximized", 1, kywc_view->maximized);
    sd_bus_message_append(reply, "(sai)", "fullscreen", 1, kywc_view->fullscreen);
    sd_bus_message_append(reply, "(sai)", "activated", 1, kywc_view->activated);
    sd_bus_message_append(reply, "(sai)", "tiled", 1, kywc_view->tiled);
    sd_bus_message_append(reply, "(sai)", "modal", 1, kywc_view->modal);
    sd_bus_message_append(reply, "(sai)", "skip_taskbar", 1, kywc_view->skip_taskbar);
    sd_bus_message_append(reply, "(sai)", "skip_switcher", 1, kywc_view->skip_switcher);

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static int list_all_modes(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct view_manager *manager = userdata;
    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "(sb)"));

    struct view_mode *mode;
    wl_list_for_each(mode, &manager->view_modes, link) {
        sd_bus_message_append(reply, "(sb)", mode->impl->name, mode == manager->mode);
    }

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

#if 0
static int set_view_mode(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *name;
    CK(sd_bus_message_read(m, "s", &name));

    if (!view_manager_set_view_mode(name)) {
        const sd_bus_error error =
            SD_BUS_ERROR_MAKE_CONST(SD_BUS_ERROR_INVALID_ARGS, "Invalid mode name.");
        return sd_bus_reply_method_error(m, &error);
    }

    return sd_bus_reply_method_return(m, NULL);
}
#endif

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("CloseWindows", "", "b", close_windows, 0),
    SD_BUS_METHOD("HaveFullScreenActiveWindow", "", "b", have_active_fullscreen_view, 0),
    SD_BUS_METHOD("GetViewAdsorption", "", "u", get_view_adsorption, 0),
    SD_BUS_METHOD("SetViewAdsorption", "u", "", set_view_adsorption, 0),
    SD_BUS_METHOD("SetCSDRoundCorner", "b", "", set_csd_round_corner, 0),
    SD_BUS_METHOD("GetMinimizeEffect", "", "u", get_minimize_effect, 0),
    SD_BUS_METHOD("SetMinimizeEffect", "u", "", set_minimize_effect, 0),
    SD_BUS_METHOD("SetResizeFilter", "uu", "", set_resize_filter, 0),
    SD_BUS_METHOD("ListAllViews", "", "a(ssi)", list_all_views, 0),
    SD_BUS_METHOD("ListViewStates", "s", "a(sai)", list_view_states, 0),

    SD_BUS_METHOD("ListAllModes", "", "a(sb)", list_all_modes, 0),
    // SD_BUS_METHOD("SetViewMode", "s", "", set_view_mode, 0),
    SD_BUS_VTABLE_END,
};

bool view_manager_config_init(struct view_manager *view_manager)
{
    view_manager->config = config_manager_add_config("Views");
    if (!view_manager->config) {
        return false;
    }
    return dbus_register_object(NULL, service_path, service_interface, service_vtable,
                                view_manager);
}

bool view_read_config(struct view_manager *view_manager)
{
    if (!view_manager->config || !view_manager->config->json) {
        return false;
    }

    json_object *data;
    if (json_object_object_get_ex(view_manager->config->json, "num_workspaces", &data)) {
        view_manager->state.num_workspaces = json_object_get_int(data);
    }
    if (json_object_object_get_ex(view_manager->config->json, "workspace_names", &data)) {
        struct array_list *array = json_object_get_array(data);
        if (array) {
            for (uint32_t i = 0; i < view_manager->state.num_workspaces; i++) {
                struct json_object *item = array_list_get_idx(array, i);
                view_manager->state.workspace_names[i] = json_object_get_string(item);
            }
        }
    }
    if (json_object_object_get_ex(view_manager->config->json, "view_adsorption", &data)) {
        view_manager->state.view_adsorption = json_object_get_int(data);
    }
    if (json_object_object_get_ex(view_manager->config->json, "csd_round_corner", &data)) {
        view_manager->state.csd_round_corner = json_object_get_boolean(data);
    }
    if (json_object_object_get_ex(view_manager->config->json, "view_mode", &data)) {
        const char *name = json_object_get_string(data);
        view_manager->mode = view_manager_mode_from_name(name);
    }
    if (json_object_object_get_ex(view_manager->config->json, "minimize_effect", &data)) {
        int minimize_effect = json_object_get_int(data);
        if (minimize_effect < MINIMIZE_EFFECT_TYPE_NUM) {
            view_manager->state.minimize_effect_type = minimize_effect;
        }
    }
    if (json_object_object_get_ex(view_manager->config->json, "resize_filter", &data)) {
        struct array_list *array = json_object_get_array(data);
        if (array) {
            for (uint32_t i = 0; i < 2; i++) {
                struct json_object *item = array_list_get_idx(array, i);
                uint32_t resize_filter = json_object_get_int(item);
                if (resize_filter <= 100) {
                    view_manager->state.resize_filter[i] = resize_filter;
                }
            }
        }
    }

    return true;
}

static bool add_workspaces_arr(struct workspace *workspace, void *data)
{
    struct json_object *arry = (struct json_object *)data;
    json_object_array_add(arry, workspace->has_custom_name ? json_object_new_string(workspace->name)
                                                           : json_object_new_string(""));
    return false;
}

static struct json_object *get_workspaces_arr(void)
{
    struct json_object *arry = json_object_new_array();
    workspace_manager_for_each_workspace(add_workspaces_arr, arry);
    return arry;
}

static struct json_object *get_resize_filter_arr(struct view_manager *view_manager)
{
    struct json_object *arry = json_object_new_array();
    for (int i = 0; i < 2; i++) {
        json_object_array_add(arry, json_object_new_int(view_manager->state.resize_filter[i]));
    }
    return arry;
}

void view_write_config(struct view_manager *view_manager)
{
    if (!view_manager->config) {
        return;
    }

    json_object_object_add(view_manager->config->json, "num_workspaces",
                           json_object_new_int(workspace_manager_get_count()));
    json_object_object_add(view_manager->config->json, "workspace_names", get_workspaces_arr());
    json_object_object_add(view_manager->config->json, "view_adsorption",
                           json_object_new_int(view_manager->state.view_adsorption));
    json_object_object_add(view_manager->config->json, "csd_round_corner",
                           json_object_new_boolean(view_manager->state.csd_round_corner));
    json_object_object_add(view_manager->config->json, "view_mode",
                           json_object_new_string(view_manager->mode->impl->name));
    json_object_object_add(view_manager->config->json, "minimize_effect",
                           json_object_new_int(view_manager->state.minimize_effect_type));
    json_object_object_add(view_manager->config->json, "resize_filter",
                           get_resize_filter_arr(view_manager));
}
