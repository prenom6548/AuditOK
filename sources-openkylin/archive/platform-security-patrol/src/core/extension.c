#include "core/extension.h"
#include "logger.h"
#include "plugin_env_utils.h"
#include "core/capability_parser.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "plugins/collector_plugin.h"
#include "plugins/capability_plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>

#define MODULE "extension"

/**
 * @brief 将字符串转换为小写
 * 
 * @param str 输入字符串（可修改）
 * @return 转换后的字符串指针
 */
// static char* to_lowercase(char *str)
// {
//     if (!str) {
//         return NULL;
//     }
    
//     for (char *p = str; *p; p++) {
//         *p = tolower((unsigned char)*p);
//     }
//     return str;
// }

/**
 * @brief 从插件文件名提取厂商和型号信息
 * 
 * 将插件名拆解为 vendor 和 model 两部分用于匹配
 * 例如："libcpu_hygon_7490.so" → vendor="hygon", model="7490"
 * 
 * @param filename 插件文件名（如 "libcpu_hygon_7490.so"）
 * @param[out] vendor 输出缓冲区（厂商名称）
 * @param vendor_size vendor 缓冲区大小
 * @param[out] model 输出缓冲区（型号数字）
 * @param model_size model 缓冲区大小
 * @return 成功返回 true，失败返回 false
 */
static bool extract_vendor_from_filename(const char *filename, 
                                                  char *vendor, size_t vendor_size)
{
    if (!filename || !vendor || vendor_size == 0) {
        return false;
    }

    /* 查找 libcpu_前缀 */
    const char *prefix = "libcpu_";
    const char *suffix = "_collector.so";
    size_t prefix_len = strlen(prefix);
    size_t suffix_len = strlen(suffix);
    size_t filename_len = strlen(filename);
    
    // 必须以libcpu_开头
    if (strncmp(filename, prefix, prefix_len) != 0) {
        return false;
    }

    // 必须以_collector.so 结尾
    if (filename_len <= prefix_len + suffix_len) {
        return false;
    }
    if (strncmp(filename + filename_len - suffix_len, suffix, suffix_len) != 0) {
        return false;
    }
    
    size_t vendor_len = filename_len - prefix_len - suffix_len;
    if (vendor_len >= vendor_size || vendor_len == 0) {
        return false;
    }

    memcpy(vendor, filename + prefix_len, vendor_len);
    vendor[vendor_len] = '\0';

    return true;
}

/**
 * @brief 在 model name 中模糊匹配厂商和型号信息
 * 
 * @param model_name CPU model name（如 "Hygon C86-4G (OPN:7490)"）
 * @param vendor 厂商名称（如 "hygon"）
 * @param model 型号数字（如 "7490"）
 * @return 匹配成功返回 true，否则返回 false
 */
static bool match_cpu_info(char *env_vendor, char *plugin_vendor)
{
    if (!env_vendor || !plugin_vendor) {
        return false;
    }
        
    /* 匹配厂商名称（不区分大小写）*/
    if (strcasestr(env_vendor, plugin_vendor) != NULL)
        return true;

    return false;
}

/**
 * @brief 检查文件名是否匹配（不区分大小写）
 * 
 * @param filename 实际文件名
 * @param pattern 匹配模式（支持前缀和后缀匹配）
 * @return 匹配返回 true，否则返回 false
 */
// static bool match_filename_case_insensitive(const char *filename, const char *pattern)
// {
//     if (!filename || !pattern) {
//         return false;
//     }
    
//     /* 转换为小写后比较 */
//     char *filename_lower = strdup(filename);
//     char *pattern_lower = strdup(pattern);
    
//     if (!filename_lower || !pattern_lower) {
//         free(filename_lower);
//         free(pattern_lower);
//         return false;
//     }
    
//     to_lowercase(filename_lower);
//     to_lowercase(pattern_lower);
    
//     bool matched = (strstr(filename_lower, pattern_lower) != NULL);
    
//     free(filename_lower);
//     free(pattern_lower);
    
//     return matched;
// }

