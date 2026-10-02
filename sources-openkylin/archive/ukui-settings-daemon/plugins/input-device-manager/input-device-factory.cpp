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

#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDir>
#include <QSettings>
#include "input-device-factory.h"
#include "input-device-helper.h"
#include "input-x-device.h"
#include "input-wayland-device.h"
#include "usd_base_class.h"
#include "input-wlcom-helper.h"
#include "wlcom-config.h"
#include "input-wlcom-device.h"

using namespace InputDeviceHelper;

/*****X Factor*****/

InputXDeviceFactor::InputXDeviceFactor(QObject *parent)
{
    m_deviceManager = reinterpret_cast<InputDeviceManager*>(parent);
    m_monitorThread = new QThread(this);
    connectMonitor();
}

InputXDeviceFactor::~InputXDeviceFactor()
{
    disconnect(m_inputMonitor, &InputMonitor::deviceAdd, this, &InputXDeviceFactor::deviceAdd);
    disconnect(m_inputMonitor, &InputMonitor::deviceRemove, this, &InputXDeviceFactor::deviceRemove);
    m_inputMonitor->stopMontior();
}

void InputXDeviceFactor::initInputDevices()
{
    int ndevices;
    XDeviceInfo* deviceList = XListInputDevices(QX11Info::display(),&ndevices);
    for (int i = 0 ; i < ndevices ; ++i) {
        InputDevice* device = filterDevice(deviceList[i]);
        if (device) {
            m_deviceManager->deviceAdd(device);
        }
    }
    XFreeDeviceList(deviceList);
}

InputDevice *InputXDeviceFactor::createInputDevice(QVariant deviceId, DeviceType type, QString name)
{
    return new InputXDevice(deviceId, type, name);
}

InputDevice* InputXDeviceFactor::filterDevice(XDeviceInfo device)
{
    InputDevice *inputDevice = nullptr;
    if (device.type == propertyToAtom(XI_MOUSE)){ //鼠标, 由于虚拟机内虚拟鼠标设备使用PS2协议，增加“VMware VMMouse”字段区分。
        if (strstr(device.name,R"(PS/2)") && !strstr(device.name, "VMware VMMouse") && !strstr(device.name, "TrackPoint")) {
            //ps2 触摸板上报为鼠标设备,所以标记为IN_TOUCHPAD
            inputDevice = createInputDevice(int(device.id), DeviceType::IN_TOUCHPAD, device.name);
        } else if (strstr(device.name, "TrackPoint")) {
            //lenovo track point设备

        } else {
            // mouse
            inputDevice = createInputDevice(int(device.id), DeviceType::IN_MOUSE, device.name);
        }
    } else if (device.type == propertyToAtom(XI_TOUCHPAD)) {//触控板
        inputDevice = createInputDevice(int(device.id), DeviceType::IN_TOUCHPAD, device.name);
    } else if (device.type == propertyToAtom(XI_TOUCHSCREEN) //触控屏
               || device.type == propertyToAtom(XI_TABLET)) {//笔
        //绝对坐标映射设备进行映射
        QDBusInterface interface("org.ukui.SettingsDaemon",
                                 "/org/ukui/SettingsDaemon/xrandr",
                                 "org.ukui.SettingsDaemon.xrandr",
                                 QDBusConnection::sessionBus());
        if (interface.isValid()) {
            interface.call("setScreenMap");
        }
    } else if (device.type == propertyToAtom(XI_KEYBOARD)) {//键盘
        //键盘设置暂时在keyboard-manager 插件，后续合入
    } else if (device.type == propertyToAtom(XI_JOYSTICK)) {//摇杆
        //to be continued
    }
    return inputDevice;
}

void InputXDeviceFactor::connectMonitor()
{
    //delete InputMonitor
    m_inputMonitor = InputMonitor::instance();
    m_inputMonitor->moveToThread(m_monitorThread);
    connect(m_monitorThread, &QThread::started, InputMonitor::instance(), &InputMonitor::startMonitor);
    m_monitorThread->start();
    connect(m_inputMonitor, &InputMonitor::deviceAdd, this, &InputXDeviceFactor::deviceAdd);
    connect(m_inputMonitor, &InputMonitor::deviceRemove, this, &InputXDeviceFactor::deviceRemove);
}

void InputXDeviceFactor::deviceAdd(int id)
{
    int ndevices;
    XDeviceInfo* deviceList = XListInputDevices(QX11Info::display(),&ndevices);
    for (int i = 0 ; i < ndevices ; ++i) {
        if (id == deviceList[i].id) {
            InputDevice* device = filterDevice(deviceList[i]);
            if (device) {
                m_deviceManager->deviceAdd(device);
            }
        }
    }
    XFreeDeviceList(deviceList);
}

