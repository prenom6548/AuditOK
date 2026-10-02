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

#include "bluetoothinterface.h"
#include "devicemanager.h"
#include "platformadaptor.h"
#include "commontool.h"
#include <ukui-log4qt.h>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include <QProcess>


BluetoothInterface::BluetoothInterface()
{
    KyInfo();
    srand(time(0));
    COMMONTOOL::instance();
    PTAD::instance();
    init();
}

BluetoothInterface::~BluetoothInterface()
{
    KyInfo();
    if(m_ukuiProcess) {
        m_ukuiProcess->deleteLater();
    }
}

QString BluetoothInterface::getIconData(QString name, int size /*= 16*/)
{
    QIcon icon =QIcon::fromTheme(name);
    QPixmap pixmap = icon.pixmap(size, size);
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return "data:image/png;base64," + data.toBase64();
}

void BluetoothInterface::init()
{
    m_blueIcon.insert(BluetoothType::Phone, ("phone-symbolic"));
    m_blueIcon.insert(BluetoothType::Computer, ("video-display-symbolic"));
    m_blueIcon.insert(BluetoothType::Headset, ("audio-headphones-symbolic"));     //
    m_blueIcon.insert(BluetoothType::Headphones, ("audio-headphones-symbolic"));
    m_blueIcon.insert(BluetoothType::AudioVideo, ("audio-speakers-symbolic"));
    m_blueIcon.insert(BluetoothType::Keyboard, ("input-keyboard-symbolic"));
    m_blueIcon.insert(BluetoothType::Mouse, ("input-mouse-symbolic"));
    m_blueIcon.insert(BluetoothType::Tablet, ("tablet-symbolic"));
    m_blueIcon.insert(BluetoothType::Uncategorized, ("bluetooth-symbolic"));
}

void BluetoothInterface::sendUpdatePairedDevices(const QList<QMap<QString, QVariant> > &devices)
{
    emit this->updatePairedDevices();
}

void BluetoothInterface::sendUpdateNotPairedDevices(const QList<QMap<QString, QVariant> > &devices)
{
    if(nullptr == m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
        connect(m_timer,&QTimer::timeout, this, [=](){
            m_timer->deleteLater();
            m_timer = nullptr;
            KyDebug() << "sendUpdateNotPairedDevices";
            emit this->updateNotPairedDevices();
        });
        m_timer->start();
    }
}

void BluetoothInterface::sendUpdatePairedDevice(const QString &device, const QMap<QString, QVariant> &attrs)
{
    BtAdapterPtr bt = PTAD::getInstance()->getDefaultBtAdapter();
    int index = -1;
    if(bt){
        index= bt->getPairedDeviceIter(device);
    }

    emit this->updatePairedDevice(index, device, attrs);

}

void BluetoothInterface::sendUpdateNotPairedDevice(const QString &device, const QMap<QString, QVariant> &attrs)
{
    if(nullptr == m_timer) {
        BtAdapterPtr bt = PTAD::getInstance()->getDefaultBtAdapter();
        int index = -1;
        if(bt){
            index= bt->getNotPairedDeviceIter(device);
        }

        KyDebug() << "sendUpdateNotPairedDevice" << index << device; 
        emit this->updateNotPairedDevice(index, device, attrs);
    }
    
}

void BluetoothInterface::sendIsInit(bool v)
{
    KyInfo() << v;
    m_btInit = v;
    emit this->isInit();
}

void BluetoothInterface::sendIsShowBluetooth(bool v)
{
    KyInfo() << v;
    emit this->isShowBluetooth();
}

void BluetoothInterface::sendBluetoothPower(bool v)
{
    KyInfo() << v;
    m_btPower = v;
    emit this->bluetoothPower();
}

void BluetoothInterface::sendBluetoothDiscovering(bool v)
{
    KyInfo() << v;
    m_btDiscovering = v;
    emit this->bluetoothDiscovering();
}

bool BluetoothInterface::getInit()
{
    KyInfo();
    return m_btInit;
}

bool BluetoothInterface::getShowBluetooth()
{
    KyInfo();
    return PTAD::getInstance()->getShowBluetooth();
}

bool BluetoothInterface::getAdapterPower()
{
    return m_btPower;
}

bool BluetoothInterface::getAdapterDiscovering()
{
    return m_btDiscovering;
}

QVariantList BluetoothInterface::getPairedDevices()
{
    QVariantList list;

    auto arrs = PTAD::getInstance()->pairedDevices();
    for(auto iter : arrs){
        iter["LoadingIcon"] = BluetoothType::LoadingIcon;
        //iter["Battery"] = rand() % 101;
        list.append(iter);
    }

    return list;
}

QVariantList BluetoothInterface::getNotPairedDevices()
{
    QVariantList list;
    auto arrs = PTAD::getInstance()->notPairedDevices();
    for(auto iter : arrs){
        iter["LoadingIcon"] = BluetoothType::LoadingIcon;
        list.append(iter);
    }
    return list;
}

void BluetoothInterface::setAdapterPower(bool v)
{
    QMap<QString, QVariant> attr;
    attr["Powered"] = v;
    PTAD::getInstance()->setBluetoothConfig(attr);
}

void BluetoothInterface::setBluetoothConfig(QMap<QString, QVariant> attrs)
{
    KyInfo() << attrs;
    PTAD::getInstance()->setBluetoothConfig(attrs);
}

void BluetoothInterface::openBluetoothSetting()
{
    if(m_ukuiProcess) {
         //m_ukuiProcess->close();
         m_ukuiProcess->deleteLater();
    }

    m_ukuiProcess = new QProcess();
    QString cmd = "ukui-control-center";
    QStringList arg;
    arg.clear();
    arg << "-m";
    arg << "Bluetooth";
    KyInfo() << arg;
    m_ukuiProcess->startDetached(cmd,arg);
}

QVariant BluetoothInterface::getBluetoothIcon(int itype)
{
    if(m_blueIcon.contains(itype)) {
       return m_blueIcon[itype];
    }

    return m_blueIcon[BluetoothType::Uncategorized];
}

QVariant BluetoothInterface::getBluetoothBatteryIcon(int v)
{
    QString t = QString("battery-level-") + QString::number(v / 10 * 10) + QString("-symbolic");
    KyInfo() << t;
    return t;
}

void BluetoothInterface::registerClient(bool v)
{
    KyInfo() << v;
    PTAD::getInstance()->registerClient(v);
}
