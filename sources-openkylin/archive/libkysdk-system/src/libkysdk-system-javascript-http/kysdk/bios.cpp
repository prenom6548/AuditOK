/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "bios.h"

Bios::Bios(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.kylin.kysdk.service";
    ServerPath = "/com/kylin/kysdk/bios";
    ServerInterface = "com.kylin.kysdk.bios";

}

Bios::Bios(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.kylin.kysdk.service";
    ServerPath = "/com/kylin/kysdk/bios";
    ServerInterface = "com.kylin.kysdk.bios";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Bios::deal_bios()
{
    qDebug()<<"bios url path is "<<path;

    if(path_list.size()>1)
    {
        return deal_bios_path_para();
    }else{
        return deal_bios_path();
    }
}

QJsonObject Bios::deal_bios_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/bios/getBiosVendor")
    {
        QString res = getBiosVendor();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/bios/getBiosVersion")
    {
        QString res = getBiosVersion();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

QJsonObject Bios::deal_bios_path_para()
{
    QJsonObject json;
    return json;
}

QString Bios::getBiosVendor()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getBiosVendor");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Bios::getBiosVersion()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getBiosVersion");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

