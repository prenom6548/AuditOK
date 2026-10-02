// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdlib.h>
#include <strings.h>

#include <kywc/log.h>

#include "plugin_p.h"
#include "server.h"
#include "util/file.h"
#include "util/macros.h"
#include "util/string.h"

static struct plugin_manager *plugin_manager = NULL;

static struct plugin *plugin_create(const char *fullpath, const char *name, bool rescan)
{
    int index = (int)strlen(name) - 3;
    if (index <= 0) {
        return NULL;
    }

    if (strcasecmp(name + index, ".so") != 0) {
        return NULL;
    }

    struct plugin *plugin;
    if (rescan) {
        wl_list_for_each(plugin, &plugin_manager->plugins, link) {
            if (strncmp(plugin->name, name, index) == 0) {
                return plugin;
            }
        }
    }

    /* create a plugin and insert to plugin manager */
    plugin = calloc(1, sizeof(*plugin));
    if (!plugin) {
        return NULL;
    }

    plugin->name = strndup(name, index);
    plugin->path = strdup(fullpath);

    plugin->manager = plugin_manager;
    wl_list_init(&plugin->entries);
    wl_list_insert(&plugin_manager->plugins, &plugin->link);

    return plugin;
}

static bool plugin_filter(const char *fullpath, const char *subpath, const char *name, void *data)
{
    if (name == DIR_NOT_EXIST) {
        return false;
    }

    if (name) {
        plugin_create(fullpath, name, *(bool *)data);
        return false;
    }

    /* only search in one level */
    if (subpath) {
        return true;
    }

    return false;
}

static void scan_all_plugins(bool rescan)
{
    kywc_log(KYWC_DEBUG, "Search all plugins in %s", PLUGIN_DIR);

    if (!file_exists(PLUGIN_DIR)) {
        kywc_log(KYWC_WARN, "Plugin dir is not exists");
        return;
    }

    file_for_each(PLUGIN_DIR, NULL, plugin_filter, &rescan);
}

void plugin_manager_rescan_plugin(void)
{
    scan_all_plugins(true);
}

static struct kywc_plugin_option *find_option_by_token(struct plugin *plugin, int32_t token,
                                                       option_type type)
{
    struct plugin_config_entry *entry;
    wl_list_for_each(entry, &plugin->entries, link) {
        struct kywc_plugin_option *option = &entry->option;
        if (option->token == token && option->type == type) {
            return option;
        }
    }

    return NULL;
}

bool kywc_plugin_get_option_boolean(void *plugin, int32_t token, bool *value)
{
    struct kywc_plugin_option *option = find_option_by_token(plugin, token, option_type_boolean);
    if (!option) {
        return false;
    }

    if (value) {
        *value = option->value.boolean;
    }
    return true;
}

bool kywc_plugin_get_option_double(void *plugin, int32_t token, double *value)
{
    struct kywc_plugin_option *option = find_option_by_token(plugin, token, option_type_double);
    if (!option) {
        return false;
    }

    if (value) {
        *value = option->value.realnum;
    }
    return true;
}

bool kywc_plugin_get_option_int(void *plugin, int32_t token, int *value)
{
    struct kywc_plugin_option *option = find_option_by_token(plugin, token, option_type_int);
    if (!option) {
        return false;
    }

    if (value) {
        *value = option->value.num;
    }
    return true;
}

const char *kywc_plugin_get_option_string(void *plugin, int32_t token)
{
    struct kywc_plugin_option *option = find_option_by_token(plugin, token, option_type_string);
    if (!option) {
        return NULL;
    }

    return option->value.str;
}

static void plugin_destroy(struct plugin *plugin)
{
    wl_list_remove(&plugin->link);
    free(plugin->interface);
    free((void *)plugin->name);
    free((void *)plugin->path);
    free(plugin);
}

static void plugin_destroy_config_entry(struct plugin_config_entry *entry)
{
    wl_list_remove(&entry->link);
    if (entry->option.type == option_type_string) {
        free((void *)entry->option.value.str);
    }
    free(entry);
}

static struct kywc_plugin_option *check_option(struct plugin *plugin,
                                               struct kywc_plugin_option *option)
{
    struct kywc_plugin_option *default_option = plugin->options;
    if (!default_option) {
        return NULL;
    }

    /* search this option in plugin->options */
    while (default_option->name) {
        if (!kywc_plugin_option_match(default_option, option)) {
            default_option++;
            continue;
        }
        return default_option;
    }

    return NULL;
}

static void plugin_merge_options(struct plugin *plugin)
{
    uint64_t mask = 0;

    /* read options from config */
    plugin_read_config(plugin);

    struct kywc_plugin_option *option;
    struct plugin_config_entry *entry, *entry_tmp;
    wl_list_for_each_safe(entry, entry_tmp, &plugin->entries, link) {
        option = check_option(plugin, &entry->option);
        /* remove if not support */
        if (!option) {
            plugin_destroy_config_entry(entry);
            continue;
        }

        entry->option.token = option->token;
        entry->default_value = option->value;
        mask |= 0x1ull << (option - plugin->options);
    }

    /* merge plugin default options */
    option = plugin->options;
    while (option && option->name) {
        /* this option was not configured */
        if (!((mask >> (option - plugin->options)) & 0x1ull)) {
            struct plugin_config_entry *entry = calloc(1, sizeof(*entry));
            if (!entry) {
                continue;
            }

            entry->default_value = option->value;
            entry->option = *option;
            if (option->type == option_type_string) {
                entry->option.value.str = strdup(option->value.str);
            }
            wl_list_insert(&plugin->entries, &entry->link);
        }
        option++;
    }
}

