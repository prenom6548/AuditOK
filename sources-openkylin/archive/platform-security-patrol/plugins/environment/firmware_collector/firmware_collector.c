/**
 * @file firmware_collector.c
 * @brief 固件信息采集插件
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "plugin_env_utils.h"
#include "logger.h"

#define MODULE "firmware_collector"

/* DMI sysfs paths */
#ifndef SYS_DMI_BIOS_VENDOR
#define SYS_DMI_BIOS_VENDOR "/sys/class/dmi/id/bios_vendor"
#endif

#ifndef SYS_DMI_BIOS_VERSION
#define SYS_DMI_BIOS_VERSION "/sys/class/dmi/id/bios_version"
#endif

#ifndef SYS_DMI_BIOS_DATE
#define SYS_DMI_BIOS_DATE "/sys/class/dmi/id/bios_date"
#endif

#ifndef SYS_DMI_PRODUCT_VENDOR
#define SYS_DMI_PRODUCT_VENDOR "/sys/class/dmi/id/sys_vendor"
#endif

#ifndef SYS_DMI_PRODUCT_NAME
#define SYS_DMI_PRODUCT_NAME "/sys/class/dmi/id/product_name"
#endif

const char* plugin_type = "collector";

/**
 * @brief 检查命令是否存在
 * 
 * @param command 命令名称
 * @return 存在返回 true，否则返回 false
 */
static bool check_command_exists(const char *command)
{
    if (!command) {
        return false;
    }

    if (strchr(command, '/')) {
        struct stat st;
        return (stat(command, &st) == 0 && S_ISREG(st.st_mode) &&
                (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH));
    }

    const char *path_env = getenv("PATH");
    if (!path_env) {
        return false;
    }

    char *path_copy = strdup(path_env);
    if (!path_copy) {
        return false;
    }

    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    while (dir) {
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);

        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode) &&
            (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)) {
            free(path_copy);
            return true;
        }

        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    return false;
}

/**
 * @brief 执行 shell 命令并获取输出
 * 
 * @param command shell 命令字符串
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 * @return 成功返回 0，失败返回 -1
 */
static int execute_shell_command(const char *command, char *output, size_t output_size)
{
    if (!command || !output || output_size == 0) {
        return -1;
    }
    
    /* 使用 popen 执行命令 */
    FILE *fp = popen(command, "r");
    if (!fp) {
        logger_error(MODULE, "Failed to execute command: %s", command);
        return -1;
    }
    
    /* 读取输出 */
    size_t total_read = 0;
    while (total_read < output_size - 1) {
        size_t bytes_read = fread(output + total_read, 1, output_size - 1 - total_read, fp);
        if (bytes_read == 0) {
            break;
        }
        total_read += bytes_read;
    }
    output[total_read] = '\0';
    
    /* 关闭管道并获取退出码 */
    int status = pclose(fp);
    if (status == -1) {
        logger_error(MODULE, "pclose failed for command: %s", command);
        return -1;
    }
    
    /* 检查命令是否成功执行 */
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        logger_warn(MODULE, "Command exited with status %d: %s", WEXITSTATUS(status), command);
        return -1;
    }
    
    return 0;
}

/**
 * @brief 从 sysfs 读取单个 DMI 属性
 * 
 * @param path DMI 文件路径
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 成功返回 ERR_OK，失败返回错误码
 */
static psp_error_t read_dmi_attribute(const char *path, char *buffer, size_t buffer_size)
{
    if (!path || !buffer || buffer_size == 0) {
        return ERR_INVALID_PARAM;
    }
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        logger_debug(MODULE, "Failed to open %s: %m", path);
        return ERR_IO;
    }
    
    if (fgets(buffer, buffer_size, fp)) {
        /* 去除行尾换行符和回车符 */
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
        
        /* 检查是否为有效数据 */
        if (len > 0 && strcmp(buffer, "Not Specified") != 0) {
            fclose(fp);
            return ERR_OK;
        }
    }
    
    fclose(fp);
    return ERR_IO;
}

/**
 * @brief 检查ACPI设备是否存在
 * 
 * @param device_id ACPI设备ID（如 "HYGT0101"）
 * @return true if exists
 */
