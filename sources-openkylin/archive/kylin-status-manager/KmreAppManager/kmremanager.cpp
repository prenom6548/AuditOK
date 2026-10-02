/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "kmremanager.h"
#include "kmrenotificationwindowmanager.h"

#include <QDebug>
#include <QDBusConnection>
#include <QString>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>

KmreManager::KmreManager(QObject *parent)
    : QObject(parent)
    , m_kmreNotificationWindowManager(new KmreNotificationWindowManager)
{
    initNotificationThread();
    connect(m_kmreNotificationWindowManager, &KmreNotificationWindowManager::canSwitchMode,
            this, [this](bool canModeSwicth) {
        emit canSwitchMode(canModeSwicth);
    });
    connectActiveWindowChangedSignal();
}

void KmreManager::connectActiveWindowChangedSignal()
{
    QDBusConnection::sessionBus().connect("org.ukui.KWin",
                                          "/KWin",
                                          "org.ukui.KWin",
                                          "activeWindowChanged", this,
                                          SLOT(handleAutoRotateSignal(int)));
}

void KmreManager::handleAutoRotateSignal(int pid)
{
    QString cmdLineFile = QString("/proc/%1/cmdline").arg(pid);
    QFile file(cmdLineFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "文件打开失败: " << pid << file.errorString();
        return ;
    }
    QByteArray cmd = file.readAll();
    if (cmd.contains("ukui-sidebar") || cmd.contains("ukui-kwin")) {
        return ;
    }

    if (cmd.contains("kylin-kmre-window")) {
        emit kmreAppEnableAutoRotate(true, false);
    } else {
        emit kmreAppRestoreAutoRotateStatus();
    }
}

void KmreManager::initNotificationThread()
{
    m_kmreNotificationWindowManager->moveToThread(&m_notificationThread);
    connect(this, &KmreManager::showNotificationWindow,
            m_kmreNotificationWindowManager, &KmreNotificationWindowManager::show);
    m_notificationThread.start();
}

bool KmreManager::kmreWindowExists() const
{
    QDBusInterface Kmreinterface("cn.kylinos.Kmre.Window",
                                 "/cn/kylinos/Kmre/Window",
                                 "cn.kylinos.Kmre.Window",
                                 QDBusConnection::sessionBus());
    if (!Kmreinterface.isValid()) {
        qDebug() << "Call to Kmre DBUS interface failed!!!"
                 << QDBusConnection::sessionBus().lastError().message();
        return false;
    }

    QDBusReply<QString> reply = Kmreinterface.call("getRunningAppList");
    QString kmreAppList;
    if (reply.isValid()) {
        kmreAppList = reply.value();
    } else {
        qDebug() << "Error calling the dbus interface of kmre!!!!!" << reply.error();
    }
    qDebug() << "The list of currently running kmre applications is :" << kmreAppList;
    return !kmreAppList.isEmpty();
}

void KmreManager::showModeSwitchPromptWindow(QString notificationText)
{
    Q_EMIT showNotificationWindow(notificationText);
}

