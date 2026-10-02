/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
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

#ifndef LOG_H
#define LOG_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <memory>

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QString>

#include <iostream>

#include <spdlog/spdlog.h>

#define KVKBD_TRACE(...)                                                       \
    do {                                                                       \
        if (!SpdlogProxy::isCleaned() &&                                       \
            spdlog::default_logger_raw() != nullptr) {                         \
            SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__);    \
        }                                                                      \
    } while (0)

#define KVKBD_DEBUG(...)                                                       \
    do {                                                                       \
        if (!SpdlogProxy::isCleaned() &&                                       \
            spdlog::default_logger_raw() != nullptr) {                         \
            SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__);    \
        }                                                                      \
    } while (0)

#define KVKBD_INFO(...)                                                        \
    do {                                                                       \
        if (!SpdlogProxy::isCleaned() &&                                       \
            spdlog::default_logger_raw() != nullptr) {                         \
            SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__);     \
        }                                                                      \
    } while (0)

#define KVKBD_WARN(...)                                                        \
    do {                                                                       \
        if (!SpdlogProxy::isCleaned() &&                                       \
            spdlog::default_logger_raw() != nullptr) {                         \
            SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__);     \
        }                                                                      \
    } while (0)

#define KVKBD_ERROR(...)                                                       \
    do {                                                                       \
        if (!SpdlogProxy::isCleaned() &&                                       \
            spdlog::default_logger_raw() != nullptr) {                         \
            SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__);    \
        }                                                                      \
    } while (0)

class SpdlogProxy {
public:
    static SpdlogProxy &getInstance() {
        static SpdlogProxy instance;
        return instance;
    }

    SpdlogProxy(const SpdlogProxy &) = delete;
    SpdlogProxy &operator=(const SpdlogProxy &) = delete;

    class LogOption {
    public:
        long long fileSize = 1024 * 1024 * 10;
        int fileCounts = 1;
        bool rotateEnable = false;
    };

    static bool isCleaned();
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &msg);

    Q_INVOKABLE static void debug(const QString &msg);
    Q_INVOKABLE static void info(const QString &msg);
    Q_INVOKABLE static void warn(const QString &msg);
    Q_INVOKABLE static void error(const QString &msg);

private:
    SpdlogProxy() { init(SpdlogProxy::LogOption()); }
    ~SpdlogProxy() { cleanUp(); }
    static void init(const LogOption &option);
    static void cleanUp();
    static QString getGsettingsLogLevel();
    static spdlog::level::level_enum transToSpdLogLevel(const QString &level);
    static QString getWritableLogFilePath();

private:
    static std::shared_ptr<spdlog::logger> m_logger;
    static std::atomic<bool> m_cleaned;
};

#endif // SPDLOGPROXY_H