plugin_manager_t* extension_create_manager(const char *plugin_dir)
{
    if (!plugin_dir) {
        logger_error(MODULE, "Invalid plugin directory");
        return NULL;
    }
    
    plugin_manager_t *manager = (plugin_manager_t*)calloc(1, sizeof(plugin_manager_t));
    if (!manager) {
        logger_error(MODULE, "Failed to allocate memory for plugin manager");
        return NULL;
    }
    
    manager->plugins = NULL;
    manager->count = 0;
    manager->capacity = 0;
    manager->plugin_dir = string_utils_strdup(plugin_dir);
    
    if (!manager->plugin_dir) {
        logger_error(MODULE, "Failed to duplicate plugin directory path");
        free(manager);
        return NULL;
    }
    
    logger_info(MODULE, "Plugin manager created (dir=%s)", plugin_dir);
    return manager;
}

void extension_free_manager(plugin_manager_t *manager)
{
    if (!manager) return;
    
    /* 卸载所有插件 */
    extension_unload_plugins(manager);
    
    free(manager->plugin_dir);
    free(manager);
    
    logger_debug(MODULE, "Plugin manager freed");
}

static bool search_environment_plugin(const char *filename, const environment_t *env)
{ 
    /* CPU 相关插件匹配 */
    if (strstr(filename, "cpu") != NULL) {
        /* 根据 CPU 厂商决定是否加载 */
        if (!env->cpu.vendor) {
            logger_debug(MODULE, "CPU vendor unknown, skip CPU plugin: %s", filename);
            return false;
        }
        
        /* 从插件文件名中提取 vendor 和 model */
        char plugin_vendor[64] = {0};
        
        if (!extract_vendor_from_filename(filename, plugin_vendor, sizeof(plugin_vendor))) {
            logger_debug(MODULE, "Failed to extract vendor from plugin: %s", filename);
            return false;
        }
        
        logger_debug(MODULE, "Plugin info extracted: vendor='%s'", plugin_vendor);
        
        if (match_cpu_info(env->cpu.vendor, plugin_vendor) == true) {
            return true;
        }
        else {
            /* 没有匹配的插件 */
            logger_debug(MODULE, "No matching CPU plugin found for: %s", env->cpu.model ? env->cpu.model : "N/A");
            return false;
        }
    }

    if (strstr(filename, "firmware_collector") != NULL) {
        return true;
    }

    // TODO: 根据操作系统名称和版本来筛选插件
    if (strstr(filename, "os") != NULL) {
        return true;
    }
    return false;
}

/**
 * @brief 检查插件是否应该被加载（基于平台匹配）
 * 
 * @param plugin_path 插件文件路径
 * @param env 环境信息
 * @return 应该加载返回 true，否则返回 false
 */
static bool should_load_plugin(const char *plugin_path, const environment_t *env)
{
    if (!plugin_path || !env) {
        return false;
    }
    
    /* 从文件名提取插件类型和信息 */
    const char *filename = strrchr(plugin_path, '/');
    filename = filename ? filename + 1 : plugin_path;
    
    logger_debug(MODULE, "Checking plugin: %s", filename);
    
    // 搜索匹配的环境感知插件
    if (search_environment_plugin(filename, env) == true) {
        return true;
    }
    
    /* 能力库插件匹配 */
    if (strstr(filename, "capability") != NULL && 
        env->cpu.vendor && strcasestr(filename, env->cpu.vendor) != NULL) {
        return true;
    }
    
    /* 其他插件默认不加载，除非明确匹配 */
    logger_debug(MODULE, "Plugin does not match any platform: %s", filename);
    return false;
}

