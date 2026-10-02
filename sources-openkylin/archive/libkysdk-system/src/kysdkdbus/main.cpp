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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include "kysdkdbus.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::systemBus();
    if(!connection.registerService("com.kylin.kysdk.service"))
    {
        qDebug() << "com.kylin.kysdk.service 服务注册失败";
        return -1;
    }

    KySdkDisk disk;
    if(!connection.registerObject("/com/kylin/kysdk/disk", &disk,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/disk 对象注册失败";
        return -1;
    }

    KySdkCpuInfo cpuinfo;
    if(!connection.registerObject("/com/kylin/kysdk/cpuinfo", &cpuinfo,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/cpuinfo 对象注册失败";
        return -1;
    }

    KySdkNetCard netcard;
    if(!connection.registerObject("/com/kylin/kysdk/netcard", &netcard,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/netcard 对象注册失败";
        return -1;
    }

    KySdkBios bios;
    if(!connection.registerObject("/com/kylin/kysdk/bios", &bios,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/bios 对象注册失败";
        return -1;
    }

    KySdkMainBoard mainboard;
    if(!connection.registerObject("/com/kylin/kysdk/mainboard", &mainboard,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/mainboard 对象注册失败";
        return -1;
    }

    KySdkPeripheralsEnum peripheralsenum;
    if(!connection.registerObject("/com/kylin/kysdk/peripheralsenum", &peripheralsenum,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/peripheralsenum 对象注册失败";
        return -1;
    }

    KySdkDisplay display;
    if(!connection.registerObject("/com/kylin/kysdk/display", &display,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/display 对象注册失败";
        return -1;
    }

    KySdkPackageInfo packageinfo;
    if(!connection.registerObject("/com/kylin/kysdk/packageinfo", &packageinfo,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/packageinfo 对象注册失败";
        return -1;
    }

    KySdkResource resource;
    if(!connection.registerObject("/com/kylin/kysdk/resource", &resource,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/resource 对象注册失败";
        return -1;
    }

    KySdkProcess process;
    if(!connection.registerObject("/com/kylin/kysdk/process", &process,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/process 对象注册失败";
        return -1;
    }
    
    KySdkResolution resolution;
    if(!connection.registerObject("/com/kylin/kysdk/sysinfo", &resolution,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/sysinfo 对象注册失败";
        return -1;
    }

    KySdkNet net;
    if(!connection.registerObject("/com/kylin/kysdk/net", &net,
                                  QDBusConnection::ExportAllContents | QDBusConnection::ExportAllSignals))
    {
        qDebug() << "/com/kylin/kysdk/net 对象注册失败";
        return -1;
    }

    KySdkRunInfo runinfo;
    if(!connection.registerObject("/com/kylin/kysdk/runinfo", &runinfo,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/runinfo 对象注册失败";
        return -1;
    }

    KySdkGps gps;
    if(!connection.registerObject("/com/kylin/kysdk/gps", &gps,
                                  QDBusConnection::ExportAllContents))
    {
        qDebug() << "/com/kylin/kysdk/gps 对象注册失败";
        return -1;
    }

    // QDBusConnection con = QDBusConnection::sessionBus();
    // KySdkRest res;
    // if(!con.registerObject("/com/kylin/kysdk/resolution", &res,
    //                               QDBusConnection::ExportAllContents))
    // {
    //     qDebug() << "/com/kylin/kysdk/resolution 对象注册失败";
    //     return -1;
    // }

    return a.exec();
}