void InputXDeviceFactor::deviceRemove(int id)
{
    m_deviceManager->deviceRemove(id);
}

void InputXDeviceFactor::onCursorChanged(const QString &key)
{
    Q_UNUSED(key);
}

/*********** InputWaylandDeviceFactor ***********/

#define UKUI_KWIN_OBJECT  "/org/ukui/KWin/InputDevice"
#define UKUI_KWIN_DEVICE_MANAGER "org.ukui.KWin.InputDeviceManager"

#define KDE_KWIN_OBJECT "/org/kde/KWin/InputDevice"
#define KDE_KWIN_DEVICE_MANAGER "org.kde.KWin.InputDeviceManager"

InputWaylandDeviceFactor::InputWaylandDeviceFactor(QObject *parent)
{
    m_deviceManager = reinterpret_cast<InputDeviceManager*>(parent);

    m_deviceInterface = new QDBusInterface(QStringLiteral(KDE_KWIN_SERVICE),
                                           QStringLiteral(KDE_KWIN_OBJECT),
                                           QStringLiteral(KDE_KWIN_DEVICE_MANAGER),
                                           QDBusConnection::sessionBus(), this);
    if (!m_deviceInterface->isValid()) {
        m_deviceInterface = new QDBusInterface(QStringLiteral(UKUI_KWIN_SERVICE),
                                               QStringLiteral(UKUI_KWIN_OBJECT),
                                               QStringLiteral(UKUI_KWIN_DEVICE_MANAGER),
                                               QDBusConnection::sessionBus(), this);
    }
    if (m_deviceInterface->isValid()) {
        connectMonitor();
    }
}

InputWaylandDeviceFactor::~InputWaylandDeviceFactor()
{
    disconnect(m_deviceInterface, SIGNAL(deviceAdded(QString)),this, SLOT(deviceAdd(QString)));
    disconnect(m_deviceInterface, SIGNAL(deviceRemoved(QString)),this, SLOT(deviceRemove(QString)));
}

void InputWaylandDeviceFactor::initInputDevices()
{
    if (m_deviceInterface->isValid()) {
        QStringList deviceList = m_deviceInterface->property("devicesSysNames").toStringList();
        for (QString device : deviceList) {
            managerAddDevice(device);
        }
    }
    //光标设置
    onCursorChanged(CURSOR_SIZE);
}

void InputWaylandDeviceFactor::connectMonitor()
{
    connect(m_deviceInterface, SIGNAL(deviceAdded(QString)),this, SLOT(deviceAdd(QString)));
    connect(m_deviceInterface, SIGNAL(deviceRemoved(QString)),this, SLOT(deviceRemove(QString)));
}

InputDevice *InputWaylandDeviceFactor::createInputDevice(QVariant deviceId, DeviceType type, QString name)
{
    return new InputWaylandDevice(deviceId, type, name);
}

InputDevice *InputWaylandDeviceFactor::filterDevice(QDBusInterface * interface)
{
    InputDevice *inputDevice = nullptr;
     USD_LOG_SHOW_PARAMS(qPrintable(interface->property("name").toString()));
    if (interface->property("pointer").toBool()) {
        if (interface->property("touchpad").toBool()) {
            QString node = interface->property("sysName").toString();
            QString name = interface->property("name").toString();
            inputDevice = createInputDevice(node, DeviceType::IN_TOUCHPAD, name);
        } else if (interface->property("touch").toBool()) {
            //绝对坐标映射设备进行映射
            QDBusInterface interface("org.ukui.SettingsDaemon",
                                     "/org/ukui/SettingsDaemon/xrandr",
                                     "org.ukui.SettingsDaemon.xrandr",
                                     QDBusConnection::sessionBus());
            if (interface.isValid()) {
                interface.call("setScreenMap");
            }
            USD_LOG(LOG_DEBUG,".....");
        } else if (interface->property("supportsPointerAcceleration").toBool()){ //非触摸板，且支持指针速度调节
            QString node = interface->property("sysName").toString();
            QString name = interface->property("name").toString();
            inputDevice = createInputDevice(node, DeviceType::IN_MOUSE, name);
        }
    } else if (interface->property("keyboard").toBool()) {
        //键盘设置在keyboard-manager插件，后续合入进来
    }
    return inputDevice;
}

