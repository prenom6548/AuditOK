// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _PLUGIN_P_H_
#define _PLUGIN_P_H_

#include "plugin.h"

enum plugin_hook_point {
    PLUGIN_HOOK_NEW_VIEW = 0,
    PLUGIN_HOOK_NEW_OUTPUT,
    PLUGIN_HOOK_LAYOUT_DAMAGE,
    PLUGIN_HOOK_LAST,
};

struct plugin_hook {
    struct wl_listener listener;
    enum plugin_hook_point point;
    uint32_t ref_count;
};

struct plugin_manager {
    struct wl_list plugins;

    /* all plugins configurations */
    struct config *config;

    struct server *server;
    struct wl_listener server_destroy;

    struct {
        uint16_t major, minor;
    } abi_version;

    struct plugin_hook hooks[PLUGIN_HOOK_LAST];
};

void plugin_manager_rescan_plugin(void);

/**
 * plugin manager configuration support
 */
bool plugin_manager_config_init(struct plugin_manager *plugin_manager);

bool plugin_need_load(struct plugin *plugin);

void plugin_read_config(struct plugin *plugin);

void plugin_write_config(struct plugin *plugin);

/**
 * load a plugin specified by name.
 */
struct plugin *plugin_manager_get_plugin(const char *name);

struct plugin *plugin_manager_load_plugin(struct plugin *plugin);

void plugin_manager_unload_plugin(struct plugin *plugin);

bool plugin_manager_enable_plugin(struct plugin *plugin, bool enable);

/* interface.c */
void plugin_manager_init_hooks(struct plugin_manager *plugin_manager);

void plugin_manager_finish_hooks(struct plugin_manager *plugin_manager);

struct plugin_interface *plugin_interface_create(struct plugin *plugin);

void plugin_enable_hooks(struct plugin *plugin, bool enabled);

#endif /* _PLUGIN_P_H_ */
