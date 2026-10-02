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
#include "input-device-manager.h"
#include "input-device-helper.h"
#include "input-device-function.h"
#include "input-wlcom-helper.h"

#include "device-monitor.h"
#include <QDBusReply>
#define TEST_PRINT
using namespace InputDeviceHelper;

extern const DeviceFunctionMap deviceFuncMap;
bool InputDeviceManager::s_mousePresent = false;

InputDeviceManager::InputDeviceManager(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
}

InputDeviceManager::~InputDeviceManager()
{
    disconnectAll();
    clearUpDeviceList();
}

void InputDeviceManager::start()
{
    connect(m_timer, &QTimer::timeout, this, &InputDeviceManager::managerStart);
    m_timer->start();
}

void InputDeviceManager::stop()
{
    disconnectAll();
    clearUpDeviceList();
}

bool InputDeviceManager::mousePresent()
{
    return s_mousePresent;
}

void InputDeviceManager::setMousePresent(bool value)
{
    s_mousePresent = value;
}

QString InputDeviceManager::GetDevInfoByProductId(uint prodId)
{
    DeviceList allDev = m_keyboardList + m_mouseList + m_touchpadList;
    QString info;
    for (InputDevice* dev : allDev) {
        if (dev->getProductId().toUInt() == prodId) {
            info = "Device name is " + dev->getDeviceName();
            info += QString("\n dev type from input is %1").arg((uint)dev->getDeviceType());
            info += QString("\n dev id is %1").arg(dev->getDeviceId().toUInt());
            info += QString("\n dev productId is %1").arg(dev->getProductId().toUInt());
            break;
        }
    }
    info += QString("\n dev type from devMonit0r is %1").arg((uint)DeviceMonitor::instance()->getDeviceType(prodId));
    return info;
}

QString InputDeviceManager::GetDevTypeMap()
{
    return DeviceMonitor::instance()->getDeviceTypeMap();
}

int InputDeviceManager::GetDevCountByType(const QString &devType) const
{
    if (!devType.compare("mouse", Qt::CaseInsensitive)) {
        return m_mouseProductIdList.count();
    } else if (!devType.compare("touchpad", Qt::CaseInsensitive)) {
        return m_touchpadProductIdList.count();
    } else if (!devType.compare("kbd", Qt::CaseInsensitive) || !devType.compare("keyboard", Qt::CaseInsensitive)) {
        return m_keyboardProductIdList.count();
    } else if (!devType.compare("trackpoint", Qt::CaseInsensitive)) {
        return m_trackpointProductList.count();
    }

    return -1;
}

void InputDeviceManager::registerDbusService()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("org.ukui.SettingsDaemon")) {
        sessionBus.registerObject("/org/ukui/SettingsDaemon/InputDeviceManager", this, QDBusConnection::ExportAllContents);
    }
}

bool InputDeviceManager::initDeviceFactor()
{
    //初始化设备factor,获取设备列表
    m_deviceFactor = InputDeviceFactorManager::createDeviceFactor(this);
    if (!m_deviceFactor) {
        return false;
    }
    m_deviceFactor->initInputDevices();
    //光标改变
    connect(m_inputGsettings, &InputGsettings::mouseChanged, m_deviceFactor, &InputDeviceFactor::onCursorChanged);
    return true;
}

void InputDeviceManager::eliminateSpecialDevice()
{
    for (InputDevice *touchpad : m_touchpadList) {
        QVariant touchProduct = touchpad->getProductId();
        USD_LOG(LOG_DEBUG,"touchProduct : %d", touchProduct.toInt());
        for (InputDevice* mouse : m_mouseList) {
            QVariant mouseProduct = mouse->getProductId();
            USD_LOG(LOG_DEBUG,"mouseProduct : %d", mouseProduct.toInt());
            if (touchProduct == mouseProduct) {
                m_mouseList.removeAll(mouse);
                removeMouse(mouseProduct.toInt());
            }
        }
    }
}

//清空设备列表
void InputDeviceManager::clearUpDeviceList()
{
    qDeleteAll(m_mouseList);
    m_mouseList.clear();
    qDeleteAll(m_touchpadList);
    m_touchpadList.clear();
    qDeleteAll(m_keyboardList);
    m_keyboardList.clear();
    qDeleteAll(m_trackpointList);
    m_trackpointList.clear();
}

