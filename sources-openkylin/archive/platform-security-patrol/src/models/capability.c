#include "models/capability.h"
#include "utils/string_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define MODULE "capability"

/* ==================== 条件管理 ==================== */

condition_t* condition_create(void)
{
    condition_t *cond = (condition_t*)calloc(1, sizeof(condition_t));
    if (!cond) {
        logger_error(MODULE, "Failed to allocate memory for condition");
        return NULL;
    }
    
    cond->field_name = NULL;
    cond->expected_values = NULL;
    cond->expected_values_count = 0;
    cond->operator = NULL;
    cond->status = NULL;
    cond->type = COND_TYPE_UNKNOWN;  /* 初始化为未知类型 */
    
    return cond;
}

void condition_free(condition_t *cond)
{
    if (!cond) return;
    
    free(cond->field_name);
    
    /* 释放期望值数组 */
    if (cond->expected_values) {
        for (size_t i = 0; i < cond->expected_values_count; i++) {
            free(cond->expected_values[i]);
        }
        free(cond->expected_values);
    }
    
    free(cond->operator);
    free(cond->status);  /* 释放 status 字段 */
    free(cond->description);  /* 释放 description 字段（可选字段，可能为NULL） */
    
    free(cond);
}

/**
 * @brief 设置条件的期望值（统一使用数组格式）
 * 
 * @param cond 条件对象
 * @param field_name 字段名
 * @param expected_values 期望值数组
 * @param count 数组长度
 * @param operator 操作符（如 "==", "in", ">=" 等）
 * @return error_t
 */
psp_error_t condition_set(condition_t *cond, const char *field_name, 
                     const char **expected_values, size_t count,
                     const char *operator)
{
    if (!cond || !field_name || !expected_values || count == 0 || !operator) {
        return ERR_INVALID_PARAM;
    }
    
    free(cond->field_name);
    cond->field_name = string_utils_strdup(field_name);
    
    /* 释放旧的期望值数组 */
    if (cond->expected_values) {
        for (size_t i = 0; i < cond->expected_values_count; i++) {
            free(cond->expected_values[i]);
        }
        free(cond->expected_values);
    }
    
    /* 分配并复制新数组 */
    cond->expected_values = (char**)calloc(count, sizeof(char*));
    if (!cond->expected_values) {
        logger_error(MODULE, "Failed to allocate memory for expected values array");
        return ERR_MEMORY;
    }
    
    for (size_t i = 0; i < count; i++) {
        cond->expected_values[i] = string_utils_strdup(expected_values[i]);
        if (!cond->expected_values[i]) {
            logger_error(MODULE, "Failed to duplicate expected value");
            /* 清理已分配的资源 */
            for (size_t j = 0; j < i; j++) {
                free(cond->expected_values[j]);
            }
            free(cond->expected_values);
            cond->expected_values = NULL;
            cond->expected_values_count = 0;
            return ERR_MEMORY;
        }
    }
    cond->expected_values_count = count;
    
    free(cond->operator);
    cond->operator = string_utils_strdup(operator);
    
    if (!cond->field_name || !cond->operator) {
        logger_error(MODULE, "Failed to set condition fields");
        return ERR_MEMORY;
    }
    
    return ERR_OK;
}

/* 保留 condition_set_multi 作为别名，保持 API 一致性 */
psp_error_t condition_set_multi(condition_t *cond, const char *field_name,
                           const char **expected_values, size_t count,
                           const char *operator)
{
    return condition_set(cond, field_name, expected_values, count, operator);
}

/* ==================== 条件组管理 ==================== */

condition_group_t* condition_group_create(bool is_and_group)
{
    condition_group_t *group = (condition_group_t*)calloc(1, sizeof(condition_group_t));
    if (!group) {
        logger_error(MODULE, "Failed to allocate memory for condition group");
        return NULL;
    }
    
    group->conditions = NULL;
    group->count = 0;
    group->capacity = 0;
    group->is_and_group = is_and_group;
    
    return group;
}

void condition_group_free(condition_group_t *group)
{
    if (!group) return;
    
    if (group->conditions) {
        for (size_t i = 0; i < group->count; i++) {
            condition_free(group->conditions[i]);
        }
        free(group->conditions);
    }
    
    free(group);
}

