#include "core/judge.h"
#include "logger.h"
#include "utils/string_utils.h"
#include "models/result.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MODULE "judge"

/* 失败详情数组大小限制 */
#define MAX_TOTAL_FAILURE_DETAILS    20   /* 总的失败详情最大数量 */
#define MAX_GROUP_FAILURE_DETAILS    10   /* 单个条件组的失败详情最大数量 */

/* 失败详情缓冲区大小 */
#define MAX_FAILED_DETAIL_SIZE       512  /* 失败详情字符串最大长度 */
#define MAX_EXPECTED_DISPLAY_SIZE    256  /* 期望值显示字符串最大长度（需为失败详情预留空间） */

/**
 * @brief 从plugin_env_list_t中获取指定key的值
 */
static const char* get_env_value(const plugin_env_list_t *env_list, const char *key)
{
    if (!env_list || !key) {
        return NULL;
    }
    
    for (size_t i = 0; i < env_list->count; i++) {
        if (env_list->items[i] && strcmp(env_list->items[i]->key, key) == 0) {
            return env_list->items[i]->value;
        }
    }
    
    return NULL;
}

/**
 * @brief 版本号比较函数
 * @return >0 if v1>v2, 0 if v1==v2, <0 if v1<v2
 */
static int compare_versions(const char *v1, const char *v2)
{
    if (!v1 || !v2) return 0;
    
    while (*v1 || *v2) {
        long num1 = 0, num2 = 0;
        
        /* 解析v1的下一个数字段 */
        while (*v1 && *v1 != '.') {
            num1 = num1 * 10 + (*v1 - '0');
            v1++;
        }
        if (*v1 == '.') v1++;
        
        /* 解析v2的下一个数字段 */
        while (*v2 && *v2 != '.') {
            num2 = num2 * 10 + (*v2 - '0');
            v2++;
        }
        if (*v2 == '.') v2++;
        
        if (num1 != num2) {
            return (num1 > num2) ? 1 : -1;
        }
    }
    
    return 0;
}

/**
 * @brief 评估单个条件
 * 
 * @param env_list 环境变量列表
 * @param cond 条件对象
 * @param failed_detail 如果失败，存储失败详情（可选）
 * @return true if condition is met
 */
static bool evaluate_condition(const plugin_env_list_t *env_list, 
                               const condition_t *cond,
                               char *failed_detail, size_t detail_size)
{
    if (!env_list || !cond) {
        return false;
    }
    
    /* 构建期望值的显示字符串（限制长度以避免后续格式化时溢出） */
    char expected_display[MAX_EXPECTED_DISPLAY_SIZE] = {0};
    if (cond->expected_values && cond->expected_values_count > 0) {
        size_t offset = 0;
        for (size_t i = 0; i < cond->expected_values_count && offset < sizeof(expected_display) - 10; i++) {
            int written;
            if (i > 0) {
                written = snprintf(expected_display + offset, sizeof(expected_display) - offset, ", ");
                if (written < 0 || (size_t)written >= sizeof(expected_display) - offset) {
                    break;  /* 缓冲区不足，停止拼接 */
                }
                offset += written;
            }
            written = snprintf(expected_display + offset, sizeof(expected_display) - offset, "%s", 
                              cond->expected_values[i]);
            if (written < 0 || (size_t)written >= sizeof(expected_display) - offset) {
                break;  /* 缓冲区不足，停止拼接 */
            }
            offset += written;
        }
    }
    
    // logger_debug(MODULE, "Evaluating condition: %s %s [%s]", 
    //             cond->field_name, cond->operator, expected_display);
    
    /* 获取实际值 */
    const char *actual_value = get_env_value(env_list, cond->field_name);
    if (!actual_value) {
        if (failed_detail && detail_size > 0) {
            snprintf(failed_detail, detail_size, "%s (missing)", cond->field_name);
        }
        logger_debug(MODULE, "Field not found: %s", cond->field_name);
        return false;
    }
    
    /* 根据操作符进行比较 */
    bool result = false;
    
    if (strcmp(cond->operator, "==") == 0 || strcmp(cond->operator, "in") == 0) {
        /* 对于 == 或 in 操作符，检查实际值是否在期望值数组中 */
        if (cond->expected_values && cond->expected_values_count > 0) {
            for (size_t i = 0; i < cond->expected_values_count; i++) {
                if (strcmp(actual_value, cond->expected_values[i]) == 0) {
                    result = true;
                    break;
                }
            }
        }
    } else if (strcmp(cond->operator, "!=") == 0) {
        /* != 操作符：检查实际值是否不在期望值数组中 */
        result = true;  /* 默认认为不匹配 */
        if (cond->expected_values && cond->expected_values_count > 0) {
            for (size_t i = 0; i < cond->expected_values_count; i++) {
                if (strcmp(actual_value, cond->expected_values[i]) == 0) {
                    result = false;  /* 找到了匹配项，所以 != 为假 */
                    break;
                }
            }
        }
    } else if (strcmp(cond->operator, ">=") == 0 || strcmp(cond->operator, "<=") == 0 ||
               strcmp(cond->operator, ">") == 0 || strcmp(cond->operator, "<") == 0) {
        /* 版本比较操作符：使用第一个期望值进行比较 */
        if (cond->expected_values && cond->expected_values_count > 0) {
            const char *expected_val = cond->expected_values[0];
            
            if (strcmp(cond->operator, ">=") == 0) {
                result = (compare_versions(actual_value, expected_val) >= 0);
            } else if (strcmp(cond->operator, "<=") == 0) {
                result = (compare_versions(actual_value, expected_val) <= 0);
            } else if (strcmp(cond->operator, ">") == 0) {
                result = (compare_versions(actual_value, expected_val) > 0);
            } else if (strcmp(cond->operator, "<") == 0) {
                result = (compare_versions(actual_value, expected_val) < 0);
            }
        }
    } else {
        logger_warn(MODULE, "Unknown operator: %s", cond->operator);
        return false;
    }
    
    /* 记录失败详情 */
    if (!result && failed_detail && detail_size > 0) {
        snprintf(failed_detail, detail_size, "%s (expected one of: [%s], actual: %s)", 
                cond->field_name, expected_display, actual_value);
    }
    
    logger_debug(MODULE, "Condition check: %s %s [%s] (actual: %s) -> %d", 
                cond->field_name, cond->operator, expected_display, actual_value, result);
    
    return result;
}