static bool check_acpi_device_exists(const char *device_id)
{
    if (!device_id) {
        return false;
    }
    
    /* 构建ACPI设备路径: /sys/bus/acpi/devices/<DEVICE_ID>:00 */
    char device_path[512];
    snprintf(device_path, sizeof(device_path), 
             "/sys/bus/acpi/devices/%s:00", device_id);
    
    struct stat st;
    bool exists = (stat(device_path, &st) == 0 && S_ISDIR(st.st_mode));
    
    if (exists) {
        logger_debug(MODULE, "ACPI device found: %s at %s", device_id, device_path);
    } else {
        logger_debug(MODULE, "ACPI device not found: %s", device_path);
    }
    
    return exists;
}

/**
 * @brief 扫描/sys/bus/acpi/devices目录，列出所有安全相关设备（用于调试）
 */
static void scan_acpi_security_devices(void)
{
    const char *acpi_base_path = "/sys/bus/acpi/devices";
    
    struct stat st;
    if (stat(acpi_base_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        logger_warn(MODULE, "ACPI devices directory not found: %s", acpi_base_path);
        return;
    }
    
    DIR *dir = opendir(acpi_base_path);
    if (!dir) {
        logger_warn(MODULE, "Failed to open ACPI devices directory: %m");
        return;
    }
    
    struct dirent *entry;
    int security_device_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        /* 跳过 . 和 .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* 查找可能的安全设备ID模式 */
        if (strstr(entry->d_name, "HYGT") != NULL ||
            strstr(entry->d_name, "HID") != NULL ||
            strstr(entry->d_name, "TPM") != NULL ||
            strstr(entry->d_name, "TCM") != NULL) {
            
            logger_info(MODULE, "  Found security-related ACPI device: %s", entry->d_name);
            security_device_count++;
        }
    }
    
    closedir(dir);
}

/**
 * @brief 采集并记录TPM ACPI设备存在性
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return true if TPM ACPI device exists
 */
static void collect_tpm_acpi_device(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return;
    }
    
    bool tpm_acpi_exists = check_acpi_device_exists("HYGT0101");
    
    plugin_env_list_add(env_list, "tpm_acpi_exists", 
                       tpm_acpi_exists ? "true" : "false", plugin_name);
    
    return;
}

/**
 * @brief 采集并记录TCM ACPI设备存在性
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return true if TCM ACPI device exists
 */
static void collect_tcm_acpi_device(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return;
    }
    
    /* TCM ACPI设备ID列表 */
    const char *tcm_device_ids[] = {
        "HYGT0201",  /* 海光TCM设备ID */
    };
    
    bool tcm_acpi_exists = false;
    const size_t tcm_count = sizeof(tcm_device_ids) / sizeof(tcm_device_ids[0]);
    
    for (size_t i = 0; i < tcm_count; i++) {
        if (check_acpi_device_exists(tcm_device_ids[i])) {
            tcm_acpi_exists = true;
            logger_debug(MODULE, "TCM ACPI device found: %s", tcm_device_ids[i]);
            break;
        }
    }
    
    if (!tcm_acpi_exists) {
        logger_debug(MODULE, "No TCM ACPI device found (checked %zu device IDs)", tcm_count);
    }
    
    plugin_env_list_add(env_list, "tcm_acpi_exists", 
                       tcm_acpi_exists ? "true" : "false", plugin_name);
    
    return;
}

/**
 * @brief 根据TPM/TCM ACPI设备状态推导TPM支持状态
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @param tpm_exists TPM ACPI设备是否存在
 * @param tcm_exists TCM ACPI设备是否存在
 */
// static void derive_tpm_support_status(plugin_env_list_t *env_list, 
//                                       const char *plugin_name,
//                                       bool tpm_exists,
//                                       bool tcm_exists)
// {
//     if (!env_list || !plugin_name) {
//         return;
//     }
    
//     /* 判定逻辑:
//      * - TPM ACPI存在 -> supported (BIOS支持且启用TPM)
//      * - TPM不存在但TCM存在 -> disabled (BIOS支持但禁用TPM,启用TCM)
//      * - 两者都不存在 -> unsupported (BIOS不支持安全芯片)
//      */
//     const char *tpm_status;
//     if (tpm_exists) {
//         tpm_status = "supported";
//     } else if (tcm_exists) {
//         tpm_status = "disabled";
//     } else {
//         tpm_status = "unsupported";
//     }
    
//     plugin_env_list_add(env_list, "tpm_support_status", tpm_status, plugin_name);
    
