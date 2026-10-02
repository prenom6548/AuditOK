#ifndef UTILS_CMD_RUNNER_H
#define UTILS_CMD_RUNNER_H

#include <stddef.h>
#include "plat_sec_patrol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 执行命令并获取输出
 * 
 * @param command 要执行的命令（完整路径）
 * @param args 参数数组（NULL 结尾）
 * @param output 输出缓冲区（可选）
 * @param output_size 输出缓冲区大小
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_runner_execute(const char *command, const char **args, 
                          char *output, size_t output_size);

/**
 * @brief 执行 shell 命令并获取输出
 * 
 * @param shell_command shell 命令字符串
 * @param output 输出缓冲区（可选）
 * @param output_size 输出缓冲区大小
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_runner_execute_shell(const char *shell_command, 
                                char *output, size_t output_size);

/**
 * @brief 检查命令是否存在
 * 
 * @param command 命令名称或路径
 * @return 存在返回 true，否则返回 false
 */
bool cmd_runner_command_exists(const char *command);

/**
 * @brief 读取命令的退出码
 * 
 * @return 最近一次执行命令的退出码
 */
int cmd_runner_get_exit_code(void);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_CMD_RUNNER_H */
