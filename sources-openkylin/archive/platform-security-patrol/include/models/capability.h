#ifndef MODELS_CAPABILITY_H
#define MODELS_CAPABILITY_H

#include "plat_sec_patrol.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 条件类型枚举
 */
typedef enum {
    COND_TYPE_UNKNOWN = 0,        /* 未知类型 */
    COND_TYPE_KERNEL_CONFIG = 1,  /* 内核配置项（CONFIG_开头） */
    COND_TYPE_FIRMWARE = 2,       /* 固件相关（版本、厂商等） */
    COND_TYPE_SOFTWARE = 3,       /* 软件包/工具 */
    COND_TYPE_CPU_MODEL = 4,      /* CPU型号 */
    COND_TYPE_DRIVER = 5,
    COND_TYPE_GENERIC = 6         /* 通用类型 */
} condition_type_t;

/**
 * @brief 单个检查条件
 */
typedef struct {
    char *field_name;           /* 字段名，如 "CONFIG_AMD_MEM_ENCRYPT" */
    char **expected_values;     /* 期望值数组（统一使用数组格式） */
    size_t expected_values_count; /* 期望值数组长度 */
    char *operator;             /* 操作符，如 "==", ">=", "<=", "in" */
    char *status;               /* 匹配时的状态，如 "support", "disable", "unsupported" */
    condition_type_t type;      /* 条件类型，用于生成友好的提示信息 */
    char *description;          /* 可选的描述信息，用于生成使能提示（如 "Please import certs"） */
} condition_t;

/**
 * @brief 条件组（AND 或 OR）
 */
typedef struct {
    condition_t **conditions;   /* 条件数组 */
    size_t count;               /* 条件数量 */
    size_t capacity;            /* 数组容量 */
    bool is_and_group;          /* true=AND组, false=OR组 */
} condition_group_t;

/**
 * @brief 层次检查规则
 */
typedef struct {
    condition_group_t **or_groups;      /* OR 条件组数组（来自 alternatives，满足其一即可） */
    size_t or_group_count;              /* OR 组数量 */
    size_t or_group_capacity;           /* OR 组容量 */
    
    condition_group_t **and_groups;     /* AND 条件组数组（来自 requirements，必须全部满足） */
    size_t and_group_count;             /* AND 组数量 */
    size_t and_group_capacity;          /* AND 组容量 */
    
    bool required;                      /* 是否必须满足 */
} layer_rule_t;

/**
 * @brief 能力定义结构
 */
typedef struct {
    char *id;                   /* 能力唯一标识 */
    char *name;                 /* 能力名称 */
    char *description;          /* 能力描述 */
    layer_rule_t cpu_rule;      /* CPU 层检查规则 */
    layer_rule_t firmware_rule; /* 固件层检查规则 */
    layer_rule_t os_rule;       /* OS 层检查规则 */
    char **dependencies;        /* 依赖的能力 ID 列表（NULL 结尾） */
    size_t dependency_count;    /* 依赖数量 */
    
    /* 禁用相关信息 */
    bool disable_supported;     /* 是否支持禁用 */
    char *disable_method;       /* 禁用方法描述或命令 */
    char *disable_warning;      /* 禁用警告信息 */
} capability_t;

/**
 * @brief 外部工具依赖声明（能力库级元数据）
 *
 * 由能力库 JSON 根级 external_tools 段解析而来，声明能力库判定所依赖的
 * 外部工具及其安装指导。核心层据此检查工具是否可用，并在缺失时向用户告警。
 */
typedef struct {
    char *name;             /* 工具标识，如 "hag" */
    char *description;      /* 用途说明（仅日志使用，可为 NULL） */
    char *check_command;    /* 存在性检查命令，缺省同 name */
    char *install_guide;    /* 安装指导 */
    char *verify_command;   /* 安装后的验证命令（可为 NULL） */
} external_tool_dep_t;

/**
 * @brief 能力列表结构
 */
typedef struct {
    capability_t **items;       /* 能力项数组 */
    size_t count;               /* 能力数量 */
    size_t capacity;            /* 数组容量 */

    /* 能力库级元数据：外部工具依赖声明 */
    external_tool_dep_t **tool_deps;  /* 外部工具依赖数组 */
    size_t tool_dep_count;            /* 外部工具依赖数量 */
    size_t tool_dep_capacity;         /* 外部工具依赖数组容量 */
} capability_list_t;

/* ==================== 条件管理函数 ==================== */

/**
 * @brief 创建单个条件对象
 * @return 成功返回 condition_t 指针，失败返回 NULL
 */
condition_t* condition_create(void);

/**
 * @brief 释放条件对象
 * @param cond 要释放的条件对象
 */
void condition_free(condition_t *cond);

