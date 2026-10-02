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

#include "platformadaptor.h"
#include "ukui-log4qt.h"
#include "bluetoothinterface.h"
#include "commontool.h"

#include <QDBusPendingReply>

#define DBUSSERVICE     "com.ukui.bluetooth"
#define DBUSPATH        "/com/ukui/bluetooth"
#define DBUSINTERFACE   "com.ukui.bluetooth"

PlatformAdaptor::PlatformAdaptor(QObject *parent) : QObject(parent),
    m_bluetooth_iface(DBUSSERVICE, DBUSPATH, DBUSINTERFACE, QDBusConnection::systemBus()),
    m_sess_bluetooth_iface(DBUSSERVICE, DBUSPATH, DBUSINTERFACE, QDBusConnection::sessionBus())
{
    this->getAdapter();

    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "updateClient",this, SLOT(updateClient()));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "adapterAddSignal",this, SLOT(adapterAddSignal(QMap<QString, QVariant>)));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "adapterAttrChanged",this, SLOT(adapterAttrChanged(QString, QMap<QString, QVariant>)));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "adapterRemoveSignal",this, SLOT(adapterRemoveSignal(QString)));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "deviceAddSignal",this, SLOT(deviceAddSignal(QMap<QString, QVariant>)));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "deviceAttrChanged",this, SLOT(deviceAttrChanged(QString, QMap<QString, QVariant>)));
    QDBusConnection::systemBus().connect(DBUSSERVICE,DBUSPATH, DBUSINTERFACE,
              "deviceRemoveSignal",this, SLOT(deviceRemoveSignal(QString, QMap<QString, QVariant>)));

}

PlatformAdaptor::~PlatformAdaptor()
{
    KyInfo();
}

bool PlatformAdaptor::getInit()
{
    return m_init;
}

bool PlatformAdaptor::getShowBluetooth()
{
    if(m_adapters.size() > 0) {
        return true;
    }

    return false;
}

bool PlatformAdaptor::getAdapterPower()
{
    if(m_dfAdapterPtr) {
        QVariant v = m_dfAdapterPtr->getAttr("Powered");
        //KyInfo() << v;
        if(v.isNull() || v.type() != QVariant::Bool) {
            return false;
        }
        return v.toBool();
    }
    return false;
}

QList<QMap<QString, QVariant>> PlatformAdaptor::pairedDevices() const
{
    if(m_dfAdapterPtr) {
        return m_dfAdapterPtr->getPairedDevices();
    }
    return QList<QMap<QString, QVariant>>();
}

QList<QMap<QString, QVariant> > PlatformAdaptor::notPairedDevices() const
{
    if(m_dfAdapterPtr) {
        return m_dfAdapterPtr->getNotPairedDevices();
    }
    return QList<QMap<QString, QVariant>>();
}

void PlatformAdaptor::setBluetoothConfig(QMap<QString, QVariant> attr)
{
    KyInfo() << attr;

    QString key;
    key = "Powered";
    if(attr.contains(key)) {
        bool res = false;
        if(!attr[key].toBool()) {
            res = this->whetherNeedInfoUser();
        }

        if (res) {
            QList<QVariant> params;
            this->asyncSessCall("showCloseBluetoothInfoDialog", params);
        } else {
            QList<QVariant> params;
            QMap<QString, QVariant> param;
            param[key] = attr[key];
            params.append(param);
            this->asyncCall("setDefaultAdapterAttr", params);
        }
    }

    key = "devConnect";
    if(attr.contains(key) && attr[key].type() == QVariant::String) {
        QList<QVariant> params;
        params.append(attr[key].toString());
        this->asyncCall(key, params);
    }

    key = "devDisconnect";
    if(attr.contains(key) && attr[key].type() == QVariant::String) {
        QList<QVariant> params;
        params.append(attr[key].toString());
        this->asyncCall(key, params);
    }
}

BtAdapterPtr PlatformAdaptor::getDefaultBtAdapter()
{
    return m_dfAdapterPtr;
}

void PlatformAdaptor::sendUpdatePairedDeviceSort(QStringList devlist)
{
    QList<QVariant> params;
    params.append(devlist);
    this->asyncCall("updatePairedDeviceSort", params);
}

void PlatformAdaptor::registerClient(bool v)
{
    QList<QVariant> params;
    if(v) {
        QMap<QString, QVariant> attrs;
        attrs["dbusid"] = QDBusConnection::systemBus().baseService();
        attrs["username"] = QString(qgetenv("USER").toStdString().data());
        attrs["type"] = 0;
        params.append(attrs);
        this->asyncCall("registerClient", params);
    }
    else {
        params.append(QDBusConnection::systemBus().baseService());
        this->asyncCall("unregisterClient", params);
    }
}

