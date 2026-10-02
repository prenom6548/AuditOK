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

#include "devicemanager.h"
#include <ukui-log4qt.h>
#include "bluetoothinterface.h"
#include "platformadaptor.h"
#include <QStringList>
#include "commontool.h"


BtDevice::BtDevice(QString id)
{
    KyInfo() << id;
    m_id = id;
}

BtDevice::~BtDevice()
{
    KyInfo() << m_id;
}

void BtDevice::setAttr(QMap<QString, QVariant> attrs)
{
    //KyInfo() << attrs;
    if(m_attrs.isEmpty()) {
        m_attrs = attrs;
        return;
    }

    auto tmp = m_attrs;
    for (const auto &key : attrs.keys()) {
        m_attrs[key] = attrs[key];
    }

    bool pair = false;
    QVariant t = this->getAttr("Paired");
    if(t.type() == QVariant::Bool) {
        pair = t.toBool();
    }

    if(tmp != m_attrs) {
        if(pair)
            BtInterface::getInstance()->sendUpdatePairedDevice(m_id, m_attrs);
        else {
            BtInterface::getInstance()->sendUpdateNotPairedDevice(m_id, m_attrs);
        }
           
    }
}

QMap<QString, QVariant> BtDevice::getAttr()
{
    return m_attrs;
}

QVariant BtDevice::getAttr(QString key)
{
    if(m_attrs.contains(key)) {
        return m_attrs[key];
    }
    return QVariant();
}

int BtDevice::getRssi()
{
    int rssi = 100;
    QVariant t = this->getAttr("Rssi");
    if(t.type() == QVariant::Int) {
        rssi = t.toInt();
    }
    return rssi;
}

bool BtDevice::isMouse()
{
    int itype = BluetoothType::Uncategorized;
    QVariant t = this->getAttr("Type");
    if (t.type() == QVariant::Int) {
        itype = t.toInt();
    }

    if (BluetoothType::Mouse == itype ) {
        return true;
    }

    return false;
}

bool BtDevice::isKeyboard()
{
    int itype = BluetoothType::Uncategorized;
    QVariant t = this->getAttr("Type");
    if (t.type() == QVariant::Int) {
        itype = t.toInt();
    }

    if (BluetoothType::Keyboard == itype ) {
        return true;
    }

    return false;
}



///////////////////BtAdapter//////////////////////////////
BtAdapter::BtAdapter(QString id)
{
    KyInfo() << id;
    m_id = id;
}

BtAdapter::~BtAdapter()
{
    KyInfo() << m_id;
}

void BtAdapter::setAttr(QMap<QString, QVariant> attrs)
{
    //KyInfo() << attrs;
    QString key;
    int newdefault = -1;
    int olddefault = -1;

    key = "DefaultAdapter";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        newdefault = attrs[key].toBool() ? 1 : 0;
    }
    if(m_attrs.contains(key) && m_attrs[key].type() == QVariant::Bool) {
        olddefault = m_attrs[key].toBool() ? 1 : 0;
    }

    for (const auto &iter : attrs.keys()) {
        m_attrs[iter] = attrs[iter];
    }

    key = "Powered";
    if(-1 != newdefault && olddefault != newdefault) {
        if(newdefault) {
            if(m_attrs.contains(key) && m_attrs[key].type() == QVariant::Bool){
                BtInterface::getInstance()->sendBluetoothPower(m_attrs[key].toBool());
            }
        }
    }
    else {
        if(1 == olddefault && attrs.contains(key) && attrs[key].type() == QVariant::Bool){
            BtInterface::getInstance()->sendBluetoothPower(attrs[key].toBool());
        }
    }


    key = "Discovering";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        if(m_attrs.contains("DefaultAdapter") && m_attrs["DefaultAdapter"].type() == QVariant::Bool
                && m_attrs["DefaultAdapter"].toBool()) {
            BtInterface::getInstance()->sendBluetoothDiscovering(attrs[key].toBool());
        }
    }

}

QMap<QString, QVariant> BtAdapter::getAttr()
{
    return m_attrs;
}

QVariant BtAdapter::getAttr(QString key)
{
    if(m_attrs.contains(key)) {
        return m_attrs[key];
    }
    return QVariant();
}

void BtAdapter::addPairedDevice(QString id, QMap<QString, QVariant> attrs)
{
    BtDevicePtr p = nullptr;
    if(m_notPairedDevices.contains(id)) {
        p = m_notPairedDevices[id];
        m_notPairedDevices.remove(id);
        this->calcNotPairedDeviceList(id);
    }

    if(!m_pairedDevices.contains(id)) {
        if(p) {
            KyInfo() << "use exist devptr: " << id;
            p->setAttr(attrs);
            m_pairedDevices[id] = p;
        }
        else {
            BtDevicePtr a(new BtDevice(id));
            a->setAttr(attrs);
            m_pairedDevices[id] = a;
        }
        this->calcDeviceList(id);
    }
    else {
        KyInfo() << "devid exist : " << id;
        m_pairedDevices[id]->setAttr(attrs);
        this->calcDeviceList(id);
    }


}

void BtAdapter::addNotPairedDevice(QString id, QMap<QString, QVariant> attrs)
{
    if(!m_notPairedDevices.contains(id)) {
        BtDevicePtr a(new BtDevice(id));
        a->setAttr(attrs);
        m_notPairedDevices[id] = a;
        this->calcNotPairedDeviceList(id);
    }
    else {
        KyInfo() << "devid exist : " << id;
        m_notPairedDevices[id]->setAttr(attrs);
        this->calcNotPairedDeviceList(id);
    }
}