/**
 * @brief 设置条件的期望值（统一使用数组格式）
 * @param cond 条件对象
 * @param field_name 字段名
 * @param expected_values 期望值数组
 * @param count 数组长度
 * @param operator 操作符（如 "==", "in", ">=" 等）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t condition_set(condition_t *cond, const char *field_name, 
                     const char **expected_values, size_t count,
                     const char *operator);

/**
 * @brief condition_set 的别名，保持 API 兼容性
 * @param cond 条件对象
 * @param field_name 字段名
 * @param expected_values 期望值数组
 * @param count 数组长度
 * @param operator 操作符
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t condition_set_multi(condition_t *cond, const char *field_name,
                           const char **expected_values, size_t count,
                           const char *operator);

/* ==================== 条件组管理函数 ==================== */

/**
 * @brief 创建条件组
 * @return 成功返回 condition_group_t 指针，失败返回 NULL
 */
condition_group_t* condition_group_create(bool is_and_group);

/**
 * @brief 释放条件组
 * @param group 要释放的条件组
 */
void condition_group_free(condition_group_t *group);

/**
 * @brief 向条件组添加条件
 * @param group 条件组
 * @param cond 条件对象（所有权转移给条件组）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t condition_group_add(condition_group_t *group, condition_t *cond);

/* ==================== 层规则管理函数 ==================== */

/**
 * @brief 初始化层规则
 * @param rule 要初始化的层规则
 */
void layer_rule_init(layer_rule_t *rule);

/**
 * @brief 释放层规则
 * @param rule 要释放的层规则
 */
void layer_rule_free(layer_rule_t *rule);

/**
 * @brief 向层规则的 OR 组数组添加条件组（来自 alternatives）
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_or_group(layer_rule_t *rule, condition_group_t *group);

/**
 * @brief 向层规则的 AND 组数组添加条件组（来自 requirements）
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_and_group(layer_rule_t *rule, condition_group_t *group);

/**
 * @brief 向层规则添加条件组（兼容旧接口，默认添加到 AND 组）
 * @deprecated 请使用 layer_rule_add_or_group 或 layer_rule_add_and_group
 * @param rule 层规则
 * @param group 条件组（所有权转移给层规则）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t layer_rule_add_group(layer_rule_t *rule, condition_group_t *group);

/* ==================== 能力列表管理函数 ==================== */

/**
 * @brief 创建能力列表
 * @return 成功返回 capability_list_t 指针，失败返回 NULL
 */
capability_list_t* capability_list_create(void);

/**
 * @brief 释放能力列表
 * @param list 要释放的能力列表
 */
void capability_list_free(capability_list_t *list);

/**
 * @brief 向能力列表添加能力项
 * @param list 能力列表
 * @param cap 能力项
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_list_add(capability_list_t *list, capability_t *cap);

/**
 * @brief 创建能力项
 * @return 成功返回 capability_t 指针，失败返回 NULL
 */
capability_t* capability_create(void);

/**
 * @brief 释放能力项
 * @param cap 要释放的能力项
 */
void capability_free(capability_t *cap);

/**
 * @brief 设置能力 ID
 * @param cap 能力项
 * @param id 能力 ID 字符串
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_set_id(capability_t *cap, const char *id);

/**
 * @brief 设置能力名称
 * @param cap 能力项
 * @param name 能力名称字符串
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_set_name(capability_t *cap, const char *name);

/**
 * @brief 设置能力描述
 * @param cap 能力项
 * @param desc 能力描述字符串
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_set_description(capability_t *cap, const char *desc);

/**
 * @brief 添加依赖能力
 * @param cap 能力项
 * @param dep_id 依赖能力 ID
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_add_dependency(capability_t *cap, const char *dep_id);

/**
 * @brief 根据 ID 查找能力
 * @param list 能力列表
 * @param id 能力 ID
 * @return 找到返回能力项指针，否则返回 NULL
 */
capability_t* capability_list_find_by_id(capability_list_t *list, const char *id);

/**
 * @brief 设置能力的禁用信息
 * @param cap 能力项
 * @param supported 是否支持禁用
 * @param method 禁用方法描述或命令
 * @param warning 警告信息（可选）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_set_disable_info(capability_t *cap, bool supported,
                                    const char *method, const char *warning);

/* ==================== 外部工具依赖管理函数 ==================== */

/**
 * @brief 创建外部工具依赖对象
 * @return 成功返回 external_tool_dep_t 指针，失败返回 NULL
 */
external_tool_dep_t* external_tool_dep_create(void);

/**
 * @brief 释放外部工具依赖对象
 * @param dep 要释放的依赖对象
 */
void external_tool_dep_free(external_tool_dep_t *dep);

/**
 * @brief 向能力列表添加外部工具依赖
 * @param list 能力列表
 * @param dep 依赖对象（所有权转移给列表）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t capability_list_add_tool_dep(capability_list_t *list, external_tool_dep_t *dep);

#ifdef __cplusplus
}
#endif

#endif /* MODELS_CAPABILITY_H */
