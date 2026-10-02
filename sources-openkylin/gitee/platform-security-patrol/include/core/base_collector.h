/**
 * @file base_collector.h
 * @brief 基础信息采集模块内部头文件
 */

#ifndef CORE_BASE_COLLECTOR_H
#define CORE_BASE_COLLECTOR_H

#include "models/environment.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 采集基础环境信息
 * 
 * 该函数负责采集 CPU、固件、操作系统的基础信息：
 * - CPU: 从/proc/cpuinfo 读取厂商、型号、特性标志
 * - 固件：从/sys/firmware/efi 等位置读取固件信息
 * - OS: 从/etc/os-release、/proc/version 等读取系统信息
 * 
 * @return 成功返回 environment_t 对象，失败返回 NULL
 */
environment_t* base_collector_collect(void);

/**
 * @brief 采集 CPU 信息
 * @param env 环境对象（用于存储采集结果）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t collect_cpu_info(environment_t *env);

/**
 * @brief 采集固件信息
 * @param env 环境对象
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t collect_firmware_info(environment_t *env);

/**
 * @brief 采集操作系统信息
 * @param env 环境对象
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t collect_os_info(environment_t *env);

#ifdef __cplusplus
}
#endif

#endif /* CORE_BASE_COLLECTOR_H */
