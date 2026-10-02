/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef UTILS_LOGGER_H
#define UTILS_LOGGER_H

#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <mutex>
#define gettid() (long)syscall(SYS_gettid)

// #define CLR_CLR         "\033[0m"       // 恢复颜色
// #define CLR_BLACK       "\033[30m"      // 黑色字
// #define CLR_RED         "\033[31m"      // 红色字
// #define CLR_GREEN       "\033[32m"      // 绿色字
// #define CLR_YELLOW      "\033[33m"      // 黄色字
// #define CLR_BLUE        "\033[34m"      // 蓝色字
// #define CLR_PURPLE      "\033[35m"      // 紫色字
// #define CLR_SKYBLUE     "\033[36m"      // 天蓝字
// #define CLR_WHITE       "\033[37m"      // 白色字
//
// #define CLR_BLK_WHT     "\033[40;37m"   // 黑底白字
// #define CLR_RED_WHT     "\033[41;37m"   // 红底白字
// #define CLR_GREEN_WHT   "\033[42;37m"   // 绿底白字
// #define CLR_YELLOW_WHT  "\033[43;37m"   // 黄底白字
// #define CLR_BLUE_WHT    "\033[44;37m"   // 蓝底白字
// #define CLR_PURPLE_WHT  "\033[45;37m"   // 紫底白字
// #define CLR_SKYBLUE_WHT "\033[46;37m"   // 天蓝底白字
// #define CLR_WHT_BLK     "\033[47;30m"   // 白底黑字

#ifndef __FILENAME__
#define __FILENAME__ (strrchr("/" __FILE__, '/') + 1)
#endif

class Logger {
public:
    enum Level { LOG_LEVEL_SILENT = 0, LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR };

    static void printLn();

    template <typename T, typename... Args>
    static void printLn(T t, Args... args) {
        std::cout << t << ' ';
        printLn(args...);
    }

    static void printErrorLn();

    template <typename T, typename... Args>
    static void printErrorLn(T t, Args... args) {
        std::cerr << t << ' ';
        printErrorLn(args...);
    }

    template <typename... Args>
    static void printLnLevel(const Logger &logger, Level level, Args... args) {
        std::lock_guard locker(logger.mutex_);
#ifdef RUN_IN_DEBUG
        if (logger.level_ == LOG_LEVEL_SILENT) {
            // do nothing
        } else if (level == LOG_LEVEL_DEBUG and level >= logger.level_) {
            printLn(currentTime(), gettid(), "\033[36mdebug\033[0m", args...);
        } else if (level == LOG_LEVEL_INFO and level >= logger.level_) {
            printLn(currentTime(), gettid(), "\033[32minfo \033[0m", args...);
        } else if (level == LOG_LEVEL_WARN and level >= logger.level_) {
            printLn(currentTime(), gettid(), "\033[33mwarn \033[0m", args...);
        } else if (level == LOG_LEVEL_ERROR and level >= logger.level_) {
            printLn(currentTime(), gettid(), "\033[31merror\033[0m", args...);
        }
#else
        // release 暂时往stderr输出
        if (logger.level_ == LOG_LEVEL_SILENT) {
            // do nothing
        } else if (level == LOG_LEVEL_DEBUG and level >= logger.level_) {
            printErrorLn(currentTime(), "\033[36mdebug\033[0m", args...);
        } else if (level == LOG_LEVEL_INFO and level >= logger.level_) {
            printErrorLn(currentTime(), "\033[32minfo \033[0m", args...);
        } else if (level == LOG_LEVEL_WARN and level >= logger.level_) {
            printErrorLn(currentTime(), "\033[33mwarn \033[0m", args...);
        } else if (level == LOG_LEVEL_ERROR and level >= logger.level_) {
            printErrorLn(currentTime(), "\033[31merror\033[0m", args...);
        }
#endif
    }

    static void printfLnLevel(const Logger &logger, Level level, const char *format) {
        std::lock_guard locker(logger.mutex_);
#ifdef RUN_IN_DEBUG
        FILE *pFile = stdout;
#else
        FILE *pFile = stderr;
#endif
        if (logger.level_ == LOG_LEVEL_SILENT) {
            // do nothing
        } else if (level == LOG_LEVEL_DEBUG and level >= logger.level_) {
            std::fprintf(pFile, "%s %s %s\n", currentTime().c_str(), "\033[36mdebug\033[0m ", format);
        } else if (level == LOG_LEVEL_INFO and level >= logger.level_) {
            std::fprintf(pFile, "%s %s %s\n", currentTime().c_str(), "\033[32minfo \033[0m", format);

        } else if (level == LOG_LEVEL_WARN and level >= logger.level_) {
            std::fprintf(pFile, "%s %s %s\n", currentTime().c_str(), "\033[33mwarn \033[0m", format);

        } else if (level == LOG_LEVEL_ERROR and level >= logger.level_) {
            std::fprintf(pFile, "%s %s %s\n", currentTime().c_str(), "\033[31merror\033[0m", format);
        }
    }

