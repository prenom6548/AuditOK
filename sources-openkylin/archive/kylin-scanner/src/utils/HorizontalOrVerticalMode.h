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

#ifndef HORVMODE_H
#define HORVMODE_H

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>
#include <QTimer>

enum deviceMode{
    PADHorizontalMode = 1,//平板横屏
    PADVerticalMode = 2,//平板竖屏
    PCMode = 3     //pc模式
};

class HorizontalOrVerticalMode : public QObject
{
    Q_OBJECT
    bool m_isPadMode = false;
    deviceMode m_statePre;
    deviceMode m_stateAfter;
    QTimer m_timer;
    void antiShake();
public:
    HorizontalOrVerticalMode();
    deviceMode defaultModeCapture();
Q_SIGNALS:
    void RotationSig(deviceMode);//true:horizontal;false:vertical
private Q_SLOTS:
    void rotationChanged(QString res);
    void modeChanged(bool res);
    void timeOutSlot();
};

#endif // HORVMODE_H
