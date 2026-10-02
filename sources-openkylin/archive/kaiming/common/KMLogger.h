/*
 * @Author: yushuoqi yushuoqi@kylinos.cn
 * @Date: 2024-04-03 11:12:35
 * @LastEditors: yushuoqi yushuoqi@kylinos.cn
 * @LastEditTime: 2024-04-18 11:25:39
 * @FilePath: /kaiming/common/KMLogger.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/* KMLogger.h
 *
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMLOGGER_H__
#define __KMLOGGER_H__

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cstring>
#include <stdarg.h>

#include "KMSingleton.h"

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

#ifndef ONLY_TO_TERMINAL

#define STACK_SIZE 1024 * 8 * 10 * 100

#define F2S()                              \
    char *tmp = new char[STACK_SIZE];      \
    std::unique_ptr<char[]> data_ptr(tmp); \
    memset(tmp, 0x00, STACK_SIZE);         \
    va_list arg_list;                      \
    va_start(arg_list, fmt);               \
    vsprintf(tmp, fmt, arg_list);          \
    va_end(arg_list);                      \
    std::string log_msg = tmp;


class KMLogger : public KMSingleton<KMLogger>
{
public:
    KMLogger(token);
    ~KMLogger();

    void setLogLevel(KMLogLevel level);
    void setLogLevel(const std::string &level);

    void inline trace(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::trace)
        {
            return;
        }

        F2S();

        log(log_msg);
    }

    void inline debug(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::debug)
        {
            return;
        }

        F2S();

        log(log_msg);
    }

    void inline info(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::info)
        {
            return;
        }

        F2S();

        log(log_msg);
    }

    void inline warn(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::warn)
        {
            return;
        }

        F2S();

        log(log_msg);
    }

    void inline error(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::err)
        {
            return;
        }

        F2S();

        log(log_msg);
    }

    void inline critical(const char *fmt, ...)
    {
        if (m_logLevel > KMLogLevel::critical)
        {
            return;
        }

        F2S();

        log(log_msg);
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

private:
    KMLogLevel fromLoglevelName(const std::string &level);

    void log(const std::string &msg);

private:
    static std::vector<std::string> m_logLevelNames;

    std::string m_logFile;
    KMLogLevel m_logLevel;
};

#else

class KMLogger : public KMSingleton<KMLogger>
{
public:
    KMLogger(token);
    ~KMLogger();

    void setLogLevel(KMLogLevel level);
    void setLogLevel(const std::string &level);

    void traceLog(const std::string &msg);
    void debugLog(const std::string &msg);
    void infoLog(const std::string &msg);
    void warnLog(const std::string &msg);
    void errorLog(const std::string &msg);
    void criticalLog(const std::string &msg);

private:
    KMLogLevel fromLoglevelName(const std::string &level);

private:
    static std::vector<std::string> m_logLevelNames;

    KMLogLevel m_logLevel;
};

#endif // !ONLY_TO_TERMINAL

#define kmlogger KMLogger::instance()

#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"

#define KMTrace(msg) KMLogger::instance().traceLog(msg)
#define KMDebug(msg) KMLogger::instance().debugLog(msg)
#define KMInfo(msg) KMLogger::instance().infoLog(msg)
#define KMWarn(msg) KMLogger::instance().warnLog(msg)
#define KMError(msg) KMLogger::instance().errorLog(msg)
#define KMCritical(msg) KMLogger::instance().criticalLog(msg)

#endif // !__KMLOGGER_H__
