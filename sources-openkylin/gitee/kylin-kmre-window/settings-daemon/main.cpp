/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QtDBus>

#include "prefmanager.h"

static QMutex mutex;
static QtMessageHandler system_default_message_handler = NULL;
void CustomMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& info) 
{
    QString log = QString::fromLocal8Bit("msg-[%1], file-[%2], func-[%3], category-[%4]\n")
            .arg(info).arg(context.file).arg(context.function).arg(context.category);
    bool bok = true;

    switch (type) {
    case QtDebugMsg:
        log.prepend("[QDEBUG]:");
        break;
    case QtWarningMsg:
        log.prepend("[QWARN]:");
        break;
    case QtCriticalMsg:
        log.prepend("[QCRITICAL]:");
        break;
    case QtFatalMsg:
        log.prepend("[QFATAL]:");
        break;
    case QtInfoMsg:
        log.prepend("[QINFO]:");
        break;
    default:
        bok = false;
        break;
    }

    if(bok){
        QMutexLocker locker(&mutex);
        QString str_file_name = "/var/log/kylin-kmre-settings-daemon.log";

        QFile file(str_file_name);

        if(!file.open(QFile::ReadWrite | QFile::Append)) {
            return;
        }

        file.write(log.toLocal8Bit().data());
        file.close();
    }

    if(bok) {
        if(NULL != system_default_message_handler) {
            system_default_message_handler(type, context, log);
        }
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    // debug log
    system_default_message_handler = qInstallMessageHandler(CustomMessageHandler);

    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.registerService(SERVICE_NAME)) {
        qDebug() << "Failed to register service";
        return -1;
    }

    PrefManager service;
    service.connect(&app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
    if (!connection.registerObject(SERVICE_PATH, SERVICE_INTERFACE, &service, 
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << "Failed to register object";
        return -1;
    }

    return app.exec();
}

