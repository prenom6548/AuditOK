#include "core/capability_parser.h"
#include "logger.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
// #include "models/environment.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MODULE "capability_parser"

/**
 * @brief 从单个条件对象创建 condition_t
 */
static condition_t* parse_single_condition(const char *category, cJSON *condition_obj)
{
    if (!category || !condition_obj || !cJSON_IsObject(condition_obj)) {
        return NULL;
    }
    
    cJSON *name_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "name");
    cJSON *value_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "value");
    cJSON *operator_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "operator");
    
    if (!cJSON_IsString(name_item) || !operator_item || !cJSON_IsString(operator_item)) {
        logger_warn(MODULE, "Invalid condition object: missing name or operator");
        return NULL;
    }
    
    const char *field_name = name_item->valuestring;
    const char *op = operator_item->valuestring;
    
    /* 使用字段名（不再添加category前缀） */
    const char *field_key = field_name;
    
    /* value 必须为数组格式 */
    if (!cJSON_IsArray(value_item)) {
        logger_warn(MODULE, "Value must be an array format for condition: %s", field_name);
        return NULL;
    }
    
    size_t count = cJSON_GetArraySize(value_item);
    if (count == 0) {
        logger_warn(MODULE, "Empty values array in condition: %s", field_name);
        return NULL;
    }
    
    /* 提取数组中的所有字符串值 */
    const char **values_array = (const char**)calloc(count, sizeof(char*));
    if (!values_array) {
        logger_error(MODULE, "Failed to allocate memory for values array");
        return NULL;
    }
    
    cJSON *val_item = NULL;
    size_t idx = 0;
    cJSON_ArrayForEach(val_item, value_item) {
        if (cJSON_IsString(val_item)) {
            values_array[idx++] = val_item->valuestring;
        }
    }
    
    if (idx == 0) {
        logger_warn(MODULE, "No valid string values in array for condition: %s", field_name);
        free(values_array);
        return NULL;
    }
    
    /* 创建条件对象并设置多值 */
    condition_t *cond = condition_create();
    if (!cond) {
        free(values_array);
        return NULL;
    }
    
    psp_error_t ret = condition_set_multi(cond, field_key, values_array, idx, op);
    free(values_array);  /* 只释放指针数组，字符串已在 condition_set_multi 中复制 */
    
    if (ret != ERR_OK) {
        condition_free(cond);
        return NULL;
    }
    
    /* 解析 status 字段（可选） */
    cJSON *status_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "status");
    if (cJSON_IsString(status_item)) {
        cond->status = strdup(status_item->valuestring);
        // logger_debug(MODULE, "Condition %s has status: %s", field_key, cond->status);
    } else {
        /* 默认状态为 support */
        cond->status = strdup("support");
    }
    
    /* 解析 type 字段（可选），用于标识条件类型 */
    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "type");
    if (cJSON_IsString(type_item)) {
        const char *type_str = type_item->valuestring;
        if (strcmp(type_str, "kernel_config") == 0) {
            cond->type = COND_TYPE_KERNEL_CONFIG;
        } else if (strcmp(type_str, "firmware") == 0) {
            cond->type = COND_TYPE_FIRMWARE;
        } else if (strcmp(type_str, "software") == 0) {
            cond->type = COND_TYPE_SOFTWARE;
        } else if (strcmp(type_str, "cpu_model") == 0) {
            cond->type = COND_TYPE_CPU_MODEL;
        } else if (strcmp(type_str, "generic") == 0) {
            cond->type = COND_TYPE_GENERIC;
        } else if (strcmp(type_str, "driver") == 0) {
            cond->type = COND_TYPE_DRIVER;
        } else {
            cond->type = COND_TYPE_UNKNOWN;
            logger_warn(MODULE, "Unknown condition type: %s for field %s", type_str, field_key);
        }
        // logger_debug(MODULE, "Condition %s has type: %s", field_key, type_str);
    } else {
        /* 如果未指定type，统一为COND_TYPE_UNKNOWN */
        cond->type = COND_TYPE_UNKNOWN;
    }
    
    /* 解析 description 字段（可选），用于生成使能提示 */
    cJSON *desc_item = cJSON_GetObjectItemCaseSensitive(condition_obj, "description");
    if (cJSON_IsString(desc_item)) {
        cond->description = strdup(desc_item->valuestring);
        // logger_debug(MODULE, "Condition %s has description: %s", field_key, cond->description);
    } else {
        cond->description = NULL;
    }
    
    return cond;
}

