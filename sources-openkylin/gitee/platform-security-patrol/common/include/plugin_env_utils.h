/**
 * @file plugin_env_utils.h
 * @brief 插件环境信息工具函数（公共库）
 */

#ifndef PLUGIN_ENV_UTILS_H
#define PLUGIN_ENV_UTILS_H

#include "plat_sec_patrol.h"
#include "plugin_env_utils.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 插件采集的环境信息项
 */
typedef struct {
    char *key;                  /* 数据键名 */
    char *value;                /* 数据值 */
    char *source;               /* 数据来源（插件名称） */
} plugin_env_item_t;

/**
 * @brief 插件采集的环境信息列表
 */
typedef struct {
    plugin_env_item_t **items; /* 数据项数组 */
    size_t count;                /* 数据项数量 */
    size_t capacity;             /* 数组容量 */
} plugin_env_list_t;

/**
 * @brief 创建插件环境列表
 * 
 * @return 成功返回列表指针，失败返回 NULL
 */
plugin_env_list_t* plugin_env_list_create(void);

/**
 * @brief 释放插件环境列表
 * 
 * @param list 要释放的列表
 */
void plugin_env_list_free(plugin_env_list_t *list);

/**
 * @brief 添加插件环境项
 * 
 * @param list 列表
 * @param key 键
 * @param value 值
 * @param source 来源（插件名称）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t plugin_env_list_add(plugin_env_list_t *list, const char *key, 
                           const char *value, const char *source);

/**
 * @brief 检查键是否存在
 * 
 * @param list 列表
 * @param key 键
 * @return 存在返回 true，否则返回 false
 */
bool plugin_env_list_has_key(plugin_env_list_t *list, const char *key);

/**
 * @brief 获取指定键的值
 * 
 * @param list 列表
 * @param key 键
 * @return 存在返回值，否则返回 NULL
 */
const char* plugin_env_list_get(plugin_env_list_t *list, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_ENV_UTILS_H */
