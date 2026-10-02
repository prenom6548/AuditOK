#include "core/executor.h"
#include "core/extension.h"
#include "logger.h"
#include "core/judge.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#include "plat_sec_patrol.h"

#define MODULE "executor"

/**
 * @brief 简单的版本号比较
 */
static int compare_versions_simple(const char *v1, const char *v2)
{
    if (!v1 || !v2) return 0;
    if (!*v1 && !*v2) return 0;
    if (!*v1) return -1;
    if (!*v2) return 1;

    while (*v1 || *v2) {
        /* 提取一个段（到 '.' 或 '-' 或 '\0' 为止） */
        const char *seg1 = v1, *seg2 = v2;
        while (*v1 && *v1 != '.' && *v1 != '-') v1++;
        while (*v2 && *v2 != '.' && *v2 != '-') v2++;

        size_t len1 = v1 - seg1, len2 = v2 - seg2;

        /* 一方已耗尽而另一方还有段 → 有内容的一方更大 */
        if (len1 == 0 && len2 > 0) return -1;
        if (len1 > 0 && len2 == 0) return 1;
        if (len1 == 0 && len2 == 0) return 0;

        /* 判断段是否纯数字 */
        int is_num1 = 1, is_num2 = 1;
        for (size_t i = 0; i < len1; i++)
            if (seg1[i] < '0' || seg1[i] > '9') { is_num1 = 0; break; }
        for (size_t i = 0; i < len2; i++)
            if (seg2[i] < '0' || seg2[i] > '9') { is_num2 = 0; break; }

        int cmp;
        if (is_num1 && is_num2) {
            /* 两段都是纯数字：按数值比较 */
            long n1 = 0, n2 = 0;
            for (size_t i = 0; i < len1; i++) n1 = n1 * 10 + (seg1[i] - '0');
            for (size_t i = 0; i < len2; i++) n2 = n2 * 10 + (seg2[i] - '0');
            cmp = (n1 > n2) - (n1 < n2);
        } else {
            /* 含字母：按字典序比较 */
            size_t min_len = len1 < len2 ? len1 : len2;
            cmp = strncmp(seg1, seg2, min_len);
            if (cmp == 0)
                cmp = (len1 > len2) - (len1 < len2);
        }

        if (cmp != 0) return cmp;

        /* 跳过分隔符 */
        if (*v1 == '.' || *v1 == '-') v1++;
        if (*v2 == '.' || *v2 == '-') v2++;
    }

    return 0;
}

/**
 * @brief 从环境列表中获取指定字段的值
 */
static const char* get_env_value(const plugin_env_list_t *env_list, const char *field_name)
{
    if (!env_list || !field_name) {
        return NULL;
    }

    for (size_t i = 0; i < env_list->count; i++) {
        if (env_list->items[i] && strcmp(env_list->items[i]->key, field_name) == 0) {
            return env_list->items[i]->value;
        }
    }
    return NULL;
}

/**
 * @brief 评估单个条件是否满足
 */
static bool is_condition_met(const plugin_env_list_t *env_list, const condition_t *cond)
{
    if (!env_list || !cond) {
        return false;
    }
    
    const char *actual_value = get_env_value(env_list, cond->field_name);
    if (!actual_value) {
        return false;
    }
    
    /* 根据操作符进行比较 */
    if (strcmp(cond->operator, "==") == 0 || strcmp(cond->operator, "in") == 0) {
        if (cond->expected_values && cond->expected_values_count > 0) {
            for (size_t i = 0; i < cond->expected_values_count; i++) {
                if (strcmp(actual_value, cond->expected_values[i]) == 0) {
                    return true;
                }
            }
        }
        return false;
    } else if (strcmp(cond->operator, "!=") == 0) {
        if (cond->expected_values && cond->expected_values_count > 0) {
            for (size_t i = 0; i < cond->expected_values_count; i++) {
                if (strcmp(actual_value, cond->expected_values[i]) == 0) {
                    return false;
                }
            }
        }
        return true;
    } else if (strcmp(cond->operator, ">=") == 0 || strcmp(cond->operator, "<=") == 0 ||
               strcmp(cond->operator, ">") == 0 || strcmp(cond->operator, "<") == 0) {
        if (cond->expected_values && cond->expected_values_count > 0) {
            const char *expected_val = cond->expected_values[0];
            int cmp = compare_versions_simple(actual_value, expected_val);
            
            if (strcmp(cond->operator, ">=") == 0) return cmp >= 0;
            if (strcmp(cond->operator, "<=") == 0) return cmp <= 0;
            if (strcmp(cond->operator, ">") == 0) return cmp > 0;
            if (strcmp(cond->operator, "<") == 0) return cmp < 0;
        }
        return false;
    }
    
    return false;
}