psp_error_t condition_group_add(condition_group_t *group, condition_t *cond)
{
    if (!group || !cond) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查容量 */
    if (group->count >= group->capacity) {
        size_t new_capacity = (group->capacity == 0) ? 8 : group->capacity * 2;
        condition_t **new_conditions = (condition_t**)realloc(group->conditions,
                                                              new_capacity * sizeof(condition_t*));
        if (!new_conditions) {
            logger_error(MODULE, "Failed to reallocate memory for conditions");
            return ERR_MEMORY;
        }
        
        group->conditions = new_conditions;
        group->capacity = new_capacity;
    }
    
    group->conditions[group->count++] = cond;
    return ERR_OK;
}

/* ==================== 层规则管理 ==================== */

void layer_rule_init(layer_rule_t *rule)
{
    if (!rule) return;
    
    rule->or_groups = NULL;
    rule->or_group_count = 0;
    rule->or_group_capacity = 0;
    
    rule->and_groups = NULL;
    rule->and_group_count = 0;
    rule->and_group_capacity = 0;
    
    rule->required = false;
}

void layer_rule_free(layer_rule_t *rule)
{
    if (!rule) return;
    
    /* 释放 OR 组 */
    if (rule->or_groups) {
        for (size_t i = 0; i < rule->or_group_count; i++) {
            condition_group_free(rule->or_groups[i]);
        }
        free(rule->or_groups);
    }
    
    /* 释放 AND 组 */
    if (rule->and_groups) {
        for (size_t i = 0; i < rule->and_group_count; i++) {
            condition_group_free(rule->and_groups[i]);
        }
        free(rule->and_groups);
    }
    
    rule->or_groups = NULL;
    rule->or_group_count = 0;
    rule->or_group_capacity = 0;
    
    rule->and_groups = NULL;
    rule->and_group_count = 0;
    rule->and_group_capacity = 0;
    
    rule->required = false;
}

/**
 * @brief 向层规则的 OR 组数组添加条件组（来自 alternatives）
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_or_group(layer_rule_t *rule, condition_group_t *group)
{
    if (!rule || !group) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查容量 */
    if (rule->or_group_count >= rule->or_group_capacity) {
        size_t new_capacity = (rule->or_group_capacity == 0) ? 4 : rule->or_group_capacity * 2;
        condition_group_t **new_groups = (condition_group_t**)realloc(rule->or_groups,
                                                                       new_capacity * sizeof(condition_group_t*));
        if (!new_groups) {
            logger_error(MODULE, "Failed to reallocate memory for OR groups");
            return ERR_MEMORY;
        }
        
        rule->or_groups = new_groups;
        rule->or_group_capacity = new_capacity;
    }
    
    rule->or_groups[rule->or_group_count++] = group;
    return ERR_OK;
}

/**
 * @brief 向层规则的 AND 组数组添加条件组（来自 requirements）
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_and_group(layer_rule_t *rule, condition_group_t *group)
{
    if (!rule || !group) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查容量 */
    if (rule->and_group_count >= rule->and_group_capacity) {
        size_t new_capacity = (rule->and_group_capacity == 0) ? 4 : rule->and_group_capacity * 2;
        condition_group_t **new_groups = (condition_group_t**)realloc(rule->and_groups,
                                                                       new_capacity * sizeof(condition_group_t*));
        if (!new_groups) {
            logger_error(MODULE, "Failed to reallocate memory for AND groups");
            return ERR_MEMORY;
        }
        
        rule->and_groups = new_groups;
        rule->and_group_capacity = new_capacity;
    }
    
    rule->and_groups[rule->and_group_count++] = group;
    return ERR_OK;
}

/**
 * @brief 向层规则添加条件组（兼容旧接口，默认添加到 AND 组）
 * @deprecated 请使用 layer_rule_add_or_group 或 layer_rule_add_and_group
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_group(layer_rule_t *rule, condition_group_t *group)
{
    /* 为了向后兼容，默认添加到 AND 组 */
    return layer_rule_add_and_group(rule, group);
}