void InputDeviceManager::connctGsettings()
{
    //gsettings 改变
    connect(m_inputGsettings,&InputGsettings::mouseChanged,this,&InputDeviceManager::onMouseChanged);
    connect(m_inputGsettings,&InputGsettings::touchpadChanged,this,&InputDeviceManager::onTouchpadChanged);
    connect(m_inputGsettings,&InputGsettings::keyboardChanged,this,&InputDeviceManager::onKeyboardChanged);
    connect(m_inputGsettings,&InputGsettings::trackpointChanged,this,&InputDeviceManager::onTrackpointChanged);

}

void InputDeviceManager::disconnectAll()
{
    //断开监听
    disconnect(m_inputGsettings,&InputGsettings::mouseChanged,this,&InputDeviceManager::onMouseChanged);
    disconnect(m_inputGsettings,&InputGsettings::touchpadChanged,this,&InputDeviceManager::onTouchpadChanged);
    disconnect(m_inputGsettings,&InputGsettings::keyboardChanged,this,&InputDeviceManager::onKeyboardChanged);
    disconnect(m_inputGsettings,&InputGsettings::trackpointChanged,this,&InputDeviceManager::onTrackpointChanged);

}

void InputDeviceManager::classifyDevice(InputDevice *device)
{
    DeviceType type = device->getDeviceType();
    switch (type) {
    case DeviceType::IN_MOUSE:
        m_mouseList.append(device);
        break;
    case DeviceType::IN_TOUCHPAD:
        m_touchpadList.append(device);
        break;
    case DeviceType::IN_KEYBOARD:
        m_keyboardList.append(device);
        break;
    case DeviceType::IN_TRACKPOINT:
        m_trackpointList.append(device);
        break;
    case DeviceType::IN_TOUCHSCREEN:
        /*
         *
         *
         */
        break;
    default:
        break;
    }
}

bool InputDeviceManager::deleteDevice(QVariant deviceId)
{
    uint productId = 0;
    for (InputDevice* device : m_mouseList) {
        if (deviceId == device->getDeviceId()) {
            m_mouseList.removeOne(device);
            productId = device->getProductId().toUInt();
            removeMouse(productId);
            return true;
        }
    }
    for (InputDevice* device : m_touchpadList) {
        if (deviceId == device->getDeviceId()) {
            m_touchpadList.removeOne(device);
            productId = device->getProductId().toUInt();
            if (m_touchpadProductIdList.contains(productId)) {
                m_touchpadProductIdList.removeAll(productId);
                TouchpadCountChanged(m_touchpadProductIdList.count());
            }
            return true;
        }
    }
    for (InputDevice* device : m_keyboardList) {
        if (deviceId == device->getDeviceId()) {
            m_keyboardList.removeOne(device);
            productId = device->getProductId().toUInt();
            if (m_keyboardProductIdList.contains(productId)) {
                m_keyboardProductIdList.removeAll(productId);
                KbdCountChanged(m_keyboardProductIdList.count());
            }
            return true;
        }
    }
    for (InputDevice* device : m_trackpointList) {
        if (deviceId == device->getDeviceId()) {
            m_trackpointList.removeOne(device);
            productId = device->getProductId().toUInt();
            if (m_trackpointProductList.contains(productId)) {
                m_trackpointProductList.removeAll(productId);
                TrackpointCountChanged(m_trackpointProductList.count());
            }
            return true;
        }
    }

    return false;
}

bool InputDeviceManager::existMouse()
{
    return m_mouseProductIdList.count();
}

