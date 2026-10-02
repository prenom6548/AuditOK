/**
 * @file result.h
 * @brief 判定结果和执行结果数据模型定义
 */

#ifndef MODELS_RESULT_H
#define MODELS_RESULT_H

#include "plat_sec_patrol.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 单条能力判定结果
 */
typedef struct {
    char *capability_id;          /* 能力 ID */
    char *capability_name;        /* 能力名称 */
    cap_status_t overall_status;  /* 整体状态 */
    layer_status_t cpu_status;    /* CPU 层状态 */
    layer_status_t firmware_status; /* 固件层状态 */
    layer_status_t os_status;     /* OS 层状态 */
    char *evidence;               /* 判定证据（JSON 格式字符串） */
    char *description;            /* 能力描述或状态说明 */
} judgement_result_t;

/**
 * @brief 判定结果列表
 */
typedef struct {
    judgement_result_t **items;    /* 结果项数组 */
    size_t count;                 /* 结果数量 */
    size_t capacity;              /* 数组容量 */
} judgement_result_list_t;

/**
 * @brief 执行结果
 */
typedef struct {
    char *capability_id;           /**< 能力 ID */
    char *action;                  /* 操作类型："enable" 或 "disable" */
    bool operation_completed;      /* 操作是否成功完成 */
    char *message;                 /* 简短描述 */
    char *guidance;                /* 配置指导（如无法自动配置） */
    char *rollback_log;            /* 回滚日志 */
} execution_result_t;

/* ==================== 判定结果内存管理函数 ==================== */

/**
 * @brief 创建判定结果
 * @return 成功返回 judgement_result_t 指针，失败返回 NULL
 */
judgement_result_t* judgement_result_create(void);

/**
 * @brief 释放判定结果
 * @param result 要释放的结果项
 */
void judgement_result_free(judgement_result_t *result);

/**
 * @brief 创建判定结果列表
 * @return 成功返回 judgement_result_list_t 指针，失败返回 NULL
 */
judgement_result_list_t* judgement_result_list_create(void);

/**
 * @brief 释放判定结果列表
 * @param list 要释放的结果列表
 */
void judgement_result_list_free(judgement_result_list_t *list);

/**
 * @brief 向结果列表添加结果项
 * @param list 结果列表
 * @param result 结果项
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t judgement_result_list_add(judgement_result_list_t *list, judgement_result_t *result);

/**
 * @brief 设置判定结果的能力 ID
 * @param result 结果项
 * @param cap_id 能力 ID
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t judgement_result_set_capability_id(judgement_result_t *result, const char *cap_id);

/**
 * @brief 设置判定结果的整体状态
 * @param result 结果项
 * @param status 整体状态
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t judgement_result_set_overall_status(judgement_result_t *result, cap_status_t status);

psp_error_t judgement_result_set_capability_id(judgement_result_t *res, const char *cap_id);

/**
 * @brief 设置总体状态
 * @param res 判定结果对象
 * @param status 总体状态
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_overall_status(judgement_result_t *res, cap_status_t status);

/**
 * @brief 设置 CPU 层状态
 * @param res 判定结果对象
 * @param status CPU 层状态
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_cpu_status(judgement_result_t *res, layer_status_t status);

/**
 * @brief 设置固件层状态
 * @param res 判定结果对象
 * @param status 固件层状态
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_firmware_status(judgement_result_t *res, layer_status_t status);

/**
 * @brief 设置 OS 层状态
 * @param res 判定结果对象
 * @param status OS 层状态
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_os_status(judgement_result_t *res, layer_status_t status);

/**
 * @brief 设置证据
 * @param res 判定结果对象
 * @param evidence 证据字符串 (JSON 格式，会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_evidence(judgement_result_t *res, const char *evidence);

/**
 * @brief 设置能力名称
 * @param result 判定结果对象
 * @param name 能力名称 (会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_capability_name(judgement_result_t *result, const char *name);

/**
 * @brief 设置描述
 * @param result 判定结果对象
 * @param desc 描述文本 (会复制，可为 NULL)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t judgement_result_set_description(judgement_result_t *result, const char *desc);

/* ==================== 执行结果内存管理函数 ==================== */

/**
 * @brief 创建执行结果
 * @return 成功返回 execution_result_t 指针，失败返回 NULL
 */
execution_result_t* execution_result_create(void);

/**
 * @brief 释放执行结果
 * @param res 要释放的执行结果
 */
void execution_result_free(execution_result_t *res);

/* ==================== 执行结果字段设置函数 ==================== */

/**
 * @brief 设置执行结果的能力 ID
 * @param res 执行结果
 * @param cap_id 能力 ID
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t execution_result_set_capability_id(execution_result_t *res, const char *cap_id);

/**
 * @brief 设置执行动作
 * @param res 执行结果
 * @param action 动作类型（"enable" 或 "disable"）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t execution_result_set_action(execution_result_t *res, const char *action);

/**
 * @brief 设置成功标志
 * @param res 执行结果对象
 * @param success 是否成功
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_success(execution_result_t *res, bool success);

/**
 * @brief 设置消息
 * @param res 执行结果
 * @param msg 消息字符串
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t execution_result_set_message(execution_result_t *res, const char *msg);

/**
 * @brief 设置配置指导
 * @param res 执行结果
 * @param guidance 指导文本
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t execution_result_set_guidance(execution_result_t *res, const char *guidance);

/**
 * @brief 设置回滚日志
 * @param res 执行结果
 * @param log 回滚日志
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t execution_result_set_rollback_log(execution_result_t *res, const char *log);

psp_error_t execution_result_set_capability_id(execution_result_t *res, const char *cap_id);

/**
 * @brief 设置动作
 * @param res 执行结果对象
 * @param action 动作字符串 ("enable" 或 "disable", 会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_action(execution_result_t *res, const char *action);

/**
 * @brief 设置成功标志
 * @param res 执行结果对象
 * @param success 是否成功
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_success(execution_result_t *res, bool success);

/**
 * @brief 设置消息
 * @param res 执行结果对象
 * @param msg 消息字符串 (会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_message(execution_result_t *res, const char *msg);

/**
 * @brief 设置配置指导
 * @param res 执行结果对象
 * @param guidance 指导文本 (会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_guidance(execution_result_t *res, const char *guidance);

/**
 * @brief 设置回滚日志
 * @param res 执行结果对象
 * @param log 回滚日志 (会复制)
 * @return ERR_OK 成功，其他错误码失败
 */
psp_error_t execution_result_set_rollback_log(execution_result_t *res, const char *log);

/* ==================== 辅助函数 ==================== */

/**
 * @brief 将状态枚举转换为字符串
 * @param status 能力状态
 * @return 状态字符串 (静态分配，不要释放)
 */
const char* cap_status_to_string(cap_status_t status);

/**
 * @brief 将字符串转换为状态枚举
 * @param str 状态字符串
 * @return 对应的能力状态，无效输入返回 CAP_STATUS_UNKNOWN
 */
cap_status_t cap_status_from_string(const char *str);

/**
 * @brief 将层次状态转换为字符串
 * @param status 层次状态
 * @return 状态字符串 (静态分配，不要释放)
 */
const char* layer_status_to_string(layer_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* MODELS_RESULT_H */
