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

#include "snmpFinder.h"
#include "common.h"

static bool uriCheck(const QString &uri, const QString &scheme)
{
    const QString uriSplit = "://";
    if (uri.isEmpty() || scheme.isEmpty()) {
        return false;
    }

    if (!uri.contains(uriSplit)) {
        return false;
    }

    QString uriScheme = uri.left(uri.indexOf(uriSplit));
    if (scheme != uriScheme) {
        return false;
    }

    if (uri.rightRef(uri.size() - uriSplit.size() - uri.indexOf(uriSplit)).isEmpty()) {
        return false;
    }

    return true;
}

snmpFinder::snmpFinder()
{

}

snmpFinder::~snmpFinder()
{

}

void snmpFinder::dowork()
{
    QString res = ::getRetFromCommand(QStringList{"/usr/lib/cups/backend/snmp"});
    if (res.isEmpty()) {
        return ;
    }
    QStringList rres = res.split("\n");
    for (QString r : rres) {
        parseSnmpInfo(r);
    }
}


QList<DeviceInformation> snmpFinder::getList()
{
    return  m_snmpInfoList;
}

void snmpFinder::parseSnmpInfo(QString &info)
{
    qDebug() << info;
    if (info.isEmpty()) {
        return ;
    }

    if (!info.contains(' ')) {
        return ;
    }

    QString type = info.left(info.indexOf(' '));
    if (type != "network") {
        return ;
    }

    info.remove(0, type.size() + 1);

    if (!info.contains(' ')) {
        return ;
    }

    QString uri = info.left(info.indexOf(' '));

    if (!uriCheck(uri, "socket")) {
        return ;
    }

    DeviceInformation dev;
    dev.host = uri.right(uri.size() - 3 - uri.indexOf("://"));

    info.remove(0, uri.size() + 1);

    if (!info.contains("\"")) {
        return ;
    }

    info.remove(0, 1);

    if (!info.contains("\"")) {
        return ;
    }

    QString makeAndModel = info.left(info.indexOf("\""));

    if (makeAndModel.contains(' ')) {
        dev.vendor = makeAndModel.left(makeAndModel.indexOf(' '));
        dev.model =  makeAndModel.right(makeAndModel.size() - 1 - makeAndModel.indexOf(' '));
    } else {
        dev.vendor = makeAndModel;
    }
    dev.connectType = ConnectType::InfoFrom_NETWORK_DETECT;
    specialDeviceCheck(dev.vendor, dev.model);
    m_snmpInfoList.append(dev);
}