void BtAdapter::setDeviceAttr(QString id, QMap<QString, QVariant> attrs)
{
    if(m_pairedDevices.contains(id)) {
        m_pairedDevices[id]->setAttr(attrs);
        this->calcDeviceList(id);
    }

    if(m_notPairedDevices.contains(id)) {
        bool rssichange = attrs.contains("Rssi");
        m_notPairedDevices[id]->setAttr(attrs);
        this->calcNotPairedDeviceList(id, rssichange);
    }
}

QList<QMap<QString, QVariant> > BtAdapter::getPairedDevices()
{
    QList<QMap<QString, QVariant> > devices;
    for (const auto & iter: m_connectDevices) {
        if(m_pairedDevices.contains(iter)) {
             devices.append(m_pairedDevices[iter]->getAttr());
        }
    }

    for (const auto & iter: m_noConnectDevices) {
        if(m_pairedDevices.contains(iter)) {
             devices.append(m_pairedDevices[iter]->getAttr());
        }
    }

    return devices;
}

QList<QMap<QString, QVariant> > BtAdapter::getNotPairedDevices()
{
    QList<QMap<QString, QVariant> > devices;
    for(const auto & iter: m_notPairedDevicelist) {
        if(m_notPairedDevices.contains(iter)) {
            devices.append(m_notPairedDevices[iter]->getAttr());
        }
    }

    return devices;
}

void BtAdapter::removeDevice(QString id)
{
    if(m_pairedDevices.contains(id)) {
        m_pairedDevices.remove(id);
        this->calcDeviceList(id);
    }

    if(m_notPairedDevices.contains(id)) {
        m_notPairedDevices.remove(id);
        this->calcNotPairedDeviceList(id);
    }
}

void BtAdapter::cleaDevices()
{
    m_pairedDevices.clear();
    m_connectDevices.clear();
    m_noConnectDevices.clear();
    m_notPairedDevices.clear();
    m_notPairedDevicelist.clear();
}

int BtAdapter::getPairedDeviceIter(const QString & id)
{
    QList<QString> tmp;
    tmp.append(m_connectDevices);
    tmp.append(m_noConnectDevices);
    return tmp.indexOf(id);
}

int BtAdapter::getNotPairedDeviceIter(const QString & id)
{
    return m_notPairedDevicelist.indexOf(id);
}

int BtAdapter::getMouseNum()
{
    int icount = 0;
    for (auto & iter : m_connectDevices) {
        if(m_pairedDevices.contains(iter)) {
            if (m_pairedDevices[iter]->isMouse()) {
                icount++;
            }
        }
    }
    return icount;
}

int BtAdapter::getKeyboardNum()
{
    int icount = 0;
    for (auto & iter : m_connectDevices) {
        if(m_pairedDevices.contains(iter)) {
            if (m_pairedDevices[iter]->isKeyboard()) {
                icount++;
            }
        }
    }
    return icount;
}

void BtAdapter::calcDeviceList(QString id)
{
    bool change = false;
    if(m_pairedDevices.contains(id)) {
        QVariant t = m_pairedDevices[id]->getAttr("Connected");
        bool isconnect = false;
        if(t.type() == QVariant::Bool) {
            isconnect = t.toBool();
        }

        if(isconnect) {
            if(m_connectDevices.contains(id)) {
                return;
            }
            else {
                m_noConnectDevices.removeOne(id);
                m_connectDevices.insert(0, id);
                change = true;
            }
        }
        else {
            if(m_noConnectDevices.contains(id))
                return;
            else {
                m_connectDevices.removeOne(id);
                m_noConnectDevices.insert(0, id);
                change = true;
            }
        }
    }
    else {
        m_noConnectDevices.removeOne(id);
        m_connectDevices.removeOne(id);
        change = true;
    }

    if(change) {
        auto devices = getPairedDevices();
        BtInterface::getInstance()->sendUpdatePairedDevices(devices);
        QStringList tmp;
        tmp.append(m_connectDevices);
        tmp.append(m_noConnectDevices);
        //tmp.res
        std::reverse(tmp.begin(), tmp.end());

        PTAD::getInstance()->sendUpdatePairedDeviceSort(tmp);

        KyInfo() << m_connectDevices << m_noConnectDevices;
    }
}

void BtAdapter::calcNotPairedDeviceList(QString id, bool rssichange /*= false*/)
{
    bool change = false;
    KyInfo() << id << rssichange;

    if(m_notPairedDevices.contains(id)) {
        if(!m_notPairedDevicelist.contains(id)) {
            this->insertDevid(id);
            change = true;
        }
    }
    else {
        if(m_notPairedDevicelist.contains(id)) {
            m_notPairedDevicelist.removeOne(id);
            change = true;
        }
    }

    if(rssichange) {
        this->insertDevid(id);
        change = true;
    }

    if(change) {
        auto devices = getNotPairedDevices();
        BtInterface::getInstance()->sendUpdateNotPairedDevices(devices);
    }
}

void BtAdapter::insertDevid(QString id)
{
    m_notPairedDevicelist.removeOne(id);
    int rssi1 = -256;
    int rssi2 = -256;
    int insertindex = 0;

    if(!m_notPairedDevices.contains(id)) {
        KyWarning() << "not exist devid:" << id;
        return;
    }

    rssi1 = m_notPairedDevices[id]->getRssi();
    for(auto iter : m_notPairedDevicelist)
    {
        if(m_notPairedDevices.contains(iter)) {
            rssi2 = m_notPairedDevices[iter]->getRssi();
            if(rssi2 < rssi1) {
                m_notPairedDevicelist.insert(insertindex, id);
                return;
            }
            else if (rssi2 == rssi1) {
                if(iter < id) {
                    m_notPairedDevicelist.insert(insertindex, id);
                    return;
                }
            }
        }
        insertindex++;
    }
    m_notPairedDevicelist.append(id);
}

