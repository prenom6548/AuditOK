/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "deviceFinder.h"
#include "usbFinder.h"
#include "snmpFinder.h"
#include <QThread>
#include <QTimer>
#include "common.h"
#include "connect.h"
#include <sane/sane.h>
#include <buriedpoint.hpp>
#include "base_info.h"
#include <libkydatacollect.h>

bool deviceFinder::checkDeviceExistInSane(DeviceInformation &deviceInformation)
{
    if (m_saneInfoList.isEmpty()) {
        return false;
    }
    if (deviceInformation.model.isEmpty() || deviceInformation.vendor.isEmpty()) {
        return false;
    }

    for (auto saneInfo : m_saneInfoList) {
        if (saneInfo.vendor == deviceInformation.vendor && saneInfo.model == deviceInformation.model) {
            return true;
        }
    }
    return false;
}

bool packageIsInstalled(const QString &debName, const QString &packageName)
{
    // brscan5chn_1.2.5-0_amd64.deb
    QString packageVersion = getPackageVersion(packageName);
    qDebug() << packageVersion;
    if (packageVersion.isEmpty()) {
        return false;
    }
    QString deb(debName);
    if (!deb.contains("_")) {
        return false;
    }

    deb.remove(0, deb.indexOf("_") + 1);

    if (!deb.contains("_")) {
        return false;
    }

    QString debVersion = deb.left(deb.indexOf("_"));
    if (debVersion.isEmpty()) {
        return false;
    }
//    if (debVersion != packageVersion) {
//        return false;
//    }
    return true;
}

bool deviceFinder::infoCheck(DeviceInformation &deviceInformation) {
//    if (m_isServerAddressConnected) {
        getPackagesNameFromHttp(deviceInformation);
//    } else {
//        if (checkDeviceExistInSane(deviceInformation)) {
//            return false;
//        } else {
//            if (deviceInformation.type == DeviceType::SCANNER) {
//                return true;
//            }
//            return false;
//        }
//    }
    qDebug() << deviceInformation;
    if (!(deviceInformation.type == DeviceType::SCANNER || deviceInformation.type == DeviceType::PRINTER)) {
        return false;
    }
    int needInstall = false;
    for (int i = 0; i < deviceInformation.packageNameList.size(); i++) {
        if (packageIsInstalled(deviceInformation.debNameList[i], deviceInformation.packageNameList[i]) == false) {
            needInstall = true;
        }
    }
    if (needInstall == true) {
        return true;
    }
    return false;
}

deviceFinder::deviceFinder(QObject *parent)
    : QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &deviceFinder::dowork);

    connect(this, &deviceFinder::finished, m_thread, &QThread::quit);
    connect(this, &deviceFinder::finished, this, &deviceFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

deviceFinder::~deviceFinder()
{

}

void deviceFinder::dowork()
{
    if (tryConnectFromHost(BaseInfo::instance().getServerAddress())) {
        QMap<QString, QString> buried_data;
        buried_data.insert(QString("functionName"), QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerServerAddressConnectFail)));
        buried_data.insert("action", QString(QString("connect to ") + BaseInfo::instance().getServerAddress() + " failed."));
        buried_data.insert("function", "in deviceFinder.cpp function dowork()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }

        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "address_connect", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);

        m_isServerAddressConnected = false;
        qDebug() << "cannot connect to server address!";
    } else {
        QMap<QString, QString> buried_data;
        buried_data.insert(QString("functionName"), QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerServerAddressConnectSuccess)));
        buried_data.insert("action", QString(QString("connect to ") + BaseInfo::instance().getServerAddress() + " succeed."));
        buried_data.insert("function", "in deviceFinder.cpp function dowork()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "address_connect", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);

        m_isServerAddressConnected = true;
    }
    if (m_isServerAddressConnected == false) {
        const SANE_Device **saneDeviceList{nullptr};
        sane_get_devices(&saneDeviceList, 0);
        for (int i = 0; saneDeviceList[i]; i++) {
            DeviceInformation info;
            info.vendor = QString("%1").arg(saneDeviceList[i]->vendor);
            info.model  = QString("%1").arg(saneDeviceList[i]->model);
            specialDeviceCheck(info.model, info.vendor);
            m_saneInfoList.append(info);
        }
    }
    usbFinder u;
    u.dowork();
    auto list = u.getList();
    qDebug() << list;
    for (int i = 0; i < list.size(); i++) {
        if (infoCheck(list[i])) {
            m_infoList.append(list[i]);
        }
    }
    list.clear();
    snmpFinder s;
    s.dowork();
    list = s.getList();
    qDebug() << list;
    for (int i = 0; i < list.size(); i++) {
        if (infoCheck(list[i])) {
            m_infoList.append(list[i]);
        }
    }

    // 添加埋点信息
    if(m_isServerAddressConnected){
        buriedDeviceInfomation(m_infoList);
    }

    succeed();
}

QList<DeviceInformation> deviceFinder::getList()
{
    return m_infoList;
}

void deviceFinder::doPackageWork()
{
    this->succeed();
}

void deviceFinder::buriedDeviceInfomation(QList<DeviceInformation> list)
{
    for(int i = 0; i < list.size(); i++){
        QMap<QString, QString> buried_data;
        buried_data.insert("functionName",  QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::kylinScannerDeviceAndDriverName)));
        buried_data.insert("deviceVendor", list[i].vendor);
        buried_data.insert("deviceName", list[i].name);
        buried_data.insert("deviceModel", list[i].model);
        buried_data.insert("deviceSerial", list[i].serial);
        buried_data.insert("deviceVid", list[i].VID);
        buried_data.insert("devicePid", list[i].PID);
        QString deb;
        for(int j = 0; j < list[i].debNameList.length(); ++j){
            deb += list[i].debNameList.at(j);
            deb += ";";
        }
        buried_data.insert("debDriver", deb);

        QString pkg;
        for(int j = 0; j < list[i].packageNameList.length(); ++j){
            pkg += list[i].packageNameList.at(j);
            deb += ";";
        }
        buried_data.insert("packageNameList", pkg);

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "device_info", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);
    }
}