psp_error_t extension_load_plugins(plugin_manager_t *manager, const environment_t *env)
{
    if (!manager || !manager->plugin_dir) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查目录是否存在 */
    if (!file_utils_is_directory(manager->plugin_dir)) {
        logger_warn(MODULE, "Plugin directory does not exist: %s", manager->plugin_dir);
        return ERR_NOT_FOUND;
    }
    
    /* 扫描所有.so 文件 */
    char **files = NULL;
    size_t file_count = 0;
    psp_error_t ret = file_utils_scan_directory(manager->plugin_dir, ".so", &files, &file_count);
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to scan plugin directory");
        return ret;
    }
    
    logger_info(MODULE, "Found %zu potential plugins in %s", file_count, manager->plugin_dir);
    
    /* 根据平台匹配加载插件 */
    int loaded_count = 0;
    int skipped_count = 0;
    
    for (size_t i = 0; i < file_count; i++) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", manager->plugin_dir, files[i]);
        
        /* 检查插件是否应该被加载 */
        if (!should_load_plugin(full_path, env)) {
            logger_debug(MODULE, "Skipping plugin (platform mismatch): %s", full_path);
            skipped_count++;
            continue;
        }
        
        logger_debug(MODULE, "Loading matched plugin: %s", full_path);
        
        /* 加载插件 */
        plugin_handle_t *plugin = extension_load_plugin(full_path);
        if (!plugin) {
            logger_warn(MODULE, "Failed to load plugin: %s", full_path);
            continue;
        }
        
        /* 添加到管理器 */
        if (manager->count >= manager->capacity) {
            size_t new_capacity = (manager->capacity == 0) ? 16 : manager->capacity * 2;
            plugin_handle_t **new_plugins = (plugin_handle_t**)realloc(manager->plugins,
                                                                       new_capacity * sizeof(plugin_handle_t*));
            if (!new_plugins) {
                logger_error(MODULE, "Failed to reallocate memory for plugins");
                extension_unload_plugin(plugin);
                
                /* 清理已扫描的文件名 */
                string_utils_free_array(files, file_count);
                return ERR_MEMORY;
            }
            
            manager->plugins = new_plugins;
            manager->capacity = new_capacity;
        }
        
        manager->plugins[manager->count++] = plugin;
        loaded_count++;
        logger_info(MODULE, "Loaded plugin: %s (type=%s)", full_path,
                   plugin->type == PLUGIN_TYPE_COLLECTOR ? "collector" : "capability");
    }
    
    /* 清理文件名数组 */
    string_utils_free_array(files, file_count);
    
    logger_info(MODULE, "Plugin loading completed: %d loaded, %d skipped", 
               loaded_count, skipped_count);
    return ERR_OK;
}

psp_error_t extension_unload_plugins(plugin_manager_t *manager)
{
    if (!manager) {
        return ERR_INVALID_PARAM;
    }
    
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->plugins[i]) {
            extension_unload_plugin(manager->plugins[i]);
        }
    }
    
    free(manager->plugins);
    manager->plugins = NULL;
    manager->count = 0;
    manager->capacity = 0;
    
    logger_info(MODULE, "Unloaded all plugins");
    return ERR_OK;
}

psp_error_t extension_invoke_collectors(plugin_manager_t *manager, plugin_env_list_t **env_list_out)
{
    if (!manager || !env_list_out) {
        return ERR_INVALID_PARAM;
    }
    
    /* 创建插件环境信息列表 */
    plugin_env_list_t *result_list = plugin_env_list_create();
    if (!result_list) {
        return ERR_MEMORY;
    }
    
    int collector_count = 0;
    int success_count = 0;
    
    for (size_t i = 0; i < manager->count; i++) {
        plugin_handle_t *plugin = manager->plugins[i];
        
        /* 只处理环境感知插件 */
        if (plugin->type != PLUGIN_TYPE_COLLECTOR) {
            continue;
        }
        
        collector_count++;
        
        /* 获取 collect 函数符号 */
        /* ISO C 禁止直接将对象指针转换为函数指针，使用联合体进行安全转换 */
        union {
            void *obj;
            collector_collect_fn fn;
        } sym;
        
        sym.obj = dlsym(plugin->handle, "collect");
        collector_collect_fn collect_fn = sym.fn;
        
        if (!collect_fn) {
            logger_warn(MODULE, "Plugin %s does not export 'collect' function", plugin->path);
            continue;
        }
        
        /* 调用 collect 函数，传入插件名称 */
        plugin_env_list_t *collected_data = collect_fn(plugin->name);
        if (!collected_data) {
            logger_warn(MODULE, "Collector plugin %s returned NULL", plugin->path);
            continue;
        }

        /* 合并到结果列表（跳过已存在的 key） */
        for (size_t j = 0; j < collected_data->count; j++) {
            plugin_env_item_t *item = collected_data->items[j];
            if (item && item->key) {
                /* 检查 key 是否已存在，避免重复 */
                if (plugin_env_list_has_key(result_list, item->key)) {
                    /* 找到已存在的 source */
                    const char *existing_source = "unknown";
                    for (size_t k = 0; k < result_list->count; k++) {
                        if (result_list->items[k] && 
                            strcmp(result_list->items[k]->key, item->key) == 0) {
                            existing_source = result_list->items[k]->source ? 
                                            result_list->items[k]->source : "unknown";
                            break;
                        }
                    }
                    
                    logger_debug(MODULE, "Key '%s' already exists from source '%s', skipping duplicate from '%s'",
                                item->key, existing_source, item->source ? item->source : "unknown");
                    continue;
                }
                
                psp_error_t ret = plugin_env_list_add(result_list, item->key, 
                                                   item->value, item->source);
                if (ret != ERR_OK) {
                    logger_warn(MODULE, "Failed to add collected data item: %s", item->key);
                }
            }
        }
        
        plugin_env_list_free(collected_data);
        success_count++;
    }
    
    logger_info(MODULE, "Executed %d collectors, %d succeeded", collector_count, success_count);
    
    /* 输出结果 */
    *env_list_out = result_list;
    return (success_count == collector_count) ? ERR_OK : ERR_GENERIC;
}

