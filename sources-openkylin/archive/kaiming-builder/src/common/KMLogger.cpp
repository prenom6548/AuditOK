/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMLogger.h"

#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <regex>
#include <iostream>

// ANSI转义序列正则表达式（匹配颜色控制码）
const std::regex ansi_regex("\x1B\\[[0-9;]*[A-Za-z]");

std::vector<std::string> KMLogger::m_logLevelNames = 
{
    "trace", "debug", "info", "warning", "error", "critical", "off"
};

KMLogger::KMLogger(token)
{
    uid_t euid = ::geteuid();
    if (euid == 0)
    {
        m_logFile = std::string("/var/log/kaiming-builder.log");
    }
    else
    {
        m_logFile = std::string(getenv("HOME")).append("/.log/kaiming-builder.log");
    }
    
    setLogLevel(KMLogLevel::info);
}

KMLogger::~KMLogger() { }

void KMLogger::setLogLevel(KMLogLevel level)
{
    m_logLevel = level;
}

void KMLogger::setLogLevel(const std::string &level)
{
    setLogLevel(fromLoglevelName(level));
}

KMLogLevel KMLogger::fromLoglevelName(const std::string &level)
{
    for (int i = 0; i < m_logLevelNames.size(); i++)
    {
        const std::string &name = m_logLevelNames.at(i);
        if (name == level)
        {
            return (KMLogLevel)i;
        }
    }

    return KMLogLevel::off;
}

void KMLogger::log(const std::string &msg)
{
    std::ofstream logfile(m_logFile, std::ios::app);
    if (!logfile)
    {
        // std::cerr << COLOR_RED << "error: " << COLOR_RESET << "Failed to open file : " << 
        // m_logFile << std::endl;
        logfile.close();
        return ;
    }

    time_t currentTime;
    struct tm* localTime;
    char buffer[80];

    // 获取当前时间的时间戳
    currentTime = time(NULL);

    // 将时间戳转换为本地时间结构体
    localTime = localtime(&currentTime);

    // 按照指定格式将本地时间结构体转换为字符串
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

    // 移除所有颜色ANSI转义序列
    std::string nocolorMsg = std::regex_replace(msg, ansi_regex, "");
    std::string cleanMsg;
    for (int i = 0; i< nocolorMsg.length(); i++)
    {
        char c = nocolorMsg[i];
        if ((c <= 0x1F || c == 0x7F) && c != '\n' && c != '\t')
        {
            continue;
        }
        else
        {
            cleanMsg += c;
        }
    }

    logfile << "[" << buffer << "] "  << cleanMsg << std::endl;
    logfile.close();
}

void KMLogger::traceLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::trace)
        return;

    std::cout << msg << std::endl;
    log(msg);
}

void KMLogger::debugLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::debug)
        return;

    std::cout << msg << std::endl;
    log(msg);
}

void KMLogger::infoLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::info)
        return;

    std::cout << msg << std::endl;
    log(msg);
}

void KMLogger::warnLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::warn)
        return;

    std::cout << COLOR_YELLOW << "warn: " << COLOR_RESET << msg << std::endl;
    log(msg);
}

void KMLogger::errorLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::err)
        return;

    std::cerr << COLOR_RED << "error: " << COLOR_RESET << msg << std::endl;
    log(msg);
}

void KMLogger::criticalLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::critical)
        return;

    std::cerr << COLOR_RED << "critical: " << COLOR_RESET << msg << std::endl;
    log(msg);
}