//     logger_info(MODULE, "TPM support status: %s (TPM ACPI: %s, TCM ACPI: %s)",
//                tpm_status, 
//                tpm_exists ? "yes" : "no",
//                tcm_exists ? "yes" : "no");
// }

/**
 * @brief 采集TPM/TCM ACPI设备状态并推导支持情况
 */
static psp_error_t collect_acpi_security_devices(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* ========== 调试: 扫描所有ACPI安全设备 ========== */
    scan_acpi_security_devices();
    
    /* ========== 1. 采集TPM ACPI设备 ========== */
    collect_tpm_acpi_device(env_list, plugin_name);
    
    /* ========== 2. 采集TCM ACPI设备 ========== */
    collect_tcm_acpi_device(env_list, plugin_name);
    
    return ERR_OK;
}

/**
 * @brief 从 hag general check 输出中提取指定字段的值
 * 
 * @param output hag general check 命令的完整输出
 * @param field_name 字段名称（如 "TKM:" 或 "firmware version:"）
 * @param value 输出缓冲区
 * @param value_size 缓冲区大小
 * @return 成功返回 true，失败返回 false
 */
static bool extract_hag_field(const char *output, const char *field_name, 
                              char *value, size_t value_size)
{
    if (!output || !field_name || !value || value_size == 0) {
        return false;
    }
    
    /* 查找字段名 */
    const char *field_pos = strstr(output, field_name);
    if (!field_pos) {
        logger_debug(MODULE, "Field '%s' not found in hag output", field_name);
        return false;
    }
    
    /* 跳过字段名和冒号 */
    const char *value_start = field_pos + strlen(field_name);
    
    /* 跳过空格 */
    while (*value_start == ' ' || *value_start == '\t') {
        value_start++;
    }
    
    /* 提取值直到行尾 */
    const char *value_end = value_start;
    while (*value_end && *value_end != '\n' && *value_end != '\r') {
        value_end++;
    }
    
    size_t value_len = value_end - value_start;
    if (value_len >= value_size) {
        value_len = value_size - 1;
    }
    
    /* 只有在有内容时才拷贝，避免 strncpy 拷贝 0 字节的警告 */
    if (value_len > 0) {
        strncpy(value, value_start, value_len);
        value[value_len] = '\0';
    } else {
        value[0] = '\0';
    }
    
    /* 去除尾部空格 */
    while (value_len > 0 && (value[value_len-1] == ' ' || value[value_len-1] == '\t')) {
        value[--value_len] = '\0';
    }
    
    return value_len > 0;
}

/**
 * @brief 通过 hag general check 命令采集 HAG 相关信息
 * 
 * @param env_list 环境变量列表
 * @param plugin_name 插件名称
 * @return ERR_OK on success
 */
static psp_error_t collect_hag_info(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查 hag 命令是否存在 */
    if (!check_command_exists("hag")) {
        logger_warn(MODULE, "hag command not found, skipping HAG info collection");
        return ERR_NOT_FOUND;
    }
    
    /* 执行 hag general check 命令 */
    char output[4096];
    int ret = execute_shell_command("hag general check", output, sizeof(output));
    if (ret != 0) {
        logger_warn(MODULE, "Failed to execute 'hag general check'");
        return ERR_IO;
    }
    
    logger_debug(MODULE, "Execute hag general check: ");
    
#define HAG_FW_VERSION_FIELD "firmware version:"    
#define HAG_TKM_FIELD "TKM:"
#define HAG_TDM_FIELD "TDM:"
    /* 提取 firmware version */
    char firmware_version[64];
    if (extract_hag_field(output, HAG_FW_VERSION_FIELD, firmware_version, sizeof(firmware_version))) {
        plugin_env_list_add(env_list, "firmware_version", firmware_version, plugin_name);
        logger_info(MODULE, "HAG firmware version: %s", firmware_version);
    } else {
        logger_warn(MODULE, "Failed to extract firmware version from hag output");
    }
    
    /* 提取 TKM 状态 */
    char tkm_status[64];
    if (extract_hag_field(output, HAG_TKM_FIELD, tkm_status, sizeof(tkm_status))) {
        plugin_env_list_add(env_list, "tkm_status", tkm_status, plugin_name);
        logger_info(MODULE, "TKM status: %s", tkm_status);
    } else {
        logger_warn(MODULE, "Failed to extract TKM status from hag output");
    }
    
    /* 提取 TDM 状态 */
    char tdm_status[64];
    if (extract_hag_field(output, HAG_TDM_FIELD, tdm_status, sizeof(tdm_status))) {
        plugin_env_list_add(env_list, "tdm_status", tdm_status, plugin_name);
        logger_info(MODULE, "TDM status: %s", tdm_status);
    } else {
        logger_warn(MODULE, "Failed to extract TDM status from hag output");
    }
    
    return ERR_OK;
}