void InputWaylandDeviceFactor::managerAddDevice(QString device)
{
    QDBusInterface *interface = new QDBusInterface(KDE_KWIN_SERVICE,
                                     KDE_KWIN_OBJECT_PATH + device,
                                     KDE_KWIN_INTERFACE,
                                     QDBusConnection::sessionBus());
    if (!interface->isValid()) {
        interface = new QDBusInterface(UKUI_KWIN_SERVICE,
                                       UKUI_KWIN_OBJECT_PATH + device,
                                       UKUI_KWIN_INTERFACE,
                                       QDBusConnection::sessionBus());
    }
    USD_LOG_SHOW_PARAMS(qPrintable(device));
    if (interface->isValid()) {
        InputDevice* inputDevice = filterDevice(interface);
        if (inputDevice) {
            m_deviceManager->deviceAdd(inputDevice);
        }
        delete interface;
        interface = nullptr;
    }
}

void InputWaylandDeviceFactor::deviceAdd(QString device)
{
    managerAddDevice(device);
}

void InputWaylandDeviceFactor::deviceRemove(QString device)
{
    USD_LOG_SHOW_PARAMS(qPrintable(device));
    m_deviceManager->deviceRemove(device);
}
void InputWaylandDeviceFactor::onCursorChanged(const QString& key)
{
    if (key == CURSOR_SIZE) {
        int cursorSize = InputGsettings::instance()->getGsettingsValue(CURSOR_SIZE).toInt();

        QString filename = QDir::homePath() + "/.config/kcminputrc";
        QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);
        mouseSettings->beginGroup("Mouse");
        mouseSettings->setValue("cursorSize", cursorSize);
        mouseSettings->endGroup();
        mouseSettings->sync();
        mouseSettings->deleteLater();
        QDBusMessage message =
                QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");

        QList<QVariant> args;
        args.append(5);//cursor size change signal
        args.append(0);//The sceond param unuse..
        message.setArguments(args);

        QDBusConnection::sessionBus().send(message);
    }
}

/*******************Input Wlcom Factor****************/
InputWlcomDeviceFactor::InputWlcomDeviceFactor(QObject *parent)
{
    m_deviceManager = reinterpret_cast<InputDeviceManager*>(parent);

    qDBusRegisterMetaType<WlcomSeat>();
    qDBusRegisterMetaType<QList<WlcomSeat>>();
    qDBusRegisterMetaType<WlcomDevice>();
    qDBusRegisterMetaType<QList<WlcomDevice>>();
    InputWlcomHelper::wlcomInputInterface();
    InputWlcomHelper::wlcomSeatInterface();

}

void InputWlcomDeviceFactor::initInputDevices()
{

    //init input list all
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral(WLCOM_SERVICE),
                                                          QStringLiteral(WLCOM_INPUT_PATH),
                                                          QStringLiteral(WLCOM_INPUT_INTERFACE),
                                                          QStringLiteral("ListAllInputs"));
    QDBusPendingCall async = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *asyncWatcher = new QDBusPendingCallWatcher(async, this);
    connect(asyncWatcher, &QDBusPendingCallWatcher::finished, [=] (QDBusPendingCallWatcher* watcher) {
        QDBusPendingReply<QList<WlcomDevice>> replay = *watcher;
        if (replay.isError()) {
            USD_LOG(LOG_WARNING," waring: wlocm all inputs list is null");
        } else {
            for (const WlcomDevice& wlDevice : replay.value()) {
                QStringList datalist = wlDevice.name.split(":");
                if (datalist.size() >= 3) {
                    int type = datalist[0].toInt();
                    uint proId = (datalist[1].toInt() << 16) + datalist[2].toInt();
                    InputDevice* device = filterDevice(type, proId, wlDevice);
                    if (device) {
                        m_deviceManager->deviceAdd(device);
                    }
                }
            }
        }
        watcher->deleteLater();
    });

    QDBusConnection::sessionBus().connect(QStringLiteral(WLCOM_SERVICE),
                                QStringLiteral(WLCOM_INPUT_PATH),
                                QStringLiteral(WLCOM_INPUT_INTERFACE),
                                QStringLiteral("input_destroy"),
                                this, SLOT(deviceRemoved(QString)));

    QDBusConnection::sessionBus().connect(QStringLiteral(WLCOM_SERVICE),
                                QStringLiteral(WLCOM_INPUT_PATH),
                                QStringLiteral(WLCOM_INPUT_INTERFACE),
                                QStringLiteral("input_create"),
                                this, SLOT(deviceAdd(QString, uint32_t)));

    //init seat list all
    QDBusMessage seatMssage = QDBusMessage::createMethodCall(QStringLiteral(WLCOM_SERVICE),
                                                          QStringLiteral(WLCOM_SEAT_PATH),
                                                          QStringLiteral(WLCOM_SEAT_INTERFACE),
                                                          QStringLiteral("ListAllSeats"));
    QDBusPendingCall asyncSeat = QDBusConnection::sessionBus().asyncCall(seatMssage);
    QDBusPendingCallWatcher *asyncWatcherSeat = new QDBusPendingCallWatcher(asyncSeat, this);
    connect(asyncWatcherSeat, &QDBusPendingCallWatcher::finished, [=] (QDBusPendingCallWatcher* watcher) {
        QDBusPendingReply<QList<WlcomSeat>> replay = *watcher;
        if (replay.isError()) {
            USD_LOG(LOG_WARNING," waring: wlocm all seats list is null");
        } else {
            for (const WlcomSeat& seat : replay.value()) {
                m_seats.push_back(seat.name);
                setCursor(seat.name);
            }
        }
        watcher->deleteLater();
    });

}

