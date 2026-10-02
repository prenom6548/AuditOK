#include "logger.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>

#define MODULE "logger"

static struct {
    log_level_t level;
    FILE *log_file;
    char *log_dir;
    bool use_syslog;
} g_logger = {
    .level = LOG_LEVEL_INFO,
    .log_file = NULL,
    .log_dir = NULL,
    .use_syslog = false
};

psp_error_t logger_init(log_level_t level, const char *log_dir)
{
    g_logger.level = level;
    
    if (log_dir) {
        g_logger.log_dir = string_utils_strdup(log_dir);
        if (!g_logger.log_dir) {
            return ERR_MEMORY;
        }
        
        /* 创建日志目录 */
        psp_error_t ret = file_utils_create_directory(log_dir, 0755);
        if (ret != ERR_OK) {
            logger_warn(MODULE, "Failed to create log directory: %s", log_dir);
        }
        
        /* 创建日志文件 */
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_info);
        
        char log_path[1024];
        snprintf(log_path, sizeof(log_path), "%s/log-%s.log", log_dir, timestamp);
        
        g_logger.log_file = fopen(log_path, "a");
        if (!g_logger.log_file) {
            logger_warn(MODULE, "Failed to create log file: %s (error: %d)", 
                       log_path, errno);
            /* 降级使用 syslog */
            g_logger.use_syslog = true;
            openlog("plat-sec-patrol", LOG_PID | LOG_CONS, LOG_USER);
        } else {
            g_logger.use_syslog = false;
        }
    } else {
        /* 使用 syslog */
        g_logger.use_syslog = true;
        openlog("plat-sec-patrol", LOG_PID | LOG_CONS, LOG_USER);
    }
    
    logger_info(MODULE, "Logger initialized (level=%s, dir=%s)", 
               log_level_to_string(level), log_dir ? log_dir : "syslog");
    
    return ERR_OK;
}

void logger_log(log_level_t level, const char *module, const char *format, ...)
{
    if (level > g_logger.level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    logger_vlog(level, module, format, args);
    va_end(args);
}

void logger_vlog(log_level_t level, const char *module, const char *format, va_list args)
{
    if (level > g_logger.level) {
        return;
    }
    
    /* 获取时间戳 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    /* 格式化消息 */
    char message[4096];
    vsnprintf(message, sizeof(message), format, args);
    
    if (g_logger.use_syslog) {
        /* 使用 syslog */
        int syslog_level;
        switch (level) {
            case LOG_LEVEL_ERROR: syslog_level = LOG_ERR; break;
            case LOG_LEVEL_WARN: syslog_level = LOG_WARNING; break;
            case LOG_LEVEL_INFO: syslog_level = LOG_INFO; break;
            case LOG_LEVEL_DEBUG: syslog_level = LOG_DEBUG; break;
            default: syslog_level = LOG_INFO;
        }
        
        /* syslog_msg 需要足够大以容纳 "[module] message" 
         * module 和 message 各最多 4096 字节，加上格式字符需要额外 4 字节
         * 使用 8192 字节确保不会溢出
         */
        char syslog_msg[8192];
        snprintf(syslog_msg, sizeof(syslog_msg), "[%s] %s", module, message);
        syslog(syslog_level, "%s", syslog_msg);
    } else if (g_logger.log_file) {
        /* 写入文件 */
        fprintf(g_logger.log_file, "[%s] [%s] [%s] %s\n", 
               timestamp, log_level_to_string(level), module, message);
        fflush(g_logger.log_file);
    } else {
        /* 输出到stderr */
        fprintf(stderr, "[%s] [%s] [%s] %s\n", 
               timestamp, log_level_to_string(level), module, message);
    }
}

void logger_close(void)
{
    logger_info(MODULE, "Logger closed");
    if (g_logger.log_file) {
        fclose(g_logger.log_file);
        g_logger.log_file = NULL;
    }
    
    if (g_logger.use_syslog) {
        closelog();
        g_logger.use_syslog = false;
    }
    
    free(g_logger.log_dir);
    g_logger.log_dir = NULL;
}

void logger_set_level(log_level_t level)
{
    g_logger.level = level;
    logger_debug(MODULE, "Log level set to: %s", log_level_to_string(level));
}

log_level_t logger_get_level(void)
{
    return g_logger.level;
}
