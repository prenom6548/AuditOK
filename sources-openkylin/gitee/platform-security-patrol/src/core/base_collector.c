#include "core/base_collector.h"
#include "logger.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/cmd_runner.h"
#include "utils/system_paths.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MODULE "base_collector"

/* 内核配置文件读取缓冲区大小（128KB，足够容纳大型内核配置） */
#define KERNEL_CONFIG_BUFFER_SIZE (128 * 1024)

/* 从/proc/cpuinfo 读取 CPU 信息 */
psp_error_t collect_cpu_info(environment_t *env)
{
    if (!env) {
        return ERR_INVALID_PARAM;
    }
    
    char *content = NULL;
    psp_error_t ret = file_utils_read_file(PROC_CPUINFO, &content, NULL);
    
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to read %s", PROC_CPUINFO);
        return ret;
    }
    
    /* 按行解析 */
    char **lines = NULL;
    size_t line_count = 0;
    ret = string_utils_split(content, "\n", &lines, &line_count);
    free(content);
    
    if (ret != ERR_OK) {
        return ret;
    }
    
    bool vendor_found = false;
    bool model_found = false;
    
    for (size_t i = 0; i < line_count; i++) {
        char *line = lines[i];
        
        /* 查找冒号分隔符 */
        char *colon = strchr(line, ':');
        if (!colon) continue;
        
        *colon = '\0';
        char *key = string_utils_trim(line);
        char *value = string_utils_trim(colon + 1);
        
        /* 提取厂商 ID（第一个 processor 的 vendor_id） */
        if (!vendor_found && strcmp(key, "vendor_id") == 0) {
            environment_set_cpu_vendor(env, value);
            vendor_found = true;
        }
        /* 提取 CPU 型号 */
        else if (!model_found && strcmp(key, "model name") == 0) {
            environment_set_cpu_model(env, value);
            model_found = true;
        }
        /* 提取 CPU 标志 */
        // else if (strcmp(key, "flags") == 0 || strcmp(key, "Features") == 0) {
        //     /* 分割标志 */
        //     char **flags = NULL;
        //     size_t flag_count = 0;
        //     string_utils_split(value, " ", &flags, &flag_count);
            
        //     for (size_t j = 0; j < flag_count; j++) {
        //         if (strlen(flags[j]) > 0) {
        //             environment_add_cpu_flag(env, flags[j]);
        //         }
        //         free(flags[j]);
        //     }
        //     free(flags);
        // }
    }
    
    /* 清理 */
    string_utils_free_array(lines, line_count);
    
    // logger_info(MODULE, "Collected CPU info: vendor=%s, model=%s, flags=%zu",
    //            env->cpu.vendor ? env->cpu.vendor : "unknown",
    //            env->cpu.model ? env->cpu.model : "unknown",
    //            env->cpu.flags_count);
    logger_info(MODULE, "Collected CPU info: vendor=%s, model=%s",
               env->cpu.vendor ? env->cpu.vendor : "unknown",
               env->cpu.model ? env->cpu.model : "unknown");    
    return ERR_OK;
}

/* 读取固件信息 */
psp_error_t collect_firmware_info(environment_t *env)
{
    if (!env) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查是否为 UEFI 固件 */
    if (file_utils_is_directory(SYS_FIRMWARE_EFI)) {
        environment_set_firmware_type(env, "UEFI");
        
        /* 尝试读取固件信息 */
        char *fw_vendor = NULL;
        psp_error_t ret = file_utils_read_file(EFI_FW_PLATFORM_SIZE, &fw_vendor, NULL);
        if (ret == ERR_OK) {
            string_utils_trim(fw_vendor);
            environment_set_firmware_vendor(env, fw_vendor);
            free(fw_vendor);
        }
    } else {
        environment_set_firmware_type(env, "BIOS");
    }
    
    /* 尝试从 DMI 读取更多信息（需要 dmidecode 工具） */
    if (cmd_runner_command_exists("dmidecode")) {
        char output[1024];
        
        /* BIOS 版本 */
        psp_error_t ret = cmd_runner_execute_shell("dmidecode -t bios | grep 'Version:' | head -1 | cut -d':' -f2",
                                              output, sizeof(output));
        if (ret == ERR_OK && strlen(output) > 0) {
            string_utils_trim(output);
            environment_set_firmware_version(env, output);
        }
    }
    
    logger_info(MODULE, "Collected firmware info: type=%s, version=%s",
               env->firmware.type ? env->firmware.type : "unknown",
               env->firmware.version ? env->firmware.version : "unknown");
    
    return ERR_OK;
}