void InputDeviceManager::testPrintDeviceList()
{
    for (InputDevice* device : m_mouseList) {
        USD_LOG(LOG_DEBUG,"mouse device name : %s - device id %s - product id %08x",device->getDeviceName().toLatin1().data(),device->getDeviceId().toString().toLatin1().data(), device->getProductId().toInt());
    }
    for (InputDevice* device : m_touchpadList) {
        USD_LOG(LOG_DEBUG,"touchpad device name : %s - device id %s - product id %08x",device->getDeviceName().toLatin1().data(),device->getDeviceId().toString().toLatin1().data(), device->getProductId().toInt());
    }
    for (InputDevice* device : m_keyboardList) {
        USD_LOG(LOG_DEBUG,"keyboardr device name : %s - device id %s - product id %08x",device->getDeviceName().toLatin1().data(),device->getDeviceId().toString().toLatin1().data(), device->getProductId().toInt());
    }
    for (InputDevice* device : m_trackpointList) {
        USD_LOG(LOG_DEBUG,"trackpoint device name : %s - device id %s - product id %08x",device->getDeviceName().toLatin1().data(),device->getDeviceId().toString().toLatin1().data(), device->getProductId().toInt());
    }
}

#define device_function(list, key, value) \
    auto func = deviceFuncMap.value(key); \
    if (func) { \
        for (InputDevice* device : list) { \
            func(value, device); \
        } \
    }

#define restore_touchpad_status(prama) \
    for (InputDevice* device : m_touchpadList) {\
        device->setEnable(prama); \
    }

void InputDeviceManager::restoreTouchpadStatus()
{
    const QVariant& value = m_inputGsettings->getGsettingsValue(KEY_TOUCHPAD_ENABLED, DeviceType::IN_TOUCHPAD);
    restore_touchpad_status(value);
}

void InputDeviceManager::onMouseChanged(const QString& key, QVariant value)
{
    USD_LOG(LOG_DEBUG,"mouse property %s is changed",key.toLatin1().data());
    if (key == QStringLiteral(KEY_MOUSE_LOCATE_POINTER)) {
        InputDeviceFunction::setLocatePointer(value);
    } else if (key == QStringLiteral(CURSOR_SIZE)) {

        // wayland 环境下，GTK 光标大小, 通过设置org.gnome.desktop.interface cursor-size生效，X环境下设置此值不生效。
        // X 环境下通过Xsettings设置光标大小， 后续光标大小设置，可考虑放入XSettings, 前提是Xsettings 需要兼容X/wayland。
        QByteArray gnomeSchemaId("org.gnome.desktop.interface");
        if (QGSettings::isSchemaInstalled(gnomeSchemaId)) {
            QGSettings(gnomeSchemaId).set(key, value);
        }

    } else {
        device_function(m_mouseList, key, value);
    }
}

void InputDeviceManager::onTouchpadChanged(const QString &key, QVariant value)
{
    USD_LOG(LOG_DEBUG,"touchpad property %s is changed",key.toLatin1().data());
    device_function(m_touchpadList, key, value);
}

void InputDeviceManager::onKeyboardChanged(const QString &key, QVariant value)
{
    USD_LOG(LOG_DEBUG,"keyboard property %s is changed",key.toLatin1().data());
    device_function(m_keyboardList, key, value);
}

void InputDeviceManager::onTrackpointChanged(const QString &key, QVariant value)
{
    USD_LOG(LOG_DEBUG,"trackpoint property %s is changed",key.toLatin1().data());
    device_function(m_trackpointList, key, value);
}

//slots
void InputDeviceManager::deviceAdd(InputDevice* device)
{
/*
 *      设备接入时，会添加多种设备类型，添加事件可能存在时间差，内核文件写入也可能是逐条写入的。
 *  设备添加的实时性要求不高，若不增加延迟，内核文件逐条写入，可能会造成设备类型识别错误的问题
 */
    classifyDevice(device);
    updateDeivceCount(device);
    eliminateSpecialDevice();
    setMousePresent(existMouse());
    restoreTouchpadStatus();

#ifdef TEST_PRINT
    testPrintDeviceList();
#endif

}

void InputDeviceManager::deviceRemove(QVariant deviceId)
{
    deleteDevice(deviceId);
    setMousePresent(existMouse());
    restoreTouchpadStatus();
#ifdef TEST_PRINT
    testPrintDeviceList();
#endif
}

