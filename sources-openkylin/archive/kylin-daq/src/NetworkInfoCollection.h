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
#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>
#include "InfoCollectionBase.h"
#include <QFileInfo>

class NetworkInfoCollection : public InfoCollectionBase
{
    Q_OBJECT
public:
    NetworkInfoCollection(QObject *parent=nullptr);
    QString getNetworkType();
    QString getNetworkMac();
private:
    static constexpr const char* s_virtualNetworkPath="/sys/devices/virtual/net/";
    static constexpr const char* s_networkPath="/sys/class/net/";
    static constexpr const char* s_wirelessKey="wireless";
    static constexpr const char* s_addressKey="address";
    static constexpr const char* s_wirelessInterfaceKey="Wireless interface";
    static constexpr const char* s_ethernetInterfaceKey="Ethernet interface";
    QString notVirtualNetworkCmd;
};

#endif // NETWORKINFO_H
