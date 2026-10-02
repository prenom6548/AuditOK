// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _PLUGIN_P_H_
#define _PLUGIN_P_H_

#include "plugin.h"

struct plugin_manager {
    struct wl_list plugins;

    /* all plugins configurations */
    struct config *config;

    struct server *server;
    struct wl_listener server_destroy;

    struct {
        uint16_t major, minor;
    } abi_version;
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

#endif /* _PLUGIN_P_H_ */
