/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef BLUETOOTHINTERFACE_H
#define BLUETOOTHINTERFACE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QIcon>
#include <QProcess>
#include <QTimer>

#include "CSingleton.h"



class BluetoothInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList deviceList READ getPairedDevices NOTIFY updatePairedDevices)
    Q_PROPERTY(QVariantList notPairDeviceList READ getNotPairedDevices NOTIFY updateNotPairedDevices)
    Q_PROPERTY(bool btPower READ getAdapterPower WRITE setAdapterPower  NOTIFY bluetoothPower)
    Q_PROPERTY(bool btDiscovering READ getAdapterDiscovering NOTIFY bluetoothDiscovering)
    Q_PROPERTY(bool btInit READ getInit NOTIFY isInit)
protected:
    BluetoothInterface();
    ~BluetoothInterface();

    QString getIconData(QString name, int size = 16);
    void init(void);

public:
    void sendUpdatePairedDevices(const QList<QMap<QString, QVariant>> & devices);

    void sendUpdateNotPairedDevices(const QList<QMap<QString, QVariant>> & devices);

    void sendUpdatePairedDevice(const QString & device, const QMap<QString, QVariant>& attrs);

    void sendUpdateNotPairedDevice(const QString & device, const QMap<QString, QVariant>& attrs);

    void sendIsInit(bool v);

    void sendIsShowBluetooth(bool v);

    void sendBluetoothPower(bool v);

    void sendBluetoothDiscovering(bool v);

public slots:
    //获取是否完成初始化
    bool getInit(void);

    //是否存在蓝牙适配器
    bool getShowBluetooth(void);

    //获取蓝牙适配器开关状态
    bool getAdapterPower(void);

    //获取蓝牙适配器扫描状态
    bool getAdapterDiscovering(void);


    QVariantList getPairedDevices(void);

    QVariantList getNotPairedDevices(void);


    void setAdapterPower(bool v);

    //传输json字符串
    void setBluetoothConfig(QMap<QString, QVariant>);

    //打开控制面板蓝牙
    void openBluetoothSetting(void);

    QVariant getBluetoothIcon(int);

    QVariant getBluetoothBatteryIcon(int);


    void registerClient(bool);
signals:
    //蓝牙模块是否初始化完成
    void isInit();

    //是否存在蓝牙适配器
    void isShowBluetooth();

    //蓝牙开关状态
    void bluetoothPower();

    //蓝牙扫描状态
    void bluetoothDiscovering();

    void updatePairedDevice(int ipos, const QString & device, const QMap<QString, QVariant>& attrs);

    void updateNotPairedDevice(int ipos, const QString & device, const QMap<QString, QVariant>& attrs);

    //更新已配对蓝牙设备列表
    //devices: list<QMap<QString, QVariant>>
    void updatePairedDevices();

    //未配对的临时设备
    void updateNotPairedDevices();

protected:
    bool m_btInit = false;
    bool m_btPower = false;
    bool m_btDiscovering = false;


    QMap<int, QVariant> m_blueIcon;

    QMap<int, QVariant> m_batteryIcon;

    QProcess * m_ukuiProcess = nullptr;

    QTimer * m_timer = nullptr;

    friend class SingleTon<BluetoothInterface>;
};

typedef SingleTon<BluetoothInterface>  BtInterface;

#endif // BLUETOOTHINTERFACE_H