    template <typename... Args>
    static void printfLnLevel(const Logger &logger, Level level, const char *format, Args... args) {
        std::lock_guard locker(logger.mutex_);
#ifdef RUN_IN_DEBUG
        FILE *pFile = stdout;
#else
        FILE *pFile = stderr;
#endif
        if (logger.level_ == LOG_LEVEL_SILENT) {
            // do nothing
        } else if (level == LOG_LEVEL_DEBUG and level >= logger.level_) {
            std::fprintf(pFile, "%s %s", currentTime().c_str(), "\033[36mdebug\033[0m ");
            std::fprintf(pFile, format, args...);
            std::fprintf(pFile, "\n");
        } else if (level == LOG_LEVEL_INFO and level >= logger.level_) {
            std::fprintf(pFile, "%s %s ", currentTime().c_str(), "\033[32minfo \033[0m");
            std::fprintf(pFile, format, args...);
            std::fprintf(pFile, "\n");
        } else if (level == LOG_LEVEL_WARN and level >= logger.level_) {
            std::fprintf(pFile, "%s %s ", currentTime().c_str(), "\033[33mwarn \033[0m");
            std::fprintf(pFile, format, args...);
            std::fprintf(pFile, "\n");
        } else if (level == LOG_LEVEL_ERROR and level >= logger.level_) {
            std::fprintf(pFile, "%s %s ", currentTime().c_str(), "\033[31merror\033[0m");
            std::fprintf(pFile, format, args...);
            std::fprintf(pFile, "\n");
        }
    }

    static std::string location(const char *fileName, int line, const char *funcName);

    static std::string currentTime();

public:
    Logger();

private:
    mutable std::mutex mutex_;
    std::atomic<Level> level_{LOG_LEVEL_INFO};
};

static Logger g_Logger{};

#ifdef RUN_IN_DEBUG

#define LOGD(...)                                                                                    \
    do {                                                                                             \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_DEBUG,                                      \
                             Logger::location(__FILENAME__, __LINE__, __FUNCTION__), ##__VA_ARGS__); \
    } while (0)

#define LOGDF(fmt, ...)                                                                                     \
    do {                                                                                                    \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_DEBUG, "[%s:%d:%s] " fmt, __FILENAME__, __LINE__, \
                              __FUNCTION__, ##__VA_ARGS__);                                                 \
    } while (0)

#define LOGI(...)                                                                                                      \
    do {                                                                                                               \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_INFO, Logger::location(__FILENAME__, __LINE__, __FUNCTION__), \
                             ##__VA_ARGS__);                                                                           \
    } while (0)

#define LOGIF(fmt, ...)                                                                                    \
    do {                                                                                                   \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_INFO, "[%s:%d:%s] " fmt, __FILENAME__, __LINE__, \
                              __FUNCTION__, ##__VA_ARGS__);                                                \
    } while (0)

#define LOGW(...)                                                                                                      \
    do {                                                                                                               \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_WARN, Logger::location(__FILENAME__, __LINE__, __FUNCTION__), \
                             ##__VA_ARGS__);                                                                           \
    } while (0)

#define LOGWF(fmt, ...)                                                                                    \
    do {                                                                                                   \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_WARN, "[%s:%d:%s] " fmt, __FILENAME__, __LINE__, \
                              __FUNCTION__, ##__VA_ARGS__);                                                \
    } while (0)

#define LOGE(...)                                                                                    \
    do {                                                                                             \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_ERROR,                                      \
                             Logger::location(__FILENAME__, __LINE__, __FUNCTION__), ##__VA_ARGS__); \
    } while (0)

#define LOGEF(fmt, ...)                                                                                     \
    do {                                                                                                    \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_ERROR, "[%s:%d:%s] " fmt, __FILENAME__, __LINE__, \
                              __FUNCTION__, ##__VA_ARGS__);                                                 \
    } while (0)

#else

#define LOGD(...)                                                               \
    do {                                                                        \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_DEBUG, ##__VA_ARGS__); \
    } while (0)

#define LOGDF(fmt, ...)                                                               \
    do {                                                                              \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOGI(...)                                                              \
    do {                                                                       \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_INFO, ##__VA_ARGS__); \
    } while (0)

#define LOGIF(fmt, ...)                                                              \
    do {                                                                             \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_INFO, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOGW(...)                                                              \
    do {                                                                       \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_WARN, ##__VA_ARGS__); \
    } while (0)

#define LOGWF(fmt, ...)                                                              \
    do {                                                                             \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_WARN, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOGE(...)                                                               \
    do {                                                                        \
        Logger::printLnLevel(g_Logger, Logger::LOG_LEVEL_ERROR, ##__VA_ARGS__); \
    } while (0)

#define LOGEF(fmt, ...)                                                               \
    do {                                                                              \
        Logger::printfLnLevel(g_Logger, Logger::LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__); \
    } while (0)

#endif

#endif  // UTILS_LOGGER_H
