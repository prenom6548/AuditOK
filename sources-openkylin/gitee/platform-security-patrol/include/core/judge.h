#ifndef CORE_JUDGE_H
#define CORE_JUDGE_H

#include "models/environment.h"
#include "models/capability.h"
#include "models/result.h"
#include "plugin_env_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 判定所有能力的状态
 * 
 * @param env 环境信息（用于CPU层判定）
 * @param env_list 插件环境变量列表（用于固件和OS层判定）
 * @param caps 能力列表
 * @return 成功返回判定结果列表，失败返回 NULL
 */
judgement_result_list_t* judge_capabilities(const plugin_env_list_t *env_list,
                                                     const capability_list_t *caps);

/**
 * @brief 判定单个能力的 CPU 层状态并设置到结果中
 * 
 * @note CPU层判定需要使用environment_t中的cpu信息
 * @param env_list 插件环境变量列表
 * @param cap 能力项
 * @param result 判定结果对象（用于设置CPU层状态）
 */
void judge_cpu_layer(const plugin_env_list_t *env_list, const capability_t *cap, 
                    judgement_result_t *result);

/**
 * @brief 判定单个能力的固件层状态并设置到结果中
 * 
 * @param env_list 插件环境变量列表
 * @param cap 能力项
 * @param result 判定结果对象（用于设置固件层状态）
 */
void judge_firmware_layer(const plugin_env_list_t *env_list, const capability_t *cap,
                         judgement_result_t *result);

/**
 * @brief 判定单个能力的 OS 层状态并设置到结果中
 * 
 * @param env_list 插件环境变量列表
 * @param cap 能力项
 * @param result 判定结果对象（用于设置OS层状态）
 */
void judge_os_layer(const plugin_env_list_t *env_list, const capability_t *cap,
                   judgement_result_t *result);

/**
 * @brief 检查能力的依赖是否满足
 * 
 * @param env 环境信息（预留参数，暂未使用）
 * @param cap 能力项
 * @param results 当前已判定的结果列表（用于检查依赖）
 * @return 依赖满足返回 true，否则返回 false
 */
bool check_dependencies(const capability_t *cap, 
                       const judgement_result_list_t *results);

/**
 * @brief 计算整体状态
 * 
 * 状态判定优先级（从高到低）：
 * 1. 硬件不支持（某层为UNSUPPORTED）→ CAP_STATUS_UNSUPPORTED
 * 2. 部分不可用/禁用（某层为UNAVAILABLE或DISABLED）→ CAP_STATUS_UNAVAILABLE/DISABLED
 * 3. 全部支持 → CAP_STATUS_AVAILABLE
 * 4. 其他情况 → CAP_STATUS_UNAVAILABLE
 * 
 * @param cpu_status CPU 层状态
 * @param firmware_status 固件层状态
 * @param os_status OS 层状态
 * @return 整体状态
 */
cap_status_t compute_overall_status(layer_status_t cpu_status, 
                                   layer_status_t firmware_status,
                                   layer_status_t os_status);

#ifdef __cplusplus
}
#endif

#endif /* CORE_JUDGE_H */
