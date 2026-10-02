#ifndef CORE_CLI_H
#define CORE_CLI_H

#include "plat_sec_patrol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CLI 配置结构
 */
typedef struct {
    command_t command;            /* 命令类型 */
    output_format_t format;       /* 输出格式 */
    char *capability_id;          /* 能力 ID（用于 enable/disable） */
    bool verbose;                 /* 是否详细输出 */
    char *plugin_dir;             /* 插件目录 */
    char *config_file;            /* 配置文件路径 */
} cli_config_t;

/**
 * @brief 运行 CLI
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 成功返回 0，失败返回错误码
 */
int cli_run(int argc, char **argv);

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数数组
 * @param config 输出配置结构
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cli_parse_args(int argc, char **argv, cli_config_t *config);

/**
 * @brief 打印帮助信息
 */
void cli_print_help(void);

/**
 * @brief 打印版本信息
 */
void cli_print_version(void);

/**
 * @brief 执行检测命令
 * @param config CLI 配置
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_check(const cli_config_t *config);

/**
 * @brief 执行使能命令
 * @param config CLI 配置
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_enable(const cli_config_t *config);

/**
 * @brief 执行禁用命令
 * @param config CLI 配置
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_disable(const cli_config_t *config);

/**
 * @brief 执行列表命令
 * @param config CLI 配置
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t cmd_list(const cli_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* CORE_CLI_H */