capability_list_t* extension_load_capabilities(plugin_manager_t *manager, const environment_t *env)
{
    if (!manager) {
        return NULL;
    }
    
    capability_list_t *result_list = capability_list_create();
    if (!result_list) {
        return NULL;
    }
    
    int capability_count = 0;
    
    for (size_t i = 0; i < manager->count; i++) {
        plugin_handle_t *plugin = manager->plugins[i];
        
        /* 只处理能力库插件 */
        if (plugin->type != PLUGIN_TYPE_CAPABILITY) {
            continue;
        }
        
        capability_count++;
        
        /* 获取 load_capabilities_json 函数符号 */
        /* ISO C 禁止直接将对象指针转换为函数指针，使用联合体进行安全转换 */
        union {
            void *obj;
            capability_json_loader_fn fn;
        } sym;
        
        sym.obj = dlsym(plugin->handle, "load_capabilities_json");
        capability_json_loader_fn loader_fn = sym.fn;
        
        if (!loader_fn) {
            logger_warn(MODULE, "Plugin %s does not export 'load_capabilities_json' function", plugin->path);
            continue;
        }
        
        /* 调用加载函数，获取 JSON 字符串 */
        char *json_data_owned = (char *)loader_fn(env);
        if (!json_data_owned) {
            logger_warn(MODULE, "Capability plugin %s returned NULL", plugin->path);
            continue;
        }

        const char *json_data = json_data_owned;
        logger_debug(MODULE, "Parsing capabilities from JSON");

        capability_list_t *cap_list = capability_parser(json_data);
        if (!cap_list) {
            logger_warn(MODULE, "Failed to parse capabilities from plugin %s", plugin->path);
            free(json_data_owned);  /* 释放插件返回的 JSON */
            continue;
        }
        
        /* 合并到结果列表 */
        for (size_t j = 0; j < cap_list->count; j++) {
            capability_t *cap = cap_list->items[j];
            if (cap) {
                /* 从原列表中移除引用（避免重复释放） */
                cap_list->items[j] = NULL;

                psp_error_t ret = capability_list_add(result_list, cap);
                if (ret != ERR_OK) {
                    logger_warn(MODULE, "Failed to add capability to result list");
                    capability_free(cap);
                }
            }
        }

        /* 合并外部工具依赖声明（同样转移所有权，避免重复释放） */
        for (size_t j = 0; j < cap_list->tool_dep_count; j++) {
            external_tool_dep_t *dep = cap_list->tool_deps[j];
            if (dep) {
                cap_list->tool_deps[j] = NULL;

                psp_error_t ret = capability_list_add_tool_dep(result_list, dep);
                if (ret != ERR_OK) {
                    logger_warn(MODULE, "Failed to add tool dependency to result list");
                    external_tool_dep_free(dep);
                }
            }
        }

        capability_list_free(cap_list);
        free(json_data_owned);  /* 释放插件返回的 JSON */
    }
    
    logger_info(MODULE, "Loaded %zu capabilities from %d plugins", result_list->count, capability_count);
    return result_list;
}

