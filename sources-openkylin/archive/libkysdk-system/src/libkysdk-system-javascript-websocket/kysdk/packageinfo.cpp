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

#include "packageinfo.h"

Packageinfo::Packageinfo(QObject *parent) : QObject(parent)
{
   dbusConnect = -1;
   ServerName = "com.kylin.kysdk.service";
   ServerInterface = "com.kylin.kysdk.packageinfo";
   ServerPath = "/com/kylin/kysdk/packageinfo";
}

QStringList Packageinfo::getPackageList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getPackageList") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QString Packageinfo::getPackageVersion(QString packageName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPackageVersion", packageName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int Packageinfo::getPackageInstalled(QString packageName, QString versionName)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPackageInstalled", packageName, versionName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}
