/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef HORIZONTALORVERTICALMODE_H
#define HORIZONTALORVERTICALMODE_H

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

enum deviceMode {
    PADHorizontalMode = 1, //平板横屏
    PADVerticalMode = 2,   //平板竖屏
    PCMode = 3             // pc模式
};

class HorizontalOrVerticalMode : public QObject
{
    Q_OBJECT
public:
    HorizontalOrVerticalMode();
    deviceMode defaultModeCapture();
Q_SIGNALS:
    void RotationSig(deviceMode); // true:horizontal;false:vertical
private Q_SLOTS:
    void rotationChanged(QString res);
    void modeChanged(bool res);
};


#endif // HORIZONTALORVERTICALMODE_H