/**
 * @brief 从条件组对象创建 condition_group_t
 */
static condition_group_t* parse_condition_group(const char *category, cJSON *group_obj, bool is_and_group)
{
    if (!category || !group_obj || !cJSON_IsObject(group_obj)) {
        return NULL;
    }
    
    cJSON *conditions_array = cJSON_GetObjectItemCaseSensitive(group_obj, "conditions");
    if (!cJSON_IsArray(conditions_array) || cJSON_GetArraySize(conditions_array) == 0) {
        logger_warn(MODULE, "Empty or invalid conditions array in group");
        return NULL;
    }
    
    /* 创建条件组 */
    condition_group_t *group = condition_group_create(is_and_group);
    if (!group) {
        return NULL;
    }
    
    /* 解析每个条件 */
    cJSON *cond_item = NULL;
    cJSON_ArrayForEach(cond_item, conditions_array) {
        if (!cJSON_IsObject(cond_item)) {
            continue;
        }
        
        condition_t *cond = parse_single_condition(category, cond_item);
        if (cond) {
            psp_error_t ret = condition_group_add(group, cond);
            if (ret != ERR_OK) {
                condition_free(cond);
            }
        }
    }
    
    if (group->count == 0) {
        condition_group_free(group);
        return NULL;
    }
    
    return group;
}

/**
 * @brief 从条件组中提取依赖项并添加到能力对象
 */
static void extract_dependencies_from_group(const char *category, cJSON *group_obj, capability_t *cap)
{
    if (!category || !group_obj || !cJSON_IsObject(group_obj) || !cap) {
        return;
    }
    
    cJSON *conditions_array = cJSON_GetObjectItemCaseSensitive(group_obj, "conditions");
    if (!cJSON_IsArray(conditions_array)) {
        return;
    }
    
    cJSON *cond_item = NULL;
    cJSON_ArrayForEach(cond_item, conditions_array) {
        if (!cJSON_IsObject(cond_item)) {
            continue;
        }
        
        cJSON *name_item = cJSON_GetObjectItemCaseSensitive(cond_item, "name");
        if (cJSON_IsString(name_item)) {
            capability_add_dependency(cap, name_item->valuestring);
            logger_debug(MODULE, "Added dependency from condition group: %s", name_item->valuestring);
        }
    }
}

/**
 * @brief 从配置对象解析层规则（结构化版本）
 * 
 * 直接创建 condition_group_t 数组，而不是生成表达式字符串
 */