/**
 * @brief 评估条件组并返回状态
 * 
 * @param env_list 环境变量列表
 * @param group 条件组
 * @param failed_details 失败详情数组（输出）
 * @param detail_count 失败详情数量（输出）
 * @param max_details 最大详情数
 * @return layer_status_t 条件组状态
 */
static layer_status_t evaluate_condition_group(const plugin_env_list_t *env_list,
                                               const condition_group_t *group,
                                               char **failed_details, 
                                               int *detail_count, 
                                               int max_details)
{
    if (!env_list || !group) {
        return LAYER_STATUS_UNKNOWN;
    }
    
    logger_debug(MODULE, "Evaluating condition group (%s, %zu conditions)", 
                group->is_and_group ? "AND" : "OR", group->count);
    
    if (group->count == 0) {
        logger_debug(MODULE, "Empty condition group, returning SUPPORTED");
        return LAYER_STATUS_SUPPORTED;  /* 空组视为满足 */
    }
    
    if (group->is_and_group) {
        /* AND 组：所有条件都必须满足，并收集最严重的status */
        bool all_met = true;
        layer_status_t worst_status = LAYER_STATUS_SUPPORTED;  /* 记录最严重的状态 */
        
        for (size_t i = 0; i < group->count; i++) {
            char failed_detail[MAX_FAILED_DETAIL_SIZE] = {0};
            bool result = evaluate_condition(env_list, group->conditions[i],
                                           failed_detail, sizeof(failed_detail));
            
            if (!result) {
                all_met = false;
                if (failed_details && *detail_count < max_details) {
                    char *detail_copy = strdup(failed_detail);
                    if (detail_copy) {
                        failed_details[*detail_count] = detail_copy;
                        (*detail_count)++;
                    } else {
                        logger_warn(MODULE, "Failed to duplicate failed detail string");
                    }
                }
                logger_debug(MODULE, "AND condition failed: %s", failed_detail);
            } else {
                /* 条件满足，获取其status并更新最严重状态 */
                const char *current_status_str = group->conditions[i]->status;
                layer_status_t current_status = LAYER_STATUS_SUPPORTED;
                
                if (current_status_str) {
                    if (strcmp(current_status_str, "unsupported") == 0) {
                        current_status = LAYER_STATUS_UNSUPPORTED;
                    } else if (strcmp(current_status_str, "disable") == 0) {
                        current_status = LAYER_STATUS_DISABLED;
                    } else {
                        current_status = LAYER_STATUS_SUPPORTED;
                    }
                }
                
                /* AND组需要选择最严重的状态（与OR组相反） */
                /* 因为AND组要求全部满足，任何一个条件的严重状态都会影响整体 */
                if (current_status == LAYER_STATUS_UNSUPPORTED) {
                    worst_status = LAYER_STATUS_UNSUPPORTED;
                } else if (current_status == LAYER_STATUS_DISABLED && worst_status != LAYER_STATUS_UNSUPPORTED) {
                    worst_status = LAYER_STATUS_DISABLED;
                }
                
                logger_debug(MODULE, "AND condition %zu met with status: %s -> %d", 
                            i, current_status_str ? current_status_str : "support", current_status);
            }
        }
        
        if (all_met) {
            logger_debug(MODULE, "All AND conditions met, returning status: %d", worst_status);
            return worst_status;
        } else {
            logger_debug(MODULE, "Some AND conditions failed, returning UNSUPPORTED");
            return LAYER_STATUS_UNSUPPORTED;
        }
    } else {
        /* OR 组：检查所有条件，选择最优的status */
        layer_status_t best_status = LAYER_STATUS_UNSUPPORTED;
        bool any_met = false;
        
        for (size_t i = 0; i < group->count; i++) {
            char failed_detail[MAX_FAILED_DETAIL_SIZE] = {0};
            bool result = evaluate_condition(env_list, group->conditions[i],
                                           failed_detail, sizeof(failed_detail));
            
            if (result) {
                any_met = true;
                
                /* 获取当前条件的status */
                const char *current_status_str = group->conditions[i]->status;
                layer_status_t current_status = LAYER_STATUS_SUPPORTED;
                
                if (current_status_str) {
                    if (strcmp(current_status_str, "unsupported") == 0) {
                        current_status = LAYER_STATUS_UNSUPPORTED;
                    } else if (strcmp(current_status_str, "disable") == 0) {
                        current_status = LAYER_STATUS_DISABLED;
                    } else {
                        current_status = LAYER_STATUS_SUPPORTED;
                    }
                }
                
                /* 选择最优的状态：SUPPORTED > DISABLED > UNSUPPORTED */
                if (current_status == LAYER_STATUS_SUPPORTED) {
                    best_status = LAYER_STATUS_SUPPORTED;
                } else if (current_status == LAYER_STATUS_DISABLED && best_status != LAYER_STATUS_SUPPORTED) {
                    best_status = LAYER_STATUS_DISABLED;
                } else if (!any_met || best_status == LAYER_STATUS_UNSUPPORTED) {
                    best_status = current_status;
                }
                
                logger_info(MODULE, "OR condition %zu matched with status: %s -> %d", 
                           i, current_status_str ? current_status_str : "support", current_status);
                
                /* 清理之前记录的失败详情 */
                for (int j = 0; j < *detail_count; j++) {
                    free(failed_details[j]);
                }
                *detail_count = 0;
            } else {
                if (*detail_count < max_details) {
                    char *detail_copy = strdup(failed_detail);
                    if (detail_copy) {
                        failed_details[*detail_count] = detail_copy;
                        (*detail_count)++;
                    } else {
                        logger_warn(MODULE, "Failed to duplicate failed detail string");
                    }
                }
            }
        }
        
        if (any_met) {
            logger_info(MODULE, "OR group final status: %d (best among matched conditions)", best_status);
            return best_status;
        } else {
            logger_debug(MODULE, "No OR conditions matched, returning UNSUPPORTED");
            return LAYER_STATUS_UNSUPPORTED;
        }
    }
}

