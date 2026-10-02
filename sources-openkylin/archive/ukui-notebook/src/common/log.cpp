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

#include <QThread>
#include <QDir>
#include <QDebug>

#include <libkylog.h>

#include "common.h"
#include "log.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "[qDebug] %s:%u\t%s:\n", QString(context.file).remove(QDir::currentPath() + "/").toStdString().c_str(), context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stdout, "[qWarning] %s:%u\t%s:\n", QString(context.file).remove(QDir::currentPath() + "/").toStdString().c_str(), context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stdout, "[qCritical] %s:%u\t%s:\n", QString(context.file).remove(QDir::currentPath() + "/").toStdString().c_str(), context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stdout, "[qFatal] %s:%u\t%s:\n", QString(context.file).remove(QDir::currentPath() + "/").toStdString().c_str(), context.line, context.function);
        break;
    default:
        break;
    }
    qDebug () << "thread: " << QThread::currentThread();
    fprintf(stdout, "\t %s\n",localMsg.constData());
    return ;
}

void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString qstr = QString("%1 (%2:%3,%4)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
    const char *str = QTC(qstr);
    switch (type) {
    case QtDebugMsg:
        klog_debug("%s\n", str);
        break;
    case QtInfoMsg:
        klog_info("%s\n", str);
        break;
    case QtWarningMsg:
        klog_warning("%s\n", str);
        break;
    case QtCriticalMsg:
        klog_err("%s\n", str);
        break;
    case QtFatalMsg:
        klog_emerg("%s\n", str);
    default:
        break;
    }

    return;
}
