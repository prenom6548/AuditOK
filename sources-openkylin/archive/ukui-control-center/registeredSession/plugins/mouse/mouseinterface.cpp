/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mouseinterface.h"

#include <QDebug>
MouseInterface::MouseInterface():UKUI::ServiceObject(nullptr)
{
    ukuiMouseGsettings     = UniversalInterface::self()->ukuiMouseGsettings();
    mateInterfaceGsettings = UniversalInterface::self()->mateInterfaceGsettings();
    ukuiStyleGsettings     = UniversalInterface::self()->ukuiStyleGsettings();

    m_pKwinInterface = new QDBusInterface("com.kylin.Wlcom",
                                       "/com/kylin/Wlcom/Effect",
                                       "com.kylin.Wlcom.Effect",
                                       QDBusConnection::sessionBus());

    initConnect();
}

MouseInterface::~MouseInterface()
{

}

void MouseInterface::initConnect()
{
    connect(ukuiMouseGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == DOMINANT_HAND_KEY) {
            Q_EMIT changed("dominantHand");
        } else if (key == SCROLL_DIRECTION_KEY) {
            Q_EMIT changed("scrollDirection");
        } else if (key == WHEEL_SPEED_KEY) {
            Q_EMIT changed("wheelSpeed");
        } else if (key == DOUBLECLICK_KEY) {
            Q_EMIT changed("doubleClick");
        } else if (key == POINTER_SPEED_KEY) {
            Q_EMIT changed("pointerSpeed");
        } else if (key == MOUSE_ACCELERATION_KEY) {
            Q_EMIT changed("mouseAcceleration");
        } else if (key == POINTER_POSITION_KEY) {
            Q_EMIT changed("pointerPosition");
        } else if (key == POINTER_SIZE_KEY) {
            Q_EMIT changed("pointerSize");
        } else if (key == SHAKE_CURSOR) {
            Q_EMIT changed("shakeCursor");
        }
    });

    connect(mateInterfaceGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == CURSOR_BLINK_KEY) {
            Q_EMIT changed("cursorBlink");
        } else if (key == CURSOR_SPEED_KEY) {
            Q_EMIT changed("cursorSpeed");
        }
    });
}

// 对属性的访问函数
const bool MouseInterface::getDominantHand()
{
    if (ukuiMouseGsettings->keys().contains(DOMINANT_HAND_KEY)) {
        return ukuiMouseGsettings->get("left-handed").toBool();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << DOMINANT_HAND_KEY;
    }
    return bool();
}

const bool MouseInterface::getScrollDirection()
{
    if (ukuiMouseGsettings->keys().contains(SCROLL_DIRECTION_KEY)) {
        return ukuiMouseGsettings->get("natural-scroll").toBool();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << SCROLL_DIRECTION_KEY;
    }
    return bool();
}

const int MouseInterface::getWheelSpeed()
{
    if (ukuiMouseGsettings->keys().contains(WHEEL_SPEED_KEY)) {
        return ukuiMouseGsettings->get("wheel-speed").toInt();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << WHEEL_SPEED_KEY;
    }
    return int();
}

const int MouseInterface::getDoubleClick()
{
    if (ukuiMouseGsettings->keys().contains(DOUBLECLICK_KEY)) {
        return ukuiMouseGsettings->get("double-click").toInt();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << DOUBLECLICK_KEY;
    }
    return int();
}

const double MouseInterface::getPointerSpeed()
{
    if (ukuiMouseGsettings->keys().contains(POINTER_SPEED_KEY)) {
        return ukuiMouseGsettings->get("motion-acceleration").toDouble();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_SPEED_KEY;
    }
    return double();
}

const bool MouseInterface::getMouseAcceleration()
{
    if (ukuiMouseGsettings->keys().contains(MOUSE_ACCELERATION_KEY)) {
        return ukuiMouseGsettings->get("mouse-accel").toBool();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << MOUSE_ACCELERATION_KEY;
    }
    return bool();
}