bool InputDeviceManager::isZoneGestureAvailable()
{
    QDBusInterface *iface = InputWlcomHelper::wlcomInputInterface();
    if (!iface || !iface->isValid()) {
        return false;
    }

    static int cached = -1;
    if (cached >= 0) {
        return cached == 1;
    }

    QDBusInterface introspect(QStringLiteral(WLCOM_SERVICE),
                              QStringLiteral(WLCOM_INPUT_PATH),
                              QStringLiteral("org.freedesktop.DBus.Introspectable"),
                              QDBusConnection::sessionBus());
    QDBusReply<QString> reply = introspect.call(QStringLiteral("Introspect"));
    const bool available = reply.isValid()
        && reply.value().contains(QStringLiteral("<method name=\"EnableZoneGesture\""));
    cached = available ? 1 : 0;
    return available;
}

void InputDeviceManager::setIsZoneGestureAvailableGsettings()
{
    m_inputGsettings->setGsettingsValue(KEY_IS_ZONE_GESTURE_AVAILABLE, isZoneGestureAvailable(), DeviceType::IN_TOUCHPAD);
}

void InputDeviceManager::initLocatePointer()
{// 没有指针设备时，显示光标位置依然需要可用
    bool value = m_inputGsettings->getGsettingsValue(KEY_MOUSE_LOCATE_POINTER).toBool();
    InputDeviceFunction::setLocatePointer(value);
}

void InputDeviceManager::initCursorSize()
{
    onMouseChanged(CURSOR_SIZE, m_inputGsettings->getGsettingsValue(CURSOR_SIZE));
}

bool InputDeviceManager::isMasterDevice(InputDevice *device)
{
    DeviceMonitor::instance()->initDeviceInfo();
    uint productId = device->getProductId().toUInt();
    DeviceType devType = DeviceMonitor::instance()->getDeviceType(productId);

    return device->getDeviceType() == DeviceType::IN_MOUSE || device->getDeviceType() == DeviceType::IN_KEYBOARD ?
                devType == device->getDeviceType() : true;

}

void InputDeviceManager::updateDeivceCount(InputDevice *device)
{
    if (!isMasterDevice(device)) {
        return;
    }

    DeviceType type = device->getDeviceType();
    uint productId = device->getProductId().toUInt();
    switch (type) {
    case DeviceType::IN_MOUSE:
        if (m_mouseProductIdList.contains(productId)) {
            return;
        }
        m_mouseProductIdList.append(productId);
        MouseCountChanged(m_mouseProductIdList.count());
        break;
    case DeviceType::IN_TOUCHPAD:
        if (m_touchpadProductIdList.contains(productId)) {
            return;
        }
        m_touchpadProductIdList.append(productId);
        TouchpadCountChanged(m_touchpadProductIdList.count());
        break;
    case DeviceType::IN_KEYBOARD:
        if (m_keyboardProductIdList.contains(productId)) {
            return;
        }
        m_keyboardProductIdList.append(productId);
        KbdCountChanged(m_keyboardProductIdList.count());
        break;
    case DeviceType::IN_TRACKPOINT:
        if (m_trackpointProductList.contains(productId)) {
            return;
        }
        m_trackpointProductList.append(productId);
        TrackpointCountChanged(m_trackpointProductList.count());
        break;
    default:
        break;
    }
}

void InputDeviceManager::removeMouse(uint prodId)
{
    if (m_mouseProductIdList.contains(prodId)) {
        m_mouseProductIdList.removeAll(prodId);
        MouseCountChanged(m_mouseProductIdList.count());
    }
}


void InputDeviceManager::managerStart()
{
    m_timer->stop();
    registerDbusService();
    m_inputGsettings = InputGsettings::instance();
    m_inputGsettings->initGsettings();
    //gsettings init faild
    if (!m_inputGsettings->resultInitGsettings()) {
        USD_LOG(LOG_WARNING,"input gsettings init faild .");
        return;
    }
    //光标显示
    initLocatePointer();
    //GTK 光标大小
    initCursorSize();
    //初始化设备工场
    if (!initDeviceFactor()) {
        USD_LOG(LOG_WARNING,"init device factor faild .");
        return;
    }
    //剔除与触摸板Product ID 一致的mouse设备
    eliminateSpecialDevice();
    //gsettings 改变
    connctGsettings();

    //因为插入鼠标禁用触摸板原因，所以在初始化所有设备后，恢复触摸板状态
    restoreTouchpadStatus();

    setIsZoneGestureAvailableGsettings();

#ifdef TEST_PRINT
    testPrintDeviceList();
#endif
}
