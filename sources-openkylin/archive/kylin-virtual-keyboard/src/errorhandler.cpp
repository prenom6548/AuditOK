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

#include "errorhandler.h"

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <execinfo.h>

#include <iostream>

#define MINIMAL_BUFFER_SIZE 256
#define BACKTRACE_SIZE 32

QString ErrorHandler::m_crashLogPath;
bool ErrorHandler::m_logDirWritable = false;
const QString fileName = "kylin-virtual-keyboard-error.log";

void ErrorHandler::init() {
    createLogFile();

    registerSignalHandler();

    void *array[BACKTRACE_SIZE] = {
        nullptr,
    };
    (void)backtrace(array, BACKTRACE_SIZE);
}

void ErrorHandler::createLogFile() {
    QString homeDir =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QStringList logPaths;
    QString primaryLogPath = homeDir + "/.log/";
    QString fallbackLogPath =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
        "/kylin-virtual-keyboard/";
    logPaths << primaryLogPath << fallbackLogPath;

    for (const QString &logPath : logPaths) {
        QDir logDir(logPath);
        if (!logDir.exists()) {
            logDir.mkpath(".");
        }
        if (!QFileInfo(logPath).isWritable()) {
            continue;
        }

        m_crashLogPath = logPath + fileName;
        m_logDirWritable = true;
        break;
    }
}

void ErrorHandler::registerSignalHandler() {
    // 注册信号处理器
    // 只关心以下信号
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
}

void ErrorHandler::signalHandler(int sig) {
    if (m_crashLogPath.isEmpty()) {
        return;
    }

    if (m_logDirWritable == false) {
        return;
    }

    int fd = open(m_crashLogPath.toLocal8Bit().constData(),
                  O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return;
    }

    writeString(fd, "=========================\n");
    writeString(fd, "Kylin Virtual Keyboard -- Get Signal No.: ");
    writeUInt64(fd, sig);
    writeString(fd, " (");
    writeString(fd, getSignalName(sig));
    writeString(fd, ")\n");

    time_t t = time(nullptr);
    writeString(fd, "Date: ");
    writeUInt64(fd, t);
    writeString(fd, " (try \"date -d @");
    writeUInt64(fd, t);
    writeString(fd, "\" if you are using GNU date)\n");

    writeString(fd, "ProcessID: ");
    writeUInt64(fd, getpid());
    writeString(fd, "\n");

    generateBacktrace(fd);

    close(fd);

    signal(sig, SIG_DFL);
    raise(sig);
}

void ErrorHandler::writeUInt64(int fd, unsigned long long number) {
    char buffer[MINIMAL_BUFFER_SIZE]{0};
    int len = snprintf(buffer, sizeof(buffer), "%llu", number);
    if (len > 0 && len < (int)sizeof(buffer)) {
        writeBuffer(fd, buffer, len);
    }
}

ssize_t ErrorHandler::writeAll(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    while (count > 0) {
        ssize_t written = write(fd, p, count);
        if (written == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += written;
        count -= written;
    }
    return 0;
}

const char *ErrorHandler::getSignalName(int sig) {
    switch (sig) {
    case SIGSEGV:
        return "SIGSEGV";
    case SIGFPE:
        return "SIGFPE";
    case SIGILL:
        return "SIGILL";
    case SIGABRT:
        return "SIGABRT";
    default:
        return "OTHER";
    }
}

void ErrorHandler::generateBacktrace(int fd) {
    void *array[BACKTRACE_SIZE]{
        nullptr,
    };
    int size = backtrace(array, BACKTRACE_SIZE);

    writeString(fd, "\nBacktrace:\n");
    writeString(fd, "Backtrace size: ");
    writeUInt64(fd, size);
    writeString(fd, "\n");
    writeString(fd, "\nBacktrace addresses:\n");

    backtrace_symbols_fd(array, size, fd);
}

void ErrorHandler::writeString(int fd, const char *str) {
    writeAll(fd, str, strlen(str));
}

void ErrorHandler::writeBuffer(int fd, const char *buffer, int len) {
    writeAll(fd, buffer, len);
}