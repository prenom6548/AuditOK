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

#ifndef DEVICEBASE_H
#define DEVICEBASE_H


#include <QList>
#include <QDebug>
#include <QObject>
#include <QVector>
#include <QString>
#include <ukui-log4qt.h>

#include "config.h"

class devicebase;
class bluetoothadapter;
class bluetoothdevice;

//devicebase
class devicebase : public QObject
{
    Q_OBJECT
public:
    devicebase();
    virtual ~devicebase(){}

    virtual void resetDeviceName(QString)    = 0 ;
    virtual QString getDevName()      = 0;
    virtual QString getDevAddress()   = 0;

private:
    QString m_dev_name;
    QString m_dev_address;
};
//devicebase end


//bluetoothadapter
class bluetoothadapter : public devicebase
{
    Q_OBJECT
public:
    bluetoothadapter(QString dev_name        ,
                     const QString dev_address     ,
                     bool    dev_block       ,
                     bool    dev_power       ,
                     bool    dev_pairing     ,
                     bool    dev_pairable    ,
                     bool    dev_connecting  ,
                     bool    dev_discovering ,
                     bool    dev_discoverable,
                     bool    dev_activeConnection,
                     bool    dev_defaultAdapterMark,
                     bool    dev_trayShow);

    bluetoothadapter(QMap<QString ,QVariant> value);

    ~bluetoothadapter(){
        //KyDebug() << "======" << m_dev_name << m_dev_address;
    }

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setAdapterPower(bool);
    bool getAdapterPower();

    void setAdapterPairing(bool);
    bool getAdapterPairing();

    void setAdapterConnecting(bool);
    bool getAdapterConnecting();

    void setAdapterPairable(bool);
    bool getAdapterPairable();

    void setAdapterDiscovering(bool);
    bool getAdapterDiscovering();

    void setAdapterDiscoverable(bool);
    bool getAdapterDiscoverable();

    void setAdapterAutoConn(bool);
    bool getAdapterAutoConn();

    void setAdapterDefaultMark(bool);
    bool getAdapterDefaultMark();

    void setAdapterTrayShow(bool);
    bool getAdapterTrayShow();

    bool audioCombine(void);
    void audioCombine(bool v);

    QMap <QString,bluetoothdevice *> m_bt_dev_list;
//    QList<bluetoothdevice *> m_bluetooth_device_list;
    QMap <QString,bluetoothdevice *> m_bt_dev_paired_list;
//    QList<bluetoothdevice *> m_bluetooth_device_paired_list;

signals:
    void adapterNameChanged(const QString &name);
    void adapterPoweredChanged(bool powered);
    void adapterPairingChanged(bool pairing);
    void adapterPairableChanged(bool pairable);
    void adapterConnectingChanged(bool connecting);

    void adapterTrayIconChanged(bool status);
    void adapterDiscoverableChanged(bool discoverable);
    void adapterDiscoveringChanged(bool discovering);
    void adapterAutoConnStatuChanged(bool status);
    void adapterDeviceAdded(bluetoothdevice * &device);
    void adapterDeviceRemoved(bluetoothdevice * &device);
    void defaultAdapterChanged(const QString &address);



private:
    QString m_dev_name                  ;
    QString m_dev_address               ;

    bool    m_dev_block                 ;
    bool    m_dev_power                 ;
    bool    m_dev_pairing               ;
    bool    m_dev_pairable              ;
    bool    m_dev_connecting            ;
    bool    m_dev_discovering           ;
    bool    m_dev_discoverable          ;
    bool    m_dev_activeConnection      ;
    bool    m_dev_defaultAdapterMark    ;
    bool    m_dev_trayShow              ;
    bool    m_audio_combine = false      ;

    void    bluetoothAdapterDataAnalysis(QMap<QString ,QVariant> value);
};
//bluetoothadapter end

//bluetoothdevice
class bluetoothdevice : public devicebase
{
    Q_OBJECT

public:
    enum BLUETOOTH_DEVICE_TYPE{
        BtPhone = 0,
        BtModem,
        BtComputer,
        BtNetwork,
        BtHeadset,
        BtHeadphones,
        BtAudiovideo,
        BtKeyboard,
        BtMouse,
        BtJoypad,
        BtTablet,
        BtPeripheral,
        BtCamera,
        BtPrinter,
        BtImaging,
        BtWearable,
        BtToy,
        BtHealth,
        uncategorized
    };
    Q_ENUM(BLUETOOTH_DEVICE_TYPE)

    bluetoothdevice(QString         dev_address             ,
                    QString         dev_name                ,
                    QString         dev_showName            ,
                    BLUETOOTH_DEVICE_TYPE     dev_type                ,
                    bool            dev_paired              ,
                    bool            dev_trusted             ,
                    bool            dev_blocked             ,
                    bool            dev_connected           ,
                    bool            dev_pairing             ,
                    bool            dev_connecting          ,
                    int             dev_battery             ,
                    int             dev_connectFailedId     ,
                    QString         dev_connectFailedDisc   ,
                    qint16          dev_rssi                ,
                    bool            dev_sendFileMark        ,
                    QString         adapter_address);
    bluetoothdevice(QMap<QString ,QVariant> value);

    ~bluetoothdevice(){
        //KyDebug() << m_dev_name << m_dev_address;
    }

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setDevShowName(QString);
    QString getDevShowName();
    QString getDevInterfaceShowName();

    void setDevType(BLUETOOTH_DEVICE_TYPE);
    BLUETOOTH_DEVICE_TYPE getDevType();

    bool isPaired();
    void devPairedChanged(bool);

    bool isConnected();
    void devConnectedChanged(bool);

    void setDevPairing(bool);
    bool getDevPairing();//暂时不使用

    void setDevConnecting(bool);
    bool getDevConnecting();

    void devMacAddressChanged(QString);

    void setDevTrust(bool);
    bool getDevTrust();

    void setDevConnFailedInfo(int,QString);

    void setDevSendFileMark(bool);
    bool getDevSendFileMark();

    qint16 getDevRssi();
    void setDevRssi(qint16);

    int getErrorId();

    void setRemoving(bool);
    bool getRemoving();

signals:

    void nameChanged(QString);
    void showNameChanged(QString);
    void rssiChanged(qint16);
    void typeChanged(BLUETOOTH_DEVICE_TYPE);
    void pairedChanged(bool);
    void connectedChanged(bool);
    void pairingChanged(bool);
    void connectingChanged(bool);
    void trustChanged(bool);
    void errorInfoRefresh(int,QString);
    void devConnectingSignal();

private:

    QString         m_dev_address               ;
    QString         m_dev_name                  ;
    QString         m_dev_showName              ;
    BLUETOOTH_DEVICE_TYPE     m_dev_type                  ;
    bool            m_dev_paired     = false    ;
    bool            m_dev_trusted    = false    ;
    bool            m_dev_blocked    = false    ;
    bool            m_dev_connected  = false    ;
    bool            m_dev_pairing    = false    ;//暂时不使用
    bool            m_dev_connecting = false    ;
    int             m_dev_battery               ;
    int             m_dev_connectFailedId       ;
    QString         m_dev_connectFailedDisc     ;
    qint16          m_dev_rssi       = -256     ;
    bool            m_dev_sendFileMark          ;
    QString         m_adapter_address           ;//暂时不使用
    bool            m_dev_removing   = false    ;//正在移除该设备

    void bluetoothDeviceDataAnalysis(QMap<QString ,QVariant> value);

};
//bluetoothdevice end

#endif // DEVICEBASE_H
