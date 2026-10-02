/**
 * @file collector_plugin.h
 * @brief 环境感知插件接口规范
 *
 * 定义环境感知插件的标准接口，所有环境感知插件必须遵循此规范。
 */

#ifndef PLUGINS_COLLECTOR_PLUGIN_H
#define PLUGINS_COLLECTOR_PLUGIN_H

#include "models/environment.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 环境感知插件接口规范
 *
 * 每个环境感知插件必须导出以下符号：
 * - const char* plugin_type = "collector"
 * - extended_env_list_t* collect(void)
 * - int plugin_init(void) [可选]
 * - void plugin_fini(void) [可选]
 * 
 * @section data_format 数据格式规范
 * 
 * 插件返回的 extended_env_list_t 应遵循以下键名约定：
 * 
 * ## CPU 相关信息 (cpu_collector)
 * - vendor: CPU 厂商标识（如 "HygonGenuine", "GenuineIntel", "AuthenticAMD"）
 * - model: CPU 型号名称
 * - flags: CPU 特性标志列表（逗号分隔）
 * - processor_count: 逻辑处理器数量
 * - security.svm: 是否支持安全虚拟机（"true"/"false"）
 * - security.vtx: 是否支持 Intel VT-x（"true"/"false"）
 * - security.amdv: 是否支持 AMD-V（"true"/"false"）
 * 
 * ## 固件相关信息 (firmware_collector)
 * - type: 固件类型（"UEFI" 或 "BIOS"）
 * - efi_bits: UEFI 位数（"32" 或 "64"）
 * - bios_vendor: 固件厂商（可选）
 * - bios_version: 固件版本（可选）
 * - tpm_acpi_exists: TPM ACPI设备是否存在（"true"/"false"）
 * - tcm_acpi_exists: TCM ACPI设备是否存在（"true"/"false"）
 * - firmware_version: HAG固件版本（可选）
 * - tkm_status: TKM状态（可选）
 * 
 * ## OS 相关信息 (os_collector)
 * - distribution: 发行版名称（如 "OpenKylin"）
 * - version: 系统版本号
 * - id: 发行版 ID（如 "openkylin"）
 * - kernel: 内核版本信息
 * - modules: 已加载模块列表（逗号分隔）
 * - modules_count: 已加载模块数量
 * - CONFIG_AMD_MEM_ENCRYPT: 内核配置项（直接使用CONFIG_前缀）
 * - CONFIG_KVM_AMD_SEV: 内核配置项
 * - tpm_device_exists: TPM设备节点是否存在（"true"/"false"）
 * - tcm_device_exists: TCM设备节点是否存在（"true"/"false"）
 * 
 * @section naming_rules 命名规则
 * 
 * 1. **扁平化命名**：所有字段采用扁平化命名，不使用层级前缀（如firmware.、os.等）
 * 2. **避免冲突**：通过人工确保JSON配置文件中各层之间的字段不重复
 * 3. **小写字母和下划线**：使用小写字母和下划线命名键名
 * 4. **布尔值**：使用 "true"/"false" 字符串
 * 5. **数值**：转换为字符串存储
 * 6. **列表**：使用逗号分隔
 * 7. **source字段**：填写插件名称（不带"_collector"后缀）
 */

/**
 * @brief 插件类型标识（必须由插件定义）
 */
extern const char* plugin_type;

/**
 * @brief 采集函数（必须由插件实现）
 *
 * 采集环境信息并返回插件环境信息列表
 * 
 * @param plugin_name 插件名称（由核心层传入，用于标识数据来源）
 * @return 成功返回 plugin_env_list_t 对象，失败返回 NULL
 *         调用者负责释放返回的列表
 */
typedef plugin_env_list_t* (*collector_collect_fn)(const char *plugin_name);

/**
 * @brief 插件初始化函数（可选）
 *
 * 在插件加载时调用，用于初始化资源
 * @return 成功返回 0，失败返回负错误码
 */
typedef int (*collector_init_fn)(void);

/**
 * @brief 插件清理函数（可选）
 *
 * 在插件卸载时调用，用于清理资源
 */
typedef void (*collector_fini_fn)(void);

#ifdef __cplusplus
}
#endif

#endif /* PLUGINS_COLLECTOR_PLUGIN_H */