/**
 * @brief 从能力列表中查找并判定指定能力的状态
 * 
 * @param cap_id 能力ID（支持后缀匹配）
 * @param caps 已加载的能力列表
 * @param env_data 已采集的环境数据
 * @param[out] out_cap 输出的目标能力指针
 * @param[out] out_judgement 输出的判定结果指针
 * @return error_t 错误码
 */
static psp_error_t judge_specific_capability(const char *cap_id, 
                                         capability_list_t *caps,
                                         plugin_env_list_t *env_data,
                                         capability_t **out_cap,
                                         judgement_result_t **out_judgement)
{
    if (!cap_id || !caps || !env_data || !out_cap || !out_judgement) {
        return ERR_INVALID_PARAM;
    }
    
    *out_cap = NULL;
    *out_judgement = NULL;
    
    /* 查找指定能力（使用精确匹配） */
    capability_t *target_cap = capability_list_find_by_id(caps, cap_id);
    if (!target_cap) {
        logger_error(MODULE, "Capability '%s' not found", cap_id);
        return ERR_NOT_FOUND;
    }
    
    /* 创建判定结果 */
    judgement_result_t *judgement = judgement_result_create();
    if (!judgement) {
        logger_error(MODULE, "Failed to create judgement result");
        return ERR_MEMORY;
    }
    
    judgement->capability_id = strdup(target_cap->id);
    if (target_cap->name) {
        judgement->capability_name = strdup(target_cap->name);
    }
    
    /* 判定各层状态 */
    judge_cpu_layer(env_data, target_cap, judgement);
    judge_firmware_layer(env_data, target_cap, judgement);
    judge_os_layer(env_data, target_cap, judgement);
    
    /* 输出结果 */
    *out_cap = target_cap;
    *out_judgement = judgement;
    
    return ERR_OK;
}

/**
 * @brief 添加建议消息到数组
 */
static inline void add_suggestion(char ***suggestions, int *count, int max_count, const char *fmt, ...)
{
    if (*count >= max_count) {
        return;
    }
    
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    
    (*suggestions)[(*count)++] = strdup(msg);
}

/**
 * @brief 生成友好的失败提示信息
 */
static inline void generate_friendly_message(const char *layer_name, 
                                            const condition_t *cond,
                                            const char *actual_value,
                                            char ***suggestions,
                                            int *suggestion_count,
                                            int max_suggestions)
{
    /* 字段名已经是扁平化的，直接使用 */
    const char *field = cond->field_name;
    const char *expected = cond->expected_values_count > 0 ? cond->expected_values[0] : "N/A";
    condition_type_t cond_type = cond->type;
       
    /* Helper macro to add suggestion */
    #define ADD_SUGGESTION(fmt, ...) \
        do { \
            if (*suggestion_count < max_suggestions) { \
                char msg[512]; \
                snprintf(msg, sizeof(msg), fmt, ##__VA_ARGS__); \
                (*suggestions)[(*suggestion_count)++] = strdup(msg); \
            } \
        } while(0)
    
    /* 根据类型生成提示，使用固定宽度标签实现对齐 */
    switch (cond_type) {
        case COND_TYPE_KERNEL_CONFIG:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: %s is not set, but this feature requires %s=y. Please enable it.",
                              layer_name, field, field);
            } else if (strcmp(cond->operator, "==") == 0 && (strcmp(expected, "y") == 0 || strcmp(expected, "m") == 0)) {
                ADD_SUGGESTION("%-10s: %s=%s, but this feature requires %s=%s. Please enable it.",
                              layer_name, field, actual_value, field, expected);
            }
            break;
        
        case COND_TYPE_FIRMWARE:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: Firmware information (%s) is not available.",
                              layer_name, field);
            } else if (strcmp(actual_value, "disabled") == 0) {
                ADD_SUGGESTION("%-10s: %s is disabled in BIOS/UEFI firmware. Please enter BIOS/UEFI setup to enable this feature.",
                              layer_name, field);
            } else if (strcmp(actual_value, "unsupported") == 0) {
                ADD_SUGGESTION("%-10s: Firmware version does not support this feature. Please contact your hardware vendor for support.",
                              layer_name);
            } else {
                ADD_SUGGESTION("%-10s: Firmware %s is %s, but requires %s. Please enter BIOS/UEFI setup to enable this feature.",
                              layer_name, field, actual_value, expected);
            }
            break;
        
        case COND_TYPE_SOFTWARE:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: software %s%s%s is not installed. Please install it using your package manager.",
                              layer_name, field, cond->operator, expected);
            } else {
                ADD_SUGGESTION("%-10s: Software %s version %s does not meet the requirement (%s %s). Please install or upgrade accordingly.",
                              layer_name, field, actual_value, cond->operator, expected);
            }
            break;
        
        case COND_TYPE_CPU_MODEL:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: CPU model information is not available.", layer_name);
            } else {
                ADD_SUGGESTION("%-10s: Current CPU model is %s, but this capability requires %s %s.",
                              layer_name, actual_value, cond->operator, expected);
            }
            break;
        
        case COND_TYPE_DRIVER:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: Required driver %s is not loaded.", layer_name, field);
            } else {
                ADD_SUGGESTION("%-10s: Driver %s is loaded, but this capability requires %s %s.",
                              layer_name, actual_value, cond->operator, expected);
            }
            break;

        case COND_TYPE_GENERIC:
        default:
            if (cond->description) {
                ADD_SUGGESTION("%-10s: %s", layer_name, cond->description);
            } else if (!actual_value) {
                ADD_SUGGESTION("%-10s: Required field %s is missing.", layer_name, field);
            } else {
                ADD_SUGGESTION("%-10s: %s is %s, but expected %s%s.", 
                    layer_name, field, actual_value, cond->operator, expected);
            }
            break;
    }
    
    #undef ADD_SUGGESTION
}