QDBusPendingCallWatcher *PlatformAdaptor::asyncCall(const QString & methed, const QList<QVariant> & params)
{
    if (!m_bluetooth_iface.isValid()) {
        KyWarning() << "not connect dbus server: " << m_bluetooth_iface.lastError().message();
        return nullptr;
    }

    QDBusPendingCall pendingCall = m_bluetooth_iface.asyncCallWithArgumentList(methed, params);

    return new QDBusPendingCallWatcher(pendingCall);
}

QDBusPendingCallWatcher *PlatformAdaptor::asyncSessCall(const QString &methed, const QList<QVariant> &params)
{
    if (!m_sess_bluetooth_iface.isValid()) {
        KyWarning() << "not connect dbus server: " << m_sess_bluetooth_iface.lastError().message();
        return nullptr;
    }

    QDBusPendingCall pendingCall = m_sess_bluetooth_iface.asyncCallWithArgumentList(methed, params);

    return new QDBusPendingCallWatcher(pendingCall);
}


void PlatformAdaptor::getAdapterFinished(QDBusPendingCallWatcher * watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QStringList adapterlist;
            adapterlist = reply.arguments().at(0).value<QStringList>();
            KyInfo() << adapterlist;

            for(auto iter: adapterlist) {
                if(!m_adapters.contains(iter)) {
                    BtAdapterPtr a(new BtAdapter(iter));
                    m_adapters[iter] = a;
                }

                this->getAdapterAttr(iter);
            }
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

    this->calcInit();

}

void PlatformAdaptor::getAdapterAttrFinished(QDBusPendingCallWatcher *watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QMap<QString, QVariant> attrs;
            QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
            arg >> attrs;

            QString id;

            QString key;
            key = "Addr";
            if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
                if(attrs[key].toString().length() > 0)
                    id = attrs[key].toString();
            }


            if(m_adapters.contains(id)) {
                m_adapters[id]->setAttr(attrs);
            }
            else {
                KyWarning() << "not exist adapter id:" << id;
                return;
            }

            key = "DefaultAdapter";
            if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
                //默认蓝牙适配器
                if(attrs[key].toBool()) {
                    m_dfAdapterPtr = m_adapters[id];
                }
                else {
                    if(m_dfAdapterPtr && m_dfAdapterPtr->id() == id) {
                        m_dfAdapterPtr = nullptr;
                    }
                    return;
                }

            }
            else {
                return;
            }

            this->getPairedDevices();
            this->getNotPairedDevices();
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

    this->calcInit();
}

void PlatformAdaptor::getPairedDevicesFinished(QDBusPendingCallWatcher *watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QStringList pairedlist;
            pairedlist = reply.arguments().at(0).value<QStringList>();
            KyInfo() << pairedlist;

            for(auto iter: pairedlist) {
                this->getPairedDeviceAttr(iter);
            }
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

    this->calcInit();
}

void PlatformAdaptor::getPairedDeviceAttrFinished(QDBusPendingCallWatcher *watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QMap<QString, QVariant> attrs;
            QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
            arg >> attrs;

            //KyInfo() << attrs;

            QString id, ad, key;
            key = "Addr";
            if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
                if(attrs[key].toString().length() > 0)
                    id = attrs[key].toString();
            }

            key = "Adapter";
            if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
                if(attrs[key].toString().length() > 0)
                    ad = attrs[key].toString();
            }

            if(m_dfAdapterPtr && m_dfAdapterPtr->id() == ad) {
                m_dfAdapterPtr->addPairedDevice(id, attrs);
            }
            else {
                KyWarning() << "not default adapter: " << ad;
            }
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

    this->calcInit();
}

void PlatformAdaptor::getNotPairedDevicesFinished(QDBusPendingCallWatcher *watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QStringList alldevlist;
            alldevlist = reply.arguments().at(0).value<QStringList>();
            KyInfo() << alldevlist;

            for(auto iter: alldevlist) {
                this->getNotPairedDeviceAttr(iter);
            }
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

    this->calcInit();
}

