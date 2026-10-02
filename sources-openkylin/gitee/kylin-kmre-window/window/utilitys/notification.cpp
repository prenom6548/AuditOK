/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#include "notification.h"
#include "window/kmrewindow.h"
#include "app_control_manager.h"

#include <QLocale>
#include <QApplication>
#include <syslog.h>
#include <unistd.h>

Notification::Notification(KmreWindow* window, QObject* parent) 
    : QObject(parent)
    , mMainWindow(window) 
    , mFlipState(false)
    , mNotifyReplacesId(0)
    , mNotifyThread(new QThread())
    , mFlickerTimer(new QTimer(this))
    , mIsNewVersion(false)
    , mKeyApp("[" + window->getPackageName() + "]")
{
    mIconBlank = QIcon(":/res/high_light.png");
    mIconPath = window->getIconPath();
    mIconApp = QIcon(mIconPath);

    this->moveToThread(mNotifyThread);
    connect(mNotifyThread, SIGNAL(started()), this, SLOT(onInitNotificationDbus()), Qt::QueuedConnection);
    //connect(mNotifyThread, &QThread::finished, this, &DbusClient::deleteLater);
    connect(mNotifyThread, &QThread::finished, mNotifyThread, &QThread::deleteLater);
    mNotifyThread->start();
    
    mFlickerTimer->setInterval(500);// default 500ms
    connect(mFlickerTimer, &QTimer::timeout, this, [&] {
        if (mFlipState) {
            mMainWindow->setWindowIcon(mIconBlank);
            mMainWindow->setTrayIcon(mIconBlank);
        }
        else {
            mMainWindow->setWindowIcon(mIconApp);
            mMainWindow->setTrayIcon(mIconApp);
        }
        mFlipState = !mFlipState;
    });

    connect(this, SIGNAL(openNotification(bool, const QString&, const QString &)), 
        this, SLOT(onOpenNotification(bool, const QString &, const QString &)), Qt::QueuedConnection);
    connect(this, SIGNAL(closeNotification()), this, SLOT(onCloseNotification()), Qt::QueuedConnection);
}

Notification::~Notification()
{
    mFlickerTimer->stop();
    if (mNotifyThread) {
        mNotifyThread->quit();
        mNotifyThread->wait();
    }
}

void Notification::onInitNotificationDbus()
{
    mNotificationInterface = std::make_unique<QDBusInterface>("org.freedesktop.Notifications",
                                                            "/org/freedesktop/Notifications",
                                                            "org.freedesktop.Notifications",
                                                            QDBusConnection::sessionBus());
    if (!mNotificationInterface) {
        syslog(LOG_ERR, "[%s] Get system notification service failed!", __func__);
        return;
    }

    if (QDBusConnection::sessionBus().connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", 
            "ActionInvoked", this, SLOT(onNotificationActionInvoked(quint32, QString)))) {
        mIsNewVersion = true;
    }

    syslog(LOG_INFO, "[%s] Using Notification %s version.", __func__, mIsNewVersion ? "new" : "old");
}

// 任务栏图标闪烁
void Notification::startFlicker() 
{
    mFlickerTimer->start();
}

void Notification::stopFlicker() 
{
    if (mFlickerTimer->isActive()) {
        mFlickerTimer->stop();
    }
    mMainWindow->setWindowIcon(mIconApp);
    mMainWindow->setTrayIcon(mIconApp);
}

void Notification::onOpenNotification(bool call, const QString &title, const QString &text)
{
    if (!mNotificationInterface) {
        return;
    }

    startFlicker();

    //notify-send -a foo -i "/usr/share/icons/Adwaita/scalable/apps/kmre.svg" Hello KMRE!
    QString pkgName = mMainWindow->getPackageName();

    QList<QVariant> args;
    QStringList actions;
#ifndef KYLIN_V10
    if (call) {
        if (mIsNewVersion) {
            actions.append(mKeyApp + QString::number(eAcceptCall)); // key
            actions.append(tr("accept"));
            actions.append(mKeyApp + QString::number(eRejectCall)); // key
            actions.append(tr("reject"));
        }
        else {
            const QString cmd = "dbus-send --session --type=method_call --dest=cn.kylinos.Kmre.Window \
                                /cn/kylinos/Kmre/Window cn.kylinos.Kmre.Window.answerCall";
            actions.append(cmd + " boolean:true");
            actions.append(tr("accept"));
            actions.append(cmd + " boolean:false");
            actions.append(tr("reject"));
        }
    }
    else {
        if (mIsNewVersion) {
            actions.append(mKeyApp + QString::number(eActiveWindow));
        }
        else {
            actions.append(QString("startapp %1").arg(pkgName));
        }
        actions.append(tr("confirm"));       //默认动作
    }
#endif

    QVariantMap hints;//QMap<QString, QVariant> hints;
    QString name = mMainWindow->getTitleName();
    QString message;
    if (call) {
        message.append(title + " ");
    }
    message.append(text.isEmpty() ? tr("You have new news") : text);

    args << name                               // appname
        << ((unsigned int) mNotifyReplacesId)  // id
        << mIconPath                           // icon
        << (name + tr(" Message"))             // summary (notification title)
        << message                             // body
        << actions                             // actions
        << hints                               // hints
        << (call ? 5000 : -1);

    QDBusMessage response = mNotificationInterface->callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if (response.type() == QDBusMessage::ReplyMessage) { //判断method的返回值
        mNotifyReplacesId = response.arguments().takeFirst().toUInt(); //保存消息ID
        syslog(LOG_DEBUG, "[%s] Send notify succeed.", __func__);
    }
    else{
        syslog(LOG_ERR, "[%s] Send notify failed!", __func__);
    }
}

void Notification::onCloseNotification()
{
    stopFlicker();
    if (mNotificationInterface && (mNotifyReplacesId != 0)) {
        QList<QVariant> args;
        args << mNotifyReplacesId; //UUID为要关闭通知的ID号
        mNotificationInterface->callWithArgumentList(QDBus::AutoDetect,"CloseNotification", args);
    }
}

void Notification::startNotification(bool call, const QString &title, const QString &text)
{
    // 任务栏图标闪烁
    QApplication::alert(mMainWindow);
    emit openNotification(call, title, text);
}

void Notification::stopNotification()
{
    emit closeNotification();
}

void Notification::onNotificationActionInvoked(quint32 id, QString key)
{
    //syslog(LOG_DEBUG, "[%s] id = '%d', key = '%s'", __func__, id, key.toStdString().c_str());
    
    key.trimmed();
    if (key.startsWith(mKeyApp)) {
        key.remove(mKeyApp);

        syslog(LOG_DEBUG, "[%s] App = '%s', key = '%s'", __func__, mKeyApp.toStdString().c_str(), key.toStdString().c_str());
        if (key == QString::number(eActiveWindow)) {
            mMainWindow->showMainWindow();
        }
        else if (key == QString::number(eAcceptCall)) {
            AppControlManager::getInstance()->answerCall(true);
        }
        else if (key == QString::number(eRejectCall)) {
            AppControlManager::getInstance()->answerCall(false);
        }
    }
}