/**
 * @file extension.h
 * @brief 扩展接口模块头文件 (插件加载与管理)
 */

#ifndef CORE_EXTENSION_H
#define CORE_EXTENSION_H

#include "plat_sec_patrol.h"
#include "plugin_env_utils.h"
#include "models/environment.h"
#include "models/capability.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 插件类型枚举
 */
//typedef enum {
//    PLUGIN_TYPE_COLLECTOR = 0,       /**< 环境感知插件 */
//    PLUGIN_TYPE_CAPABILITY = 1       /**< 能力库插件 */
//} plugin_type_t;

/**
 * @brief 插件句柄结构
 */
typedef struct {
    void *handle;                 /* dlopen 返回的句柄 */
    char *path;                   /* 插件路径 */
    char *name;                   /* 插件名称（不含lib前缀和.so后缀） */
    plugin_type_t type;           /* 插件类型 */
} plugin_handle_t;

/**
 * @brief 插件管理器结构
 */
typedef struct {
    plugin_handle_t **plugins;    /* 插件数组 */
    size_t count;                 /* 插件数量 */
    size_t capacity;              /* 数组容量 */
    char *plugin_dir;             /* 插件目录 */
} plugin_manager_t;

/**
 * @brief 创建插件管理器
 * 
 * @param plugin_dir 插件目录路径
 * @return 成功返回 plugin_manager_t 对象，失败返回 NULL
 */
plugin_manager_t* extension_create_manager(const char *plugin_dir);

/**
 * @brief 释放插件管理器
 * 
 * @param manager 插件管理器
 */
void extension_free_manager(plugin_manager_t *manager);

/**
 * @brief 加载指定目录下的所有插件（根据平台匹配）
 * 
 * @param manager 插件管理器
 * @param env 环境信息（用于平台匹配）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t extension_load_plugins(plugin_manager_t *manager, const environment_t *env);

/**
 * @brief 卸载所有插件
 * 
 * @param manager 插件管理器
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t extension_unload_plugins(plugin_manager_t *manager);

/**
 * @brief 调用环境感知插件的 collect 函数
 * 
 * @param manager 插件管理器
 * @param env_list_out 输出参数，接收扩展环境信息列表
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t extension_invoke_collectors(plugin_manager_t *manager, plugin_env_list_t **env_list_out);

/**
 * @brief 调用能力库插件的 load_capabilities 函数
 * 
 * @param manager 插件管理器
 * @param env 环境信息（用于筛选合适的能力库）
 * @return 成功返回 capability_list_t 对象，失败返回 NULL
 */
capability_list_t* extension_load_capabilities(plugin_manager_t *manager, const environment_t *env);

/**
 * @brief 从文件加载单个插件
 * 
 * @param path 插件文件路径
 * @return 成功返回 plugin_handle_t 对象，失败返回 NULL
 */
plugin_handle_t* extension_load_plugin(const char *path);

/**
 * @brief 卸载单个插件
 * 
 * @param plugin 插件句柄
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t extension_unload_plugin(plugin_handle_t *plugin);

/**
 * @brief 获取插件类型
 * 
 * @param plugin 插件句柄
 * @return 插件类型
 */
plugin_type_t extension_get_plugin_type(const plugin_handle_t *plugin);

/**
 * @brief 扫描目录中的所有.so 文件
 * 
 * @param dir_path 目录路径
 * @param[out] files 文件路径数组（输出）
 * @param[out] count 文件数量（输出）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t extension_scan_plugins(const char *dir_path, char ***files, size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* CORE_EXTENSION_H */
