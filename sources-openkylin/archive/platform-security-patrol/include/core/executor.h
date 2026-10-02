#ifndef CORE_EXECUTOR_H
#define CORE_EXECUTOR_H

#include "models/environment.h"
#include "models/result.h"
#include "models/capability.h"
#include "plugin_env_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 使能/禁用指定能力
 * 
 * @param cmd 指令类型: CMD_ENABLE or CMD_DISABLE
 * @param cap_id 能力 ID
 * @param plugin_dir 插件目录路径
 * @param env 环境信息（可选，NULL 则重新采集）
 * @return 执行结果
 */
execution_result_t* executor_control(command_t cmd, char *cap_id, plugin_env_list_t *env_data, capability_list_t *caps);

/**
 * @brief 执行前置校验
 * 
 * 检查权限、依赖、当前状态等
 * 
 * @param cap_id 能力 ID
 * @param action 动作类型（"enable" 或 "disable"）
 * @param env 环境信息
 * @return 通过返回 ERR_OK，失败返回错误码
 */
psp_error_t executor_precheck(const char *cap_id, const char *action, const environment_t *env);

/**
 * @brief 备份配置文件
 * 
 * @param config_path 配置文件路径
 * @param backup_path 备份文件路径（输出）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t executor_backup_config(const char *config_path, char **backup_path);

/**
 * @brief 恢复配置文件
 * 
 * @param backup_path 备份文件路径
 * @param config_path 原配置文件路径
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t executor_restore_config(const char *backup_path, const char *config_path);

/**
 * @brief 验证配置是否生效
 * 
 * @param cap_id 能力 ID
 * @return 验证通过返回 true，否则返回 false
 */
bool executor_validate(const char *cap_id);

#ifdef __cplusplus
}
#endif

#endif /* CORE_EXECUTOR_H */