/**
 * @brief 读取操作系统发行版信息
 */
static psp_error_t collect_os_distribution(environment_t *env)
{
    if (!env) {
        return ERR_INVALID_PARAM;
    }
    
    /* 尝试读取 /etc/os-release */
    char *content = NULL;
    psp_error_t ret = file_utils_read_file(ETC_OS_RELEASE, &content, NULL);
    
    if (ret == ERR_OK) {
        char **lines = NULL;
        size_t line_count = 0;
        string_utils_split(content, "\n", &lines, &line_count);
        
        for (size_t i = 0; i < line_count; i++) {
            char *line = lines[i];
            
            if (string_utils_starts_with(line, "PRETTY_NAME=")) {
                char *value = line + strlen("PRETTY_NAME=");
                /* 去除引号 */
                if (*value == '"') value++;
                char *end = strrchr(value, '"');
                if (end) *end = '\0';
                environment_set_os_distribution(env, value);
            } else if (string_utils_starts_with(line, "VERSION_ID=")) {
                char *value = line + strlen("VERSION_ID=");
                if (*value == '"') value++;
                char *end = strrchr(value, '"');
                if (end) *end = '\0';
                environment_set_os_version(env, value);
            }
        }
        
        string_utils_free_array(lines, line_count);
        free(content);
    }
    
    logger_debug(MODULE, "Collected OS distribution: %s %s",
                env->os.distribution ? env->os.distribution : "unknown",
                env->os.version ? env->os.version : "unknown");
    
    return ERR_OK;
}

/**
 * @brief 读取内核版本信息
 */
static psp_error_t collect_kernel_version(environment_t *env)
{
    if (!env) {
        return ERR_INVALID_PARAM;
    }
    
    /* 读取内核版本 */
    char *content = NULL;
    psp_error_t ret = file_utils_read_file(PROC_VERSION, &content, NULL);
    
    if (ret == ERR_OK) {
        /* 提取内核版本号 */
        char *start = strstr(content, "version ");
        if (start) {
            start += strlen("version ");
            char *end = strchr(start, ' ');
            if (end) {
                *end = '\0';
            }
            environment_set_os_kernel(env, start);
        }
        free(content);
    }
    
    logger_debug(MODULE, "Collected kernel version: %s",
                env->os.kernel ? env->os.kernel : "unknown");
    
    return ERR_OK;
}

/**
 * @brief 收集操作系统信息（包括发行版、内核、模块、配置）
 */
psp_error_t collect_os_info(environment_t *env)
{
    psp_error_t ret = ERR_OK;
    if (!env) {
        return ERR_INVALID_PARAM;
    }
    
    logger_info(MODULE, "Collecting OS information");
    
    /* 1. 读取发行版信息 */
    ret = collect_os_distribution(env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect OS distribution");
    }
    
    /* 2. 读取内核版本 */
    ret = collect_kernel_version(env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect kernel version");
    }
    
    // /* 3. 读取已加载模块 */
    // ret = collect_loaded_modules(env);
    // if (ret != ERR_OK) {
    //     logger_warn(MODULE, "Failed to collect loaded modules");
    // }
    
    // /* 4. 读取内核配置 */
    // ret = collect_kernel_config(env);
    // if (ret != ERR_OK) {
    //     logger_debug(MODULE, "Kernel config collection skipped");
    // }
    
    logger_info(MODULE, "Collected OS info: distro=%s, kernel=%s",
               env->os.distribution ? env->os.distribution : "unknown",
               env->os.kernel ? env->os.kernel : "unknown");
    
    return ERR_OK;
}

environment_t* base_collector_collect(void)
{
    logger_info(MODULE, "Starting base collection");
    
    /* 创建环境对象 */
    environment_t *env = environment_create();
    if (!env) {
        logger_error(MODULE, "Failed to create environment object");
        return NULL;
    }
    
    /* 采集 CPU 信息 */
    psp_error_t ret = collect_cpu_info(env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect CPU info, continuing anyway");
    }
    
    /* 采集固件信息 */
    ret = collect_firmware_info(env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect firmware info, continuing anyway");
    }
    
    /* 采集 OS 信息 */
    ret = collect_os_info(env);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to collect OS info, continuing anyway");
    }
    
    logger_info(MODULE, "Base collection completed");
    return env;
}
