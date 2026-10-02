/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

/*
 * Log 类中实现了 Qt 的日志注册函数并且调用了 kysdk-log 库中的日志模块，调用方需要链接 Qt库 及 kysdk-log 库
 */

#ifndef LOG_HPP_
#define LOG_HPP_

#include <qapplication.h>
#include <libkylog.h>
#include <QString>
#include <QByteArray>

namespace kabase
{

class Log
{
public:
    Log() = default;
    ~Log() = default;

    static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *message = localMsg.constData();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";

        switch (type) {
        case QtDebugMsg:
            klog_debug("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtInfoMsg:
            klog_info("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtWarningMsg:
            klog_warning("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtCriticalMsg:
            klog_err("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        case QtFatalMsg:
            klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, function);
            break;
        }

        return;
    };
};

}

#endif
