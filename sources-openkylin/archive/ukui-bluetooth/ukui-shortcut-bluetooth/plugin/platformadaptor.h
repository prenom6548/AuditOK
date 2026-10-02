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

#ifndef PLATFORMADAPTOR_H
#define PLATFORMADAPTOR_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusContext>
#include <QDBusReply>
#include <QMap>
#include <QStringList>

#include "devicemanager.h"
#include "CSingleton.h"

class PlatformAdaptor : public QObject
{
    Q_OBJECT
protected:
    explicit PlatformAdaptor(QObject *parent = nullptr);
    ~PlatformAdaptor(void);


public:
    bool getInit(void);

    bool getShowBluetooth(void);

    bool getAdapterPower(void);

    QList<QMap<QString, QVariant>> pairedDevices(void) const;

    QList<QMap<QString, QVariant>> notPairedDevices(void) const;

    void setBluetoothConfig(QMap<QString, QVariant>);

    BtAdapterPtr getDefaultBtAdapter(void);

    void sendUpdatePairedDeviceSort(QStringList);

    void registerClient(bool);

protected:
    QDBusPendingCallWatcher * asyncCall(const QString & methed, const QList<QVariant> & params);

    QDBusPendingCallWatcher * asyncSessCall(const QString & methed, const QList<QVariant> & params);


    void getAdapter(void);
    void getAdapterAttr(QString id);
    void getPairedDevices(void);
    void getPairedDeviceAttr(QString id);
    void getNotPairedDevices(void);
    void getNotPairedDeviceAttr(QString id);

    bool whetherNeedInfoUser();

    void calcInit(void);
protected slots:
    void getAdapterFinished(QDBusPendingCallWatcher * watcher);
    void getAdapterAttrFinished(QDBusPendingCallWatcher * watcher);
    void getPairedDevicesFinished(QDBusPendingCallWatcher * watcher);
    void getPairedDeviceAttrFinished(QDBusPendingCallWatcher * watcher);
    void getNotPairedDevicesFinished(QDBusPendingCallWatcher * watcher);
    void getNotPairedDeviceAttrFinished(QDBusPendingCallWatcher * watcher);


    void adapterAddSignal(QMap<QString, QVariant>);
    void adapterAttrChanged(QString, QMap<QString, QVariant>);
    void adapterRemoveSignal(QString);
    void deviceAddSignal(QMap<QString, QVariant>);
    void deviceAttrChanged(QString, QMap<QString, QVariant>);
    void deviceRemoveSignal(QString, QMap<QString, QVariant>);
    void updateClient(void);

protected:
    //初始化
    bool m_init = false;

    volatile int m_pending_count = 0;

    QDBusInterface m_bluetooth_iface;

    QDBusInterface m_sess_bluetooth_iface;

    BtAdapterPtr m_dfAdapterPtr;

    QMap<QString, BtAdapterPtr> m_adapters;

protected:
    friend class SingleTon<PlatformAdaptor>;
};

typedef SingleTon<PlatformAdaptor>  PTAD;

#endif // PLATFORMADAPTOR_H