void PlatformAdaptor::getNotPairedDeviceAttrFinished(QDBusPendingCallWatcher *watcher)
{
    m_pending_count--;
    QDBusMessage reply = watcher->reply();
    if(reply.type() == QDBusMessage::ReplyMessage) {
        if(reply.arguments().size() > 0) {
            QMap<QString, QVariant> attrs;
            QDBusArgument arg = reply.arguments().at(0).value<QDBusArgument>();
            arg >> attrs;

            //KyInfo() << attrs;

            QString id, ad, key;
            key = "Addr";
            if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
                if(attrs[key].toString().length() > 0)
                    id = attrs[key].toString();
            }

            key = "Adapter";
            if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
                if(attrs[key].toString().length() > 0)
                    ad = attrs[key].toString();
            }

            key = "Paired";
            if(attrs.contains(key) && attrs[key].type() == QVariant::Bool && attrs[key].toBool()) {
                KyInfo() << id << " paired";
                goto END;
            }

            if(m_dfAdapterPtr && m_dfAdapterPtr->id() == ad) {
                m_dfAdapterPtr->addNotPairedDevice(id, attrs);
            }
            else {
                KyWarning() << "not default adapter: " << ad;
            }
        }
    }
    else {
        KyWarning() << reply.errorMessage();
    }

END:

    this->calcInit();
}

void PlatformAdaptor::adapterAddSignal(QMap<QString, QVariant> attrs)
{
    QString key, devid;
    int dfa = -1;
    key = "DefaultAdapter";

    // -1:不存在； 0: 非默认蓝牙适配器； 1： 默认蓝牙适配器
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        dfa = attrs[key].toBool() ? 1 : 0;
    }

    key = "Addr";
    if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
        if(attrs[key].toString().length() > 0)
            devid = attrs[key].toString();
    }

    if(!m_adapters.contains(devid)) {
        BtAdapterPtr a(new BtAdapter(devid));
        m_adapters[devid] = a;
    }

    if(1 == dfa) {
        if(m_dfAdapterPtr) {
            m_dfAdapterPtr->cleaDevices();
        }
        m_dfAdapterPtr = m_adapters[devid];
    }

    if(m_adapters.contains(devid)) {
       m_adapters[devid]->setAttr(attrs);
    }
}

void PlatformAdaptor::adapterAttrChanged(QString dev, QMap<QString, QVariant> attrs)
{
    QString key;
    int dfa = -1;
    key = "DefaultAdapter";

    // -1:不存在； 0: 非默认蓝牙适配器； 1： 默认蓝牙适配器
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        dfa = attrs[key].toBool() ? 1 : 0;
    }

    if(m_adapters.contains(dev)) {
        m_adapters[dev]->setAttr(attrs);
    }
    else {
        BtAdapterPtr a(new BtAdapter(dev));
        m_adapters[dev] = a;
        a->setAttr(attrs);
    }

    if(m_dfAdapterPtr && m_dfAdapterPtr->id() == dev) {
        if(0 == dfa) {
            m_dfAdapterPtr->cleaDevices();
            m_dfAdapterPtr = nullptr;
        }
    }
    else{
        if(1 == dfa) {
            if(m_adapters.contains(dev)) {
                m_dfAdapterPtr = m_adapters[dev];
                m_dfAdapterPtr->cleaDevices();
                this->getPairedDevices();
                this->getNotPairedDevices();
            }

        }
    }

}

void PlatformAdaptor::adapterRemoveSignal(QString devid)
{
    if(m_adapters.contains(devid)) {
        m_adapters.remove(devid);
    }

    if(m_dfAdapterPtr && m_dfAdapterPtr->id() == devid) {
        m_dfAdapterPtr = nullptr;
    }

    if(m_adapters.size() == 0) {
        BtInterface::getInstance()->sendIsShowBluetooth(false);
    }
}

void PlatformAdaptor::deviceAddSignal(QMap<QString, QVariant> attrs)
{
    QString key, devid, adapter;
    int paired = -1;
    key = "Paired";

    // -1:不存在； 0: 非默认蓝牙适配器； 1： 默认蓝牙适配器
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        paired = attrs[key].toBool() ? 1 : 0;
    }

    key = "Addr";
    if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
        if(attrs[key].toString().length() > 0)
            devid = attrs[key].toString();
    }

    key = "Adapter";
    if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
        if(attrs[key].toString().length() > 0)
            adapter = attrs[key].toString();
    }

    if(1 == paired) {
        if(m_dfAdapterPtr && m_dfAdapterPtr->id() == adapter) {
            m_dfAdapterPtr->addPairedDevice(devid, attrs);
        }
        else {
            KyWarning() << "not default adapter: " << adapter << ", devid: " << devid;
        }
    }
    else {
        if(m_dfAdapterPtr && m_dfAdapterPtr->id() == adapter) {
            //KyInfo() << "add not pair device";
            m_dfAdapterPtr->addNotPairedDevice(devid, attrs);
        }
    }
}