/**
 * @brief 检查条件组并报告失败信息
 */
static inline void check_and_report_failures(const char *layer_name,
                                            condition_group_t **groups,
                                            size_t group_count,
                                            const plugin_env_list_t *env_list,
                                            char ***suggestions,
                                            int *suggestion_count,
                                            int max_suggestions)
{
    for (size_t g = 0; g < group_count; g++) {
        condition_group_t *group = groups[g];
        bool group_satisfied = false;
        
        /* 对于OR组，只要有一个条件满足就算通过；对于AND组，所有条件都要满足 */
        if (group->is_and_group) {
            /* AND组：检查所有条件 */
            bool all_met = true;
            for (size_t c = 0; c < group->count; c++) {
                condition_t *cond = group->conditions[c];
                if (!is_condition_met(env_list, cond)) {
                    all_met = false;
                    const char *actual_value = get_env_value(env_list, cond->field_name);
                    generate_friendly_message(layer_name, cond, actual_value,
                                            suggestions, suggestion_count, max_suggestions);
                }
            }
            group_satisfied = all_met;
        } else {
            /* OR组：至少一个条件满足即可 */
            for (size_t c = 0; c < group->count; c++) {
                condition_t *cond = group->conditions[c];
                if (is_condition_met(env_list, cond)) {
                    group_satisfied = true;
                    break;
                }
            }
            /* 如果OR组不满足，报告所有条件的失败 */
            if (!group_satisfied) {
                for (size_t c = 0; c < group->count; c++) {
                    condition_t *cond = group->conditions[c];
                    const char *actual_value = get_env_value(env_list, cond->field_name);
                    generate_friendly_message(layer_name, cond, actual_value,
                                            suggestions, suggestion_count, max_suggestions);
                }
            }
        }
    }
}

/**
 * @brief 生成使能建议信息
 */