/**
 * @brief 评估层级规则并返回状态
 * 
 * @param env_list 插件环境变量列表
 * @param rule 层级规则
 * @param failed_details 失败详情数组（输出）
 * @param detail_count 失败详情数量（输出）
 * @return layer_status_t 层级状态
 */
static layer_status_t evaluate_layer_rule(const plugin_env_list_t *env_list,
                                         const layer_rule_t *rule,
                                         char ***failed_details, 
                                         int *detail_count)
{
    if (!env_list || !rule) {
        return LAYER_STATUS_UNKNOWN;
    }
    
    *detail_count = 0;
    
    /* 如果没有条件组，返回 supported */
    if ((rule->or_group_count == 0) && (rule->and_group_count == 0)) {
        logger_debug(MODULE, "No condition groups in rule, returning SUPPORTED");
        return LAYER_STATUS_SUPPORTED;
    }
    
    logger_debug(MODULE, "Evaluating layer rule: %zu OR groups, %zu AND groups", 
                rule->or_group_count, rule->and_group_count);
    
    /* 分配失败详情数组 */
    char **details = (char**)malloc(sizeof(char*) * MAX_TOTAL_FAILURE_DETAILS);
    if (!details) {
        return LAYER_STATUS_UNKNOWN;
    }
    
    int count = 0;
    
    /* ========== 评估 OR 组（alternatives）：至少一个满足即可 ========== */
    layer_status_t or_status = LAYER_STATUS_SUPPORTED;  /* 默认supported，如果没有OR组或所有OR组都失败才改变 */
    bool any_or_met = false;
    
    if (rule->or_group_count > 0) {
        logger_debug(MODULE, "Evaluating %zu OR groups", rule->or_group_count);
        
        /* 有OR组时，初始设为unsupported，只有匹配成功才会更新 */
        or_status = LAYER_STATUS_UNSUPPORTED;
        
        for (size_t i = 0; i < rule->or_group_count; i++) {
            char *group_failed_details[MAX_GROUP_FAILURE_DETAILS] = {NULL};
            int group_detail_count = 0;
            
            /* 直接获取该OR组的状态 */
            layer_status_t group_status = evaluate_condition_group(env_list, 
                                                                   rule->or_groups[i],
                                                                   group_failed_details, 
                                                                   &group_detail_count, 10);
            
            if (group_status != LAYER_STATUS_UNSUPPORTED) {
                any_or_met = true;
                
                logger_debug(MODULE, "OR group %zu matched with status: %d", i, group_status);
                
                /* 选择最好的状态：SUPPORTED > DISABLED > UNSUPPORTED */
                /* OR组是替代方案，应该选择最优的那个 */
                if (group_status == LAYER_STATUS_SUPPORTED) {
                    or_status = LAYER_STATUS_SUPPORTED;
                } else if (group_status == LAYER_STATUS_DISABLED && or_status != LAYER_STATUS_SUPPORTED) {
                    or_status = LAYER_STATUS_DISABLED;
                } else if (!any_or_met || or_status == LAYER_STATUS_UNSUPPORTED) {
                    or_status = group_status;
                }
                
                /* 清理之前记录的失败详情 */
                for (int j = 0; j < count; j++) {
                    free(details[j]);
                }
                count = 0;
            } else {
                logger_debug(MODULE, "OR group %zu not satisfied", i);
                /* 记录失败详情 */
                for (int j = 0; j < group_detail_count && count < MAX_TOTAL_FAILURE_DETAILS; j++) {
                    details[count++] = group_failed_details[j];
                }
            }
        }
        
        if (!any_or_met) {
            or_status = LAYER_STATUS_UNSUPPORTED;
            logger_debug(MODULE, "No OR group satisfied, or_status = %d", or_status);
        } else {
            logger_debug(MODULE, "At least one OR group satisfied, or_status = %d", or_status);
        }
    } else {
        /* 没有OR组，默认为supported */
        or_status = LAYER_STATUS_SUPPORTED;
        logger_debug(MODULE, "No OR groups, or_status = %d", or_status);
    }
    
    /* ========== 评估 AND 组（requirements）：必须全部满足 ========== */
    layer_status_t and_status = LAYER_STATUS_SUPPORTED;
    bool all_and_met = true;
    
    if (rule->and_group_count > 0) {
        logger_debug(MODULE, "Evaluating %zu AND groups", rule->and_group_count);
        
        for (size_t i = 0; i < rule->and_group_count; i++) {
            char *group_failed_details[MAX_GROUP_FAILURE_DETAILS] = {NULL};
            int group_detail_count = 0;
            
            /* 直接获取该AND组的状态 */
            layer_status_t group_status = evaluate_condition_group(env_list, 
                                                                   rule->and_groups[i],
                                                                   group_failed_details, 
                                                                   &group_detail_count, 10);
            
            if (group_status != LAYER_STATUS_SUPPORTED) {
                all_and_met = false;
                logger_debug(MODULE, "AND group %zu not satisfied with status: %d", i, group_status);
                /* 合并失败详情 */
                for (int j = 0; j < group_detail_count && count < MAX_TOTAL_FAILURE_DETAILS; j++) {
                    details[count++] = group_failed_details[j];
                }
            } else {
                logger_debug(MODULE, "AND group %zu satisfied", i);
                /* 清理成功的组的失败详情 */
                for (int j = 0; j < group_detail_count; j++) {
                    free(group_failed_details[j]);
                }
            }
        }
        
        if (!all_and_met) {
            /* AND 组不满足，统一返回 unsupported */
            and_status = LAYER_STATUS_UNSUPPORTED;
            logger_debug(MODULE, "Not all AND groups satisfied, and_status = UNSUPPORTED");
        } else {
            logger_debug(MODULE, "All AND groups satisfied, and_status = %d", and_status);
        }
    } else {
        logger_debug(MODULE, "No AND groups, and_status = %d", and_status);
        /* 没有AND组，默认为supported */
        and_status = LAYER_STATUS_SUPPORTED;
    }
    
    /* 最终状态：取OR组和AND组中最严重的状态 */
    layer_status_t final_status;
    if (or_status == LAYER_STATUS_UNSUPPORTED || and_status == LAYER_STATUS_UNSUPPORTED) {
        final_status = LAYER_STATUS_UNSUPPORTED;
    } else if (or_status == LAYER_STATUS_DISABLED || and_status == LAYER_STATUS_DISABLED) {
        final_status = LAYER_STATUS_DISABLED;
    } else {
        final_status = LAYER_STATUS_SUPPORTED;
    }
    
    *failed_details = details;
    *detail_count = count;
    
    logger_debug(MODULE, "Layer rule evaluation result: %d", final_status);
    
    return final_status;
}