void PlatformAdaptor::deviceAttrChanged(QString devid, QMap<QString, QVariant> attrs)
{
    QString key, adapter;
    int paired = -1;
    key = "Paired";

    // -1:不存在； 0: 未配对设备； 1： 已配对设备
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        paired = attrs[key].toBool() ? 1 : 0;
    }

    if(1 == paired) {
        this->getPairedDeviceAttr(devid);
    }
    else if(-1 == paired) {
        if(m_dfAdapterPtr) {
            m_dfAdapterPtr->setDeviceAttr(devid, attrs);
        }
    }
}

void PlatformAdaptor::deviceRemoveSignal(QString devid, QMap<QString, QVariant> attrs)
{
    QString key, adapter;

    key = "Adapter";
    if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
        if(attrs[key].toString().length() > 0)
            adapter = attrs[key].toString();
    }


    if(m_adapters.contains(adapter)) {
        m_adapters[adapter]->removeDevice(devid);
    }
    else {
        KyWarning() << "not exist adapter: " << adapter;
    }
}

void PlatformAdaptor::updateClient()
{
    KyInfo();
    m_adapters.clear();
    m_dfAdapterPtr = nullptr;
    m_init = false;
    BtInterface::getInstance()->sendIsInit(m_init);
    this->getAdapter();
}


void PlatformAdaptor::getAdapter()
{
    QDBusPendingCallWatcher * watcher = this->asyncCall("getAllAdapterAddress", QList<QVariant>());
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getAdapterFinished);
    }
    else {
        KyWarning() << "null pending";
    }

}


void PlatformAdaptor::getAdapterAttr(QString id)
{
    QList<QVariant> params;
    params.append(id);
    params.append("");
    QDBusPendingCallWatcher * watcher = this->asyncCall("getAdapterAttr", params);
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getAdapterAttrFinished);
    }
    else {
        KyWarning() << "null pending";
    }

}

void PlatformAdaptor::getPairedDevices()
{
    QDBusPendingCallWatcher * watcher = this->asyncCall("getDefaultAdapterPairedDev", QList<QVariant>());
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getPairedDevicesFinished);
    }
    else {
        KyWarning() << "null pending";
    }

}

void PlatformAdaptor::getPairedDeviceAttr(QString id)
{
    QList<QVariant> params;
    params.append(id);
    QDBusPendingCallWatcher * watcher = this->asyncCall("getDevAttr", params);
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getPairedDeviceAttrFinished);
    }
    else {
        KyWarning() << "null pending";
    }

}

void PlatformAdaptor::getNotPairedDevices()
{
    QDBusPendingCallWatcher * watcher = this->asyncCall("getDefaultAdapterAllDev", QList<QVariant>());
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getNotPairedDevicesFinished);
    }
    else {
        KyWarning() << "null pending";
    }
}

void PlatformAdaptor::getNotPairedDeviceAttr(QString id)
{
    QList<QVariant> params;
    params.append(id);
    QDBusPendingCallWatcher * watcher = this->asyncCall("getDevAttr", params);
    if(watcher) {
        m_pending_count++;
        this->connect(watcher, &QDBusPendingCallWatcher::finished, this, &PlatformAdaptor::getNotPairedDeviceAttrFinished);
    }
    else {
        KyWarning() << "null pending";
    }
}

bool PlatformAdaptor::whetherNeedInfoUser()
{
    int bluetoothMouseNum = 0;
    int bluetoothKeyboardNum = 0;

    if(nullptr != m_dfAdapterPtr) {
        bluetoothMouseNum = m_dfAdapterPtr->getMouseNum();
        bluetoothKeyboardNum = m_dfAdapterPtr->getKeyboardNum();
    }

    if (0 == bluetoothMouseNum && 0 == bluetoothKeyboardNum) {
        return false;
    }

    int systemMouseAndTouchPadAmount = COMMONTOOL::getInstance()->getSystemCurrentMouseAndTouchPadDevCount();
    int systemKeyBoardAmount = COMMONTOOL::getInstance()->getSystemCurrentKeyBoardDevCount();

    KyDebug() << bluetoothMouseNum << bluetoothKeyboardNum << systemMouseAndTouchPadAmount << systemKeyBoardAmount;

    if ( -1 == systemKeyBoardAmount && -1 == systemMouseAndTouchPadAmount ) {
        return false;
    }

    if ((bluetoothMouseNum != 0) && (systemMouseAndTouchPadAmount == bluetoothMouseNum)) {
        return true;
    }

    if ((bluetoothKeyboardNum != 0) && (systemKeyBoardAmount == bluetoothKeyboardNum)) {
        return true;
    }

    return false;
}

void PlatformAdaptor::calcInit()
{
    if(0 == m_pending_count) {
        if(!m_init) {
            KyInfo() << "init suc";
            m_init = true;
            BtInterface::getInstance()->sendIsInit(m_init);
        }

    }
}