static psp_error_t parse_layer_rule_structured(const char *category, 
                                           cJSON *config_obj, 
                                           layer_rule_t *rule,
                                           capability_t *cap)
{
    if (!category || !config_obj || !cJSON_IsObject(config_obj) || !rule || !cap) {
        return ERR_INVALID_PARAM;
    }
    
    cJSON *alternatives = cJSON_GetObjectItemCaseSensitive(config_obj, "alternatives");
    cJSON *requirements = cJSON_GetObjectItemCaseSensitive(config_obj, "requirements");
    
    bool has_alternatives = (cJSON_IsArray(alternatives) && cJSON_GetArraySize(alternatives) > 0);
    bool has_requirements = (cJSON_IsArray(requirements) && cJSON_GetArraySize(requirements) > 0);
    
    if (!has_alternatives && !has_requirements) {
        logger_error(MODULE, "%s configuration must contain at least one of 'alternatives' or 'requirements'", category);
        return ERR_INVALID_PARAM;
    }
    
    /* 设置 required 标志 */
    cJSON *required_item = cJSON_GetObjectItemCaseSensitive(config_obj, "required");
    rule->required = (cJSON_IsBool(required_item) && cJSON_IsTrue(required_item));
    
    /* ========== 解析 alternatives（OR 逻辑，每个元素是一个 AND 组） ========== */
    if (has_alternatives) {
        logger_debug(MODULE, "Parsing %s alternatives with %d options", 
                    category, cJSON_GetArraySize(alternatives));
        
        cJSON *alt_item = NULL;
        cJSON_ArrayForEach(alt_item, alternatives) {
            if (!cJSON_IsObject(alt_item)) {
                continue;
            }
            
            condition_group_t *group = NULL;
            
            /* 检查是否是条件组格式 */
            cJSON *conditions = cJSON_GetObjectItemCaseSensitive(alt_item, "conditions");
            if (cJSON_IsArray(conditions)) {
                /* 多条件组合（AND 逻辑） */
                group = parse_condition_group(category, alt_item, true);
                
                /* 从条件组中提取依赖项 */
                extract_dependencies_from_group(category, alt_item, cap);
            } else {
                /* 单一条件，创建一个只包含一个条件的 AND 组 */
                condition_t *cond = parse_single_condition(category, alt_item);
                if (cond) {
                    group = condition_group_create(true);
                    if (group) {
                        condition_group_add(group, cond);
                    }
                }
                
                /* 添加依赖 */
                cJSON *name_item = cJSON_GetObjectItemCaseSensitive(alt_item, "name");
                if (cJSON_IsString(name_item)) {
                    capability_add_dependency(cap, name_item->valuestring);
                }
            }
            
            if (group) {
                /* alternatives 中的组应该是 OR 关系 */
                layer_rule_add_or_group(rule, group);
            }
        }
    }
    
    /* ========== 解析 requirements（AND 逻辑，所有条件在一个组中） ========== */
    if (has_requirements) {
        logger_debug(MODULE, "Parsing %s requirements with %d conditions", 
                    category, cJSON_GetArraySize(requirements));
        
        /* 创建一个 AND 组包含所有 requirements */
        condition_group_t *req_group = condition_group_create(true);
        if (!req_group) {
            return ERR_MEMORY;
        }
        
        cJSON *req_item = NULL;
        cJSON_ArrayForEach(req_item, requirements) {
            if (!cJSON_IsObject(req_item)) {
                continue;
            }
            
            condition_t *cond = parse_single_condition(category, req_item);
            if (cond) {
                /* 添加依赖 */
                cJSON *name_item = cJSON_GetObjectItemCaseSensitive(req_item, "name");
                if (cJSON_IsString(name_item)) {
                    capability_add_dependency(cap, name_item->valuestring);
                }
                
                condition_group_add(req_group, cond);
            }
        }
        
        if (req_group->count > 0) {
            /* requirements 中的组应该是 AND 关系 */
            layer_rule_add_and_group(rule, req_group);
        } else {
            condition_group_free(req_group);
        }
    }
    
    // logger_info(MODULE, "Parsed %s rule: %zu OR groups + %zu AND groups, required=%d", 
    //            category, rule->or_group_count, rule->and_group_count, rule->required);
    
    return ERR_OK;
}

/**
 * @brief 从单个能力矩阵项解析能力
 * 
 * JSON结构（新格式）：
 * {
 *   "cap_name": "csv1",
 *   "dependencies": {
 *     "cpu": {
 *       "alternatives": [...],
 *       "requirements": [...]
 *     },
 *     "firmware": {
 *       "alternatives": [...],
 *       "requirements": [...]
 *     },
 *     "os": {
 *       "alternatives": [...],
 *       "requirements": [...]
 *     }
 *   }
 * }
 */
