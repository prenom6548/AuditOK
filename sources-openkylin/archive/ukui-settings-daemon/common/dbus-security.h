/* -*- Mode: C; indent-tabs-mode: nil; tab-width: 4 -*-
* -*- coding: utf-8 -*-
*
* Copyright (C) 2020 KylinSoft Co., Ltd.
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

#ifndef DBUSSECURITY_H
#define DBUSSECURITY_H

#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>
#include <errno.h>
#include <QStringList>
#include "clib-syslog.h"

#define SSC_CUSTOMIZED_CONFIG_DIR   "/usr/share/ukui-config/%1/"
static const QStringList programWhiteList={ //管控接口可调用的组件list
    "/usr/bin/ukui-settings-daemon",
    "/usr/lib/qt5/bin/qdbusviewer",
    "/usr/bin/save-param",
};

#if 1
#define  checkEnviron(ret,pid)    \
    do {   \
        QString filePath = QString("/proc/%1/environ").arg(pid);    \
        QFile file(filePath);   \
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {    \
        qDebug() << "file.open failed" << filePath; \
        ret = false;  break; \
    }   \
    QTextStream in(&file);  \
    QString data = in.readAll();    \
    file.close();   \
    data.replace('\0', '\n');   \
    QStringList environVars = data.split('\n'); \
    for (const QString &var : environVars){ \
        if (!var.isEmpty()) {   \
            int pos = var.indexOf('='); \
            if (pos != -1) {    \
                QString key = var.left(pos);    \
                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT"){  \
                    SYS_LOG(LOG_DEBUG,"checkEnviron error!key:%s",key.toLatin1().data());\
                    ret = false;break;   \
                }   \
            }   \
        }   \
    }   \
    ret = true;    \
    }while(0);

#define isAllowedCaller(a)   \
do {   \
    bool macroRet = false;\
    QString execPath="";    \
    QString clientService = message().service();    \
    QDBusConnectionInterface *intf = connection().interface();  \
    unsigned int  pid = intf->servicePid(clientService).value();    \
    checkEnviron(macroRet, pid);     \
    if(!macroRet) {   \
        SYS_LOG(LOG_DEBUG,"clientService error!clientService:%s",clientService.toLatin1().data());\
        sendErrorReply(QDBusError::ErrorType::Failed,QString("[%1] dbus method control,env forbidden").arg((pid))); \
        a = false; \
        break;   \
    }  \
    QFileInfo file(QString("/proc/%1/exe").arg(pid));   \
    if (file.exists()) {    \
        execPath = file.canonicalFilePath(); \
        bool hadBreakIntisAllowedCaller = false;    \
        Q_FOREACH (const QString& whitePath,programWhiteList){  \
            if (execPath == whitePath){ \
                  SYS_LOG(LOG_DEBUG,"ready check execPath right!clientService%s:%s--->%s",file.path().toLatin1().data(),execPath.toLatin1().data(),whitePath.toLatin1().data());    \
                a = true;  hadBreakIntisAllowedCaller = true; break;\
            }   \
            else {  \
                    SYS_LOG(LOG_DEBUG,"ready check execPath error!clientService%s:%s--->%s",file.path().toLatin1().data(),execPath.toLatin1().data(),whitePath.toLatin1().data());    \
            }           \
        }   \
        if (hadBreakIntisAllowedCaller) break;  \
    }   \
   sendErrorReply(QDBusError::ErrorType::Failed,QString("[%1] dbus method control,operation not permitted").arg((execPath)));   \
   a = false;    \
} while(0);

#define PEEK_LIMIT() bool checret; \
                            isAllowedCaller(checret)  \
                            if (!checret) return;

#define PEEK_LIMIT_RET(a)   bool checret; \
                            isAllowedCaller(checret)  \
                            if (!checret) {return a;}
#endif

#endif // DBUSSECURITY_H
