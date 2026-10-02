#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>

#include "common.h"

void log_message(enum LogLevel level, const char *format, ...) {
    char log_content[MAX_LOG_SIZE];
    va_list arg;
    va_start(arg, format);
    vsnprintf(log_content, MAX_LOG_SIZE - 1, format, arg);
    va_end(arg);

    int syslog_level;
    switch (level) {
        case _LOG_ERROR:
            syslog_level = LOG_ERR;
            break;
        case _LOG_WARN:
            syslog_level = LOG_WARNING;
            break;
        case _LOG_NOTICE:
            syslog_level = LOG_NOTICE;
            break;
        case _LOG_INFO:
            syslog_level = LOG_INFO;
            break;
        case _LOG_DEBUG:
            syslog_level = LOG_DEBUG;
            break;
        default:
            return;
    }

    openlog("kba", LOG_PID, LOG_AUTH);
    syslog(syslog_level, "%s", log_content);
    closelog();

    return;
}