static capability_t* parse_capability_from_matrix_item(cJSON *cap_item,
                                                        const char *cpu_model)
{
    if (!cap_item || !cJSON_IsObject(cap_item) || !cpu_model) {
        return NULL;
    }
    
    /* 获取 cap_name */
    cJSON *cap_name_item = cJSON_GetObjectItemCaseSensitive(cap_item, "cap_name");
    if (!cJSON_IsString(cap_name_item)) {
        logger_warn(MODULE, "Missing or invalid cap_name in capability item");
        return NULL;
    }
    
    const char *cap_name = cap_name_item->valuestring;
    logger_debug(MODULE, "Parsing capability: %s for CPU model: %s", cap_name, cpu_model);
    
    /* 创建能力对象 */
    capability_t *cap = capability_create();
    if (!cap) {
        return NULL;
    }
    
    /* 设置能力名称为 cap_name（如 csv1） */
    capability_set_name(cap, cap_name);
    
    /* 设置能力 ID 为 cap_name（直接使用，不再拼接 cpu_model） */
    capability_set_id(cap, cap_name);
    
    /* 生成描述 */
    char cap_desc[512];
    snprintf(cap_desc, sizeof(cap_desc), "%s security capability for %s platform", 
             cap_name, cpu_model);
    capability_set_description(cap, cap_desc);
    
    /* 获取 dependencies 对象 */
    cJSON *dependencies = cJSON_GetObjectItemCaseSensitive(cap_item, "dependencies");
    if (!cJSON_IsObject(dependencies)) {
        logger_warn(MODULE, "Missing dependencies object for capability: %s", cap_name);
        capability_free(cap);
        return NULL;
    }
    
    /* 解析 CPU 规则（结构化版本） */
    cJSON *cpu_dep = cJSON_GetObjectItemCaseSensitive(dependencies, "cpu");
    if (cJSON_IsObject(cpu_dep)) {
        logger_debug(MODULE, "Found CPU dependencies for capability: %s", cap_name);
        psp_error_t ret = parse_layer_rule_structured("cpu", cpu_dep, &cap->cpu_rule, cap);
        if (ret != ERR_OK) {
            logger_warn(MODULE, "Failed to parse CPU rule for capability: %s", cap_name);
        }
    } else {
        logger_warn(MODULE, "Missing CPU dependencies for capability: %s", cap_name);
    }
    
    /* 解析固件规则（结构化版本） */
    cJSON *fw_dep = cJSON_GetObjectItemCaseSensitive(dependencies, "firmware");
    if (cJSON_IsObject(fw_dep)) {
        logger_debug(MODULE, "Found firmware dependencies for capability: %s", cap_name);
        psp_error_t ret = parse_layer_rule_structured("firmware", fw_dep, &cap->firmware_rule, cap);
        if (ret != ERR_OK) {
            logger_warn(MODULE, "Failed to parse firmware rule for capability: %s", cap_name);
        }
    } else {
        logger_warn(MODULE, "Missing firmware dependencies for capability: %s", cap_name);
    }
    
    /* 解析 OS 规则（结构化版本） */
    cJSON *os_dep = cJSON_GetObjectItemCaseSensitive(dependencies, "os");
    if (cJSON_IsObject(os_dep)) {
        logger_debug(MODULE, "Found OS dependencies for capability: %s", cap_name);
        psp_error_t ret = parse_layer_rule_structured("os", os_dep, &cap->os_rule, cap);
        if (ret != ERR_OK) {
            logger_warn(MODULE, "Failed to parse OS rule for capability: %s", cap_name);
        }
    } else {
        logger_warn(MODULE, "Missing OS dependencies for capability: %s", cap_name);
    }
    
    /* 解析 disable_info（可选） */
    cJSON *disable_info = cJSON_GetObjectItemCaseSensitive(cap_item, "disable_info");
    if (cJSON_IsObject(disable_info)) {
        cJSON *supported_item = cJSON_GetObjectItemCaseSensitive(disable_info, "supported");
        cJSON *method_item = cJSON_GetObjectItemCaseSensitive(disable_info, "method");
        cJSON *warning_item = cJSON_GetObjectItemCaseSensitive(disable_info, "warning");
        
        bool supported = false;
        const char *method = NULL;
        const char *warning = NULL;
        
        if (cJSON_IsBool(supported_item)) {
            supported = cJSON_IsTrue(supported_item);
        }
        
        if (cJSON_IsString(method_item)) {
            method = method_item->valuestring;
        }
        
        if (cJSON_IsString(warning_item)) {
            warning = warning_item->valuestring;
        }
        
        psp_error_t ret = capability_set_disable_info(cap, supported, method, warning);
        if (ret != ERR_OK) {
            logger_warn(MODULE, "Failed to set disable info for capability: %s", cap_name);
        } else {
            logger_info(MODULE, "Disable info parsed for %s: supported=%d", 
                       cap_name, supported);
        }
    }
    
    logger_debug(MODULE, "Parsed capability: id=%s, name=%s, dependencies=%zu",
                 cap->id, cap->name, cap->dependency_count);

    return cap;
}