static char* generate_enable_guidance(const capability_t *cap, 
                                      const judgement_result_t *result,
                                      const plugin_env_list_t *env_list)
{
    if (!cap || !result) {
        return strdup("Unable to generate guidance");
    }
    
    /* 如果能力已可用，无需操作 */
    if (result->overall_status == CAP_STATUS_AVAILABLE) {
        return strdup("Capability is already available, no action needed");
    }
    
    /* 动态收集所有失败建议 */
    char **suggestions = NULL;
    int suggestion_count = 0;
    int max_suggestions = 20;
    
    suggestions = (char**)malloc(sizeof(char*) * max_suggestions);
    if (!suggestions) {
        return strdup("Memory allocation failed");
    }
    
    /* Check CPU Layer - report failures for unsupported or disabled */
    if (result->cpu_status == LAYER_STATUS_UNSUPPORTED || 
        result->cpu_status == LAYER_STATUS_DISABLED) {
        check_and_report_failures("CPU", cap->cpu_rule.or_groups, cap->cpu_rule.or_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
        check_and_report_failures("CPU", cap->cpu_rule.and_groups, cap->cpu_rule.and_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
    }
    
    /* Check Firmware Layer - report failures for unsupported or disabled */
    if (result->firmware_status == LAYER_STATUS_UNSUPPORTED || 
        result->firmware_status == LAYER_STATUS_DISABLED) {
        check_and_report_failures("Firmware", cap->firmware_rule.or_groups, cap->firmware_rule.or_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
        check_and_report_failures("Firmware", cap->firmware_rule.and_groups, cap->firmware_rule.and_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
    }
    
    /* Check OS Layer - report failures for unsupported or disabled */
    if (result->os_status == LAYER_STATUS_UNSUPPORTED || 
        result->os_status == LAYER_STATUS_DISABLED) {
        check_and_report_failures("OS", cap->os_rule.or_groups, cap->os_rule.or_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
        check_and_report_failures("OS", cap->os_rule.and_groups, cap->os_rule.and_group_count, 
                                 env_list, &suggestions, &suggestion_count, max_suggestions);
    }
    
    /* 如果没有建议，返回通用提示 */
    if (suggestion_count == 0) {
        free(suggestions);
        return strdup("Capability requirements not met. Please check system compatibility.");
    }
    
    /* 合并所有建议消息 */
    size_t total_len = 0;
    for (int i = 0; i < suggestion_count; i++) {
        int prefix_len = snprintf(NULL, 0, "%d. ", i + 1);
        total_len += prefix_len + strlen(suggestions[i]);
        if (i > 0) {
            total_len += 1; /* newline separator */
        }
    }
    
    char *result_str = (char*)malloc(total_len + 1);
    if (!result_str) {
        for (int i = 0; i < suggestion_count; i++) {
            free(suggestions[i]);
        }
        free(suggestions);
        return strdup("Memory allocation failed");
    }
    
    result_str[0] = '\0';
    for (int i = 0; i < suggestion_count; i++) {
        if (i > 0) {
            strcat(result_str, "\n");
        }
        // Add serial number before each suggestion
        char numbered_suggestion[1024];
        snprintf(numbered_suggestion, sizeof(numbered_suggestion), "%d. %s", i + 1, suggestions[i]);
        strcat(result_str, numbered_suggestion);
        free(suggestions[i]);
    }
    
    free(suggestions);
    return result_str;
}

psp_error_t executor_backup_config(const char *config_path, char **backup_path)
{
    if (!config_path || !backup_path) {
        return ERR_INVALID_PARAM;
    }
    
    /* 生成备份文件名 */
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", localtime(&now));
    
    *backup_path = (char*)malloc(strlen(config_path) + strlen(timestamp) + 8);
    if (!*backup_path) {
        logger_error(MODULE, "Failed to allocate memory for backup path");
        return ERR_MEMORY;
    }
    
    sprintf(*backup_path, "%s.backup.%s", config_path, timestamp);
    
    /* 复制文件 */
    char *content = NULL;
    psp_error_t ret = file_utils_read_file(config_path, &content, NULL);
    if (ret != ERR_OK) {
        logger_warn(MODULE, "Failed to read config file for backup: %s", config_path);
        free(*backup_path);
        *backup_path = NULL;
        return ret;
    }
    
    ret = file_utils_write_file(*backup_path, content, 0);
    free(content);
    
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to write backup file: %s", *backup_path);
        free(*backup_path);
        *backup_path = NULL;
        return ret;
    }
    
    logger_info(MODULE, "Created backup: %s", *backup_path);
    return ERR_OK;
}

psp_error_t executor_restore_config(const char *backup_path, const char *config_path)
{
    if (!backup_path || !config_path) {
        return ERR_INVALID_PARAM;
    }
    
    /* 读取备份文件 */
    char *content = NULL;
    psp_error_t ret = file_utils_read_file(backup_path, &content, NULL);
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to read backup file: %s", backup_path);
        return ret;
    }
    
    /* 写回原文件 */
    ret = file_utils_write_file(config_path, content, 0);
    free(content);
    
    if (ret != ERR_OK) {
        logger_error(MODULE, "Failed to restore config file: %s", config_path);
        return ret;
    }
    
    logger_info(MODULE, "Restored config from backup: %s", backup_path);
    return ERR_OK;
}

bool executor_validate(const char *cap_id)
{
    /* TODO: 实现验证逻辑 */
    (void)cap_id;
    
    logger_debug(MODULE, "Validation not implemented for capability: %s", cap_id);
    return true;
}

void print_disable_info(capability_list_t *caps, char *cap_id)
{
    /* 查找指定的能力（直接使用ID精确匹配） */
    capability_t *cap = capability_list_find_by_id(caps, cap_id);
    if (!cap) {
        logger_error(MODULE, "Capability '%s' not found", cap_id);
        printf("Error: Capability '%s' not found.\n\n", cap_id);
        printf("Use 'plat-sec-patrol list' to see available capabilities.\n");
        
        //capability_list_free(caps);
        return;
    }
    
    /* 输出禁用信息 */
    printf("\n=== Disable Information for '%s' ===\n\n", cap->name ? cap->name : cap->id);
    
    if (cap->description) {
        printf("Description: %s\n", cap->description);
    }
    
    printf("\nDisable Supported: %s\n", cap->disable_supported ? "Yes" : "No");
    
    if (!cap->disable_supported) {
        printf("\nThis capability cannot be actively disabled by software.\n");
        printf("It is a hardware-level security feature that can only be:\n");
        printf("  - Detected and reported by this tool\n");
        printf("  - Disabled through BIOS/UEFI settings (if supported by firmware)\n");
        printf("  - Physically removed or disconnected (not recommended)\n");
        
        if (cap->disable_method) {
            printf("\nNote: %s\n", cap->disable_method);
        }
    } else {
        printf("\nDisable Method:\n");
        if (cap->disable_method) {
            printf("  %s\n", cap->disable_method);
        } else {
            printf("  No specific method provided.\n");
        }
        
        if (cap->disable_warning) {
            printf("\nWarning:\n");
            printf("  %s\n", cap->disable_warning);
        }
        
        printf("\nNote: Disabling security capabilities may reduce system protection.\n");
        printf("      Proceed with caution and ensure you understand the implications.\n");
    }
}

execution_result_t* executor_control(command_t cmd, char *cap_id, plugin_env_list_t *env_data, capability_list_t *caps)
{
    logger_info(MODULE, "Enabling capability: %s", cap_id);
    
    /* 创建执行结果 */
    execution_result_t *result = execution_result_create();
    if (!result) {
        return NULL;
    }
    
    execution_result_set_capability_id(result, cap_id);
    if (cmd == CMD_ENABLE) {
        execution_result_set_action(result, "enable");
    }
    else if (cmd == CMD_DISABLE) {
        execution_result_set_action(result, "disable");
    }
    else {
        logger_error(MODULE, "Invalid command type: %d", (int)cmd);
        execution_result_set_success(result, false);
        execution_result_set_message(result, "Invalid command type");
        return result;
    }
    
    if (!cap_id || !env_data || !caps) {
        execution_result_set_success(result, false);
        execution_result_set_message(result, "Invalid parameters");
        execution_result_set_guidance(result, "Please provide valid capability ID and environment information");
        return result;
    }
    
    /* 直接判定指定能力的状态（使用外部传入的env_data和caps） */
    capability_t *target_cap = NULL;
    judgement_result_t *judgement = NULL;
    
    psp_error_t ret = judge_specific_capability(cap_id, caps, env_data, &target_cap, &judgement);
    if (ret != ERR_OK || !target_cap || !judgement) {
        execution_result_set_success(result, false);
        execution_result_set_message(result, "Failed to judge capability status");
        execution_result_set_guidance(result, "Check capability ID and ensure it exists in capability list");
        
        if (judgement) judgement_result_free(judgement);
        return result;
    }
    
    /* 计算整体状态 */
    cap_status_t status = compute_overall_status(judgement->cpu_status, 
                                                 judgement->firmware_status, 
                                                 judgement->os_status);
    
    /* 根据状态生成响应 */
    if (status == CAP_STATUS_AVAILABLE) {
        if (cmd == CMD_ENABLE) {
            execution_result_set_success(result, true);
            execution_result_set_message(result, "Capability is already available");
            execution_result_set_guidance(result, "No action needed, capability is fully supported");
        }
        else {
            print_disable_info(caps, cap_id);
        }
    } else {
        if (cmd == CMD_ENABLE) {
            execution_result_set_success(result, false);
            execution_result_set_message(result, "Capability is not currently available");
            
            /* 生成详细的修复建议 */
            char *guidance = generate_enable_guidance(target_cap, judgement, env_data);
            execution_result_set_guidance(result, guidance);
            free(guidance);
        }
        else {
            execution_result_set_success(result, false);
            execution_result_set_message(result, "Capability is unavailable now, no need to disable it");
        }
    }
    
    logger_info(MODULE, "%s completed for %s: status=%s", cmd == CMD_ENABLE ? "Enable" : "Dsiable",
               cap_id, cap_status_to_string(status));
    
    /* 清理资源 */
    judgement_result_free(judgement);
    
    return result;
}
