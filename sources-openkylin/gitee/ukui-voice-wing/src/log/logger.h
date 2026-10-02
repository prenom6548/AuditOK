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

#ifndef LOGGER_H
#define LOGGER_H

#include <QByteArray>
#include <QDebug>
#include <QString>

class LogStream;

class Logger
{
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error,
    };

    static Logger &instance();

    void init();

private:
    friend class LogStream;

    Logger() = default;

    static const char *fileBaseName(const char *path);
    static QString tagFromFile(const char *path);
    void write(Level level,
               const char *tag,
               const char *file,
               const char *function,
               int line,
               const QString &message);

    QString levelString(Level level) const;
    QString logDirPath() const;
    QString logFilePath() const;
    QString debugLogFilePath() const;
    QString formatLocation(const char *file, const char *function, int line) const;

    bool m_initialized = false;
};

class LogStream
{
public:
    LogStream(Logger::Level level, const char *file, const char *function, int line);
    ~LogStream();

    template<typename T>
    LogStream &operator<<(const T &value)
    {
        QDebug(&m_message) << value;
        return *this;
    }

private:
    QString m_message;
    QByteArray m_tagUtf8;
    Logger::Level m_level;
    const char *m_tag;
    const char *m_file;
    const char *m_function;
    int m_line;
};

// 统一日志接口：仅传入日志级别，模块名/文件/函数/行号由宏自动注入
#define VW_LOG(level) LogStream(level, __FILE__, __FUNCTION__, __LINE__)

#endif // LOGGER_H
