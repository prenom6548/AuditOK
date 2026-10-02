/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 */


#ifndef CUSTOM_LOG_H
#define CUSTOM_LOG_H

#include <QString>

enum class LogType : int {
    INFO = 0,
    DEBUG,
    WARNING,
    CRITICAL,
    FATAL,
};

class CustomLog
{
private:
    QString filePath = QString("");

public:
    CustomLog(const QString &name);
    ~CustomLog();
    void setName(const QString &name);
    void info(const QString &info);
    void writeLog(LogType type, const QString &str);
};

#endif // CUSTOM_LOG_H