/**
 * @brief 采集固件信息（从 sysfs 读取）
 */
static psp_error_t collect_firmware_info(plugin_env_list_t *env_list, const char *plugin_name)
{
    if (!env_list || !plugin_name) {
        return ERR_INVALID_PARAM;
    }
    
    char buffer[256];
    bool any_success = false;
    
    /* 从 sysfs 读取 BIOS 厂商 */
    if (read_dmi_attribute(SYS_DMI_BIOS_VENDOR, buffer, sizeof(buffer)) == ERR_OK) {
        plugin_env_list_add(env_list, "bios_vendor", buffer, plugin_name);
        any_success = true;
    } else {
        logger_warn(MODULE, "Failed to read BIOS vendor from %s", SYS_DMI_BIOS_VENDOR);
    }
    
    /* 从 sysfs 读取 BIOS 版本 */
    if (read_dmi_attribute(SYS_DMI_BIOS_VERSION, buffer, sizeof(buffer)) == ERR_OK) {
        plugin_env_list_add(env_list, "bios_version", buffer, plugin_name);
        any_success = true;
    } else {
        logger_warn(MODULE, "Failed to read BIOS version from %s", SYS_DMI_BIOS_VERSION);
    }
    
    // /* 从 sysfs 读取 BIOS 日期（可选） */
    // if (read_dmi_attribute(SYS_DMI_BIOS_DATE, buffer, sizeof(buffer)) == ERR_OK) {
    //     plugin_env_list_add(env_list, "firmware.bios_date", buffer, plugin_name);
    //     logger_debug(MODULE, "BIOS date: %s", buffer);
    // }
    
    // /* 从 sysfs 读取系统厂商（可选） */
    // if (read_dmi_attribute(SYS_DMI_PRODUCT_VENDOR, buffer, sizeof(buffer)) == ERR_OK) {
    //     plugin_env_list_add(env_list, "firmware.sys_vendor", buffer, plugin_name);
    //     logger_debug(MODULE, "System vendor: %s", buffer);
    // }
    
    // /* 从 sysfs 读取产品名称（可选） */
    // if (read_dmi_attribute(SYS_DMI_PRODUCT_NAME, buffer, sizeof(buffer)) == ERR_OK) {
    //     plugin_env_list_add(env_list, "firmware.product_name", buffer, plugin_name);
    //     logger_debug(MODULE, "Product name: %s", buffer);
    // }
    
    /* 采集TPM/TCM ACPI设备状态 */
    psp_error_t ret = collect_acpi_security_devices(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect ACPI security devices info");
    }
    
    /* 采集 HAG 相关信息（firmware version 和 TKM） */
    ret = collect_hag_info(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect HAG info (this is optional)");
    }
    
    /* 如果至少读取到一个有效字段，认为成功 */
    if (!any_success) {
        logger_warn(MODULE, "Failed to detect firmware information from sysfs. "
                           "Please ensure running with sufficient privileges.");
        return ERR_IO;
    }
    
    return ERR_OK;
}

int plugin_init(void)
{
    logger_debug(MODULE, "Firmware collector initialized");
    return 0;
}

/**
 * @brief 采集函数（必须实现）
 * 
 * 返回插件环境信息列表，由 core 层统一管理
 */
plugin_env_list_t* collect(const char *plugin_name)
{
    if (!plugin_name) {
        logger_error(MODULE, "Plugin name is NULL");
        return NULL;
    }
    
    logger_info(MODULE, "Collecting firmware information (plugin: %s)", plugin_name);
    
    /* 创建插件环境列表 */
    plugin_env_list_t *env_list = plugin_env_list_create();
    if (!env_list) {
        logger_error(MODULE, "Failed to create environment list");
        return NULL;
    }
    
    /* 采集固件信息 */
    psp_error_t ret = collect_firmware_info(env_list, plugin_name);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect firmware info");
    }
    
    logger_info(MODULE, "Firmware collection completed with %zu items", env_list->count);
    return env_list;
}

void plugin_fini(void)
{
    logger_debug(MODULE, "Firmware collector finalized");
}