/**
 * @brief 生成详细的缺失配置描述
 */
static void generate_missing_config_description(/*const capability_t *cap,*/
                                               layer_status_t cpu_status,
                                               layer_status_t firmware_status,
                                               layer_status_t os_status,
                                               char *desc, size_t desc_size)
{
    if (!desc || desc_size == 0) {
        return;
    }
    
    memset(desc, 0, desc_size);
    
    /* CPU层不支持 */
    if (cpu_status == LAYER_STATUS_UNSUPPORTED) {
        snprintf(desc, desc_size, "CPU hardware does not support this capability");
        return;
    }
    
    /* CPU层禁用 */
    if (cpu_status == LAYER_STATUS_DISABLED) {
        snprintf(desc, desc_size, "CPU supports this capability but it is disabled");
        return;
    }
    
    /* 固件层不支持 */
    if (firmware_status == LAYER_STATUS_UNSUPPORTED) {
        snprintf(desc, desc_size, "Firmware does not support this capability");
        return;
    }
    
    /* 固件层禁用 */
    if (firmware_status == LAYER_STATUS_DISABLED) {
        snprintf(desc, desc_size, "Firmware supports this capability but it is disabled");
        return;
    }
    
    /* OS层不支持 */
    if (os_status == LAYER_STATUS_UNSUPPORTED) {
        snprintf(desc, desc_size, "OS does not support this capability");
        return;
    }
    
    /* OS层禁用 */
    if (os_status == LAYER_STATUS_DISABLED) {
        snprintf(desc, desc_size, "OS supports this capability but it is disabled");
        return;
    }
    
    /* 默认情况 */
    snprintf(desc, desc_size, "Capability is not available");
}