InputDevice *InputWlcomDeviceFactor::filterDevice(int type, uint productId, const WlcomDevice &device)
{
    InputDevice *inputDevice = nullptr;
    if (type == WLR_INPUT_DEVICE_POINTER) {
        //多指，并且has_pointer_accel为真标记为触摸板
        if (device.has_pointer_accel && device.name.contains("TrackPoint")) {
            inputDevice = createInputDevice(device.name, DeviceType::IN_TRACKPOINT, productId, device.name);
        } else if (device.tap_finger_count && device.has_pointer_accel ||
                (device.name.contains("ExPS/2") && UsdBaseClass::isNotebook())) {
            inputDevice = createInputDevice(device.name, DeviceType::IN_TOUCHPAD, productId, device.name);
        } else {
            inputDevice = createInputDevice(device.name, DeviceType::IN_MOUSE, productId, device.name);
        }
    } else if (type == WLR_INPUT_DEVICE_KEYBOARD) {
        inputDevice = createInputDevice(device.name, DeviceType::IN_KEYBOARD, productId, device.name);
    } else if (type == WLR_INPUT_DEVICE_TOUCH ||
               type == WLR_INPUT_DEVICE_TABLET_TOOL) {
        //绝对坐标映射设备进行映射
        QDBusInterface interface("org.ukui.SettingsDaemon",
                                 "/org/ukui/SettingsDaemon/xrandr",
                                 "org.ukui.SettingsDaemon.xrandr",
                                 QDBusConnection::sessionBus());
        if (interface.isValid()) {
            interface.call("setScreenMap");
        }
    }
    return inputDevice;
}

InputDevice *InputWlcomDeviceFactor::createInputDevice(const QVariant& deviceId, DeviceType type, const QVariant& productId, QString name)
{
    return new InputWlcomDevice(deviceId, type, productId, name);
}

void InputWlcomDeviceFactor::setCursor(const QString &seat)
{
    const QString& cursorTheme = InputGsettings::instance()->getGsettingsValue(CURSOR_THEME).toString();
    uint32_t cursorSize = InputGsettings::instance()->getGsettingsValue(CURSOR_SIZE).toInt();
    InputWlcomHelper::setCursor(seat, cursorTheme, cursorSize);
}

void InputWlcomDeviceFactor::deviceAdd(const QString& device, uint32_t props)
{
    WlcomDevice wlDevcie;
    wlDevcie.name = device;
    wlDevcie.prop = props;
    QStringList datalist = device.split(":");
    if (datalist.size() >= 3) {
        int type = datalist[0].toInt();
        uint proId = (datalist[1].toInt() << 16) + datalist[2].toInt();
        InputDevice* device = filterDevice(type, proId, wlDevcie);
        if (device) {
            m_deviceManager->deviceAdd(device);
        }
    }
}

void InputWlcomDeviceFactor::deviceRemoved(const QString& device)
{
    USD_LOG_SHOW_PARAMS(qPrintable(device));
    m_deviceManager->deviceRemove(device);
}

void InputWlcomDeviceFactor::onCursorChanged(const QString &key)
{
    if (key == CURSOR_SIZE || key == CURSOR_THEME) {
        for (const QString& name : m_seats) {
            setCursor(name);
        }
    }
}

/******************Input Wlcom Factor******************/

/*InputDeviceFactorManager*/

InputDeviceFactor *InputDeviceFactorManager::createDeviceFactor(InputDeviceManager* manager)
{
    InputDeviceFactor* deviceFactor = nullptr;
    if (UsdBaseClass::isWayland()) {
        if (UsdBaseClass::isWlcom()) {
            deviceFactor = new InputWlcomDeviceFactor(manager);
        } else {
            deviceFactor = new InputWaylandDeviceFactor(manager);
        }
    } else {
        if (!supportXinputExtension()) {
            USD_LOG(LOG_WARNING,"X Input extension not available");
            return nullptr;
        }
        deviceFactor = new InputXDeviceFactor(manager);
    }
    return deviceFactor;
}


