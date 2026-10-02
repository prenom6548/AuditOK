/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "KMSingleton.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cstring>

#include <stdarg.h>

#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"

#define F2S()                     \
    char tmp[1024];               \
    memset(tmp, 0x00, 1024);      \
    va_list arg_list;             \
    va_start(arg_list, fmt);      \
    vsprintf(tmp, fmt, arg_list); \
    va_end(arg_list);             \
    std::string log_msg = tmp;

enum KMLogLevel
{
    trace = 0,
    debug,
    info,
    warn,
    err,
    critical,
    off,
};
class KMLogger : public KMSingleton<KMLogger>
{
public:
    KMLogger(token);
    ~KMLogger();

    void setLogLevel(KMLogLevel level);
    void setLogLevel(const std::string &level);
    void setLogPath(const std::string &logPath)
    {
        m_logFile = logPath + "/kaiming-builder.log";
    }

    void inline trace(const char *fmt, ...)
    {
        F2S();
        traceLog(log_msg);
    }

    void inline debug(const char *fmt, ...)
    {
        F2S();
        debugLog(log_msg);
    }

    void inline info(const char *fmt, ...)
    {
        F2S();
        infoLog(log_msg);
    }

    void inline warn(const char *fmt, ...)
    {
        F2S();
        warnLog(log_msg);
    }

    void inline error(const char *fmt, ...)
    {
        F2S();
        errorLog(log_msg);
    }

    void inline critical(const char *fmt, ...)
    {
        F2S();
        criticalLog(log_msg);
    }

    void inline pinfo(const char *fmt, ...)
    {
        F2S();
        infoLog(log_msg);
    }

    void inline perror(const char *fmt, ...)
    {
        F2S();
        errorLog(log_msg);
    }

    void traceLog(const std::string &msg);
    void debugLog(const std::string &msg);
    void infoLog(const std::string &msg);
    void warnLog(const std::string &msg);
    void errorLog(const std::string &msg);
    void criticalLog(const std::string &msg);

    void log(const std::string &msg);

private:
    KMLogLevel fromLoglevelName(const std::string &level);

private:
    static std::vector<std::string> m_logLevelNames;
    std::string m_logFile;
    KMLogLevel m_logLevel;
};

#define km_logger KMLogger::instance()

#define KMTrace(msg) KMLogger::instance().traceLog(msg)
#define KMDebug(msg) KMLogger::instance().debugLog(msg)
#define KMInfo(msg) KMLogger::instance().infoLog(msg)
#define KMWarn(msg) KMLogger::instance().warnLog(msg)
#define KMError(msg) KMLogger::instance().errorLog(msg)
#define KMCritical(msg) KMLogger::instance().criticalLog(msg)

#endif // !__LOGGER_H__
