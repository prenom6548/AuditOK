#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

static void log_impl(int pri, const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char *msg;
    if (vasprintf(&msg, fmt, args) != -1) {
        syslog(pri, "[%s:%s:%d] %s", file, func, line, msg);
    } else {
        syslog(pri, "[%s:%s:%d] (format error) %s", file, func, line, fmt);
    }
    va_end(args);
}

#define PRINT_DEBUG 1

#define PRINT_DBG(...)                                                          \
    do {                                                                        \
        if (PRINT_DEBUG) {                                                      \
            log_impl(LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
        }                                                                       \
    } while (0);

#define PRINT_NOTICES 1
#define PRINT_NOTICE(...)                                                        \
    do {                                                                         \
        if (PRINT_NOTICES) {                                                     \
            log_impl(LOG_NOTICE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
        }                                                                        \
    } while (0);

#endif // !LOGHELPER_H