bool MouseInterface::isEffectSupported()
{
    QDBusReply<bool> isEffectSupportedReply = UniversalInterface::self()->wlcomDbus()->call("IsHardwareRendering");
    qDebug() << Q_FUNC_INFO << "isEffectSupported" << isEffectSupportedReply.value();
    return isEffectSupportedReply.value();
}

const bool MouseInterface::getTailLightEffect()
{
    QDBusReply<QDBusArgument> reply = m_pKwinInterface->call("ListAllEffects");

    if (reply.isValid()) {
        QDBusArgument argument = reply.value();

        // 解析数组
        argument.beginArray();
        while (!argument.atEnd()) {
            EffectStruct item;
            argument >> item;

            qDebug() << "Name:" << item.name << "Value:" << item.value << "Flag:" << item.flag;

            if (item.name == "mouse_trail") {
                return item.flag;
            }
        }
        argument.endArray();
    } else {
        return false;
        qDebug() << "Error:" << reply.error().message();
    }

    return bool();
}

const bool MouseInterface::getPointerPosition()
{
    if (ukuiMouseGsettings->keys().contains(POINTER_POSITION_KEY)) {
        return ukuiMouseGsettings->get("locate-pointer").toBool();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_POSITION_KEY;
    }
    return bool();
}

const bool MouseInterface::getShakeCursor()
{
    if (ukuiMouseGsettings->keys().contains(SHAKE_CURSOR)) {
        return ukuiMouseGsettings->get("shake-cursor").toBool();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << SHAKE_CURSOR;
    }
    return bool();
}

const int  MouseInterface::getPointerSize()
{
    if (ukuiMouseGsettings->keys().contains(POINTER_SIZE_KEY)) {
        return ukuiMouseGsettings->get("cursor-size").toInt();
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_SIZE_KEY;
    }
    return int();
}

const bool MouseInterface::getCursorBlink()
{
    if (mateInterfaceGsettings->keys().contains(CURSOR_BLINK_KEY)) {
        return mateInterfaceGsettings->get("cursor-blink").toBool();
    } else {
        qCritical() << "mateInterfaceGsettings not contains the key: " << CURSOR_BLINK_KEY;
    }
    return bool();
}

const int MouseInterface::getCursorSpeed()
{
    if (mateInterfaceGsettings->keys().contains(CURSOR_SPEED_KEY)) {
        return mateInterfaceGsettings->get("cursor-blink-time").toInt();
    } else {
        qCritical() << "mateInterfaceGsettings not contains the key: " << CURSOR_SPEED_KEY;
    }
    return int();
}

void MouseInterface::setDominantHand(const bool rightHand)
{
    if (ukuiMouseGsettings->keys().contains(DOMINANT_HAND_KEY)) {
        ukuiMouseGsettings->set("left-handed", rightHand);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << DOMINANT_HAND_KEY;
    }
}

void MouseInterface::setScrollDirection(const bool reverse)
{
    if (ukuiMouseGsettings->keys().contains(SCROLL_DIRECTION_KEY)) {
        ukuiMouseGsettings->set("natural-scroll", reverse);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << SCROLL_DIRECTION_KEY;
    }
}

void MouseInterface::setWheelSpeed(const int value)
{
    if (ukuiMouseGsettings->keys().contains(WHEEL_SPEED_KEY)) {
        ukuiMouseGsettings->set("wheel-speed", value);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << WHEEL_SPEED_KEY;
    }
}

void MouseInterface::setDoubleClick(const int value)
{
    if (ukuiMouseGsettings->keys().contains(DOUBLECLICK_KEY)) {
        ukuiMouseGsettings->set("double-click", value);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << DOUBLECLICK_KEY;
    }
}

