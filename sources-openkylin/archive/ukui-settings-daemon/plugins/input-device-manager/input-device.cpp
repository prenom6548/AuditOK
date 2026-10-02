/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "input-device.h"
#include "input-gsettings.h"
#include "input-device-function.h"
extern "C" {
#include "clib-syslog.h"
}
extern const DeviceFunctionMap deviceFuncMap;
InputDevice::InputDevice(QVariant id, DeviceType type, QString name, QObject *parent)
    :QObject(parent)
    , m_deviceId(id)
    , m_type(type)
    , m_deviceName(name)
{

}

bool InputDevice::isMouse()
{
    return m_type == DeviceType::IN_MOUSE;
}

bool InputDevice::isTouchpad()
{
    return m_type == DeviceType::IN_TOUCHPAD;
}

void InputDevice::initDeviceProperty()
{
    //新增设备时初始化属性列表，根据gsetings 配置设置属性。
    QList<QString> keys = InputGsettings::instance()->getGsettingsKeys(m_type);
    if (keys.isEmpty()) {
        USD_LOG(LOG_DEBUG,"get gsettings keys is empty .");
        return;
    }
    for (const QString& key : keys) {
        QVariant value = getGsettingsValue(key);
        if (key == QStringLiteral(KEY_MOUSE_LOCATE_POINTER)) {
            InputDeviceFunction::setLocatePointer(value);
        } else {
            auto func = deviceFuncMap.value(key);
            if (func) {
                func(value,this);
            }
        }
    }
}

QVariant InputDevice::getGsettingsValue(const QString &key)
{
    return InputGsettings::instance()->getGsettingsValue(key, m_type);
}

void InputDevice::setGsettingsValue(const QString &key, const QVariant &value)
{
    InputGsettings::instance()->setGsettingsValue(key, value, m_type);
}
