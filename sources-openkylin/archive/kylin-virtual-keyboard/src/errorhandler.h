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

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QString>

#ifndef SIGUNUSED
#define SIGUNUSED 29
#endif

class ErrorHandler {
public:
    ErrorHandler() = delete;
    ~ErrorHandler() = delete;
    ErrorHandler(const ErrorHandler &) = delete;
    ErrorHandler &operator=(const ErrorHandler &) = delete;

    static void init();

    static void setCrashLogPath(const QString &path);

private:
    static void createLogFile();
    static void registerSignalHandler();

    static void signalHandler(int sig);

    static void writeString(int fd, const char *str);

    static void writeBuffer(int fd, const char *buffer, int len);

    static void writeUInt64(int fd, unsigned long long number);

    static ssize_t writeAll(int fd, const void *buf, size_t count);

    static const char *getSignalName(int sig);

    static void generateBacktrace(int fd);

private:
    static QString m_crashLogPath;
    static bool m_logDirWritable;
};

#endif // ERRORHANDLER_H
