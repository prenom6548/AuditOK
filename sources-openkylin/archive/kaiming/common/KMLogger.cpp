/*
 * @Author: yushuoqi yushuoqi@kylinos.cn
 * @Date: 2024-03-27 17:57:36
 * @LastEditors: yushuoqi yushuoqi@kylinos.cn
 * @LastEditTime: 2024-04-18 10:14:33
 * @FilePath: /kaiming/common/kmlogger.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/* kmlogger.cpp
 *
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMLogger.h"

#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <filesystem>

std::vector<std::string> KMLogger::m_logLevelNames = {
    "trace", "debug", "info", "warning", "error", "critical", "off"  
};

#ifndef ONLY_TO_TERMINAL

static bool isSudoOrRoot()
{
    uid_t euid = ::geteuid();
    if (euid == 0)
    {
        return true;
    }

    const char *user = std::getenv("USER");
    if (user != nullptr && std::string(user) == "root")
    {
        return true;
    }
    return false;
}

/**
 * @brief 根据不同权限将日志输出到不同目录
 *
 * sudo, root 权限（安装、升级需要）日志路径： /var/log/kaiming.log
 * 普通用户可获取 $HOME 环境变量： $HOME/.log/kaiming.log
 * 普通用户不可获取 $HOME 环境变量： /tmp/kaiming.log
 *
 * @return string
 */
static std::string getLogPath()
{
    if (isSudoOrRoot())
    {
        return std::string("/var/log/kaiming.log");
    }

    const char *homeDir = std::getenv("HOME");

    if (homeDir == nullptr || *homeDir == '0')
    {
        return std::string("/tmp/kaiming.log");
    }

    const std::string logDir = std::string(homeDir).append("/.log");
    if (!std::filesystem::exists(logDir)) 
    {
        std::error_code ec;
        if (!std::filesystem::create_directories(logDir, ec)) 
        {
            std::cerr << "创建目录失败: " << logDir << ", 错误: " << ec.message() << std::endl;
            return std::string(homeDir).append("/kaiming.log");
        }
    }
    return logDir + "/kaiming.log";
}

KMLogger::KMLogger(token)
{
    m_logFile = getLogPath();
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
        std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << "Failed to open file : " << m_logFile << std::endl;
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

    logfile << "[" << buffer << "] "  << msg << std::endl;
    logfile.close();
}

void KMLogger::traceLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::trace)
    {
        return;
    }

    std::cout << "[trace]: " << msg << std::endl;
    log(msg);
}

void KMLogger::debugLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::debug)
    {
        return;
    }

    std::cout << "[debug]: " << msg << std::endl;
    log(msg);
}

void KMLogger::infoLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::info)
    {
        return;
    }

    std::cout << "[info]: " << msg << std::endl;
    log(msg);
}

void KMLogger::warnLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::warn)
    {
        return;
    }

    std::cout << COLOR_YELLOW << "[warn]: " << COLOR_RESET << msg << std::endl;
    log(msg);
}

void KMLogger::errorLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::err)
    {
        return;
    }

    std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << msg << std::endl;
    log(msg);
}

void KMLogger::criticalLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::critical)
    {
        return;
    }

    std::cerr << COLOR_RED << "[critical]: " << COLOR_RESET << msg << std::endl;
    log(msg);
}

#else

KMLogger::KMLogger(token)
{
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

void KMLogger::traceLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::trace)
    {
        return;
    }

    std::cout << "[trace]: " << msg << std::endl;
}

void KMLogger::debugLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::debug)
    {
        return;
    }

    std::cout << "[debug]: " << msg << std::endl;
}

void KMLogger::infoLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::info)
    {
        return;
    }

    std::cout << "[info]: " << msg << std::endl;
}

void KMLogger::warnLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::warn)
    {
        return;
    }

    std::cout << COLOR_YELLOW << "[warn]: " << COLOR_RESET << msg << std::endl;
}

void KMLogger::errorLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::err)
    {
        return;
    }

    std::cerr << COLOR_RED << "[error]: " << COLOR_RESET << msg << std::endl;
}

void KMLogger::criticalLog(const std::string &msg)
{
    if (m_logLevel > KMLogLevel::critical)
    {
        return;
    }

    std::cerr << COLOR_RED << "[critical]: " << COLOR_RESET << msg << std::endl;
}

#endif // !ONLY_TO_TERMINAL
