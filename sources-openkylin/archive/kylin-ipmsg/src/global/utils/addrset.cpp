/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QMainWindow>
#include <QRegExp>
#include <QDebug>
#include <QEvent>
#undef Unsorted
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QDesktopServices>
#include <QHostInfo>
#include <QNetworkInterface>
#include "addrset.h"

AddrSet::AddrSet(QWidget *parent) : QWidget(parent) {}

AddrSet::~AddrSet()
{
    if (NULL != m_addrset) {
        delete m_addrset;
        m_addrset = NULL;
    }
}

AddrSet *AddrSet::getInstance()
{
    static AddrSet *m_addrset = nullptr;
    if (NULL == m_addrset) {
        m_addrset = new AddrSet();
        m_addrset->setAccessibleName("kylin-ipmsg_addrset_AddrSet_addrset");
    }
    return m_addrset;
}


//获取IP地址
QString AddrSet::getHostIpAddress()
{
    QString ipAddress = "";
    QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();
    // 获取第一个本主机的IPv4地址
//    int nListSize = ipAddresses.size();
//    for (int i = 0; i < nListSize; ++i) {
    for (const QHostAddress &a : ipAddresses) {
        if (a != QHostAddress::LocalHost && a.toIPv4Address()) {
            ipAddress = a.toString();
            break;
        }
    }
    // 如果没有找到，则以本地IP地址为IP
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}

//获取MAC地址
QString AddrSet::getHostMacAddress()
{
    QList<QNetworkInterface> networkInterface = QNetworkInterface::allInterfaces(); // 获取所有网络接口列表
    int nCnt = networkInterface.count();
    QString macAddress = "";
    for (int i = 0; i < nCnt; i++) {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if (networkInterface[i].flags().testFlag(QNetworkInterface::IsUp) && networkInterface[i].flags().testFlag(QNetworkInterface::IsRunning)
            && !networkInterface[i].flags().testFlag(QNetworkInterface::IsLoopBack)) {
            macAddress = networkInterface[i].hardwareAddress();
            break;
        }
    }
    return macAddress;
}

QString AddrSet::getLocalHost()
{
    //获取主机名
    QString localHost = QHostInfo::localHostName();
    return localHost;
}