/**
 * @brief 生成失败详情的描述字符串
 */
static void generate_failure_description(char **failed_details, int detail_count,
                                        char *desc, size_t desc_size)
{
    if (!desc || desc_size == 0) {
        return;
    }
    
    if (detail_count == 0) {
        snprintf(desc, desc_size, "Requirement not met");
        return;
    }
    
    size_t offset = 0;
    offset += snprintf(desc + offset, desc_size - offset, "Failed checks: ");
    
    for (int i = 0; i < detail_count && offset < desc_size - 1; i++) {
        if (i > 0) {
            offset += snprintf(desc + offset, desc_size - offset, ", ");
        }
        offset += snprintf(desc + offset, desc_size - offset, "%s", failed_details[i]);
    }
}

/**
 * @brief 释放失败详情数组
 */
static void free_failed_details(char **failed_details, int count)
{
    if (!failed_details) return;
    
    for (int i = 0; i < count; i++) {
        free(failed_details[i]);
    }
    free(failed_details);
}

/**
 * @brief 判定CPU层状态并设置到结果中
 */
void judge_cpu_layer(const plugin_env_list_t *env_list, const capability_t *cap, 
                    judgement_result_t *result)
{
    if (!env_list || !cap || !result) {
        return;
    }
    
    logger_debug(MODULE, "Judging CPU layer for capability: %s", cap->id ? cap->id : "unknown");
    
    /* 评估规则并直接设置状态 */
    char **failed_details = NULL;
    int detail_count = 0;
    result->cpu_status = evaluate_layer_rule(env_list, &cap->cpu_rule, 
                                            &failed_details, &detail_count);
    
    /* 记录失败详情 */
    if (result->cpu_status == LAYER_STATUS_UNSUPPORTED && detail_count > 0) {
        char failure_desc[512];
        generate_failure_description(failed_details, detail_count, 
                                    failure_desc, sizeof(failure_desc));
        logger_warn(MODULE, "CPU layer unsupported for %s: %s", 
                   cap->name ? cap->name : cap->id, failure_desc);
    }
    
    free_failed_details(failed_details, detail_count);
    logger_debug(MODULE, "CPU layer status for %s: %d", cap->id, result->cpu_status);
}

