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
#ifndef ROTATECHANGEINFO_H
#define ROTATECHANGEINFO_H

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
#include <QScreen>
#include <QPair>

static const QString KYLIN_ROTATION_PATH = "/";

static const QString KYLIN_ROTATION_SERVICE = "com.kylin.statusmanager.interface";

static const QString KYLIN_ROTATION_INTERFACE = "com.kylin.statusmanager.interface";

class RotateChangeInfo : public QObject
{
    Q_OBJECT
public:
    explicit RotateChangeInfo(QObject *parent = nullptr);
    QPair<int,int> getCurrentScale();
    bool getCurrentMode();
    bool m_existDbus = true;

private:
    int m_screenWidth;
    int m_screenHeight;
    bool m_isPCMode;

signals:
    void sigRotationChanged(bool mode,int scale_W,int scale_H);

public slots:
    void slotRotateChanged();
    void slotModeChanged(bool isPadMode);

};

#endif // ROTATECHANGEINFO_H