void MouseInterface::setPointerSpeed(const double value)
{
    if (ukuiMouseGsettings->keys().contains(POINTER_SPEED_KEY)) {
        ukuiMouseGsettings->set("motion-acceleration", value);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_SPEED_KEY;
    }
}

void MouseInterface::setMouseAcceleration(const bool checked)
{
    if (ukuiMouseGsettings->keys().contains(MOUSE_ACCELERATION_KEY)) {
        ukuiMouseGsettings->set("mouse-accel", checked);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << MOUSE_ACCELERATION_KEY;
    }
}

void MouseInterface::setTailLightEffect(const bool checked)
{
    m_pKwinInterface->call("EnableEffect", "mouse_trail", checked);
    int value = (checked ? 1 : 0);
    QDBusReply<void> reply = m_pKwinInterface->call("SetEffectOption", "mouse_trail", "force_enabled", QVariant::fromValue(QDBusVariant(value)));
    if (!reply.isValid()) {
        qWarning() << "D-Bus call failed:" << reply.error().message();
        // 处理错误
    } else {
        qDebug() << "Mouse trail enabled:" << checked;
    }
}

void MouseInterface::setPointerPosition(const bool checked)
{
    if (ukuiMouseGsettings->keys().contains(POINTER_POSITION_KEY)) {
        ukuiMouseGsettings->set("locate-pointer", checked);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_POSITION_KEY;
    }
}

void MouseInterface::setShakeCursor(const bool checked)
{
    if (ukuiMouseGsettings->keys().contains(SHAKE_CURSOR)) {
        ukuiMouseGsettings->set("shake-cursor", checked);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << SHAKE_CURSOR;
    }
}
void MouseInterface::setPointerSize(const int value)
{
    if (ukuiMouseGsettings->keys().contains(POINTER_SIZE_KEY)) {
        ukuiMouseGsettings->set("cursor-size", value);
    } else {
        qCritical() << "ukuiMouseGsettings not contains the key: " << POINTER_SIZE_KEY;
    }
}

void MouseInterface::setCursorBlink(const bool checked)
{
    if (mateInterfaceGsettings->keys().contains(CURSOR_BLINK_KEY)) {
        mateInterfaceGsettings->set("cursor-blink", checked);
    } else {
        qCritical() << "mateInterfaceGsettings not contains the key: " << CURSOR_BLINK_KEY;
    }

    if (ukuiStyleGsettings->keys().contains(CURSOR_BLINK_KEY)) {
        ukuiStyleGsettings->set("cursor-blink", checked);
    } else {
        qCritical() << "ukuiStyleGsettings not contains the key: " << CURSOR_BLINK_KEY;
    }
}

void MouseInterface::setCursorSpeed(const int value)
{
    if (mateInterfaceGsettings->keys().contains(CURSOR_SPEED_KEY)) {
        mateInterfaceGsettings->set("cursor-blink-time", value);
    } else {
        qCritical() << "mateInterfaceGsettings not contains the key: " << CURSOR_SPEED_KEY;
    }

    if (ukuiStyleGsettings->keys().contains(CURSOR_BLINK_KEY)) {
        ukuiStyleGsettings->set("cursor-blink-time", value);
    } else {
        qCritical() << "ukuiStyleGsettings not contains the key: " << CURSOR_BLINK_KEY;
    }
}

void MouseInterface::setStyleCursorSpeed(const int value)
{
    if (ukuiStyleGsettings->keys().contains(CURSOR_BLINK_KEY)) {
        ukuiStyleGsettings->set("cursor-blink-time", value);
    } else {
        qCritical() << "ukuiStyleGsettings not contains the key: " << CURSOR_BLINK_KEY;
    }
}

void MouseInterface::resetKey(QString key)
{
    if (key == "wheelSpeed") {
        ukuiMouseGsettings->reset(WHEEL_SPEED_KEY);
    } else if (key == "cursorSize") {
        ukuiMouseGsettings->reset(POINTER_SIZE_KEY);
    }
}
