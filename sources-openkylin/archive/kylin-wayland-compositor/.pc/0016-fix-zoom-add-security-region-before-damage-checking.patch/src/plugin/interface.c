// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdlib.h>

#include "output.h"
#include "plugin_p.h"
#include "security/security.h"
#include "view/view.h"

static void plugin_hook_point_handler(struct wl_listener *listener, void *data)
{
    struct plugin_hook *hook = wl_container_of(listener, hook, listener);
    struct plugin_manager *plugin_manager =
        wl_container_of(hook, plugin_manager, hooks[hook->point]);

    struct plugin *plugin, *tmp;
    wl_list_for_each_safe(plugin, tmp, &plugin_manager->plugins, link) {
        if (!plugin->loaded || !plugin->enabled || !plugin->hooks) {
            continue;
        }

        switch (hook->point) {
        case PLUGIN_HOOK_NEW_VIEW:
            if (plugin->hooks->new_view) {
                plugin->hooks->new_view(data);
            }
            break;
        case PLUGIN_HOOK_NEW_OUTPUT:
            if (plugin->hooks->new_output) {
                plugin->hooks->new_output(data);
            }
            break;
        case PLUGIN_HOOK_LAYOUT_DAMAGE:
            if (plugin->hooks->layout_damage) {
                plugin->hooks->layout_damage(data);
            }
        default:
            break;
        }
    }
}

void plugin_manager_init_hooks(struct plugin_manager *plugin_manager)
{
    for (int point = 0; point < PLUGIN_HOOK_LAST; point++) {
        struct plugin_hook *hook = &plugin_manager->hooks[point];
        hook->listener.notify = plugin_hook_point_handler;
        wl_list_init(&hook->listener.link);
        hook->point = point;
        hook->ref_count = 0;
    }
}

void plugin_manager_finish_hooks(struct plugin_manager *plugin_manager)
{
    for (int point = 0; point < PLUGIN_HOOK_LAST; point++) {
        struct plugin_hook *hook = &plugin_manager->hooks[point];
        assert(hook->ref_count == 0);
        wl_list_remove(&hook->listener.link);
    }
}

static void plugin_manager_enable_hook(struct plugin_manager *plugin_manager,
                                       enum plugin_hook_point point, bool enabled)
{
    struct plugin_hook *hook = &plugin_manager->hooks[point];
    bool old = hook->ref_count > 0;

    if (enabled) {
        hook->ref_count++;
    } else {
        assert(hook->ref_count > 0);
        hook->ref_count--;
    }

    bool now = hook->ref_count > 0;
    if (old == now) {
        return;
    }

    wl_list_remove(&hook->listener.link);
    wl_list_init(&hook->listener.link);
    if (!now) {
        return;
    }

    switch (point) {
    case PLUGIN_HOOK_NEW_VIEW:
        kywc_view_add_new_listener(&hook->listener);
        break;
    case PLUGIN_HOOK_NEW_OUTPUT:
        output_manager_add_new_enabled_listener(&hook->listener);
        break;
    case PLUGIN_HOOK_LAYOUT_DAMAGE:
        output_manager_add_layout_damage_listener(&hook->listener);
        break;
    default:
        break;
    }
}

void plugin_enable_hooks(struct plugin *plugin, bool enabled)
{
    const struct kywc_plugin_hooks *hooks = plugin->hooks;
    if (!hooks) {
        return;
    }

    struct plugin_manager *plugin_manager = plugin->manager;

    if (hooks->new_view) {
        plugin_manager_enable_hook(plugin_manager, PLUGIN_HOOK_NEW_VIEW, enabled);
    }
    if (hooks->new_output) {
        plugin_manager_enable_hook(plugin_manager, PLUGIN_HOOK_NEW_OUTPUT, enabled);
    }
    if (hooks->layout_damage) {
        plugin_manager_enable_hook(plugin_manager, PLUGIN_HOOK_LAYOUT_DAMAGE, enabled);
    }
}

static void plugin_set_hooks(void *plugin, const struct kywc_plugin_hooks *hooks)
{
    struct plugin *p = plugin;
    if (p->hooks) {
        return;
    }

    p->hooks = hooks;
    plugin_enable_hooks(p, true);
}

static struct security *security_view_create(const struct kywc_view *view,
                                             enum security_decision decision,
                                             const struct security_interface *impl, void *data)
{
    struct view *_view = wl_container_of(view, _view, base);
    return security_create_for_node(&_view->tree->node, decision, impl, data);
}

static struct security *security_output_create(const struct kywc_output *output,
                                               enum security_decision decision,
                                               const struct security_interface *impl, void *data)
{
    struct output *_output = wl_container_of(output, _output, base);
    return security_create_for_output(_output->scene_output, decision, impl, data);
}

const struct kywc_plugin_interface *kywc_plugin_get_interface(void *plugin)
{
    struct plugin *p = plugin;
    if (p->interface) {
        return p->interface;
    }

    struct kywc_plugin_interface *interface = calloc(1, sizeof(*interface));
    if (!interface) {
        return NULL;
    }

    struct plugin_manager *manager = p->manager;
    interface->abi_version = p->info->abi_version;
    interface->display = manager->server->display;

    interface->set_hooks = plugin_set_hooks;

    interface->security_add_global_filter = security_add_global_filter;
    interface->security_remove_global_filter = security_remove_global_filter;

    interface->security_view_create = security_view_create;
    interface->security_output_create = security_output_create;
    interface->security_get_user_data = security_get_user_data;
    interface->security_set_user_data = security_set_user_data;
    interface->security_ref = security_ref;
    interface->security_unref = security_unref;

    interface->security_set_protector_mode = security_set_protector_mode;
    interface->security_set_protector_color = security_set_protector_color;
    interface->security_set_protector_image = security_set_protector_image;

    return interface;
}