capability_t* capability_create(void)
{
    capability_t *cap = (capability_t*)calloc(1, sizeof(capability_t));
    if (!cap) {
        logger_error(MODULE, "Failed to allocate memory for capability");
        return NULL;
    }
    
    cap->id = NULL;
    cap->name = NULL;
    cap->description = NULL;
    layer_rule_init(&cap->cpu_rule);
    layer_rule_init(&cap->firmware_rule);
    layer_rule_init(&cap->os_rule);
    cap->dependencies = NULL;
    cap->dependency_count = 0;
    
    /* 初始化disable_info字段 */
    cap->disable_supported = false;
    cap->disable_method = NULL;
    cap->disable_warning = NULL;
    
    // logger_debug(MODULE, "Capability object created");
    return cap;
}

void capability_free(capability_t *cap)
{
    if (!cap) return;
    
    free(cap->id);
    free(cap->name);
    free(cap->description);
    layer_rule_free(&cap->cpu_rule);
    layer_rule_free(&cap->firmware_rule);
    layer_rule_free(&cap->os_rule);
    
    if (cap->dependencies) {
        for (size_t i = 0; i < cap->dependency_count; i++) {
            free(cap->dependencies[i]);
        }
        free(cap->dependencies);
    }
    
    /* 释放disable_info字段 */
    free(cap->disable_method);
    free(cap->disable_warning);
    
    cap->id = NULL;
    cap->name = NULL;
    cap->description = NULL;
    cap->dependencies = NULL;
    cap->dependency_count = 0;
    cap->disable_supported = false;
    cap->disable_method = NULL;
    cap->disable_warning = NULL;
    
    free(cap);
    logger_debug(MODULE, "Capability object freed");
}

