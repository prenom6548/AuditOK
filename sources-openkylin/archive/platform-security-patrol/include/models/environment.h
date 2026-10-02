/**
 * @file environment.h
 * @brief 环境信息数据模型定义
 */

#ifndef MODELS_ENVIRONMENT_H
#define MODELS_ENVIRONMENT_H

#include "plat_sec_patrol.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CPU 信息结构
 */
typedef struct {
    char *vendor;           /* CPU 厂商 */
    char *model;            /* CPU 型号 */
    // char **flags;           /* CPU 特性标志列表（NULL 结尾） */
    // size_t flags_count;     /* 标志数量 */
} cpu_info_t;

/**
 * @brief 固件信息结构
 */
typedef struct {
    char *type;                     /* "UEFI" 或 "BIOS" */
    char *vendor;                   /* 固件厂商 */
    char *version;                  /* 固件版本 */
    // char *secure_firmware_version;  /* 安全固件版本 */
    // char *tpm_firmware_version;     /* TPM 固件版本 */
} firmware_info_t;

/**
 * @brief 操作系统信息结构
 */
typedef struct {
    char *distribution;         /* 发行版名称 */
    char *version;              /* 系统版本 */
    char *kernel;               /* 内核版本 */
    // char **kernel_config;       /* 内核配置项（CONFIG_XXX=y 格式） */
    // size_t config_count;        /* 配置项数量 */
    // char **modules;             /* 已加载模块列表 */
    // size_t modules_count;       /* 模块数量 */
} os_info_t;

/**
 * @brief 整体环境信息结构
 */
typedef struct {
    cpu_info_t cpu;
    firmware_info_t firmware;
    os_info_t os;
    void *raw;                  /* 预留，可存储原始数据（JSON 等） */
} environment_t;



/**
 * @brief 创建环境对象
 * @return 成功返回 environment_t 指针，失败返回 NULL
 */
environment_t* environment_create(void);

/**
 * @brief 释放环境对象
 * @param env 要释放的环境对象
 */
void environment_free(environment_t *env);

/**
 * @brief 设置 CPU 厂商信息
 * @param env 环境对象
 * @param vendor 厂商标识字符串
 */
void environment_set_cpu_vendor(environment_t *env, const char *vendor);

/**
 * @brief 设置 CPU 型号
 * @param env 环境对象
 * @param model 型号字符串
 */
void environment_set_cpu_model(environment_t *env, const char *model);

/**
 * @brief 添加 CPU 特性标志
 * @param env 环境对象
 * @param flag 特性标志字符串
 * @return 成功返回 ERR_OK，失败返回错误码
 */
// error_t environment_add_cpu_flag(environment_t *env, const char *flag);

/**
 * @brief 设置固件类型
 * @param env 环境对象
 * @param type 固件类型（"UEFI" 或 "BIOS"）
 */
void environment_set_firmware_type(environment_t *env, const char *type);

/**
 * @brief 设置固件厂商
 * @param env 环境对象
 * @param vendor 厂商标识字符串
 */
void environment_set_firmware_vendor(environment_t *env, const char *vendor);

/**
 * @brief 设置固件版本
 * @param env 环境对象
 * @param version 版本字符串
 */
void environment_set_firmware_version(environment_t *env, const char *version);

/**
 * @brief 设置安全固件版本
 * @param env 环境对象
 * @param version 版本字符串
 */
// void environment_set_secure_firmware_version(environment_t *env, const char *version);

/**
 * @brief 设置 TPM 固件版本
 * @param env 环境对象
 * @param version 版本字符串
 */
// void environment_set_tpm_firmware_version(environment_t *env, const char *version);

/**
 * @brief 设置操作系统发行版
 * @param env 环境对象
 * @param distro 发行版名称
 */
void environment_set_os_distribution(environment_t *env, const char *distro);

/**
 * @brief 设置操作系统版本
 * @param env 环境对象
 * @param version 版本字符串
 */
void environment_set_os_version(environment_t *env, const char *version);

/**
 * @brief 设置内核版本
 * @param env 环境对象
 * @param kernel 内核版本字符串
 */
void environment_set_os_kernel(environment_t *env, const char *kernel);

/**
 * @brief 添加内核配置项
 * @param env 环境对象
 * @param config 配置项字符串（格式："CONFIG_XXX=y"）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
// error_t environment_add_kernel_config(environment_t *env, const char *config);

/**
 * @brief 添加已加载模块
 * @param env 环境对象
 * @param module 模块名称
 * @return 成功返回 ERR_OK，失败返回错误码
 */
// error_t environment_add_module(environment_t *env, const char *module);

/**
 * @brief 检查 CPU 是否包含特定标志
 * @param env 环境对象
 * @param flag 要检查的标志
 * @return 存在返回 true，否则返回 false
 */
// bool environment_has_cpu_flag(const environment_t *env, const char *flag);

/**
 * @brief 检查内核是否启用了特定配置
 * @param env 环境对象
 * @param config_name 配置项名称（如 "CONFIG_IOMMU"）
 * @return 启用返回 true，否则返回 false
 */
// bool environment_has_kernel_config(const environment_t *env, const char *config_name);

/**
 * @brief 检查是否加载了特定模块
 * @param env 环境对象
 * @param module_name 模块名称
 * @return 已加载返回 true，否则返回 false
 */
// bool environment_has_module(const environment_t *env, const char *module_name);

/* ==================== 插件环境信息函数 ==================== */

// /**
//  * @brief 创建插件环境信息列表
//  * @return 成功返回 plugin_env_list_t 指针，失败返回 NULL
//  */
// plugin_env_list_t* plugin_env_list_create(void);

// /**
//  * @brief 释放插件环境信息列表
//  * @param list 要释放的列表
//  */
// void plugin_env_list_free(plugin_env_list_t *list);

// /**
//  * @brief 向插件环境列表添加数据项
//  * @param list 插件环境列表
//  * @param key 数据键名
//  * @param value 数据值
//  * @param source 数据来源
//  * @return 成功返回 ERR_OK，失败返回错误码
//  */
// error_t plugin_env_list_add(plugin_env_list_t *list, const char *key, 
//                            const char *value, const char *source);

#ifdef __cplusplus
}
#endif

#endif /* MODELS_ENVIRONMENT_H */
