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
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <libkylog.h>
#include "SingleApplication.h"
#include "DataCommon.h"
#include "BasicInfoModule.h"


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
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    //  设置环境变量
    setenv("LANGUAGE","en",1);
    //  设置日志信息
    //  设置退出信号函数
    // trappingSignals();
    SingleApplication application(argc, argv);
    if(!application.isRunningFunction())
    {
        DataCommon dataCommon;
        return application.exec();
    }

    sleep(1);
    return 0;
}

