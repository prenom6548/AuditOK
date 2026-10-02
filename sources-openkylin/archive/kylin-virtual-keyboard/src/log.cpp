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

#include "log.h"

#include <vector>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

const QString APP_LOG_NAME = "kylin-virtual-keyboard.log";
const QString LOGGER_NAME = "kvkbd_log";
const QString GSETTINGS_ID = "org.ukui.virtualkeyboard";
const QString LOG_LEVEL_KEY = "logLevel";

std::shared_ptr<spdlog::logger> SpdlogProxy::m_logger = nullptr;
std::atomic<bool> SpdlogProxy::m_cleaned{false};

void SpdlogProxy::init(const LogOption &option) {
    QString logFileName = getWritableLogFilePath();

    const auto logLevelString = getGsettingsLogLevel();
    spdlog::level::level_enum level = transToSpdLogLevel(logLevelString);

    std::vector<spdlog::sink_ptr> sinks;

    // 添加控制台 sink,捕获控制台输出，主要对qml
    spdlog::sink_ptr console_sink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_sink);

    // 添加文件 sink，使用try-catch处理文件创建失败的情况
    try {
        spdlog::sink_ptr file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFileName.toStdString(), option.fileSize, option.fileCounts,
                option.rotateEnable);
        sinks.push_back(file_sink);
    } catch (const spdlog::spdlog_ex &e) {
        // 文件创建失败，只使用控制台输出
        qWarning() << "Failed to create log file:" << logFileName
                   << "Error:" << e.what()
                   << "Falling back to console output only.";
    }

    m_logger = std::make_shared<spdlog::logger>(LOGGER_NAME.toStdString(),
                                                sinks.begin(), sinks.end());
    m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%!:%#] %v");
    m_logger->set_level(level);
    m_logger->flush_on(level);

    // 注册 Qt 消息处理器
    qInstallMessageHandler(messageHandler);
    spdlog::register_logger(m_logger);
    spdlog::set_default_logger(m_logger);
    KVKBD_INFO("log file:{}", logFileName.toStdString());
}

void SpdlogProxy::cleanUp() {
    static std::once_flag cleanup_flag;
    std::call_once(cleanup_flag, []() {
        m_cleaned.store(true);
        qInstallMessageHandler(nullptr);
        try {
            if (m_logger) {
                m_logger->flush();
            }

            if (spdlog::get(LOGGER_NAME.toStdString())) {
                spdlog::drop(LOGGER_NAME.toStdString());
            }

            if (m_logger) {
                m_logger.reset();
            }
        } catch (const std::exception &e) {
            std::cerr << "Error during log cleanup: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error during log cleanup" << std::endl;
        }
    });
}

bool SpdlogProxy::isCleaned() { return m_cleaned.load(); }

void SpdlogProxy::messageHandler(QtMsgType type,
                                 const QMessageLogContext &context,
                                 const QString &msg) {
    if (m_cleaned.load() || m_logger == nullptr)
        return;

    spdlog::level::level_enum level;

    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    int line = context.line;

    switch (type) {
    case QtDebugMsg:
        level = spdlog::level::debug;
        break;
    case QtInfoMsg:
        level = spdlog::level::info;
        break;
    case QtWarningMsg:
        level = spdlog::level::warn;
        break;
    case QtCriticalMsg:
        level = spdlog::level::err;
        break;
    case QtFatalMsg:
        level = spdlog::level::critical;
        break;
    default:
        level = spdlog::level::info;
        break;
    }
    // 记录到 spdlog（包含文件名、行号、函数名）
    spdlog::log(spdlog::source_loc{file, line, function}, level, "{}",
                msg.toStdString());
}

void SpdlogProxy::debug(const QString &msg) {
    if (m_cleaned.load() || m_logger == nullptr)
        return;
    m_logger->debug("[log-proxy] {}", msg.toStdString());
}

void SpdlogProxy::info(const QString &msg) {
    if (m_cleaned.load() || m_logger == nullptr)
        return;
    m_logger->info("[log-proxy] {}", msg.toStdString());
}

void SpdlogProxy::warn(const QString &msg) {
    if (m_cleaned.load() || m_logger == nullptr)
        return;
    m_logger->warn("[log-proxy] {}", msg.toStdString());
}

void SpdlogProxy::error(const QString &msg) {
    if (m_cleaned.load() || m_logger == nullptr)
        return;
    m_logger->error("[log-proxy] {}", msg.toStdString());
}

QString SpdlogProxy::getGsettingsLogLevel() {
    QString result = "info";
    if (!QGSettings::isSchemaInstalled(GSETTINGS_ID.toUtf8())) {
        return result;
    }

    QGSettings gsettings = QGSettings(GSETTINGS_ID.toUtf8());
    if (gsettings.keys().contains(LOG_LEVEL_KEY)) {
        QVariant value = gsettings.get(LOG_LEVEL_KEY);
        if (value.isValid()) {
            result = value.toString();
        }
    }
    return result;
}

spdlog::level::level_enum
SpdlogProxy::transToSpdLogLevel(const QString &level) {
    if (level == "debug") {
        return spdlog::level::debug;
    } else if (level == "info") {
        return spdlog::level::info;
    } else if (level == "warn") {
        return spdlog::level::warn;
    } else if (level == "error") {
        return spdlog::level::err;
    } else {
        return spdlog::level::info;
    }
}

QString SpdlogProxy::getWritableLogFilePath() {
    QString homeDir =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QStringList logPaths;
    QString primaryLogPath = homeDir + "/.log/";
    QString fallbackLogPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
        "/kylin-virtual-keyboard/";
    logPaths << primaryLogPath << fallbackLogPath;

    QString logFileName;
    for (const QString &logPath : logPaths) {
        QDir logDir(logPath);
        if (!logDir.exists()) {
            logDir.mkpath(".");
        }
        if (!QFileInfo(logPath).isWritable()) {
            continue;
        }

        logFileName = logPath + APP_LOG_NAME;
        break;
    }

    return logFileName;
}