static bool load_plugin(struct plugin *plugin, bool force)
{
    /* plugin was loaded failed already */
    if (plugin->invalid) {
        return false;
    }

    kywc_log(KYWC_INFO, "Loading plugin %s", plugin->path);
    /* use RTLD_NOW to check symbols in plugin */
    void *handle = dlopen(plugin->path, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        kywc_log(KYWC_ERROR, "Failed to load %s: %s", plugin->name, dlerror());
        plugin->invalid = true;
        return false;
    }

    char *symbol = string_create("%s_plugin_data", plugin->name);
    if (!symbol) {
        dlclose(handle);
        return false;
    }

    struct kywc_plugin_data *pdata = dlsym(handle, symbol);
    free(symbol);
    /* check plugin data */
    if (!pdata || !pdata->info || !pdata->setup || !pdata->teardown) {
        kywc_log(KYWC_ERROR, "%s: invalid symbol", plugin->name);
        plugin->invalid = true;
        dlclose(handle);
        return false;
    }

    /* check abi version, major equal and minor not older */
    uint16_t major = GET_ABI_MAJOR(pdata->info->abi_version);
    uint16_t minor = GET_ABI_MINOR(pdata->info->abi_version);
    if (major != plugin_manager->abi_version.major || minor > plugin_manager->abi_version.minor) {
        kywc_log(KYWC_ERROR, "%s: ABI version %d.%d is not match %d.%d", plugin->name, major, minor,
                 plugin_manager->abi_version.major, plugin_manager->abi_version.minor);
        plugin->invalid = true;
        dlclose(handle);
        return false;
    }

    force |= pdata->info->flags & KYWC_PLUGIN_FIXED;
    if (!force && !plugin_need_load(plugin)) {
        kywc_log(KYWC_INFO, "%s: mark disabled in config", plugin->name);
        dlclose(handle);
        return false;
    }

    plugin->info = pdata->info;
    plugin->options = pdata->options;
    plugin->setup = pdata->setup;
    plugin->option = pdata->option;
    plugin->teardown = pdata->teardown;

    plugin->handle = handle;
    plugin->loaded = true;

    /* merge plugin options with config and default */
    plugin_merge_options(plugin);

    kywc_log(KYWC_INFO, "Add plugin %s: %d.%d.%d (ABI %d.%d) by %s", plugin->info->name,
             GET_PLUGIN_MAJOR_VERSION(plugin->info->version),
             GET_PLUGIN_MINOR_VERSION(plugin->info->version),
             GET_PLUGIN_MICRO_VERSION(plugin->info->version), major, minor, plugin->info->vendor);

    return true;
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&plugin_manager->server_destroy.link);

    struct plugin *plugin, *plugin_tmp;
    wl_list_for_each_safe(plugin, plugin_tmp, &plugin_manager->plugins, link) {
        plugin_manager_unload_plugin(plugin);
        plugin_destroy(plugin);
    }

    plugin_manager_finish_hooks(plugin_manager);
    free(plugin_manager);
    plugin_manager = NULL;
}

struct plugin_manager *plugin_manager_create(struct server *server)
{
    plugin_manager = calloc(1, sizeof(*plugin_manager));
    if (!plugin_manager) {
        return NULL;
    }

    wl_list_init(&plugin_manager->plugins);
    plugin_manager->server = server;
    plugin_manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &plugin_manager->server_destroy);

    plugin_manager->abi_version.major = 1;
    plugin_manager->abi_version.minor = 0;
    plugin_manager_config_init(plugin_manager);
    plugin_manager_init_hooks(plugin_manager);
    kywc_log(KYWC_INFO, "Plugin manager is created with ABI verison %d.%d",
             plugin_manager->abi_version.major, plugin_manager->abi_version.minor);

    /* scan all plugins in plugin dir */
    scan_all_plugins(false);

    struct plugin *plugin, *plugin_tmp;
    wl_list_for_each_safe(plugin, plugin_tmp, &plugin_manager->plugins, link) {
        /* load plugin, remove if invalid */
        if (!load_plugin(plugin, false) || !plugin_manager_enable_plugin(plugin, true)) {
            plugin_manager_unload_plugin(plugin);
        }
    }

    return plugin_manager;
}

struct plugin *plugin_manager_get_plugin(const char *name)
{
    if (STRING_INVALID(name)) {
        return NULL;
    }

    struct plugin *plugin;
    wl_list_for_each(plugin, &plugin_manager->plugins, link) {
        if (!strcmp(name, plugin->name)) {
            return plugin;
        }
    }

    return NULL;
}

struct plugin *plugin_manager_load_plugin(struct plugin *plugin)
{
    if (!plugin->loaded && !load_plugin(plugin, true)) {
        return NULL;
    }

    return plugin;
}

bool plugin_manager_enable_plugin(struct plugin *plugin, bool enable)
{
    if (enable == plugin->enabled) {
        return false;
    }
    if (!plugin->loaded) {
        kywc_log(KYWC_WARN, "Plugin %s is not loaded when enable", plugin->name);
        return false;
    }

    if (enable) {
        if (!plugin->setup(plugin, &plugin->teardown_data)) {
            kywc_log(KYWC_ERROR, "Plugin %s setup failed", plugin->name);
            return false;
        }
    } else {
        plugin_enable_hooks(plugin, false);
        plugin->teardown(plugin->teardown_data);
        plugin->hooks = NULL;
    }

    plugin->enabled = enable;
    return true;
}

void plugin_manager_unload_plugin(struct plugin *plugin)
{
    if (plugin->loaded) {
        plugin_manager_enable_plugin(plugin, false);
        dlclose(plugin->handle);
    }

    /* free all config entries */
    struct plugin_config_entry *entry, *tmp_entry;
    wl_list_for_each_safe(entry, tmp_entry, &plugin->entries, link) {
        plugin_destroy_config_entry(entry);
    }

    plugin->loaded = false;
}