/**
 * @brief 判定固件层状态并设置到结果中
 */
void judge_firmware_layer(const plugin_env_list_t *env_list, const capability_t *cap,
                         judgement_result_t *result)
{
    if (!env_list || !cap || !result) {
        return;
    }
    
    logger_debug(MODULE, "Judging firmware layer for capability: %s", cap->id ? cap->id : "unknown");
    
    /* 评估规则并直接设置状态 */
    char **failed_details = NULL;
    int detail_count = 0;
    result->firmware_status = evaluate_layer_rule(env_list, &cap->firmware_rule, 
                                                 &failed_details, &detail_count);
    
    /* 记录失败详情 */
    if (result->firmware_status == LAYER_STATUS_UNSUPPORTED && detail_count > 0) {
        char failure_desc[512];
        generate_failure_description(failed_details, detail_count, 
                                    failure_desc, sizeof(failure_desc));
        logger_warn(MODULE, "Firmware layer unsupported for %s: %s", 
                   cap->name ? cap->name : cap->id, failure_desc);
    }
    
    free_failed_details(failed_details, detail_count);
    logger_debug(MODULE, "Firmware layer status for %s: %d", cap->id, result->firmware_status);
}

/**
 * @brief 判定OS层状态并设置到结果中
 */
void judge_os_layer(const plugin_env_list_t *env_list, const capability_t *cap,
                   judgement_result_t *result)
{
    if (!env_list || !cap || !result) {
        return;
    }
    
    logger_debug(MODULE, "Judging OS layer for capability: %s", cap->id ? cap->id : "unknown");
    
    /* 评估规则并直接设置状态 */
    char **failed_details = NULL;
    int detail_count = 0;
    result->os_status = evaluate_layer_rule(env_list, &cap->os_rule, 
                                           &failed_details, &detail_count);
    
    /* 记录失败详情 */
    if (result->os_status == LAYER_STATUS_UNSUPPORTED && detail_count > 0) {
        char failure_desc[512];
        generate_failure_description(failed_details, detail_count, 
                                    failure_desc, sizeof(failure_desc));
        logger_warn(MODULE, "OS layer unsupported for %s: %s", 
                   cap->name ? cap->name : cap->id, failure_desc);
    }
    
    free_failed_details(failed_details, detail_count);
    logger_debug(MODULE, "OS layer status for %s: %d", cap->id, result->os_status);
}

bool check_dependencies(const capability_t *cap, 
                       const judgement_result_list_t *results)
{   
    if (!cap || !cap->dependencies || cap->dependency_count == 0) {
        /* 没有依赖，认为满足 */
        return true;
    }
    
    if (!results) {
        /* 没有结果列表，无法检查依赖 */
        return false;
    }
    
    /* 检查每个依赖的能力是否可用 */
    for (size_t i = 0; i < cap->dependency_count; i++) {
        const char *dep_id = cap->dependencies[i];
        
        /* 在结果列表中查找该依赖 */
        bool found = false;
        bool dep_available = false;
        
        for (size_t j = 0; j < results->count; j++) {
            judgement_result_t *result = results->items[j];
            if (result && result->capability_id && 
                strcmp(result->capability_id, dep_id) == 0) {
                found = true;
                
                /* 检查依赖的状态 */
                if (result->overall_status == CAP_STATUS_AVAILABLE ||
                    result->overall_status == CAP_STATUS_SUPPORTED) {
                    dep_available = true;
                }
                break;
            }
        }
        
        if (!found || !dep_available) {
            logger_debug(MODULE, "Dependency not met: %s", dep_id);
            return false;
        }
    }
    
    logger_debug(MODULE, "All dependencies met");
    return true;
}