/**
 * @brief 解析单个外部工具依赖声明
 *
 * JSON结构：
 * {
 *   "name": "hag",                (必填)
 *   "description": "...",         (可选)
 *   "check_command": "hag",       (可选，缺省同 name)
 *   "install_guide": "...",       (可选)
 *   "verify_command": "..."       (可选)
 * }
 */
static external_tool_dep_t* parse_external_tool_item(cJSON *tool_obj)
{
    if (!tool_obj || !cJSON_IsObject(tool_obj)) {
        return NULL;
    }

    /* name 为必填字段 */
    cJSON *name_item = cJSON_GetObjectItemCaseSensitive(tool_obj, "name");
    if (!cJSON_IsString(name_item) || strlen(name_item->valuestring) == 0) {
        logger_warn(MODULE, "Missing or invalid 'name' in external tool declaration");
        return NULL;
    }

    external_tool_dep_t *dep = external_tool_dep_create();
    if (!dep) {
        return NULL;
    }

    dep->name = strdup(name_item->valuestring);
    if (!dep->name) {
        external_tool_dep_free(dep);
        return NULL;
    }

    /* description 字段（可选） */
    cJSON *desc_item = cJSON_GetObjectItemCaseSensitive(tool_obj, "description");
    if (cJSON_IsString(desc_item)) {
        dep->description = strdup(desc_item->valuestring);
    }

    /* check_command 字段（可选，缺省回退为 name） */
    cJSON *check_item = cJSON_GetObjectItemCaseSensitive(tool_obj, "check_command");
    if (cJSON_IsString(check_item) && strlen(check_item->valuestring) > 0) {
        dep->check_command = strdup(check_item->valuestring);
    } else {
        dep->check_command = strdup(dep->name);
    }

    /* install_guide 字段（可选） */
    cJSON *install_item = cJSON_GetObjectItemCaseSensitive(tool_obj, "install_guide");
    if (cJSON_IsString(install_item)) {
        dep->install_guide = strdup(install_item->valuestring);
    }

    /* verify_command 字段（可选） */
    cJSON *verify_item = cJSON_GetObjectItemCaseSensitive(tool_obj, "verify_command");
    if (cJSON_IsString(verify_item)) {
        dep->verify_command = strdup(verify_item->valuestring);
    }

    if (!dep->check_command) {
        logger_error(MODULE, "Failed to set check_command for external tool: %s", dep->name);
        external_tool_dep_free(dep);
        return NULL;
    }

    return dep;
}

/**
 * @brief 从根级 external_tools 数组解析外部工具依赖声明
 *
 * @param json_root JSON根对象
 * @param list 能力列表（解析结果存入 list->tool_deps）
 */
static void parse_external_tools(cJSON *json_root, capability_list_t *list)
{
    if (!json_root || !list) {
        return;
    }

    cJSON *tools_array = cJSON_GetObjectItemCaseSensitive(json_root, "external_tools");
    if (!tools_array) {
        logger_debug(MODULE, "No external_tools section in capability library");
        return;
    }

    if (!cJSON_IsArray(tools_array)) {
        logger_warn(MODULE, "Invalid external_tools section: expected array");
        return;
    }

    cJSON *tool_item = NULL;
    cJSON_ArrayForEach(tool_item, tools_array) {
        external_tool_dep_t *dep = parse_external_tool_item(tool_item);
        if (dep) {
            psp_error_t ret = capability_list_add_tool_dep(list, dep);
            if (ret != ERR_OK) {
                logger_warn(MODULE, "Failed to add external tool dependency: %s",
                           dep->name ? dep->name : "unknown");
                external_tool_dep_free(dep);
            }
        }
    }

    logger_info(MODULE, "Parsed %zu external tool dependencies", list->tool_dep_count);
}

