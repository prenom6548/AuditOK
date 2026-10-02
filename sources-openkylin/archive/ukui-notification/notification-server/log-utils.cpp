/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "log-utils.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
//T QAtomicInteger::load() const
#define LOAD load()
#else
//T QAtomicInteger::loadRelaxed() const
#define LOAD loadRelaxed()
#endif

#define LOG_FILE_COUNT         2
#define MAX_LOG_FILE_SIZE      4194304
#define MAX_LOG_CHECK_INTERVAL 43200000

quint64 LogUtils::m_startUpTime = 0;
QAtomicInt LogUtils::m_logFileId = -1;
QString LogUtils::m_logFileName;

const static QString LOG_FILE_PATH = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/ukui-notification/";


void LogUtils::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    checkLogFile();

    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentDate = QDate::currentDate().toString("yyyy-MM-dd").toLocal8Bit();
    QByteArray currentTime = QTime::currentTime().toString().toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    FILE *log_file = fopen(m_logFileName.arg(m_logFileId.LOAD).toLocal8Bit().constData(), "a+");

    switch (type) {
    case QtDebugMsg:
        if (!log_file) {
            break;
        }
        fprintf(log_file, "[%s %s] [Debug] %s (%s:%u, %s)\n", currentDate.constData(), currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(log_file? log_file: stdout, "\033[32m[%s %s] [Info]  %s \033[0m(%s:%u, %s)\n", currentDate.constData(), currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(log_file? log_file: stderr, "\033[33m[%s %s] [Warning]  %s \033[0m(%s:%u, %s)\n", currentDate.constData(), currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(log_file? log_file: stderr, "\033[31m[%s %s] [Critical] %s \033[0m(%s:%u, %s)\n", currentDate.constData(), currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(log_file? log_file: stderr, "\033[1;31m[%s %s] [Fatal] %s \033[0m(%s:%u, %s)\n", currentDate.constData(), currentTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    }

    if (log_file) {
        fclose(log_file);
    }
}

void LogUtils::initLogFile(const QString &fileName)
{
    QDir dir;
    if (!dir.exists(LOG_FILE_PATH)) {
        if (!dir.mkpath(LOG_FILE_PATH)) {
            qWarning() << "Unable to create" << LOG_FILE_PATH;
            return;
        }
    }

    m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_logFileName = LOG_FILE_PATH + fileName + "-%0.log";

    for (int i = 0; i < LOG_FILE_COUNT; ++i) {
        auto currentLogFile = m_logFileName.arg(i);
        if (QFile::exists(currentLogFile)) {
            if (checkFileSize(currentLogFile)) {
                m_logFileId = i;
                break;
            }
        } else {
            QFile file(currentLogFile);
            file.open(QIODevice::WriteOnly);
            file.close();
        }
    }


    if (m_logFileId < 0) {
        m_logFileId = 0;
        clearFile(m_logFileName.arg(m_logFileId));
    }

    qInfo() << "Current log file:" << m_logFileName.arg(m_logFileId);
}

void LogUtils::checkLogFile()
{
    quint64 logTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    quint64 spacing = std::max(logTime, m_startUpTime) - std::min(logTime, m_startUpTime);

    if (spacing <= MAX_LOG_CHECK_INTERVAL && checkFileSize(m_logFileName.arg(m_logFileId.LOAD))) {
        return;
    }

    m_logFileId.fetchAndStoreRelaxed((m_logFileId.LOAD + 1) % LOG_FILE_COUNT);
    auto currentLogFile = m_logFileName.arg(m_logFileId.LOAD);
    if (!checkFileSize(currentLogFile)) {
        clearFile(currentLogFile);
    }
}

bool LogUtils::checkFileSize(const QString &fileName)
{
    return QFile(fileName).size() < MAX_LOG_FILE_SIZE;
}

void LogUtils::clearFile(const QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write("");
    file.flush();
    file.close();
}
