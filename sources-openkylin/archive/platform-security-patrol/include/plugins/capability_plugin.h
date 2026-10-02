/**
 * @file capability_plugin.h
 * @brief 安全能力库插件接口定义
 */

#ifndef PLUGINS_CAPABILITY_PLUGIN_H
#define PLUGINS_CAPABILITY_PLUGIN_H

#include "models/environment.h"
#include "models/capability.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 能力库插件接口规范
 * 
 * 每个能力库插件必须导出以下符号：
 * - const char* plugin_type = "capability"
 * - const char* load_capabilities_json(const environment_t *env)
 * - void plugin_fini(void) [可选]
 */

/**
 * @brief 插件类型标识（必须由插件定义）
 * extern const char* plugin_type;
 */
extern const char* plugin_type;

/**
 * @brief 能力加载函数（必须由插件实现）
 * 
 * 返回 JSON 格式的字符串，由 core 层负责解析
 * @param env 环境信息（用于筛选合适的能力库）
 * @return 成功返回 JSON 字符串，失败返回 NULL
 */
typedef const char* (*capability_json_loader_fn)(const environment_t *env);

/**
 * @brief 插件清理函数（可选）
 */
typedef void (*capability_fini_fn)(void);

/* ==================== 标准插件符号名称 ==================== */

#define CAPABILITY_PLUGIN_TYPE_SYMBOL      "plugin_type"
#define CAPABILITY_LOAD_JSON_SYMBOL        "load_capabilities_json"
#define CAPABILITY_FINI_SYMBOL             "plugin_fini"

/* ==================== 辅助宏 ==================== */

/**
 * @brief 声明一个标准的能力库插件
 * 
 * 使用示例:
 * DECLARE_CAPABILITY_PLUGIN(hygon_capability)
 * 
 * 需要实现:
 * static int hygon_capability_init(void) { ... }
 * static const char* hygon_capability_load_json(const environment_t *env) { ... }
 * static void hygon_capability_fini(void) { ... }
 */
#define DECLARE_CAPABILITY_PLUGIN(name) \
    static int name##_init(void); \
    static const char* name##_load_json(const environment_t *env); \
    static void name##_fini(void); \
    \
    const char* plugin_type __attribute__((used)) = "capability"; \
    int plugin_init(void) __attribute__((weak, alias(#name "_init"))); \
    const char* load_capabilities_json(const environment_t *env) __attribute__((alias(#name "_load_json"))); \
    void plugin_fini(void) __attribute__((weak, alias(#name "_fini"))); \
    \
    static int name##_init(void)

#ifdef __cplusplus
}
#endif

#endif /* PLUGINS_CAPABILITY_PLUGIN_H */
