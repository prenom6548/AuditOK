/*
 * Copyright 2024 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYSLOGHANDLER_HPP
#define SYSLOGHANDLER_HPP

#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QDebug>
#include <syslog.h>

class SyslogHandler
{
public:
    static void staticMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QMutexLocker locker(&mutex);
        int syslogPriority;
        switch (type) {
        case QtDebugMsg:
            syslogPriority = LOG_DEBUG;
            break;
        case QtInfoMsg:
            syslogPriority = LOG_INFO;
            break;
        case QtWarningMsg:
            syslogPriority = LOG_WARNING;
            break;
        case QtCriticalMsg:
            syslogPriority = LOG_CRIT;
            break;
        case QtFatalMsg:
            syslogPriority = LOG_ALERT;
//            abort(); // 通常，QtFatalMsg 会导致程序终止，所以我们在这里也调用 abort()
            break;
        default:
            syslogPriority = LOG_NOTICE; // 默认情况下使用 LOG_NOTICE
            break;
        }
        syslog(syslogPriority, "%s", msg.toUtf8().constData());
    }

    static SyslogHandler* instance()
    {
        static SyslogHandler singleton;
        return &singleton;
    }

private:
    SyslogHandler() {} // 私有构造函数，防止外部实例化
    ~SyslogHandler(); // 私有析构函数，防止外部删除

    static QMutex mutex; // 用于线程安全的静态成员变量
};



#endif // SYSLOGHANDLER_HPP