/**
 * @brief 从顶层JSON对象解析所有能力
 *
 * JSON结构（插件层已按CPU型号过滤）：
 * {
 *   "CPU_Model": {
 *     "cap_matrix": [
 *       {"cap_name": "xxx", "dependencies": {...}},
 *       ...
 *     ]
 *   }
 * }
 * 
 * 注意：插件层可能已经过滤掉不匹配的CPU型号，所以这里可能只包含一个型号
 */
static capability_list_t* parse_capabilities(cJSON *json_root)
{
    if (!json_root || !cJSON_IsObject(json_root)) {
        logger_error(MODULE, "Invalid JSON root object");
        return NULL;
    }
    
    /* 创建能力列表 */
    capability_list_t *list = capability_list_create();
    if (!list) {
        return NULL;
    }
    
    int total_count = 0;
    int parsed_count = 0;
    int failed_count = 0;
    
    /* 遍历每个CPU型号（插件层可能已过滤，只剩一个） */
    cJSON *cpu_model_obj = NULL;
    cJSON_ArrayForEach(cpu_model_obj, json_root) {
        if (!cJSON_IsObject(cpu_model_obj)) {
            continue;
        }
        
        const char *cpu_model = cpu_model_obj->string;
        if (!cpu_model) {
            logger_warn(MODULE, "CPU model object missing key");
            continue;
        }

        /* 跳过能力库级元数据段（非CPU型号），由 parse_external_tools 单独解析 */
        if (strcmp(cpu_model, "external_tools") == 0) {
            continue;
        }

        logger_info(MODULE, "Processing CPU model: %s", cpu_model);
        
        /* 获取 cap_matrix 数组 */
        cJSON *cap_matrix = cJSON_GetObjectItemCaseSensitive(cpu_model_obj, "cap_matrix");
        if (!cJSON_IsArray(cap_matrix)) {
            logger_warn(MODULE, "Missing or invalid cap_matrix array for CPU model: %s", cpu_model);
            continue;
        }
        
        /* 遍历能力矩阵数组 */
        cJSON *cap_item = NULL;
        cJSON_ArrayForEach(cap_item, cap_matrix) {
            if (!cJSON_IsObject(cap_item)) {
                continue;
            }
            
            total_count++;
            
            /* 解析单个能力项 */
            capability_t *cap = parse_capability_from_matrix_item(cap_item, cpu_model);
            if (cap) {
                psp_error_t ret = capability_list_add(list, cap);
                if (ret == ERR_OK) {
                    parsed_count++;
                    logger_debug(MODULE, "Added capability: %s (deps: %zu)", 
                               cap->id, cap->dependency_count);
                } else {
                    capability_free(cap);
                    failed_count++;
                    logger_warn(MODULE, "Failed to add capability to list");
                }
            } else {
                failed_count++;
                logger_warn(MODULE, "Failed to parse capability item");
            }
        }
    }
    
    logger_info(MODULE, "Parsed capabilities from nested structure: %d total, %d succeeded, %d failed",
               total_count, parsed_count, failed_count);
    
    return list;
}

capability_list_t* capability_parser(const char *json_data)
{
    if (!json_data) {
        logger_error(MODULE, "NULL JSON data provided");
        return NULL;
    }
    
    /* 解析 JSON */
    cJSON *json_root = cJSON_Parse(json_data);
    if (!json_root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        logger_error(MODULE, "Failed to parse JSON: %s", error_ptr ? error_ptr : "unknown error");
        return NULL;
    }
    
    /* 检查是否为对象（嵌套结构） */
    if (!cJSON_IsObject(json_root)) {
        logger_error(MODULE, "JSON root must be an object with CPU models as keys");
        cJSON_Delete(json_root);
        return NULL;
    }
    
    /* 使用嵌套结构解析器 */
    /* 注意：插件层可能已经根据CPU型号过滤了JSON，所以这里直接解析即可 */
    capability_list_t *list = parse_capabilities(json_root);

    /* 解析能力库级元数据：外部工具依赖声明 */
    if (list) {
        parse_external_tools(json_root, list);
    }

    cJSON_Delete(json_root);

    return list;
}
