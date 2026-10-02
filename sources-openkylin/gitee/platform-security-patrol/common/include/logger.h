/**
 * @file logger.h
 * @brief 日志审计模块内部头文件
 */

#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#include "plat_sec_patrol.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化日志模块
 * 
 * @param level 日志级别（低于此级别的日志不会记录）
 * @param log_dir 日志目录（NULL 则使用 syslog）
 * @return 成功返回 ERR_OK，失败返回错误码
 */
psp_error_t logger_init(log_level_t level, const char *log_dir);

/**
 * @brief 记录日志
 * 
 * @param level 日志级别
 * @param module 模块名称
 * @param format 格式字符串
 * @param ... 可变参数
 */
void logger_log(log_level_t level, const char *module, const char *format, ...);

/**
 * @brief 记录日志（va_list 版本）
 * 
 * @param level 日志级别
 * @param module 模块名称
 * @param format 格式字符串
 * @param args 可变参数列表
 */
void logger_vlog(log_level_t level, const char *module, const char *format, va_list args);

/**
 * @brief 关闭日志模块
 */
void logger_close(void);

/**
 * @brief 设置当前日志级别
 * 
 * @param level 日志级别
 */
void logger_set_level(log_level_t level);

/**
 * @brief 获取当前日志级别
 * 
 * @return 当前日志级别
 */
log_level_t logger_get_level(void);

/**
 * @brief 便捷宏：记录错误日志
 */
#define logger_error(module, ...) logger_log(LOG_LEVEL_ERROR, module, __VA_ARGS__)

/**
 * @brief 便捷宏：记录警告日志
 */
#define logger_warn(module, ...) logger_log(LOG_LEVEL_WARN, module, __VA_ARGS__)

/**
 * @brief 便捷宏：记录信息日志
 */
#define logger_info(module, ...) logger_log(LOG_LEVEL_INFO, module, __VA_ARGS__)

/**
 * @brief 便捷宏：记录调试日志
 */
#define logger_debug(module, ...) logger_log(LOG_LEVEL_DEBUG, module, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* CORE_LOGGER_H */
