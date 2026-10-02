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
 
#include "NetworkInfoCollection.h"

constexpr const char* NetworkInfoCollection::s_virtualNetworkPath;
constexpr const char* NetworkInfoCollection::s_networkPath;
constexpr const char* NetworkInfoCollection::s_wirelessKey;
constexpr const char* NetworkInfoCollection::s_addressKey;
constexpr const char* NetworkInfoCollection::s_wirelessInterfaceKey;
constexpr const char* NetworkInfoCollection::s_ethernetInterfaceKey;
NetworkInfoCollection::NetworkInfoCollection(QObject *parent)
    : InfoCollectionBase(parent)
    , notVirtualNetworkCmd(QString("ls %1|grep -v \"$(ls %2)\"").arg(s_networkPath,s_virtualNetworkPath))
{

}

QString NetworkInfoCollection::getNetworkType()
{
    QList<QString> networkList;
    QList<QString> returnNetworkTypeList;
    getPopenResult(notVirtualNetworkCmd,networkList);
    foreach (QString tempNetworkPath, networkList) {
        QFileInfo tempFileInfo(QString("%1/%2/%3").arg(s_networkPath,tempNetworkPath,s_wirelessKey));
        if (tempFileInfo.exists() && tempFileInfo.isDir()) {
            returnNetworkTypeList.append(QString(s_wirelessInterfaceKey));
        } else {
            returnNetworkTypeList.append(QString(s_ethernetInterfaceKey));
        }
    }
    return returnNetworkTypeList.join(s_separator);
}

QString NetworkInfoCollection::getNetworkMac()
{
    QList<QString> networkList;
    QList<QString> returnNetworkTypeList;
    getPopenResult(notVirtualNetworkCmd,networkList);
    foreach (QString tempNetworkPath, networkList) {
        QString tempStr;
        getOnePopenResult(QString("cat %1/%2/%3").arg(s_networkPath,tempNetworkPath,s_addressKey),tempStr);
        if (!tempStr.isEmpty())
        {
            returnNetworkTypeList.append(tempStr);
        }
    }
    return returnNetworkTypeList.join(s_separator);
}
