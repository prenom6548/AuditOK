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

#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <QIcon>
#include <QThread>
#include <QDBusInterface>
#include <memory>

class KmreWindow;

class Notification : public QObject
{
    Q_OBJECT
public:
    explicit Notification(KmreWindow* window, QObject* parent = nullptr);
    ~Notification();

    void startNotification(bool call = false, const QString &title = "", const QString &text = "");
    void stopNotification();

signals:
    void openNotification(bool call, const QString &title, const QString &text);
    void closeNotification();

private slots:
    void onOpenNotification(bool call, const QString &title, const QString &text);
    void onCloseNotification();
    void onInitNotificationDbus();

private slots:
    void onNotificationActionInvoked(quint32 id, QString key);

private:
    void startFlicker();
    void stopFlicker();

private:
    QString mIconPath;
    QIcon mIconApp;
    QIcon mIconBlank;
    bool mFlipState;
    int mNotifyReplacesId;
    QThread* mNotifyThread;
    KmreWindow* mMainWindow = nullptr;
    QTimer* mFlickerTimer = nullptr;
    std::unique_ptr<QDBusInterface> mNotificationInterface;
    bool mIsNewVersion;
    const QString mKeyApp;
    enum {
        eUnknow = -1,
        eActiveWindow = 0,
        eAcceptCall = 1,
        eRejectCall = 2,
        eTypeMax,
    };
};