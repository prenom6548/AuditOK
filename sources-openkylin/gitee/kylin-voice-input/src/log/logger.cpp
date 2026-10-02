/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <cstring>

namespace {

QMutex g_logMutex;

} // namespace

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

const char *Logger::fileBaseName(const char *path)
{
    if (!path) {
        return "";
    }
    const char *slash = std::strrchr(path, '/');
    if (slash) {
        return slash + 1;
    }
    slash = std::strrchr(path, '\\');
    return slash ? slash + 1 : path;
}

QString Logger::tagFromFile(const char *path)
{
    QString tag = QString::fromUtf8(fileBaseName(path));
    const int dot = tag.lastIndexOf(QLatin1Char('.'));
    if (dot > 0) {
        tag = tag.left(dot);
    }
    return tag;
}

void Logger::init()
{
    QMutexLocker locker(&g_logMutex);
    if (m_initialized) {
        return;
    }

    const QString dirPath = logDirPath();
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        return;
    }

    m_initialized = true;
}

void Logger::write(Level level,
                   const char *tag,
                   const char *file,
                   const char *function,
                   int line,
                   const QString &message)
{
    if (!tag || message.isEmpty()) {
        return;
    }

    QMutexLocker locker(&g_logMutex);
    if (!m_initialized) {
        init();
    }
    if (!m_initialized) {
        return;
    }

    QString targetPath;
    if (level == Level::Debug) {
        targetPath = debugLogFilePath();
        if (!QFileInfo::exists(targetPath)) {
            return;
        }
    } else {
        targetPath = logFilePath();
    }

    QFile logFile(targetPath);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    const QString lineText = QStringLiteral("[%1] [%2] [%3] [%4] %5")
                                 .arg(QDateTime::currentDateTime().toString(
                                          QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                                      levelString(level),
                                      QString::fromUtf8(tag),
                                      formatLocation(file, function, line),
                                      message);

    QTextStream out(&logFile);
    out << lineText << '\n';
}

QString Logger::formatLocation(const char *file, const char *function, int line) const
{
    const QString fileName = QString::fromUtf8(file ? fileBaseName(file) : "");
    const QString funcName = QString::fromUtf8(function ? function : "");
    if (line > 0) {
        return QStringLiteral("%1:%2 %3").arg(fileName, QString::number(line), funcName);
    }
    return QStringLiteral("%1 %2").arg(fileName, funcName);
}

QString Logger::levelString(Level level) const
{
    switch (level) {
    case Level::Debug:
        return QStringLiteral("DEBUG");
    case Level::Info:
        return QStringLiteral("INFO");
    case Level::Warning:
        return QStringLiteral("WARN");
    case Level::Error:
        return QStringLiteral("ERROR");
    }
    return QStringLiteral("UNKNOWN");
}

QString Logger::logDirPath() const
{
    return QDir::homePath() + QStringLiteral("/.log/ukui-voice-wing");
}

QString Logger::logFilePath() const
{
    return logDirPath() + QStringLiteral("/ukui-voice-wing.log");
}

QString Logger::debugLogFilePath() const
{
    return logDirPath() + QStringLiteral("/ukui-voice-wing-debug.log");
}

LogStream::LogStream(Logger::Level level,
                     const char *file,
                     const char *function,
                     int line)
    : m_tagUtf8(Logger::tagFromFile(file).toUtf8())
    , m_level(level)
    , m_tag(m_tagUtf8.constData())
    , m_file(file)
    , m_function(function)
    , m_line(line)
{
}

LogStream::~LogStream()
{
    const QString msg = m_message.trimmed();
    if (!msg.isEmpty() && m_tag) {
        Logger::instance().write(m_level, m_tag, m_file, m_function, m_line, msg);
    }
}
