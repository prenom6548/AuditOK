// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <wayland-server-core.h>

#include <kywc/plugin.h>

struct server;

struct plugin_config_entry {
    struct wl_list link;
    struct kywc_plugin_option option;
    option_value default_value;
};

struct plugin {
    struct wl_list link;
    struct plugin_manager *manager;
    /* plugin name, maybe different from info->name */
    const char *name;
    /* full path of the plugin file */
    const char *path;

    const struct kywc_plugin_info *info;
    struct kywc_plugin_option *options;
    plugin_setup_func setup;
    plugin_option_func option;
    plugin_teardown_func teardown;

    void *handle;
    void *teardown_data; /* return by setup_func */

    struct kywc_plugin_interface *interface;
    const struct kywc_plugin_hooks *hooks;

    /* config entries */
    struct wl_list entries;
    bool loaded, enabled;
    bool invalid;
};

/**
 * load all plugins specified in config file.
 */
struct plugin_manager *plugin_manager_create(struct server *server);

#endif /* _PLUGIN_H_ */