psp_error_t capability_set_id(capability_t *cap, const char *id)
{
    if (!cap || !id) {
        return ERR_INVALID_PARAM;
    }
    
    free(cap->id);
    cap->id = string_utils_strdup(id);
    
    if (!cap->id) {
        logger_error(MODULE, "Failed to set capability ID: %s", id);
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability ID: %s", id);
    return ERR_OK;
}

psp_error_t capability_set_name(capability_t *cap, const char *name)
{
    if (!cap || !name) {
        return ERR_INVALID_PARAM;
    }
    
    free(cap->name);
    cap->name = string_utils_strdup(name);
    
    if (!cap->name) {
        logger_error(MODULE, "Failed to set capability name: %s", name);
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability name: %s", name);
    return ERR_OK;
}

psp_error_t capability_set_description(capability_t *cap, const char *desc)
{
    if (!cap || !desc) {
        return ERR_INVALID_PARAM;
    }
    
    free(cap->description);
    cap->description = string_utils_strdup(desc);
    
    if (!cap->description) {
        logger_error(MODULE, "Failed to set capability description: %s", desc);
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability description: %s", desc);
    return ERR_OK;
}

psp_error_t capability_add_dependency(capability_t *cap, const char *dep_id)
{
    if (!cap || !dep_id) {
        return ERR_INVALID_PARAM;
    }
    
    size_t new_count = cap->dependency_count + 1;
    char **new_deps = (char**)realloc(cap->dependencies,
                                      new_count * sizeof(char*));
    if (!new_deps) {
        logger_error(MODULE, "Failed to reallocate memory for dependencies");
        return ERR_MEMORY;
    }
    
    new_deps[new_count - 1] = string_utils_strdup(dep_id);
    if (!new_deps[new_count - 1]) {
        logger_error(MODULE, "Failed to duplicate dependency: %s", dep_id);
        return ERR_MEMORY;
    }
    
    cap->dependencies = new_deps;
    cap->dependency_count = new_count;
    
    logger_debug(MODULE, "Added dependency: %s", dep_id);
    return ERR_OK;
}

capability_list_t* capability_list_create(void)
{
    capability_list_t *list = (capability_list_t*)calloc(1, sizeof(capability_list_t));
    if (!list) {
        logger_error(MODULE, "Failed to allocate memory for capability list");
        return NULL;
    }
    
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;

    list->tool_deps = NULL;
    list->tool_dep_count = 0;
    list->tool_dep_capacity = 0;

    // logger_debug(MODULE, "Capability list created");
    return list;
}

void capability_list_free(capability_list_t *list)
{
    if (!list) return;

    if (list->items) {
        for (size_t i = 0; i < list->count; i++) {
            if (list->items[i]) {
                capability_free(list->items[i]);
            }
        }
        free(list->items);
    }

    /* 释放外部工具依赖声明 */
    if (list->tool_deps) {
        for (size_t i = 0; i < list->tool_dep_count; i++) {
            if (list->tool_deps[i]) {
                external_tool_dep_free(list->tool_deps[i]);
            }
        }
        free(list->tool_deps);
    }

    list->items = NULL;
    list->count = 0;
    list->capacity = 0;

    list->tool_deps = NULL;
    list->tool_dep_count = 0;
    list->tool_dep_capacity = 0;

    free(list);
    logger_debug(MODULE, "Capability list freed");
}

psp_error_t capability_list_add(capability_list_t *list, capability_t *cap)
{
    if (!list || !cap) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查容量 */
    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 16 : list->capacity * 2;
        capability_t **new_items = (capability_t**)realloc(list->items,
                                                           new_capacity * sizeof(capability_t*));
        if (!new_items) {
            logger_error(MODULE, "Failed to reallocate memory for capability list");
            return ERR_MEMORY;
        }
        
        list->items = new_items;
        list->capacity = new_capacity;
    }
    
    list->items[list->count++] = cap;
    // logger_debug(MODULE, "Added capability to list (count=%zu)", list->count);
    return ERR_OK;
}

capability_t* capability_list_find_by_id(capability_list_t *list, const char *id)
{
    if (!list || !id) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        capability_t *cap = list->items[i];
        if (cap && cap->id && strcmp(cap->id, id) == 0) {
            return cap;
        }
    }
    
    return NULL;
}

psp_error_t capability_set_disable_info(capability_t *cap, bool supported, 
                                    const char *method, const char *warning)
{
    if (!cap) {
        return ERR_INVALID_PARAM;
    }
    
    cap->disable_supported = supported;
    
    /* 设置禁用方法 */
    free(cap->disable_method);
    if (method) {
        cap->disable_method = string_utils_strdup(method);
        if (!cap->disable_method) {
            logger_error(MODULE, "Failed to set disable method");
            return ERR_MEMORY;
        }
    } else {
        cap->disable_method = NULL;
    }
    
    /* 设置警告信息 */
    free(cap->disable_warning);
    if (warning) {
        cap->disable_warning = string_utils_strdup(warning);
        if (!cap->disable_warning) {
            logger_error(MODULE, "Failed to set disable warning");
            return ERR_MEMORY;
        }
    } else {
        cap->disable_warning = NULL;
    }
    
    logger_debug(MODULE, "Set disable info: supported=%d, method=%s",
                 supported, method ? method : "N/A");
    return ERR_OK;
}

/* ==================== 外部工具依赖管理 ==================== */

external_tool_dep_t* external_tool_dep_create(void)
{
    external_tool_dep_t *dep = (external_tool_dep_t*)calloc(1, sizeof(external_tool_dep_t));
    if (!dep) {
        logger_error(MODULE, "Failed to allocate memory for external tool dependency");
        return NULL;
    }

    dep->name = NULL;
    dep->description = NULL;
    dep->check_command = NULL;
    dep->install_guide = NULL;
    dep->verify_command = NULL;

    return dep;
}

void external_tool_dep_free(external_tool_dep_t *dep)
{
    if (!dep) return;

    free(dep->name);
    free(dep->description);
    free(dep->check_command);
    free(dep->install_guide);
    free(dep->verify_command);

    free(dep);
}

psp_error_t capability_list_add_tool_dep(capability_list_t *list, external_tool_dep_t *dep)
{
    if (!list || !dep) {
        return ERR_INVALID_PARAM;
    }

    /* 检查容量 */
    if (list->tool_dep_count >= list->tool_dep_capacity) {
        size_t new_capacity = (list->tool_dep_capacity == 0) ? 4 : list->tool_dep_capacity * 2;
        external_tool_dep_t **new_deps = (external_tool_dep_t**)realloc(
            list->tool_deps, new_capacity * sizeof(external_tool_dep_t*));
        if (!new_deps) {
            logger_error(MODULE, "Failed to reallocate memory for tool dependency list");
            return ERR_MEMORY;
        }

        list->tool_deps = new_deps;
        list->tool_dep_capacity = new_capacity;
    }

    list->tool_deps[list->tool_dep_count++] = dep;
    logger_debug(MODULE, "Added external tool dependency: %s",
                 dep->name ? dep->name : "unknown");
    return ERR_OK;
}