plugin_handle_t* extension_load_plugin(const char *path)
{
    if (!path) {
        return NULL;
    }
    
    /* 动态加载库 */
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        logger_error(MODULE, "Failed to load plugin: %s (error: %s)", path, dlerror());
        return NULL;
    }
    
    /* 获取 plugin_type 符号 */
    const char *type_str = NULL;
    /* ISO C 禁止直接将对象指针转换为函数指针，使用联合体进行安全转换 */
    union {
        void *obj;
        const char **str;
    } type_sym;
    
    type_sym.obj = dlsym(handle, "plugin_type");
    type_str = type_sym.str ? *type_sym.str : NULL;
    
    if (!type_str) {
        logger_error(MODULE, "Plugin %s does not export 'plugin_type' symbol", path);
        dlclose(handle);
        return NULL;
    }
    
    /* 创建插件句柄 */
    plugin_handle_t *plugin = (plugin_handle_t*)calloc(1, sizeof(plugin_handle_t));
    if (!plugin) {
        logger_error(MODULE, "Failed to allocate memory for plugin handle");
        dlclose(handle);
        return NULL;
    }
    
    plugin->handle = handle;
    plugin->path = string_utils_strdup(path);
    
    if (!plugin->path) {
        logger_error(MODULE, "Failed to duplicate plugin path");
        dlclose(handle);
        free(plugin);
        return NULL;
    }
    
    /* 提取插件名称（从路径中提取文件名，去除lib前缀和.so后缀） */
    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;
    
    /* 复制文件名 */
    plugin->name = strdup(filename);
    if (!plugin->name) {
        logger_error(MODULE, "Failed to allocate memory for plugin name");
        free(plugin->path);
        dlclose(handle);
        free(plugin);
        return NULL;
    }
    
    /* 移除 .so 后缀 */
    char *so_ext = strstr(plugin->name, ".so");
    if (so_ext) {
        *so_ext = '\0';
    }
    
    /* 移除 lib 前缀 */
    if (strncmp(plugin->name, "lib", 3) == 0) {
        memmove(plugin->name, plugin->name + 3, strlen(plugin->name) - 2);
    }
    
    /* 确定插件类型 */
    if (strcmp(type_str, "collector") == 0) {
        plugin->type = PLUGIN_TYPE_COLLECTOR;
    } else if (strcmp(type_str, "capability") == 0) {
        plugin->type = PLUGIN_TYPE_CAPABILITY;
    } else {
        logger_warn(MODULE, "Unknown plugin type: %s in %s", type_str, path);
        plugin->type = PLUGIN_TYPE_COLLECTOR; /* 默认 */
    }
    
    /* 尝试调用初始化函数（如果存在） */
    collector_init_fn init_fn;
    {
        union {
            void *obj;
            collector_init_fn fn;
        } sym;
        
        sym.obj = dlsym(handle, "plugin_init");
        init_fn = sym.fn;
    }
    
    if (init_fn) {
        int ret = init_fn();
        if (ret != 0) {
            logger_warn(MODULE, "Plugin initialization failed: %s", path);
        }
    }

    return plugin;
}

psp_error_t extension_unload_plugin(plugin_handle_t *plugin)
{
    if (!plugin) {
        return ERR_INVALID_PARAM;
    }
    
    /* 尝试调用清理函数（如果存在） */
    collector_fini_fn fini_fn;
    {
        union {
            void *obj;
            collector_fini_fn fn;
        } sym;
        
        sym.obj = dlsym(plugin->handle, "plugin_fini");
        fini_fn = sym.fn;
    }
    
    if (fini_fn) {
        fini_fn();
    }
    
    /* 关闭动态库 */
    if (plugin->handle) {
        dlclose(plugin->handle);
    }
    
    logger_debug(MODULE, "Unloaded plugin: %s", plugin->path);
    free(plugin->name);
    free(plugin->path);
    free(plugin);
    return ERR_OK;
}

plugin_type_t extension_get_plugin_type(const plugin_handle_t *plugin)
{
    if (!plugin) {
        return PLUGIN_TYPE_COLLECTOR;
    }
    
    return plugin->type;
}

psp_error_t extension_scan_plugins(const char *dir_path, char ***files, size_t *count)
{
    return file_utils_scan_directory(dir_path, ".so", files, count);
}
