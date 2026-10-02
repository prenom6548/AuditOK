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

#ifndef INPUTDEVICEMANAGER_H
#define INPUTDEVICEMANAGER_H
#include <QObject>
#include <QTimer>
#include <QThread>
#include "input-device.h"
#include "input-gsettings.h"
#include "input-device-factory.h"

class InputDeviceFactor;
class InputDeviceManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.ukui.SettingsDaemon.InputDeviceManager")
    typedef QList<InputDevice*> DeviceList;
public:
    explicit InputDeviceManager(QObject *parent = nullptr);
    ~InputDeviceManager();
    void start();
    void stop();
    static bool mousePresent();
    static void setMousePresent(bool value);
    friend class InputXDeviceFactor;
    friend class InputWaylandDeviceFactor;
    friend class InputWlcomDeviceFactor;

Q_SIGNALS:
    void MouseCountChanged(int count);
    void KbdCountChanged(int count);
    void TouchpadCountChanged(int count);
    void TrackpointCountChanged(int count);

public Q_SLOTS:
    QString GetDevInfoByProductId(uint prodId);
    QString GetDevTypeMap();

    int GetDevCountByType(const QString& devType) const;

private:
    void registerDbusService();

    bool initDeviceFactor();
    /**
     * @brief 剔除特殊设备
     * 新版内核 无效mouse设备跟随touchpad上报，product id 一致
     */
    void eliminateSpecialDevice();

    /**
     * @brief 清空设备列表
     */
    void clearUpDeviceList();

    /**
     * @brief gsettings 监听
     */
    void connctGsettings();

    /**
     * @brief 根据设备类型筛选设备
     * @param device
     */
    void classifyDevice(InputDevice* device);

    /**
     * @brief 断开所有连接
     */
    void disconnectAll();

    /**
     * @brief 删除设备
     * @param deviceId 设备id
     * @return
     */
    bool deleteDevice(QVariant deviceId);

    /**
     * @brief 检测鼠标存在
     * @return true
     */
    bool existMouse();

    /**
     * @brief 测试打印设备列表
     */
    void testPrintDeviceList();

    /**
     * @brief 检测鼠标存在禁用触摸板 初始化时，设备插入时，设备移除时。
     */
    void restoreTouchpadStatus();

    /**
     * @brief deviceAdd
     * @param device
     */
    void deviceAdd(InputDevice* device);

    /**
     * @brief deviceRemove
     * @param deviceId
     */
    void deviceRemove(QVariant deviceId);

    /**
     * @brief 初始化ctrl键光标显示
     */
    void initLocatePointer();

    /**
     * @brief 初始化GTK 光标大小
     */
    void initCursorSize();

    bool isMasterDevice(InputDevice* device);

    void updateDeivceCount(InputDevice* device);

    void removeMouse(uint prodId);
private:
    void setIsZoneGestureAvailableGsettings();

    static bool isZoneGestureAvailable();

private Q_SLOTS:
    void onMouseChanged(const QString&, QVariant);
    void onTouchpadChanged(const QString&, QVariant);
    void onKeyboardChanged(const QString&, QVariant);
    void onTrackpointChanged(const QString&, QVariant);
    void managerStart();
private:
    InputDeviceFactor* m_deviceFactor;
    InputGsettings* m_inputGsettings;
    DeviceList m_mouseList;
    DeviceList m_touchpadList;
    DeviceList m_keyboardList;
    DeviceList m_trackpointList;
    QTimer    *m_timer;
    static bool s_mousePresent;
    QList<uint> m_mouseProductIdList;
    QList<uint> m_touchpadProductIdList;
    QList<uint> m_keyboardProductIdList;
    QList<uint> m_trackpointProductList;
};

#endif // INPUTDEVICEMANAGER_H