cap_status_t compute_overall_status(layer_status_t cpu_status, 
                                   layer_status_t firmware_status,
                                   layer_status_t os_status)
{
    /* 简化逻辑：Overall Status 只有 available 和 unavailable
     * - 所有层都是 supported → available
     * - 任意一层是 unsupported/disabled/unknown/unavailable → unavailable
     */
    
    /* 如果所有层都支持，整体可用 */
    if (cpu_status == LAYER_STATUS_SUPPORTED &&
        firmware_status == LAYER_STATUS_SUPPORTED &&
        os_status == LAYER_STATUS_SUPPORTED) {
        return CAP_STATUS_AVAILABLE;
    }
    
    /* 其他情况都不可用 */
    return CAP_STATUS_UNAVAILABLE;
}

judgement_result_list_t* judge_capabilities(const plugin_env_list_t *env_list,
                                                     const capability_list_t *caps)
{
    if (!caps || !env_list) {
        return NULL;
    }
    
    logger_info(MODULE, "Starting capability judgement for %zu capabilities", caps->count);
    
    /* 创建结果列表 */
    judgement_result_list_t *results = judgement_result_list_create();
    if (!results) {
        return NULL;
    }
    
    /* 逐个判定能力 */
    for (size_t i = 0; i < caps->count; i++) {
        capability_t *cap = caps->items[i];
        if (!cap) continue;
        
        /* 创建结果项 */
        judgement_result_t *result = judgement_result_create();
        if (!result) {
            logger_warn(MODULE, "Failed to create result for capability %zu", i);
            continue;
        }
        
        /* 设置能力 ID 和名称 */
        free(result->capability_id);
        result->capability_id = strdup(cap->id);
        if (cap->name) {
            free(result->capability_name);
            result->capability_name = strdup(cap->name);
        }
        
        /* 判定各层状态（直接设置到result中） */
        judge_cpu_layer(env_list, cap, result);
        judge_firmware_layer(env_list, cap, result);
        judge_os_layer(env_list, cap, result);
        
        /* 计算并设置整体状态 */
        result->overall_status = compute_overall_status(result->cpu_status, 
                                                       result->firmware_status, 
                                                       result->os_status);
        
        /* 生成详细的描述信息（包含缺失配置提醒） */
        char desc[512] = {0};
        if (result->overall_status == CAP_STATUS_UNSUPPORTED || result->overall_status == CAP_STATUS_UNAVAILABLE) {
            /* 生成详细的缺失配置描述 */
            generate_missing_config_description(result->cpu_status, result->firmware_status, 
                                              result->os_status, desc, sizeof(desc));
        } else if (cap->description && strlen(cap->description) > 0) {
            strncpy(desc, cap->description, sizeof(desc) - 1);
        } else {
            snprintf(desc, sizeof(desc), "Capability is available");
        }
        
        /* 释放旧的description并设置新的 */
        free(result->description);
        result->description = strdup(desc);
        
        /* 生成证据（JSON格式，包含详细信息） */
        char evidence[2048];
        snprintf(evidence, sizeof(evidence),
                "{\"cpu\":\"%s\",\"firmware\":\"%s\",\"os\":\"%s\"}",
                layer_status_to_string(result->cpu_status),
                layer_status_to_string(result->firmware_status),
                layer_status_to_string(result->os_status));
        
        /* 释放旧的evidence并设置新的 */
        free(result->evidence);
        result->evidence = strdup(evidence);
        
        /* 添加到结果列表 */
        psp_error_t ret = judgement_result_list_add(results, result);
        if (ret != ERR_OK) {
            judgement_result_free(result);
        }
        
        logger_info(MODULE, "Judged capability %s: overall=%s, cpu=%s, fw=%s, os=%s",
                   cap->id ? cap->id : "unknown",
                   cap_status_to_string(result->overall_status),
                   layer_status_to_string(result->cpu_status),
                   layer_status_to_string(result->firmware_status),
                   layer_status_to_string(result->os_status));
    }
    
    logger_info(MODULE, "Capability judgement completed, %zu results", results->count);
    return results;
}
