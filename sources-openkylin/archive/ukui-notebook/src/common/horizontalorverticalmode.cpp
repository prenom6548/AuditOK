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

#include "horizontalorverticalmode.h"

static const QString KYLIN_ROTATION_PATH = "/";

static const QString KYLIN_ROTATION_SERVICE = "com.kylin.statusmanager.interface";

static const QString KYLIN_ROTATION_INTERFACE = "com.kylin.statusmanager.interface";

HorizontalOrVerticalMode::HorizontalOrVerticalMode()
{
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("rotations_change_signal"), this, SLOT(rotationChanged(QString)));
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(modeChanged(bool)));
}
deviceMode HorizontalOrVerticalMode::defaultModeCapture()
{ // method
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(
        KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (!reply_pcORpad.isValid()) {
        return PCMode;
    }
    if (reply_pcORpad.value()) { //平板模式
        QDBusMessage message = QDBusMessage::createMethodCall(
            KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            return PADHorizontalMode; //横屏
        } else {
            return PADVerticalMode; //竖屏
        }
        return PADHorizontalMode; //横屏

    } else {
        return PCMode; // pc模式
    }
    return PCMode; // pc模式
}

void HorizontalOrVerticalMode::rotationChanged(QString res)
{ // signal
    if (res == QString::fromLocal8Bit("normal") || res == QString::fromLocal8Bit("upside-down")) {
        Q_EMIT RotationSig(PADHorizontalMode); //平板横屏
    } else {
        Q_EMIT RotationSig(PADVerticalMode); //平板竖屏
    }
}
void HorizontalOrVerticalMode::modeChanged(bool res)
{
    if (res) {
        QDBusMessage message = QDBusMessage::createMethodCall(
            KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            Q_EMIT RotationSig(PADHorizontalMode); //平板横屏

        } else {
            Q_EMIT RotationSig(PADVerticalMode); //平板竖屏
        }
    } else {
        Q_EMIT RotationSig(PCMode); // pc模式
    }
    Q_EMIT RotationSig(PCMode); // pc模式
}
