/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <arpa/inet.h>

#include <bitset>

#include "kylin-dbus-interface.h"
#include "utils.h"
#include "wificonfigdialog.h"
#include "return_code.h"
#include <vector>

#include <QTextCodec>
#include <QByteArray>
#include <QMetaType>
#include <QDBusMetaType>
#include <QScreen>
#include <QBitArray>
namespace
{

void quitThread(QThread *thread)
{
    Q_ASSERT(thread);
    if (thread) {
        thread->quit();
        if (!thread->wait(2000)) {
            thread->terminate();
            thread->wait();
        }
    }
}

} // namespace

KylinDBus::KylinDBus(MainWindow *mainWindow, QObject *parent) : QObject(parent)
{
    // TODO:    this->mw = mainWindow;

    getBlockNeededDevice(); //获取需要屏蔽的网卡信息（除去NM_DEVICE_TYPE_ETHERNET NM_DEVICE_TYPE_WIFI两类）
    getObjectPath();        //获取dbus中 lan 与 WiFi 的device路径
    getPhysicalCarrierState(0); //初始化获取网线插入状态
    getLanHwAddressState();     //获取有线网Mac地址
    getWiredCardName();         //获取有线网卡名称
    // initTaskbarGsetting(); //初始化taskbar的GSetting方法
    getWifiSwitchState();   //初始化wifi开关GSetting通信方法
    initTransparentState(); //初始化窗口透明度的GSetting方法
    getLanIpChanged();

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"), QString("PropertiesChanged"), this,
                                         SLOT(onPropertiesChanged(QVariantMap)));

    QDBusConnection::systemBus().connect(
        QString("org.freedesktop.NetworkManager"), QString("/org/freedesktop/NetworkManager"),
        QString("org.freedesktop.NetworkManager"), QString("AutoConnect"), this, SLOT(onAutoConnect()));

    // TODO:    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
    //                                          QString("/org/freedesktop/NetworkManager"),
    //                                          QString("org.freedesktop.NetworkManager"),
    //                                          QString("DeviceAdded"), mw, SLOT(onNetworkDeviceAdded(QDBusObjectPath) )
    //                                          );

    // TODO:    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
    //                                          QString("/org/freedesktop/NetworkManager"),
    //                                          QString("org.freedesktop.NetworkManager"),
    //                                          QString("DeviceRemoved"), mw,
    //                                          SLOT(onNetworkDeviceRemoved(QDBusObjectPath) ) );

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager/Settings"),
                                         QString("org.freedesktop.NetworkManager.Settings"), QString("NewConnection"),
                                         this, SLOT(onNewConnection(QDBusObjectPath)));

    QDBusConnection::systemBus().connect(
        QString("org.freedesktop.NetworkManager"), QString("/org/freedesktop/NetworkManager/Settings"),
        QString("org.freedesktop.NetworkManager.Settings"), QString("ConnectionRemoved"), this,
        SLOT(onConnectionRemoved(QDBusObjectPath)));

    time = new QTimer(this);
    time->setTimerType(Qt::PreciseTimer);
    QObject::connect(time, SIGNAL(timeout()), this, SLOT(slot_timeout()));

    if (mw) {
        //        QObject::connect(this, SIGNAL(updateWiredList(int)), mw, SLOT(onBtnNetListClicked(int)));
        //        QObject::connect(this, SIGNAL(newConnAdded(int)), mw, SLOT(onNewConnAdded(int)));
        //        QObject::connect(this, SIGNAL(updateWirelessList()), mw, SLOT(on_wifi_changed()));
        // TODO:        connect(this, &KylinDBus::updateWiredList, mw, &MainWindow::onBtnNetListClicked);
        // TODO:        connect(this, &KylinDBus::newConnAdded, mw, &MainWindow::onNewConnAdded);
        // TODO:        connect(this, &KylinDBus::updateWirelessList, mw, &MainWindow::on_wifi_changed);
    }

    mUtils = new Utils();
    mUtilsThread = new QThread(this);
    mUtils->moveToThread(mUtilsThread);
    connect(mUtilsThread, &QThread::finished, mUtils, &Utils::deleteLater);
    connect(this, SIGNAL(requestSendDesktopNotify(QString)), mUtils, SLOT(onRequestSendDesktopNotify(QString)),
            Qt::QueuedConnection);
    QTimer::singleShot(1, this, [=] {
        mUtilsThread->start();
    });
}

KylinDBus::~KylinDBus()
{
    quitThread(mUtilsThread);
}

///////////////////////////////////////////////////////////////////////////////
//下方使用Dbus 进程通信方法

//获取dbus中 lan 与 WiFi 的device路径
void KylinDBus::getObjectPath()
{
    foreach (QDBusObjectPath mPath, multiWiredPaths) {
        QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"), QString(mPath.path()),
                                                QString("org.freedesktop.NetworkManager.Device.Wired"),
                                                QString("PropertiesChanged"), this,
                                                SLOT(onLanWiredPropertyChanged(QVariantMap)));

        QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"), QString(mPath.path()),
                                                QString("org.freedesktop.DBus.Properties"),
                                                QString("PropertiesChanged"), this,
                                                SLOT(onLanPropertyChanged(QString, QVariantMap, QStringList)));
        multiWiredPaths.removeOne(mPath);
    }

    foreach (QDBusObjectPath mPath, multiWirelessPaths) {
        QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"), QString(mPath.path()),
                                                QString("org.freedesktop.DBus.Properties"),
                                                QString("PropertiesChanged"), this,
                                                SLOT(onWlanPropertyChanged(QString, QVariantMap, QStringList)));
        multiWirelessPaths.removeOne(mPath);
    }

    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                               "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

    //先获取所有的网络设备的设备路径
    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface.call("GetAllDevices");
    if (!obj_reply.isValid()) {
        qDebug() << "execute dbus method 'GetAllDevices' is invalid in func getObjectPath()";
    }

    QList<QDBusObjectPath> obj_paths = obj_reply.value();

    //再判断有无有线设备和无线设备的路径
    foreach (QDBusObjectPath obj_path, obj_paths) {
        QDBusInterface interface("org.freedesktop.NetworkManager", obj_path.path(),
                                 "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus());

        QDBusReply<QString> reply = interface.call("Introspect");
        if (!reply.isValid()) {
            qDebug() << "execute dbus method 'Introspect' is invalid in func getObjectPath()";
        }

        if (reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wired") != -1) {
            //表明有有线网设备
            if (m_unvisibleDevicePaths.contains(obj_path.path())) {
                qDebug() << "[getObjectPath] unvisible device : " << obj_path.path();
                continue;
            }
            multiWiredPaths.append(obj_path);
        } else if (reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wireless") != -1) {
            //表明有wifi设备
            multiWirelessPaths.append(obj_path);
            isWirelessCardOn = true;
        }
    }

    if (!multiWiredPaths.isEmpty()) {
        foreach (QDBusObjectPath mWiredPath, multiWiredPaths) {
            QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"), QString(mWiredPath.path()),
                                                 QString("org.freedesktop.NetworkManager.Device.Wired"),
                                                 QString("PropertiesChanged"), this,
                                                 SLOT(onLanWiredPropertyChanged(QVariantMap)));

            QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"), QString(mWiredPath.path()),
                                                 QString("org.freedesktop.DBus.Properties"),
                                                 QString("PropertiesChanged"), this,
                                                 SLOT(onLanPropertyChanged(QString, QVariantMap, QStringList)));
        }
    } else {
        qDebug() << "Can not find wired device object path when using dbus.";
    }

    if (!multiWirelessPaths.isEmpty()) {
        foreach (QDBusObjectPath mWirelessPath, multiWirelessPaths) {
            QDBusConnection::systemBus().connect(
                QString("org.freedesktop.NetworkManager"), QString(mWirelessPath.path()),
                QString("org.freedesktop.DBus.Properties"), QString("PropertiesChanged"), this,
                SLOT(onWlanPropertyChanged(QString, QVariantMap, QStringList)));
        }

        getWirelessCardName(); //获取无线网卡名称
    } else {
        qDebug() << "Can not find wireless device object path when using dbus.";
    }
}

void KylinDBus::getBlockNeededDevice()
{
    QDBusInterface interfaceDev("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    if (!interfaceDev.isValid()) {
        qWarning() << "Error network dbus service get fail";
        return;
    }

    //先获取所有的网络设备的设备路径
    QDBusReply<QList<QDBusObjectPath>> obj_reply = interfaceDev.call("GetAllDevices");
    if (!obj_reply.isValid()) {
        qDebug() << "execute dbus method 'GetAllDevices' is invalid in func getObjectPath()";
        return;
    }

    QList<QDBusObjectPath> obj_paths = obj_reply.value();
    m_unvisibleDeviceNames.clear();
    m_unvisibleDevicePaths.clear();
    foreach (QDBusObjectPath objPath, obj_paths) {
        QDBusInterface interface("org.freedesktop.NetworkManager", objPath.path(),
                                 "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());

        int deviceType = interface.property("DeviceType").toInt();
        if ((deviceType != NM_DEVICE_TYPE_ETHERNET) && (deviceType != NM_DEVICE_TYPE_WIFI)) {
            QStringList Udi = interface.property("Udi").toString().split("/");
            if (!Udi.isEmpty()) {
                m_unvisibleDeviceNames.append(Udi.back());
                m_unvisibleDevicePaths.append(objPath.path());
                qDebug() << "[getBlockNeededDevice] unvisibleDevices add " << Udi.back() << " path:" << objPath.path();
            }
        }
    }
    return;
}

//获取有线网Mac地址
void KylinDBus::getLanHwAddressState()
{
    foreach (QString mStr, multiWiredMac) {
        multiWiredMac.removeOne(mStr);
    }

    foreach (QDBusObjectPath mPath, multiWiredPaths) {
        QDBusInterface lanInterface("org.freedesktop.NetworkManager", mPath.path(), "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());

        QDBusReply<QVariant> lanReply =
            lanInterface.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "HwAddress");
        if (!lanReply.isValid()) {
            qDebug() << "can not get the attribute 'HwAddress' in func getLanHwAddressState()";
        } else {
            QString dbusLanMac = lanReply.value().toString();
            multiWiredMac.append(dbusLanMac);
        }
    }

    if (multiWiredMac.size() > 0) {
        dbusMacDefault = multiWiredMac.at(0);
    } else {
        dbusMacDefault = "--";
    }
}

//获取有线网卡名称
void KylinDBus::getWiredCardName()
{
    foreach (QString mStr, multiWiredIfName) {
        multiWiredIfName.removeOne(mStr);
    }

    foreach (QDBusObjectPath mPath, multiWiredPaths) {
        QDBusInterface lanInterface("org.freedesktop.NetworkManager", mPath.path(), "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());

        QDBusReply<QVariant> lanReply = lanInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
        if (!lanReply.isValid()) {
            qDebug() << "can not get the attribute 'Interface' in func getWiredCardName()";
        } else {
            QString dbusLanCardName = lanReply.value().toString();
            multiWiredIfName.append(dbusLanCardName);
        }
    }
}

//获取无线网卡名称
void KylinDBus::getWirelessCardName()
{
    if (multiWirelessPaths.size() == 0) {
        dbusWiFiCardName = "";
        return;
    }

    QDBusInterface lanInterface("org.freedesktop.NetworkManager", multiWirelessPaths.at(0).path(),
                                "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusReply<QVariant> lanReply = lanInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
    if (!lanReply.isValid()) {
        qDebug() << "can not get the attribute 'Interface' in func getWirelessCardName()";
    } else {
        dbusWiFiCardName = lanReply.value().toString();
    }
}

/**
 * @brief KylinDBus::getLanInfo
 * 获取有线网络具体信息，并校验网络名称，以防止命令行获取的网络名称中含有空格等不能识别
 * @param uuid       网络uuid
 * @param isActNet   是否是已连接网络
 * @param lanName    网络名称,入参
 */
void KylinDBus::getLanInfo(QString uuid, bool isActNet, QString &lanName)
{
    dbusIfName = "--";
    QDBusInterface settingsInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                                     "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    if (uuid == nullptr || uuid.isEmpty()) {
        return;
    }

    QDBusReply<QDBusObjectPath> objPathReply = settingsInterface.call("GetConnectionByUuid", uuid);

    QDBusObjectPath objPath = objPathReply.value();
    QDBusInterface connectionInterface("org.freedesktop.NetworkManager", objPath.path(),
                                       "org.freedesktop.NetworkManager.Settings.Connection",
                                       QDBusConnection::systemBus());

    if (!connectionInterface.isValid()) {
        qWarning() << "[KylinDBus] getLanInfo: connectionInterface unvaliable";
        return;
    }

    QDBusMessage result = connectionInterface.call("GetSettings"); // get information of this network

    if (result.arguments().isEmpty()) {
        qWarning() << "[KylinDBus] getLanInfo: result  isEmpty";
        return;
    }

    const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QMap<QString, QVariant>> map;
    dbusArg1st >> map;

    for (QString outsideKey : map.keys()) {
        QMap<QString, QVariant> outsideMap = map.value(outsideKey);
        if (outsideKey == "connection") {
            for (QString inner_key : outsideMap.keys()) {
                // 获取网络名,命令行读取不能判断名称后面的空格，需要此方法校验
                if (QString::compare("id", inner_key) == 0) {
                    QString tmpName = outsideMap.value(inner_key).toString();
                    if (QString::compare(tmpName, lanName) != 0) {
                        qDebug() << "[KylinDBus] getLanInfo: ";
                        qDebug() << "the name(" << lanName << ") obtained from command line doesn't match dbus("
                                 << tmpName << "), use dbus";
                        lanName = tmpName;
                    }
                } else if (QString::compare("interface-name", inner_key) == 0) {
                    dbusIfName = outsideMap.value("interface-name").toString();
                }
            }
        } else if (outsideKey == "ipv4") {
            for (QString innerKey : outsideMap.keys()) {
                //获取ipv4
                if (innerKey == "address-data") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    QVector<QMap<QString, QVariant>> addressVector;
                    QMap<QString, QVariant> map;

                    dbusArg2nd.beginArray();

                    while (!dbusArg2nd.atEnd()) {
                        QMap<QString, QVariant> tempMap;
                        dbusArg2nd >> tempMap; // append map to a vector here if you want to keep it
                        addressVector.append(tempMap);
                    }
                    dbusArg2nd.endArray();

                    dbusLanIpv4.clear();
                    for (int i = 0; i < addressVector.size(); ++i) {
                        dbusLanIpv4.append(addressVector.at(i).value("address").toString());
                    }
                } else if (isActNet && innerKey == "dns") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    int strDns = 0;
                    dbusArg2nd.beginArray();
                    while (!dbusArg2nd.atEnd()) {
                        dbusArg2nd >> strDns; // append map to a vector here if you want to keep it
                        if (strDns == 0) {
                            // dbusArg2nd为空时，strDns没有从dbusArg2nd获取到结果
                            dbusArg2nd.endArray();
                        }
                    }
                    dbusArg2nd.endArray();
                    dbusActLanDNS = strDns;
                }
            }
        } else if (outsideKey == "ipv6") {
            for (QString innerKey : outsideMap.keys()) {
                if (innerKey == "address-data") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    QMap<QString, QVariant> map;

                    dbusArg2nd.beginArray();
                    while (!dbusArg2nd.atEnd()) {
                        dbusArg2nd >> map; // append map to a vector here if you want to keep it
                        break;
                    }
                    dbusArg2nd.endArray();

                    dbusLanIpv6 = map.value("address").toString();
                } else if (innerKey == "method") {
                    dbusLanIpv6Method = outsideMap.value(innerKey).toString();
                }
            }
        }
    }
}

void KylinDBus::getWifiIp(QString uuid)
{
    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QList<QDBusObjectPath> m_objNets = m_reply.value();
    foreach (QDBusObjectPath objNet, m_objNets) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objNet.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");

        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        for (QString outside_key : map.keys()) {
            QMap<QString, QVariant> outsideMap = map.value(outside_key);
            if (outside_key == "connection") {
                for (QString search_key : outsideMap.keys()) {
                    if (search_key == "uuid") {
                        if (uuid == outsideMap.value(search_key).toString()) {
                            for (QString key : map.keys()) {
                                QMap<QString, QVariant> innerMap = map.value(key);
                                if (key == "ipv4") {
                                    for (QString inner_key : innerMap.keys()) {
                                        if (inner_key == "address-data") {
                                            const QDBusArgument &dbusArg2nd =
                                                innerMap.value(inner_key).value<QDBusArgument>();
                                            QMap<QString, QVariant> m_map;
                                            dbusArg2nd.beginArray();
                                            while (!dbusArg2nd.atEnd()) {
                                                dbusArg2nd >> m_map;
                                            }
                                            dbusArg2nd.endArray();
                                            dbusWifiIpv4 = m_map.value("address").toString();
                                        } else if (inner_key == "method") {
                                            dbusWifiIpv4Method = innerMap.value(inner_key).toString();
                                        }
                                    }
                                }
                                if (key == "ipv6") {
                                    for (QString inner_key : innerMap.keys()) {
                                        if (inner_key == "address-data") {
                                            const QDBusArgument &dbusArg2nd =
                                                innerMap.value(inner_key).value<QDBusArgument>();
                                            QMap<QString, QVariant> m_map;

                                            dbusArg2nd.beginArray();
                                            while (!dbusArg2nd.atEnd()) {
                                                dbusArg2nd
                                                    >> m_map; // append map to a vector here if you want to keep it
                                            }
                                            dbusArg2nd.endArray();

                                            dbusWifiIpv6 = m_map.value("address").toString();
                                        } else if (inner_key == "method") {
                                            dbusWifiIpv6Method = innerMap.value(inner_key).toString();
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        continue;
                    }
                }
            }
        }
    }
}

//获取有线网络的MAC
QString KylinDBus::getLanMAC(QString ifname)
{
    QString macAddress = "--";

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "AllDevices");
    if (result.arguments().isEmpty()) {
        return macAddress;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath; //保存设备的路径
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfacePro("org.freedesktop.NetworkManager", objPath.path(), "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = interfacePro.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
        if (ifname == reply.value().toString()) {
            //先获取已连接有线网络对应的设备路径
            QDBusInterface interfaceMac("org.freedesktop.NetworkManager", objPath.path(),
                                        "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> reply =
                interfaceMac.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "HwAddress");
            macAddress = reply.value().toString();
        }
    }
    dbusArgs.endArray();

    return macAddress;
}

//获取已经连接的有线网ip
void KylinDBus::getConnectNetIp(QString netUuid)
{
    dbusWifiIpv4 = "";
    dbusActiveWifiIpv4 = "";
    dbusWifiIpv4Method = "";
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfacePro("org.freedesktop.NetworkManager", objPath.path(), "org.freedesktop.DBus.Properties",
                                    QDBusConnection::systemBus());

        QDBusReply<QVariant> replyType =
            interfacePro.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
        QDBusReply<QVariant> replyUuid =
            interfacePro.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");
        if (replyType.value().toString() == "ethernet" || replyType.value().toString() == "802-3-ethernet"
            || replyType.value().toString() == "bluetooth") { //有线网或蓝牙共享网络
            if (replyUuid.value().toString() == netUuid) {
                // ipv4的路径信息和ip信息
                QDBusInterface interfaceIp4("org.freedesktop.NetworkManager", objPath.path(),
                                            "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIp4 =
                    interfaceIp4.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip4Config");
                QList<QVariant> outArgsIp4 = replyIp4.arguments();
                if (!outArgsIp4.isEmpty()) {
                    QVariant firstIp4 = outArgsIp4.at(0);
                    QDBusVariant dbvFirstIp4 = firstIp4.value<QDBusVariant>();
                    QVariant vFirstIp4 = dbvFirstIp4.variant();
                    QDBusObjectPath dbusPathIp4 = vFirstIp4.value<QDBusObjectPath>();

                    QDBusInterface interfaceIpv4("org.freedesktop.NetworkManager", dbusPathIp4.path(),
                                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                    QDBusMessage replyIpv4 =
                        interfaceIpv4.call("Get", "org.freedesktop.NetworkManager.IP4Config", "AddressData");

                    QList<QVariant> outArgsIpv4 = replyIpv4.arguments();
                    if (!outArgsIpv4.isEmpty()) {
                        QVariant firstIpv4 = outArgsIpv4.at(0);
                        QDBusVariant dbvFirstIpv4 = firstIpv4.value<QDBusVariant>();
                        QVariant vFirstIpv4 = dbvFirstIpv4.variant();

                        const QDBusArgument &dbusArgIpv4 = vFirstIpv4.value<QDBusArgument>();
                        QList<QVariantMap> mDatasIpv4;
                        dbusArgIpv4 >> mDatasIpv4;
                        if (false == mDatasIpv4.isEmpty()) {
                            foreach (QVariantMap mDataIpv4, mDatasIpv4) {
                                dbusActiveLanIpv4.append(mDataIpv4.value("address").toString());
                            }
                        }
                    }
                }

                // ipv6的路径信息和ip信息
                QDBusInterface interfaceIp6("org.freedesktop.NetworkManager", objPath.path(),
                                            "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIp6 =
                    interfaceIp6.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip6Config");
                QList<QVariant> outArgsIp6 = replyIp6.arguments();
                if (outArgsIp6.isEmpty())
                    continue;

                QVariant firstIp6 = outArgsIp6.at(0);
                QDBusVariant dbvFirstIp6 = firstIp6.value<QDBusVariant>();
                QVariant vFirstIp6 = dbvFirstIp6.variant();
                QDBusObjectPath dbusPathIp6 = vFirstIp6.value<QDBusObjectPath>();

                QDBusInterface interfaceIpv6("org.freedesktop.NetworkManager", dbusPathIp6.path(),
                                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIpv6 =
                    interfaceIpv6.call("Get", "org.freedesktop.NetworkManager.IP6Config", "AddressData");

                QList<QVariant> outArgsIpv6 = replyIpv6.arguments();
                if (outArgsIpv6.isEmpty())
                    continue;

                QVariant firstIpv6 = outArgsIpv6.at(0);
                QDBusVariant dbvFirstIpv6 = firstIpv6.value<QDBusVariant>();
                QVariant vFirstIpv6 = dbvFirstIpv6.variant();

                const QDBusArgument &dbusArgIpv6 = vFirstIpv6.value<QDBusArgument>();
                QList<QVariantMap> mDatasIpv6;
                dbusArgIpv6 >> mDatasIpv6;
                if (mDatasIpv6.isEmpty())
                    continue;

                foreach (QVariantMap mDataIpv6, mDatasIpv6) {
                    dbusActiveLanIpv6 = mDataIpv6.value("address").toString();
                    break;
                }
            }

        } else { //无线网
            if (replyUuid.value().toString() == netUuid) {
                // ipv4的路径信息和ip信息
                QDBusInterface interfaceIp4("org.freedesktop.NetworkManager", objPath.path(),
                                            "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIp4 =
                    interfaceIp4.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip4Config");
                QList<QVariant> outArgsIp4 = replyIp4.arguments();
                if (!outArgsIp4.isEmpty()) {
                    QVariant firstIp4 = outArgsIp4.at(0);
                    QDBusVariant dbvFirstIp4 = firstIp4.value<QDBusVariant>();
                    QVariant vFirstIp4 = dbvFirstIp4.variant();
                    QDBusObjectPath dbusPathIp4 = vFirstIp4.value<QDBusObjectPath>();

                    QDBusInterface interfaceIpv4("org.freedesktop.NetworkManager", dbusPathIp4.path(),
                                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                    QDBusMessage replyIpv4 =
                        interfaceIpv4.call("Get", "org.freedesktop.NetworkManager.IP4Config", "AddressData");

                    QList<QVariant> outArgsIpv4 = replyIpv4.arguments();
                    if (!outArgsIpv4.isEmpty()) {
                        QVariant firstIpv4 = outArgsIpv4.at(0);
                        QDBusVariant dbvFirstIpv4 = firstIpv4.value<QDBusVariant>();
                        QVariant vFirstIpv4 = dbvFirstIpv4.variant();

                        const QDBusArgument &dbusArgIpv4 = vFirstIpv4.value<QDBusArgument>();
                        QList<QVariantMap> mDatasIpv4;
                        dbusArgIpv4 >> mDatasIpv4;
                        if (false == mDatasIpv4.isEmpty()) {
                            foreach (QVariantMap mDataIpv4, mDatasIpv4) {
                                dbusActiveWifiIpv4 = mDataIpv4.value("address").toString();
                            }
                        }
                    }
                }

                // ipv6的路径信息和ip信息
                QDBusInterface interfaceIp6("org.freedesktop.NetworkManager", objPath.path(),
                                            "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIp6 =
                    interfaceIp6.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip6Config");
                QList<QVariant> outArgsIp6 = replyIp6.arguments();
                if (outArgsIp6.isEmpty())
                    continue;
                QVariant firstIp6 = outArgsIp6.at(0);
                QDBusVariant dbvFirstIp6 = firstIp6.value<QDBusVariant>();
                QVariant vFirstIp6 = dbvFirstIp6.variant();
                QDBusObjectPath dbusPathIp6 = vFirstIp6.value<QDBusObjectPath>();

                QDBusInterface interfaceIpv6("org.freedesktop.NetworkManager", dbusPathIp6.path(),
                                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusMessage replyIpv6 =
                    interfaceIpv6.call("Get", "org.freedesktop.NetworkManager.IP6Config", "AddressData");

                QList<QVariant> outArgsIpv6 = replyIpv6.arguments();
                if (outArgsIpv6.isEmpty())
                    continue;
                QVariant firstIpv6 = outArgsIpv6.at(0);
                QDBusVariant dbvFirstIpv6 = firstIpv6.value<QDBusVariant>();
                QVariant vFirstIpv6 = dbvFirstIpv6.variant();

                const QDBusArgument &dbusArgIpv6 = vFirstIpv6.value<QDBusArgument>();
                QList<QVariantMap> mDatasIpv6;
                dbusArgIpv6 >> mDatasIpv6;
                if (mDatasIpv6.isEmpty())
                    continue;
                foreach (QVariantMap mDataIpv6, mDatasIpv6) {
                    dbusActiveWifiIpv6 = mDataIpv6.value("address").toString();
                }
            }
        }
    }
    dbusArgs.endArray();
}

//监听有线网络ip变化
void KylinDBus::getLanIpChanged()
{
    m_lanPathList.clear();
    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QList<QDBusObjectPath> m_objNets = m_reply.value();
    foreach (QDBusObjectPath objNet, m_objNets) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objNet.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        for (QString key : map.keys()) {
            if (key == "802-3-ethernet") {
                oldSettingPaths.append(objNet);
                QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"), objNet.path(),
                                                     QString("org.freedesktop.NetworkManager.Settings.Connection"),
                                                     QString("Updated"), this, SLOT(onLanIpPropertiesChanged()));
                m_lanPathList.append(objNet.path());
            } else if (key == "802-11-wireless") {
                QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"), objNet.path(),
                                                     QString("org.freedesktop.NetworkManager.Settings.Connection"),
                                                     QString("Updated"), this, SLOT(onWifiIpPropertiesChanged()));
            }
        }
    }
}

//获取wifi的mac地址
void KylinDBus::getWifiMac(QString netName)
{
    dbusWifiMac = "";

    if (multiWirelessPaths.isEmpty()) {
        return;
    }

    QDBusInterface interface("org.freedesktop.NetworkManager", multiWirelessPaths.at(0).path(),
                             "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = interface.call("GetAllAccessPoints"); // get accesspoint for each wifi
    QList<QDBusObjectPath> objPaths = reply.value();

    foreach (QDBusObjectPath objPath, objPaths) {
        QDBusInterface ssid_interface("org.freedesktop.NetworkManager", objPath.path(),
                                      "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusReply<QVariant> ssid_replys =
            ssid_interface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Ssid");
        QString str_name = ssid_replys.value().toString();

        if (str_name == netName) {
            // if the ssid of the accesspoint is just the netName, we can get this hardware address
            QDBusInterface path_interface("org.freedesktop.NetworkManager", objPath.path(),
                                          "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> path_reply =
                path_interface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "HwAddress");
            dbusWifiMac = path_reply.value().toString();
        }
    }
}

//获取当前能获取到的wifi的数量， 每个accesspoint 代表一个wifi
int KylinDBus::getAccessPointsNumber()
{
    if (multiWirelessPaths.isEmpty()) {
        return 0;
    }
    QDBusInterface interface("org.freedesktop.NetworkManager", multiWirelessPaths.at(0).path(),
                             "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = interface.call("GetAllAccessPoints");
    QList<QDBusObjectPath> objPaths = reply.value();

    return objPaths.size();
}

//返回当前已创建有线的个数
int KylinDBus::getWiredNetworkNumber()
{
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                             "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> reply = interface.call("ListConnections");
    QList<QDBusObjectPath> objNetPaths = reply.value();

    int wiredNetworkNumber = 0;
    foreach (QDBusObjectPath objNetPath, objNetPaths) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objNetPath.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        for (QString key : map.keys()) {
            if (key == "802-3-ethernet") {
                wiredNetworkNumber += 1;
            }
        }
    }

    return wiredNetworkNumber;
}

QStringList KylinDBus::getWifiSsidList()
{
    QStringList wifiSsidList;

    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QDBusObjectPath specific_connection;
    specific_connection.setPath("/");

    QList<QDBusObjectPath> m_objSettingPaths = m_reply.value();
    foreach (QDBusObjectPath objSettingPath, m_objSettingPaths) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objSettingPath.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        if (map.value("802-11-wireless").value("mode").toString() == "infrastructure") {
            wifiSsidList.append(map.value("connection").value("id").toString());
        }
    }

    return wifiSsidList;
}

QString KylinDBus::checkHasWifiConfigFile(QString wifiName)
{
    QString wifiUuid = "";

    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QDBusObjectPath specific_connection;
    specific_connection.setPath("/");

    QList<QDBusObjectPath> m_objSettingPaths = m_reply.value();
    foreach (QDBusObjectPath objSettingPath, m_objSettingPaths) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objSettingPath.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        if (map.value("connection").value("type").toString() == "802-11-wireless") {
            if (map.value("802-11-wireless").value("ssid").toString() == wifiName) {
                // qDebug() << "-------------------------> wifi ssid : "
                // <<map.value("802-11-wireless").value("ssid").toString(); qDebug() << "-------------------------> wifi
                // id : " <<map.value("connection").value("id").toString();
                wifiUuid = map.value("connection").value("uuid").toString();
            }
        }
    }

    return wifiUuid;
}

void KylinDBus::toCreateNewLan()
{
    int i = 1;
    if (multiWiredIfName.size() != 0) {
        foreach (QString strIfName, multiWiredIfName) {
            // QString cmdStr = "nmcli connection add con-name '有线连接 " + QString::number(i,10) + "' ifname '" +
            // strIfName + "' type ethernet";
            QString cmdStr = "nmcli connection add con-name '" + tr("Wired connection") + " " + QString::number(i, 10)
                             + "' ifname '" + strIfName + "' type ethernet";
            Utils::m_system(cmdStr.toUtf8().data());
            i += 1;
        }
    }
}

//根据网卡接口的名称获取接口对应网线是否接入
bool KylinDBus::getWiredCableStateByIfname(QString ifname)
{
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "AllDevices");
    if (result.arguments().isEmpty()) {
        return false;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;
        QDBusInterface interfaceInterface("org.freedesktop.NetworkManager", objPath.path(),
                                          "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusReply<QVariant> replyInterface =
            interfaceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
        if (replyInterface.value().toString() == ifname) {
            QDBusInterface interfaceCarrier("org.freedesktop.NetworkManager", objPath.path(),
                                            "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> replyCarrier =
                interfaceCarrier.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "Carrier");

            if (replyCarrier.value().toBool()) {
                return true;
            }
        }
    }
    dbusArgs.endArray();

    return false;
}

//根据网卡接口的名称获取uuid
QString KylinDBus::getConnLanNameByIfname(QString ifname)
{
    QString uuidName = "--";
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return uuidName;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceDevice("org.freedesktop.NetworkManager", objPath.path(),
                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusMessage replyDevices =
            interfaceDevice.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Devices");
        if (replyDevices.arguments().isEmpty()) {
            continue;
        }
        QList<QVariant> outArgsDevices = replyDevices.arguments();
        QVariant firstDevices = outArgsDevices.at(0);
        QDBusVariant dbvFirstDevices = firstDevices.value<QDBusVariant>();
        QVariant vFirstDevices = dbvFirstDevices.variant();
        QDBusArgument dbusArgsDevices = vFirstDevices.value<QDBusArgument>();

        QDBusObjectPath objPathDevices;
        dbusArgsDevices.beginArray();
        while (!dbusArgsDevices.atEnd()) {
            dbusArgsDevices >> objPathDevices;
            QDBusInterface interfaceInterface("org.freedesktop.NetworkManager", objPathDevices.path(),
                                              "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> replyInterface =
                interfaceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
            if (replyInterface.value().toString() == ifname) {
                QDBusInterface interfaceUuid("org.freedesktop.NetworkManager", objPath.path(),
                                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

                QDBusReply<QVariant> replyDevices =
                    interfaceUuid.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");
                uuidName = replyDevices.value().toString();
            }
        }
        dbusArgsDevices.endArray();
    }
    dbusArgs.endArray();

    return uuidName;
}

//新增了一个网络，伴随着多了一个网络配置文件
void KylinDBus::onNewConnection(QDBusObjectPath objPath)
{
    QDBusInterface m_interface("org.freedesktop.NetworkManager", objPath.path(),
                               "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
    QDBusMessage result = m_interface.call("GetSettings");
    if (result.arguments().isEmpty()) {
        onWiredSettingNumChanged();
        return;
    }
    const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QMap<QString, QVariant>> map;
    dbusArg1st >> map;

    for (QString key : map.keys()) {
        if (key == "802-3-ethernet") {
            emit this->updateWiredList(1); // send this signal to update wired network list
            emit this->newConnAdded(0);
            m_lanPathList.append(objPath.path());
            // TODO:            emit mw->wiredConnectionAdded();
            qDebug() << "A new wired network was created.";
            break;
        } else if (key == "802-11-wireless") {
            qDebug() << "A new wireless network was created.";
            sleep(1);
            bool has_wpa_psk = false;
            bool has_psk_flags = false;
            QDBusInterface interfaceSet("org.freedesktop.NetworkManager", objPath.path(),
                                        "org.freedesktop.NetworkManager.Settings.Connection",
                                        QDBusConnection::systemBus());
            QDBusMessage resultSet = interfaceSet.call("GetSettings");
            if (resultSet.arguments().isEmpty()) {
                continue;
            }
            const QDBusArgument &dbusArg1stSet = resultSet.arguments().at(0).value<QDBusArgument>();
            QMap<QString, QMap<QString, QVariant>> mapSet;
            dbusArg1stSet >> mapSet;
            for (QString setKey : mapSet.keys()) {
                QMap<QString, QVariant> subSetMap = mapSet.value(setKey);
                if (setKey == "802-11-wireless-security") {
                    for (QString searchKey : subSetMap.keys()) {
                        if (searchKey == "key-mgmt") {
                            qDebug() << "key-mgmt : " << subSetMap.value(searchKey).toString();
                            has_wpa_psk = true;
                        }
                        if (searchKey == "psk-flags") {
                            qDebug() << "psk-flags : " << subSetMap.value(searchKey).toInt();
                            has_psk_flags = true;
                        }
                    }
                }
            }

            if (has_wpa_psk && has_psk_flags) {
                QPoint pos = QCursor::pos();
                QRect primaryGeometry;
                for (QScreen *screen : qApp->screens()) {
                    if (screen->geometry().contains(pos)) {
                        primaryGeometry = screen->geometry();
                    }
                }
                if (primaryGeometry.isEmpty()) {
                    primaryGeometry = qApp->primaryScreen()->geometry();
                }

                QApplication::setQuitOnLastWindowClosed(false);
                WiFiConfigDialog *wifiConfigDialog = new WiFiConfigDialog();
                wifiConfigDialog->move(primaryGeometry.width() / 2 - wifiConfigDialog->width() / 2,
                                       primaryGeometry.height() / 2 - wifiConfigDialog->height() / 2);
                wifiConfigDialog->show();
                wifiConfigDialog->raise();
            } else {
                emit this->newConnAdded(1);
            }

            break;
        }
    }

    onWiredSettingNumChanged();
}

//减少了一个网络，伴随着减少了一个网络配置文件
void KylinDBus::onConnectionRemoved(QDBusObjectPath objPath)
{
    if (m_lanPathList.contains(objPath.path())) {
        m_lanPathList.removeOne(objPath.path());
        qDebug() << "An old network was removed from configure directory.";
        // TODO:        if (mw->is_btnLanList_clicked == 1) {
        //             emit this->updateWiredList(1); //send this signal to update wired network list
        //         }
        // TODO:        emit mw->wiredConnectionRemoved();
    }

    onWiredSettingNumChanged();
}

//有线网的个数，也即有线网配置的个数变化
void KylinDBus::onWiredSettingNumChanged()
{
    //先取消之前建立的信号槽连接
    foreach (QDBusObjectPath objSettingPath, oldSettingPaths) {
        QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"), objSettingPath.path(),
                                                QString("org.freedesktop.NetworkManager.Settings.Connection"),
                                                QString("Updated"), this, SLOT(onLanIpPropertiesChanged()));
    }

    //再建立新的信号槽连接
    getLanIpChanged();
}

//应用启动时，获取已经连接的网络信息，包括该网络的路径和类型
void KylinDBus::initConnectionInfo()
{
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        //获取当前wifi的开关状态
        QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");
        qDebug() << "debug: *****初始的无线网络开关状态是: " << m_result.value().toBool();
        oldWifiSwitchState = m_result.value().toBool();
        return;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;
        oldPaths.append(objPath);

        QDBusInterface interface("org.freedesktop.NetworkManager", objPath.path(), "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
        oldPathInfo.append(reply.value().toString());
    }
    dbusArgs.endArray();

    //获取当前wifi的开关状态
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");
    qDebug() << "debug: *****初始的无线网络开关状态是: " << m_result.value().toBool();
    oldWifiSwitchState = m_result.value().toBool();
}

//获取已经连接有线网络的ssid和uuid和state
QList<QString> KylinDBus::getAtiveLanSsidUuidState()
{
    QList<QString> strSsidUuidState;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return strSsidUuidState;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "ethernet" || reply.value().toString() == "802-3-ethernet"
            || reply.value().toString() == "bluetooth" || reply.value().toString() == "vpn") {
            QDBusInterface interfaceInfo("org.freedesktop.NetworkManager", objPath.path(),
                                         "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
            QDBusReply<QVariant> replyId =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Id");
            QDBusReply<QVariant> replyUuid =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");
            QDBusReply<QVariant> replyState =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "State");

            QDBusMessage replyDevicepaths =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Devices");
            if (!replyDevicepaths.arguments().isEmpty()) {
                QVariant firstDP = replyDevicepaths.arguments().at(0);
                QDBusVariant dbvFirstDP = firstDP.value<QDBusVariant>();
                QVariant varDP = dbvFirstDP.variant();
                QDBusArgument dbusArgsDP = varDP.value<QDBusArgument>();

                QDBusObjectPath objPathDP;
                dbusArgsDP.beginArray();
                while (!dbusArgsDP.atEnd()) {
                    dbusArgsDP >> objPathDP;
                    if (!m_unvisibleDevicePaths.isEmpty() && m_unvisibleDevicePaths.contains(objPathDP.path())) {
                        qWarning() << "[getAtiveLanSsidUuidState] Device unvisible: " << objPathDP.path();
                        continue;
                    }

                    strSsidUuidState.append(replyId.value().toString());
                    strSsidUuidState.append(replyUuid.value().toString());
                    if (replyState.value().toUInt() == 1) {
                        strSsidUuidState.append("connecting");
                    }
                    if (replyState.value().toUInt() == 2) {
                        strSsidUuidState.append("connected");
                    }
                }
                dbusArgsDP.endArray();
            }
        }
    }
    dbusArgs.endArray();

    return strSsidUuidState;
}

//断开所有有线连接
void KylinDBus::disConnectWiredConnect()
{
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "ethernet" || reply.value().toString() == "802-3-ethernet") {
            QDBusInterface connInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                         "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
            connInterface.call("DeactivateConnection", QVariant::fromValue(objPath));
        }
    }
    dbusArgs.endArray();
}

//获取已连接wifi的uuid
QString KylinDBus::getActiveWifiUuid()
{
    QString uuid;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty())
        return uuid;
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "wifi" || reply.value().toString() == "802-11-wireless") {
            QDBusInterface interfaceInfo("org.freedesktop.NetworkManager", objPath.path(),
                                         "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
            QDBusReply<QVariant> replyUuid =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");
            uuid = replyUuid.value().toString();
        }
    }
    dbusArgs.endArray();

    return uuid;
}

QStringList KylinDBus::getActiveLan()
{
    QStringList connLanInfo;
    // int wifistrength = 0;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "ethernet" || reply.value().toString() == "802-3-ethernet") {
            if (m_unvisibleDevicePaths.contains(objPath.path())) {
                qWarning() << "[getActiveLan] unviseble device " << objPath.path();
                continue;
            }
            connLanInfo.append(objPath.path());
        }
    }
    dbusArgs.endArray();

    // return wifistrength;
    return connLanInfo;
}

QStringList KylinDBus::getActiveWlanAndWifiSignal()
{
    QStringList connWlanInfo;
    // int wifistrength = 0;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "wifi" || reply.value().toString() == "802-11-wireless") {
            connWlanInfo.append(objPath.path());

            QDBusInterface interfaceSpecificObject("org.freedesktop.NetworkManager", objPath.path(),
                                                   "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusMessage resultSpecificObject = interfaceSpecificObject.call(
                "Get", "org.freedesktop.NetworkManager.Connection.Active", "SpecificObject");
            QList<QVariant> outArgsSpecificObject = resultSpecificObject.arguments();
            QVariant firstSpecificObject = outArgsSpecificObject.at(0);
            QDBusVariant dbvFirstSpecificObject = firstSpecificObject.value<QDBusVariant>();
            QVariant vFirstSpecificObject = dbvFirstSpecificObject.variant();
            QDBusObjectPath objPathSpecificObject = vFirstSpecificObject.value<QDBusObjectPath>();

            QDBusInterface interfaceAP("org.freedesktop.NetworkManager", objPathSpecificObject.path(),
                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> replyStrength =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Strength"); // signal

            QString strStrength;
            QByteArray sesultStrength = replyStrength.value().toByteArray();
            int wifiSignal = sesultStrength[0] & 0x000000FF;
            wifiSignal |= ((sesultStrength[1] << 8) & 0x0000FF00);
            wifiSignal |= ((sesultStrength[2] << 16) & 0x00FF0000);
            wifiSignal |= ((sesultStrength[3] << 24) & 0xFF000000);
            strStrength = QString::number(wifiSignal);
            connWlanInfo.append(strStrength);
            // wifistrength = wifiSignal;
        }
    }
    dbusArgs.endArray();

    // return wifistrength;
    return connWlanInfo;
}

//根据Dbus路径获取wifi的ssid（排除空格干扰）
QString KylinDBus::getWifiSsid(QString accessPointPath)
{
    QDBusInterface interface("org.freedesktop.NetworkManager", accessPointPath, "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Ssid");
    if (result.arguments().isEmpty())
        return "";
    return result.arguments().at(0).value<QDBusVariant>().variant().toString();
}

//检查wifi连接状态
int KylinDBus::checkWifiConnectivity()
{
    int wifiState = WIFI_DISCONNECTED;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return wifiState;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> replyType =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (replyType.value().toString() == "wifi" || replyType.value().toString() == "802-11-wireless") {
            QDBusReply<QVariant> replyState =
                interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "State");
            wifiState = replyState.value().toUInt(); //正在连接的状态是1，连接上的状态是2
        }
    }
    dbusArgs.endArray();

    return wifiState;
}

bool KylinDBus::checkNetworkConnectivity()
{
    bool hasNetworkConnecting = false;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return hasNetworkConnecting;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> replyType =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (replyType.value().toString() == "802-3-ethernet" || replyType.value().toString() == "802-11-wireless") {
            QDBusReply<QVariant> replyState =
                interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "State");
            if (replyState.value().toUInt() == 1) {
                hasNetworkConnecting = true;
            }
        }
    }
    dbusArgs.endArray();

    return hasNetworkConnecting;
}

//获取已经连接无线网络的ssid和uuid
QList<QString> KylinDBus::getAtiveWifiBSsidUuid(QStringList wifilist)
{
    QList<QString> strBSsidUuid;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    if (result.arguments().isEmpty()) {
        return strBSsidUuid;
    }
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;

        QDBusInterface interfaceType("org.freedesktop.NetworkManager", objPath.path(),
                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> reply =
            interfaceType.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

        if (reply.value().toString() == "wifi" || reply.value().toString() == "802-11-wireless") {
            QDBusInterface interfaceInfo("org.freedesktop.NetworkManager", objPath.path(),
                                         "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            //获取已经连接wifi的uuid
            QDBusReply<QVariant> replyUuid =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");
            // qDebug() << "wifi uuid : "<< replyUuid.value().toString();
            strBSsidUuid.append(replyUuid.value().toString());


            //再获取bssid
            QDBusMessage resultConnection =
                interfaceInfo.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Connection");

            QList<QVariant> outArgsConnection = resultConnection.arguments();
            if (outArgsConnection.isEmpty())
                continue;
            QVariant firstConnection = outArgsConnection.at(0);
            QDBusVariant dbvFirstConnection = firstConnection.value<QDBusVariant>();
            QVariant vFirstConnection = dbvFirstConnection.variant();
            QDBusObjectPath dbusArgsConnection = vFirstConnection.value<QDBusObjectPath>();

            QDBusInterface interfaceSet("org.freedesktop.NetworkManager", dbusArgsConnection.path(),
                                        "org.freedesktop.NetworkManager.Settings.Connection",
                                        QDBusConnection::systemBus());
            QDBusMessage resultSet = interfaceSet.call("GetSettings");
            if (resultSet.arguments().isEmpty())
                continue;
            const QDBusArgument &dbusArg1stSet = resultSet.arguments().at(0).value<QDBusArgument>();
            QMap<QString, QMap<QString, QVariant>> mapSet;
            dbusArg1stSet >> mapSet;

            for (QString setKey : mapSet.keys()) {
                QMap<QString, QVariant> subSetMap = mapSet.value(setKey);
                if (setKey == "802-11-wireless") {
                    for (QString searchKey : subSetMap.keys()) {
                        if (searchKey == "seen-bssids") {
                            // qDebug() << "wifi bssid : "<<subSetMap.value(searchKey).toStringList();
                            QStringList strBssidList = subSetMap.value(searchKey).toStringList();
                            foreach (QString strBssid, strBssidList) {
                                strBSsidUuid.append(strBssid);
                            }
                        }
                    }
                }
            }
        }
    }
    dbusArgs.endArray();

    //    //获取已经连接wifi的bssid
    //    if (wifilist.size() != 0) {
    //        // 填充可用网络列表
    //        QString headLine = wifilist.at(0);
    //        int indexSignal,indexSecu, indexFreq, indexBSsid, indexName;
    //        headLine = headLine.trimmed();

    //        bool isChineseExist = headLine.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
    //        if (isChineseExist) {
    //            indexSignal = headLine.indexOf("SIGNAL");
    //            indexSecu = headLine.indexOf("安全性");
    //            indexFreq = headLine.indexOf("频率") + 4;
    //            indexBSsid = headLine.indexOf("BSSID") + 6;
    //            indexName = indexBSsid + 19;
    //        } else {
    //            indexSignal = headLine.indexOf("SIGNAL");
    //            indexSecu = headLine.indexOf("SECURITY");
    //            indexFreq = headLine.indexOf("FREQ");
    //            indexBSsid = headLine.indexOf("BSSID");
    //            indexName = indexBSsid + 19;
    //        }

    //        for (int i = 1, j = 0; i < wifilist.size(); i ++) {
    //            QString line = wifilist.at(i);
    //            QString winuse = line.mid(0, indexSignal).trimmed();
    //            QString wbssid = line.mid(indexBSsid, 17).trimmed();
    //            if (winuse == "*") {
    //                strBSsidUuid.append(wbssid);
    //            }
    //        }
    //    }

    return strBSsidUuid;
}


//使用dbus的方法获取wifi的信息，形成包含每个wifi信息的列表
void KylinDBus::toGetWifiList()
{
    QStringList slist;

    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage resultAllDevices = interface.call("Get", "org.freedesktop.NetworkManager", "AllDevices");
    QList<QVariant> outArgsAllDevices = resultAllDevices.arguments();
    QVariant firstAllDevices = outArgsAllDevices.at(0);
    QDBusVariant dbvFirstAllDevices = firstAllDevices.value<QDBusVariant>();
    QVariant vFirstAllDevices = dbvFirstAllDevices.variant();
    QDBusArgument dbusArgsAllDevices = vFirstAllDevices.value<QDBusArgument>();

    QDBusObjectPath objPathDevice;
    dbusArgsAllDevices.beginArray();
    while (!dbusArgsAllDevices.atEnd()) {
        dbusArgsAllDevices >> objPathDevice;

        QDBusInterface interfacePro("org.freedesktop.NetworkManager", objPathDevice.path(),
                                    "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusMessage resultAccessPoints =
            interfacePro.call("Get", "org.freedesktop.NetworkManager.Device.Wireless", "AccessPoints");
        if (resultAccessPoints.arguments().isEmpty()) {
            continue;
        }
        QList<QVariant> outArgsAccessPoints = resultAccessPoints.arguments();
        QVariant firstAccessPoints = outArgsAccessPoints.at(0);
        QDBusVariant dbvFirstAccessPoints = firstAccessPoints.value<QDBusVariant>();
        QVariant vFirstAccessPoints = dbvFirstAccessPoints.variant();
        QDBusArgument dbusArgsAccessPoints = vFirstAccessPoints.value<QDBusArgument>();

        QDBusObjectPath objPathAccessPoint;
        dbusArgsAccessPoints.beginArray();
        while (!dbusArgsAccessPoints.atEnd()) {
            dbusArgsAccessPoints >> objPathAccessPoint;

            QDBusInterface interfaceAP("org.freedesktop.NetworkManager", objPathAccessPoint.path(),
                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

            QDBusReply<QVariant> replyFlags =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Flags"); // in-use
            QDBusReply<QVariant> replyStrength =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Strength"); // signal
            QDBusReply<QVariant> replyWpaFlags =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "WpaFlags"); // security
            QDBusReply<QVariant> replyFrequency =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Frequency"); // freq
            QDBusReply<QVariant> replyHwAddress =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "HwAddress"); // bssid
            QDBusReply<QVariant> replySsid =
                interfaceAP.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Ssid"); // ssid

            QString strFlags = replyFlags.value().toString();

            QString strStrength;
            QByteArray sesultStrength = replyStrength.value().toByteArray();
            int addr = sesultStrength[0] & 0x000000FF;
            addr |= ((sesultStrength[1] << 8) & 0x0000FF00);
            addr |= ((sesultStrength[2] << 16) & 0x00FF0000);
            addr |= ((sesultStrength[3] << 24) & 0xFF000000);
            strStrength = QString::number(addr);

            QString strWpaFlags = replyWpaFlags.value().toString();

            QString strFrequency = replyFrequency.value().toString();

            QString strHwAddress = replyHwAddress.value().toString();

            QString strSsid = replySsid.value().toString();

            slist.append(strFlags);
            slist.append(strStrength);
            slist.append(strWpaFlags);
            slist.append(strFrequency);
            slist.append(strHwAddress);
            slist.append(strSsid);
        }
        dbusArgsAccessPoints.endArray();
    }
    dbusArgsAllDevices.endArray();

    emit toGetWifiListFinished(slist);
}

//网络连接变化时，如有新增或减少的网络，发信号通知更新主界面
void KylinDBus::onPropertiesChanged(QVariantMap qvm)
{
    // TODO:    if (mw->is_stop_check_net_state) {
    //         return;
    //     }
    for (QString keyStr : qvm.keys()) {
        //有关已连接网络变化的信号
        if (keyStr == "ActiveConnections") {
            // 第一步 获取当前已连接网络的对象路径和对应的网络类型(ethernet or wifi)
            const QDBusArgument &dbusArg = qvm.value(keyStr).value<QDBusArgument>();
            QList<QDBusObjectPath> newPaths;
            newPaths.clear();
            dbusArg >> newPaths;
            QStringList newPathInfo;
            qDebug() << "             ";
            foreach (QDBusObjectPath objPath, newPaths) {
                QDBusInterface interface("org.freedesktop.NetworkManager", objPath.path(),
                                         "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

                QDBusReply<QVariant> reply =
                    interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
                newPathInfo.append(reply.value().toString());
            }
            // 第二步 同上一次获取的已连接网络相比较，处理相比于上次减少的网络连接
            for (int i = 0; i < oldPaths.size(); i++) {
                QDBusObjectPath old_path = oldPaths.at(i);
                if (newPaths.size() == 0) {
                    if (oldPathInfo.size() == oldPaths.size()) {
                        qDebug() << "debug: 已连接网络个数由1减少到0";
                        // TODO:                        mw->onExternalConnectionChange(oldPathInfo.at(i), false);
                    }
                } else {
                    for (int j = 0; j < newPaths.size(); j++) {
                        QDBusObjectPath new_path = newPaths.at(j);
                        if (new_path == old_path) {
                            break; // stop if new_path also in oldPaths
                        }

                        if (j == newPaths.size() - 1) {
                            if (oldPathInfo.size() == oldPaths.size()) {
                                qDebug() << "debug: 已连接网络个数由3减少到2，或由2减少到1";
                                // TODO: mw->onExternalConnectionChange(oldPathInfo.at(i), false);
                            }
                        }
                    }
                }
            }

            // 第三步 同上一次获取的已连接网络相比较，处理相比于上次增加的网络连接
            for (int i = 0; i < newPaths.size(); i++) {
                QDBusObjectPath new_path = newPaths.at(i);
                if (oldPaths.size() == 0) {
                    qDebug() << "debug: 已连接网络个数由0增加到1";
                    // TODO:                    mw->onExternalConnectionChange(newPathInfo.at(i), true);
                } else {
                    for (int j = 0; j < oldPaths.size(); j++) {
                        QDBusObjectPath old_path = oldPaths.at(j);
                        if (new_path == old_path) {
                            break; // stop if new_path also in oldPaths
                        }

                        if (j == oldPaths.size() - 1) {
                            qDebug() << "debug: 已连接网络个数由1增加到2，或2增加到3";
                            // TODO:                            mw->onExternalConnectionChange(newPathInfo.at(i), true);
                        }
                    }
                }
            }

            bool isChangeOldPathInfo = true;
            for (int k = 0; k < newPathInfo.size(); k++) {
                if (newPathInfo.at(k) == "") {
                    isChangeOldPathInfo = false;
                }
            }
            if (isChangeOldPathInfo) {
                oldPathInfo = newPathInfo;
            }
            oldPaths = newPaths;
        }

        //网络连接connectivity属性的变化
        if (keyStr == "Connectivity") {
            qDebug() << "Connectivity property changed";
            // TODO:            mw->getActiveInfoAndSetTrayIcon();
        }

        //收到wifi开关打开或关闭的信号后，进行处理
        if (keyStr == "WirelessEnabled") {
            qDebug() << "WirelessEnabled property changed";
            bool newWifiSwitchState = qvm.value("WirelessEnabled").toBool();
            if (oldWifiSwitchState == false && newWifiSwitchState == true) {
                qDebug() << "wifi开关已经打开";
                // TODO:                mw->onExternalWifiSwitchChange(true);
            }
            if (oldWifiSwitchState == true && newWifiSwitchState == false) {
                qDebug() << "wifi开关已经关闭";
                // TODO:                mw->onExternalWifiSwitchChange(false);
            }

            // oldWifiSwitchState = newWifiSwitchState; //更新状态用于下一次
        }
    }
}

//检查网络状态获取Connectivity的值
int KylinDBus::getNetworkConectivity()
{
    int connectivityState = 0;
    QDBusInterface interfacePro("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusReply<QVariant> reply = interfacePro.call("Get", "org.freedesktop.NetworkManager", "Connectivity");
    if (!reply.isValid()) {
        qDebug() << Q_FUNC_INFO << " can not get the attribute 'Connectivity' ";
    } else {
        connectivityState = reply.value().toInt();
    }

    return connectivityState;
}

//检查LAN连接状态获取Connectivity的值
int KylinDBus::getLanConnectivity(QDBusObjectPath path)
{
    return getLanOrWlanConnectivity(path);
}

//检查WLAN连接状态获取Connectivity的值
int KylinDBus::getWlanConnectivity(QDBusObjectPath path)
{
    return getLanOrWlanConnectivity(path);
}

int KylinDBus::getLanOrWlanConnectivity(QDBusObjectPath path)
{
    int connectivityState = 0;

    QDBusInterface interfaceDevice("org.freedesktop.NetworkManager", path.path(), "org.freedesktop.DBus.Properties",
                                   QDBusConnection::systemBus());

    QDBusMessage replyDevices =
        interfaceDevice.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Devices");
    QList<QVariant> outArgsDevices = replyDevices.arguments();
    QVariant firstDevices = outArgsDevices.at(0);
    QDBusVariant dbvFirstDevices = firstDevices.value<QDBusVariant>();
    QVariant vFirstDevices = dbvFirstDevices.variant();
    QDBusArgument dbusArgsDevices = vFirstDevices.value<QDBusArgument>();

    QDBusObjectPath objPathDevices;
    dbusArgsDevices.beginArray();
    while (!dbusArgsDevices.atEnd()) {
        dbusArgsDevices >> objPathDevices;
        QDBusInterface interface_conn("org.freedesktop.NetworkManager", objPathDevices.path(),
                                      "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusReply<QVariant> reply_conn =
            interface_conn.call("Get", "org.freedesktop.NetworkManager.Device", "Ip4Connectivity");
        if (!reply_conn.isValid()) {
            qDebug() << Q_FUNC_INFO << "Can not get the attribute 'Ip4Connectivity' ";
        } else {
            connectivityState = reply_conn.value().toInt();
        }
    }
    dbusArgsDevices.endArray();

    return connectivityState;
}

//接收到自动连接的信号过后执行自动连接wifi
void KylinDBus::onAutoConnect()
{
    // syslog(LOG_DEBUG, "Receive a auto-connect signal to reconnect wifi");
    qDebug() << "Receive a auto-connect signal to reconnect wifi";
    // TODO:    if (mw->isRadioWifiTurningOn) {
    //         qDebug() << "But it is turning wifi switch on now, stop to reconnect wifi";
    //     } else {
    //         mw->toReconnectWifi();
    //     }
}

//获取是否连接有线网网线
void KylinDBus::getPhysicalCarrierState(int n)
{
    qDebug() << "Enter function to get physical carrier state.";
    multiWiredCableState.clear();

    foreach (QDBusObjectPath localPath, multiWiredPaths) {
        QDBusInterface interface("org.freedesktop.NetworkManager", localPath.path(), "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

        // Carrier值为true，插了网线；为false，未插网线
        QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.Device.Wired", "Carrier");

        if (reply.value().toString() == "true") {
            qDebug() << "multiWiredCableState.append(true)";
            multiWiredCableState.append("true");
        } else if (reply.value().toString() == "false") {
            qDebug() << "multiWiredCableState.append(false)";
            multiWiredCableState.append("false");
        } else {
            qDebug() << "Error occurred when get the property 'Carrier' of Wired";
        }
    }

    isWiredCableOn = false;
    foreach (QString state, multiWiredCableState) {
        if (state == "true") {
            isWiredCableOn = true;
        }
    }

    // TODO:    if (n == 1)
    //         this->mw->onPhysicalCarrierChanged(isWiredCableOn);
}

void KylinDBus::onLanPropertyChanged(QString strInfo, QVariantMap qvm, QStringList strlistInfo)
{
    for (QString keyStr : qvm.keys()) {
        if (keyStr == "Ip4Connectivity") {
            // qDebug() << "On Lan Ip4Connectivity Property Changed value is: "<< qvm.value("Ip4Connectivity").toInt();
            // TODO:            mw->getActiveInfoAndSetTrayIcon();
        }
    }
}

//无线网属性变化时，执行该函数
void KylinDBus::onWlanPropertyChanged(QString strInfo, QVariantMap qvm, QStringList strlistInfo)
{
    for (QString keyStr : qvm.keys()) {
        if (keyStr == "Ip4Connectivity") {
            // qDebug() << "On Wlan Ip4Connectivity Property Changed value is : "<<
            // qvm.value("Ip4Connectivity").toInt();;
            // TODO:            mw->getActiveInfoAndSetTrayIcon();
        }
    }
}

//有线网属性变化时，执行该函数。由于可能在短时间收到几条相同属性变化信息，所以在短时间内，执行一次
void KylinDBus::onLanWiredPropertyChanged(QVariantMap qvm)
{
    if (!isRunningFunction) {
        qDebug() << "Kylin-nm receive a PropertiesChanged signal about 'carrier' properties.";
        isRunningFunction = true; // function onLanPropertyChanged is running
        time->start(3000);
        QString str = qvm.value("Carrier").toString();
        if (str == "false" || str == "true") {
            getPhysicalCarrierState(1);
        }
    }
}

void KylinDBus::slot_timeout()
{
    isRunningFunction = false;
    time->stop();
}

//有线网的Ip属性变化时的响应函数
void KylinDBus::onLanIpPropertiesChanged()
{
    // TODO:    if (!mw->isHandlingWiredCableOn) {
    //         QTimer::singleShot(0.5 * 1000, this, [ = ]() {
    //防止卡顿，延时一小段时间后再发信号
    //            emit this->updateWiredList(0);
    //        });
    //        emit this->updateWiredList(0);
    //    }
}

//无线网的Ip属性变化时的响应函数
void KylinDBus::onWifiIpPropertiesChanged()
{
    // TODO:    if (mw->is_stop_check_net_state) {
    //         return;
    //     }
    emit this->updateWirelessList();
}

//利用dbus的方法对已经连接的有线网进行再次连接
void KylinDBus::reConnectWiredNet(QString netUuid)
{
    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QDBusObjectPath specific_connection;
    specific_connection.setPath("/");

    QList<QDBusObjectPath> m_objSettingPaths = m_reply.value();
    foreach (QDBusObjectPath objSettingPath, m_objSettingPaths) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objSettingPath.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        for (QString outside_key : map.keys()) {
            QMap<QString, QVariant> outsideMap = map.value(outside_key);
            if (outside_key == "connection") {
                for (QString search_key : outsideMap.keys()) {
                    if (search_key == "uuid") {
                        if (netUuid == outsideMap.value(search_key).toString()) {

                            QDBusInterface conn_interface(
                                "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

                            QDBusInterface interface("org.freedesktop.NetworkManager",
                                                     "/org/freedesktop/NetworkManager",
                                                     "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

                            //获取已经连接了那些网络，及这些网络对应的网络类型(ethernet or wifi)
                            QDBusMessage result =
                                interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
                            if (result.arguments().isEmpty()) {
                                continue;
                            }
                            QList<QVariant> outArgs = result.arguments();
                            QVariant first = outArgs.at(0);
                            QDBusVariant dbvFirst = first.value<QDBusVariant>();
                            QVariant vFirst = dbvFirst.variant();
                            QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();

                            QDBusObjectPath connWiredDivicePath; //已经连接的有线网对应的device路径
                            QDBusObjectPath objPath;
                            dbusArgs.beginArray();
                            while (!dbusArgs.atEnd()) {
                                dbusArgs >> objPath;

                                QDBusInterface interfacePro("org.freedesktop.NetworkManager", objPath.path(),
                                                            "org.freedesktop.DBus.Properties",
                                                            QDBusConnection::systemBus());

                                QDBusReply<QVariant> reply = interfacePro.call(
                                    "Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
                                if (reply.value().toString() == "ethernet"
                                    || reply.value().toString() == "802-3-ethernet") {
                                    //先获取已连接有线网络对应的设备路径
                                    QDBusInterface interfaceConn("org.freedesktop.NetworkManager", objPath.path(),
                                                                 "org.freedesktop.DBus.Properties",
                                                                 QDBusConnection::systemBus());
                                    QDBusMessage replyConn = interfaceConn.call(
                                        "Get", "org.freedesktop.NetworkManager.Connection.Active", "Devices");
                                    if (replyConn.arguments().isEmpty()) {
                                        continue;
                                    }
                                    QList<QVariant> outArgsIp4 = replyConn.arguments();
                                    QVariant firstIp4 = outArgsIp4.at(0);
                                    QDBusVariant dbvFirstIp4 = firstIp4.value<QDBusVariant>();
                                    QVariant vFirstIp4 = dbvFirstIp4.variant();
                                    const QDBusArgument &dbusArgIpv4 = vFirstIp4.value<QDBusArgument>();
                                    QList<QDBusObjectPath> mDatasIpv4;
                                    dbusArgIpv4 >> mDatasIpv4;
                                    connWiredDivicePath = mDatasIpv4.at(0);
                                    // qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx" << mDatasIpv4.at(0).path();
                                }
                            }
                            dbusArgs.endArray();

                            /*
                             * 获取到所需的信息后，接下来进行连接
                             * conn_interface.call("ActivateConnection" 用到的参数
                             * 1. QVariant::fromValue(objSettingPath)  有线网络对应的Setting Path
                             * 2. QVariant::fromValue(connWiredDivicePath) 有线网络对应的Devices Path
                             * 3. QVariant::fromValue(specific_connection) 其他选项路径，可设置为"/"
                             */
                            QDBusReply<QDBusObjectPath> connectionReply = conn_interface.call(
                                "ActivateConnection", QVariant::fromValue(objSettingPath),
                                QVariant::fromValue(connWiredDivicePath), QVariant::fromValue(specific_connection));
                        }
                    }
                }
            }
        } // end for(QString outside_key : map.keys() )

    } // end foreach (QDBusObjectPath objSettingPath, m_objSettingPaths)
}

//利用dbus的方法连接有线网
bool KylinDBus::toConnectWiredNet(QString netUuid, QString netIfName)
{
    bool isConnectUp = false; //是否连接上

    QDBusInterface m_interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
                               "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> m_reply = m_interface.call("ListConnections");

    QDBusObjectPath specific_connection;
    specific_connection.setPath("/");

    QList<QDBusObjectPath> m_objSettingPaths = m_reply.value();
    foreach (QDBusObjectPath objSettingPath, m_objSettingPaths) {
        QDBusInterface m_interface("org.freedesktop.NetworkManager", objSettingPath.path(),
                                   "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
        QDBusMessage result = m_interface.call("GetSettings");
        if (result.arguments().isEmpty()) {
            continue;
        }
        const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QMap<QString, QVariant>> map;
        dbusArg1st >> map;

        for (QString outside_key : map.keys()) {
            QMap<QString, QVariant> outsideMap = map.value(outside_key);
            if (outside_key == "connection") {
                for (QString search_key : outsideMap.keys()) {
                    if (search_key == "uuid") {
                        if (netUuid == outsideMap.value(search_key).toString()) {
                            QDBusInterface connInterface(
                                "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

                            //获取对应的设备路径
                            QDBusObjectPath connWiredDivicePath;
                            connWiredDivicePath.setPath("/");
                            QDBusInterface interfaceDevice(
                                "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

                            QDBusMessage resultDevice =
                                interfaceDevice.call("Get", "org.freedesktop.NetworkManager", "AllDevices");
                            if (resultDevice.arguments().isEmpty()) {
                                continue;
                            }
                            QList<QVariant> outArgsDevice = resultDevice.arguments();
                            QVariant firstDevice = outArgsDevice.at(0);
                            QDBusVariant dbvFirstDevice = firstDevice.value<QDBusVariant>();
                            QVariant vFirstDevice = dbvFirstDevice.variant();
                            QDBusArgument dbusArgsDevice = vFirstDevice.value<QDBusArgument>();

                            QDBusObjectPath objPathDevice;
                            dbusArgsDevice.beginArray();
                            while (!dbusArgsDevice.atEnd()) {
                                dbusArgsDevice >> objPathDevice;
                                QDBusInterface interfaceInterfaceDevice(
                                    "org.freedesktop.NetworkManager", objPathDevice.path(),
                                    "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

                                QDBusReply<QVariant> replyInterfaceDevice = interfaceInterfaceDevice.call(
                                    "Get", "org.freedesktop.NetworkManager.Device", "Interface");
                                if (replyInterfaceDevice.value().toString() == netIfName) {
                                    connWiredDivicePath = objPathDevice;
                                }
                            }
                            dbusArgsDevice.endArray();

                            //下面执行连接网络
                            QDBusReply<QDBusObjectPath> connectionReply = connInterface.call(
                                "ActivateConnection", QVariant::fromValue(objSettingPath),
                                QVariant::fromValue(connWiredDivicePath), QVariant::fromValue(specific_connection));
                            if (!connectionReply.value().path().isEmpty()) {
                                isConnectUp = true;
                            }
                        }
                    }
                }
            }
        } // end for(QString outside_key : map.keys() )

    } // end foreach (QDBusObjectPath objSettingPath, m_objSettingPaths)

    return isConnectUp;
}

//要求底层的驱动进行扫描AP
void KylinDBus::requestScanWifi()
{
    if (multiWirelessPaths.size() == 0)
        return;

    qRegisterMetaType<QMap<QString, QVariant>>("QMap<QString, QVariant>");
    qDBusRegisterMetaType<QMap<QString, QVariant>>();
    QDBusInterface interface("org.freedesktop.NetworkManager", multiWirelessPaths.at(0).path(),
                             "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());
    QMap<QString, QVariant> my_map;
    my_map = {};
    interface.call("RequestScan", my_map);
}

//显示桌面通知
void KylinDBus::showDesktopNotify(QString message)
{
    emit requestSendDesktopNotify(message);
}

//获取任务栏位置，上下左右
int KylinDBus::getTaskBarPos(QString str)
{
    QDBusInterface interface("com.ukui.panel.desktop", "/", "com.ukui.panel.desktop", QDBusConnection::sessionBus());

    QDBusReply<int> reply = interface.call("GetPanelPosition", str);
    if (reply.isValid()) {
        return reply;
    } else {
        return 0; //如果没有成功获取，返回一个默认值
    }
}

//获取任务栏高度
int KylinDBus::getTaskBarHeight(QString str)
{
    QDBusInterface interface("com.ukui.panel.desktop", "/", "com.ukui.panel.desktop", QDBusConnection::sessionBus());

    QDBusReply<int> reply = interface.call("GetPanelSize", str);
    if (reply.isValid()) {
        return reply;
    } else {
        return 46; //如果没有成功获取，返回一个默认值
    }
}

///////////////////////////////////////////////////////////////////////////////
//下方使用Gsetting 进程通信方法

//创建获取任务栏信息的GSetting对象
void KylinDBus::initTaskbarGsetting()
{
    if (QGSettings::isSchemaInstalled("org.ukui.panel.settings")) {
        m_tastbar_gsettings = new QGSettings("org.ukui.panel.settings");
    }
}

//使用GSetting方法获取任务栏高度
int KylinDBus::getTaskbarHeight()
{
    if (!m_tastbar_gsettings) {
        return 46;
    }

    QStringList keys = m_tastbar_gsettings->keys();
    if (keys.contains("panelsize")) {
        int hh = m_tastbar_gsettings->get("panelsize").toInt();
        return hh;
    } else {
        return 46;
    }
}

//使用GSetting方法获取任务栏位置，上下左右
int KylinDBus::getTaskbarPos()
{
    if (!m_tastbar_gsettings) {
        return 0;
    }

    QStringList keys = m_tastbar_gsettings->keys();
    if (keys.contains("panelposition")) {
        int pp = m_tastbar_gsettings->get("panelposition").toInt();
        return pp;
    } else {
        return 0;
    }
}

//创建监听wifi打开或关闭信息的GSetting对象
void KylinDBus::getWifiSwitchState()
{
    if (QGSettings::isSchemaInstalled("org.ukui.control-center.wifi.switch")) {

        m_gsettings = new QGSettings("org.ukui.control-center.wifi.switch");

        // 监听key的value是否发生了变化
        connect(m_gsettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "switch") {
                if (isWirelessCardOn) {
                    bool judge = getSwitchStatus(key);
                    if (judge) {
                        //                        mw->onBtnWifiClicked(2); //打开wifi开关
                        qDebug() << "receive a signal to turn on wifi switch from control-center";
                    } else {
                        //                        mw->onBtnWifiClicked(3); //关闭wifi开关
                        qDebug() << "receive a signal to turn off wifi switch from control-center";
                    }
                }
            }
        });
    }
}

//收到控制面板发来的消息后，获取wifi开关信息
bool KylinDBus::getSwitchStatus(QString key)
{
    if (!m_gsettings) {
        return true;
    }
    const QStringList list = m_gsettings->keys();
    if (!list.contains(key)) {
        return true;
    }
    bool res = m_gsettings->get(key).toBool();
    return res;
}

//通知控制面板wifi开关的信息
void KylinDBus::setWifiSwitchState(bool signal)
{
    if (!m_gsettings) {
        return;
    }

    const QStringList list = m_gsettings->keys();

    if (!list.contains("switchor")) {
        return;
    }
    m_gsettings->set("switchor", signal);
}

//通知控制面板无线网卡的状态信息
void KylinDBus::setWifiCardState(bool signal)
{
    if (!m_gsettings) {
        return;
    }

    const QStringList list = m_gsettings->keys();

    if (!list.contains("wificard")) {
        return;
    }
    m_gsettings->set("wificard", signal);
}

//创建获取窗口透明度信息的GSetting的对象
void KylinDBus::initTransparentState()
{
    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_transparency_gsettings = new QGSettings("org.ukui.control-center.personalise");
    }
}

//使用GSetting获取当前窗口应该使用的透明度
double KylinDBus::getTransparentData()
{
    if (!m_transparency_gsettings) {
        return 0.7;
    }

    QStringList keys = m_transparency_gsettings->keys();
    if (keys.contains("transparency")) {
        double tp = m_transparency_gsettings->get("transparency").toDouble();
        return tp;
    } else {
        return 0.7;
    }
}

//获取当前首选链接
QString KylinDBus::getPrimaryConnection()
{
    QDBusInterface interface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusMessage resultAllDevices = interface.call("Get", "org.freedesktop.NetworkManager", "PrimaryConnection");
    QList<QVariant> outArgsAllDevices = resultAllDevices.arguments();
    QVariant firstAllDevices = outArgsAllDevices.at(0);
    QDBusVariant dbvFirstAllDevices = firstAllDevices.value<QDBusVariant>();
    QVariant vFirstAllDevices = dbvFirstAllDevices.variant();
    const QDBusObjectPath &dbusArgsAllDevices = vFirstAllDevices.value<QDBusObjectPath>();

    m_currentPrimaryConnection = dbusArgsAllDevices.path();

    qDebug() << "当前首选网络为：" << m_currentPrimaryConnection;
    return m_currentPrimaryConnection;
}

//获取当前首选链接
QString KylinDBus::getPrimarySetting()
{
    if (m_currentPrimaryConnection.isEmpty()) {
        return "";
    }
    // ipv4的路径信息和ip信息
    QDBusInterface interface("org.freedesktop.NetworkManager", m_currentPrimaryConnection,
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusMessage resultPrimarySetting =
        interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Connection");

    QList<QVariant> outArgsAllDevices = resultPrimarySetting.arguments();
    QVariant firstAllDevices = outArgsAllDevices.at(0);
    QDBusVariant dbvFirstAllDevices = firstAllDevices.value<QDBusVariant>();
    QVariant vFirstAllDevices = dbvFirstAllDevices.variant();
    const QDBusObjectPath &dbusArgsAllDevices = vFirstAllDevices.value<QDBusObjectPath>();

    m_currentPrimarySetting = dbusArgsAllDevices.path();

    qDebug() << "首选网络的设置为：" << m_currentPrimarySetting;

    return m_currentPrimarySetting;
}

//获取当前首选链接详细配置
//返回为json形式
QString KylinDBus::getPrimaryConfig()
{
    if (m_currentPrimaryConnection.isEmpty() || m_currentPrimarySetting.isEmpty()) {
        return "";
    }
    // ipv4的路径信息和ip信息
    QDBusInterface interface("org.freedesktop.NetworkManager", m_currentPrimarySetting,
                             "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());

    if (!interface.isValid()) {
        qWarning() << "interface unvaliable";
        return "";
    }

    QDBusMessage result = interface.call("GetSettings"); // get information of this network

    if (result.arguments().isEmpty()) {
        qWarning() << "GetSettings: result  isEmpty";
        return "";
    }

    const QDBusArgument &dbusArg1st = result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QMap<QString, QVariant>> map;
    dbusArg1st >> map;

    for (QString outsideKey : map.keys()) {
        QMap<QString, QVariant> outsideMap = map.value(outsideKey);
        if (outsideKey == "connection") {
            for (QString inner_key : outsideMap.keys()) {
                // 获取网络名,命令行读取不能判断名称后面的空格，需要此方法校验
                if (QString::compare("id", inner_key) == 0) {
                    m_currentPrimaryNetName = outsideMap.value(inner_key).toString();

                } else if (QString::compare("interface-name", inner_key) == 0) {
                    m_currentPrimaryNetCard = outsideMap.value("interface-name").toString();
                }
            }
        } else if (outsideKey == "ipv4") {
            for (QString innerKey : outsideMap.keys()) {
                //获取ipv4
                if (innerKey == "address-data") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    QVector<QMap<QString, QVariant>> addressVector;
                    QMap<QString, QVariant> map;

                    dbusArg2nd.beginArray();

                    while (!dbusArg2nd.atEnd()) {
                        QMap<QString, QVariant> tempMap;
                        dbusArg2nd >> tempMap; // append map to a vector here if you want to keep it
                        addressVector.append(tempMap);
                    }
                    dbusArg2nd.endArray();

                    m_ipv4List.clear();
                    for (int i = 0; i < addressVector.size(); ++i) {
                        m_ipv4List.append(addressVector.at(i).value("address").toString());
                    }
                } else if (innerKey == "address") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    QMap<QString, QVariant> map;

                    dbusArg2nd.beginArray();
                    while (!dbusArg2nd.atEnd()) {
                        dbusArg2nd >> map; // append map to a vector here if you want to keep it
                        break;
                    }
                    dbusArg2nd.endArray();

                    m_ipv6 = map.value("address").toString();
                } else if (innerKey == "dns") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    int strDns = 0;
                    dbusArg2nd.beginArray();
                    while (!dbusArg2nd.atEnd()) {
                        dbusArg2nd >> strDns; // append map to a vector here if you want to keep it
                        if (strDns == 0) {
                            // dbusArg2nd为空时，strDns没有从dbusArg2nd获取到结果
                            dbusArg2nd.endArray();
                        }
                    }
                    dbusArg2nd.endArray();
                    m_DNS = strDns;
                } else if (innerKey == "method") {
                    if (QString(outsideMap.value(innerKey).toString()).compare("auto", Qt::CaseInsensitive) == 0) {
                        m_isCurrentPrimaryIPv4DHCP = true;
                    } else {
                        m_isCurrentPrimaryIPv4DHCP = false;
                    }
                }
            }
        } else if (outsideKey == "ipv6") {
            for (QString innerKey : outsideMap.keys()) {
                if (innerKey == "address-data") {
                    const QDBusArgument &dbusArg2nd = outsideMap.value(innerKey).value<QDBusArgument>();
                    QMap<QString, QVariant> map;

                    dbusArg2nd.beginArray();
                    while (!dbusArg2nd.atEnd()) {
                        dbusArg2nd >> map; // append map to a vector here if you want to keep it
                        break;
                    }
                    dbusArg2nd.endArray();

                    m_ipv6 = map.value("address").toString();
                } else if (innerKey == "method") {
                    if (QString(outsideMap.value(innerKey).toString()).compare("auto", Qt::CaseInsensitive) == 0) {
                        m_isCurrentPrimaryIPv6DHCP = true;
                    } else {
                        m_isCurrentPrimaryIPv6DHCP = false;
                    }
                }
            }
        }
    }
    return "";
}
bool KylinDBus::isWiredConnection()
{
    // ipv4的路径信息和ip信息
    QDBusInterface interface("org.freedesktop.NetworkManager", m_currentPrimaryConnection,
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusMessage resultPrimarySetting =
        interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");

    QList<QVariant> outArgsAllDevices = resultPrimarySetting.arguments();
    QVariant firstAllDevices = outArgsAllDevices.at(0);
    QDBusVariant dbvFirstAllDevices = firstAllDevices.value<QDBusVariant>();
    QVariant vFirstAllDevices = dbvFirstAllDevices.variant();
    const QVariant &dbusArgsAllDevices = vFirstAllDevices.value<QVariant>();

    QString primaryNetType = dbusArgsAllDevices.toString();
    if (primaryNetType == "ethernet" || primaryNetType == "802-3-ethernet") {
        qDebug() << "当前用的是有线网卡，网卡类型为：" << primaryNetType;
        return true;
    } else {
        qDebug() << "当前用的是无线网卡，网卡类型为：" << primaryNetType;
        return false;
    }
}
QString KylinDBus::getPrimaryConnUUID()
{
    if (m_currentPrimaryConnection.isEmpty()) {
        return "";
    }
    // ipv4的路径信息和ip信息
    QDBusInterface interface("org.freedesktop.NetworkManager", m_currentPrimaryConnection,
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusMessage resultPrimarySetting =
        interface.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Uuid");

    QList<QVariant> outArgsAllDevices = resultPrimarySetting.arguments();
    QVariant firstAllDevices = outArgsAllDevices.at(0);
    QDBusVariant dbvFirstAllDevices = firstAllDevices.value<QDBusVariant>();
    QVariant vFirstAllDevices = dbvFirstAllDevices.variant();
    const QVariant &dbusArgsAllDevices = vFirstAllDevices.value<QVariant>();

    QString primaryUuid = dbusArgsAllDevices.toString();

    qDebug() << "首选网络的UUID为：" << primaryUuid;
    return primaryUuid;
}
bool KylinDBus::isPrimaryNetDHCP()
{
    bool res = (m_isCurrentPrimaryIPv4DHCP && m_isCurrentPrimaryIPv6DHCP);
    if (res) {
        qDebug() << "首选网络是DHCP获取IP!";
    } else {
        qDebug() << "首选网络是手动获取IP!";
    }
    return res;
}

bool KylinDBus::isSameVlan()
{
	if (m_currentPrimaryConnection.isEmpty() || m_currentPrimarySetting.isEmpty()) {
		qCritical() << "Pre subjects are not finished!";
		return false;
	}

	QDBusInterface interface("org.freedesktop.NetworkManager",
				m_currentPrimarySetting,
				"org.freedesktop.NetworkManager.Settings.Connection",
				QDBusConnection::systemBus());
	if (!interface.isValid()) {
		qCritical() << "interface unvaliable";
		return false;
	}

	QDBusMessage result = interface.call("GetSettings");
	if (result.arguments().isEmpty()) {
		qCritical() << "GetSettings: result  isEmpty";
		return false;
	}

	QDBusArgument settingsDbusArg = result.arguments().at(0).value<QDBusArgument>();
	QMap<QString, QMap<QString, QVariant>> settings;
	settingsDbusArg >> settings;

	QString ip;		/* 点分十进制 */
	QString gateway;	/* 点分十进制 */
	QString mask;		/* 32位二进制 */
	if (settings.contains("ipv4")) {
		QMap<QString, QVariant> ipv4 = settings.value("ipv4");
		if (ipv4.contains("address-data")) {
			const QDBusArgument addressDataDbusArg = ipv4.value("address-data").value<QDBusArgument>();
			QVector<QMap<QString, QVariant>> addressData;
			addressDataDbusArg >> addressData;
			if (addressData.size() != 1) {
				qDebug() << "multiple network cards";
			} else {
				QMap<QString, QVariant> addressItem = addressData.at(0);
				if (addressItem.contains("address"))
					ip = addressItem.value("address").toString();

				if (addressItem.contains("prefix")) {
					uint prefix = addressItem.value("prefix").toUInt();
					for (uint i = 0; i < prefix; i++)
						mask.append('1');

					for (uint j = 0; j < 32 - prefix; j++)
						mask.append('0');
				}
			}
		}

		if (ipv4.contains("gateway"))
			gateway = ipv4.value("gateway").toString();
	}

	qDebug() << "ip: " << ip;
	qDebug() << "gateway: " << gateway;
	qDebug() << "mask: " << mask;

	struct in_addr ipAddr;
	struct in_addr gatewayAddr;
	if (inet_pton(AF_INET, ip.toStdString().c_str(), (void *)&ipAddr) != 1) {
		qCritical() << "ip addr conversion fail !";
		return false;
	}
	if (inet_pton(AF_INET, gateway.toStdString().c_str(), (void *)&gatewayAddr) != 1) {
		qCritical() << "gateway conversion fail !";
		return false;
	}

	uint32_t ipn = ntohl(ipAddr.s_addr);
	uint32_t gatewayn = ntohl(gatewayAddr.s_addr);

	std::bitset<32> bitIp(ipn);
	std::bitset<32> bitGateway(gatewayn);
	std::bitset<32> bitMask(mask.toStdString());

	if ((bitIp & bitMask) == (bitGateway & bitMask))
		return true;

	return false;
}

bool KylinDBus::isDHCPOK()
{
    if (m_currentPrimaryConnection.isEmpty()) {
        qCritical() << "首选网络路径为空！";
        return false;
    }
    QDBusInterface interfaceActiveConn("org.freedesktop.NetworkManager", m_currentPrimaryConnection,
                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusMessage resultDHCP4Setting =
        interfaceActiveConn.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Dhcp4Config");

    QList<QVariant> outArgsAllDHCPs = resultDHCP4Setting.arguments();
    QVariant firstDHCPConfigs = outArgsAllDHCPs.at(0);
    QDBusVariant dbvFirstDHCPConfigs = firstDHCPConfigs.value<QDBusVariant>();
    QVariant vFirstAllDHCP = dbvFirstDHCPConfigs.variant();
    const QDBusObjectPath &dbusArgsAllDHCPs = vFirstAllDHCP.value<QDBusObjectPath>();

    QDBusInterface interfaceDHCP4Config("org.freedesktop.NetworkManager",

                                        dbusArgsAllDHCPs.path(), "org.freedesktop.DBus.Properties",
                                        QDBusConnection::systemBus());
    QDBusMessage resultDHCP4Config =
        interfaceDHCP4Config.call("Get", "org.freedesktop.NetworkManager.DHCP4Config", "Options");

    if (resultDHCP4Config.arguments().isEmpty()) {
        qWarning() << "[KylinDBus] getLanInfo: result  isEmpty";
        return false;
    }

    QList<QVariant> outArgsAllDHCPConfigs = resultDHCP4Config.arguments();

    if (!outArgsAllDHCPConfigs.isEmpty()) {
        QVariant firstDHCPConfigArgs = outArgsAllDHCPConfigs.at(0);
        QDBusVariant dbvFirstDHCPConfigArgs = firstDHCPConfigArgs.value<QDBusVariant>();
        QVariant vFirstAllDHCPArg = dbvFirstDHCPConfigArgs.variant();

        const QDBusArgument &dbusArgsAllDHCPs = vFirstAllDHCPArg.value<QDBusArgument>();
        QMap<QString, QVariant> mDatasDHCPArgs;
        dbusArgsAllDHCPs >> mDatasDHCPArgs;

        if (!mDatasDHCPArgs.isEmpty()) {
            for (QString inner_key : mDatasDHCPArgs.keys()) {
                if (inner_key == "ip_address") {
                    QString ipv4 = mDatasDHCPArgs.value(inner_key).toString();
                    m_DHCP4IP = ipv4StringToInteger(ipv4);
                    qDebug() << "DHCP IPV4 is: " << ipv4;
                    qDebug() << "DHCP IPV4 is: " << ipv4INT2STR(m_DHCP4IP);
                }
                if (inner_key == "subnet_mask") {
                    QString subnet_mask = mDatasDHCPArgs.value(inner_key).toString();
                    m_DHCP4NetMask = subMaskStringToInteger(subnet_mask);
                    qDebug() << "DHCP subnet_mask is: " << m_DHCP4NetMask;
                }
                if (inner_key == "routers") {
                    QString routers = mDatasDHCPArgs.value(inner_key).toString();
                    m_DHCP4NetGateWay = ipv4StringToInteger(routers);
                    qDebug() << "DHCP routers is: " << routers;
                    qDebug() << "DHCP routers is: " << ipv4INT2STR(m_DHCP4NetGateWay);
                }
            }

            quint32 mask = 0;
            for (quint32 i = 31; i > (32 - m_DHCP4NetMask); --i) {
                mask |= 1 << i;
            }

            qDebug() << "掩码为" << (mask);
            qDebug() << "IP与掩码后" << (m_DHCP4IP & mask);
            qDebug() << "网关与掩码后" << (m_DHCP4NetGateWay & mask);

            if ((m_DHCP4IP & mask) != (m_DHCP4NetGateWay & mask)) {
                qWarning() << "DHCP分配的ipv4取掩码后与网关不一致!";
                qDebug() << "DHCP分配的ipv4取掩码后的str为：" << ipv4INT2STR(m_DHCP4IP & mask);
                qDebug() << "DHCP分配的网关取掩码后的str为：" << ipv4INT2STR(m_DHCP4NetGateWay & mask);
                qWarning() << "DHCP分配失败!";
                return false;
            } else {
                qDebug() << "ipv4取掩码后与网关一致!";
                qDebug() << "ipv4取掩码后的str为：" << ipv4INT2STR(m_DHCP4IP & mask);
                qDebug() << "网关取掩码后的str为：" << ipv4INT2STR(m_DHCP4NetGateWay & mask);
                qWarning() << "DHCP分配正常!";
                return true;
            }
        } else {
            qCritical() << "DBUS返回DHCP参数为空！";
            return false;
        }

    } else {
        qCritical() << "DBUS返回DHCP参数为空！";
        return false;
    }

    return false;
}

bool KylinDBus::isDNSSet()
{
    if (!getDNS().isEmpty())
        return true;

    return false;
}

QStringList KylinDBus::getDNS()
{
    QStringList res;

    if (m_currentPrimaryConnection.isEmpty()) {
        qCritical() << "无活动连接！";
        return res;
    } else {
        QDBusInterface interfaceActiveConn("org.freedesktop.NetworkManager", m_currentPrimaryConnection,
                                           "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusMessage resultIP4Config =
            interfaceActiveConn.call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip4Config");

        QList<QVariant> outIP4ConfigArgs = resultIP4Config.arguments();
        QVariant firstipv4Configs = outIP4ConfigArgs.at(0);
        QDBusVariant dbvFirstIPV4Configs = firstipv4Configs.value<QDBusVariant>();
        QVariant vFirstAllIPV4 = dbvFirstIPV4Configs.variant();
        const QDBusObjectPath &dbusArgsAllIPV4s = vFirstAllIPV4.value<QDBusObjectPath>();

        m_currentActiveConnectionIPV4Config = dbusArgsAllIPV4s.path();

        if (m_currentActiveConnectionIPV4Config.isEmpty()) {
            qCritical() << "获取IPv4Config路径失败！";
            return res;
        } else {
            QDBusInterface interfaceIPV4Config("org.freedesktop.NetworkManager", m_currentActiveConnectionIPV4Config,
                                               "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
            QDBusMessage resultIPV4Config =
                interfaceIPV4Config.call("Get", "org.freedesktop.NetworkManager.IP4Config", "NameserverData");
            if (resultIPV4Config.arguments().isEmpty()) {
                qWarning() << "[KylinDBus] getDNSInfo: result  isEmpty";
                return res;
            }

            QList<QVariant> outArgs = resultIPV4Config.arguments();
            QVariant first = outArgs.at(0);
            QDBusVariant dbvFirst = first.value<QDBusVariant>();
            QVariant vFirst = dbvFirst.variant();
            QDBusArgument dbusArgs = vFirst.value<QDBusArgument>();
            QList<QMap<QString, QVariant>> dnsList;
            dbusArgs.beginArray();
            while (!dbusArgs.atEnd()) {
                QMap<QString, QVariant> dns;
                dbusArgs >> dns;
                dnsList.append(dns);
            }

            for (const auto &item: dnsList) {
                QStringList keys = item.keys();
                for (const auto &key: keys) {
                    if (key == "address") {
                        QString dns = item.value("address").toString();
                        if (!dns.isEmpty())
                            res.append(dns);
                    }
                }
            }
        }
        return res;
    }
}

QString KylinDBus::ipv4INT2STR(quint32 intIP)
{
    quint32 ip1 = intIP & 0xFF;
    quint32 ip2 = (intIP >> 8) & 0xFF;
    quint32 ip3 = (intIP >> 16) & 0xFF;
    quint32 ip4 = (intIP >> 24) & 0xFF;
    QString ip4Str = QString("%1.%2.%3.%4").arg(ip4).arg(ip3).arg(ip2).arg(ip1);
    return ip4Str;
}
quint32 KylinDBus::ipv4StringToInteger(const QString &ip)
{
    QStringList ips = ip.split(".");
    if (ips.size() == 4) {
        return ips.at(0).toInt() << 24 | ips.at(1).toInt() << 16 | ips.at(2).toInt() << 8 | ips.at(3).toInt();
    }
    return 0;
}
int KylinDBus::subMaskStringToInteger(const QString &ip)
{
    int subDigital = 0;
    QStringList ips = ip.split(".");
    if (ips.size() == 4) {
        for (auto ipStr : ips) {
            int ip = ipStr.toInt();
            if (ip == 255) {
                subDigital += 8;
            } else if (ip == 0) {
                continue;
            } else {
                while (ip > 0) {
                    subDigital += ip % 2;
                    ip /= 2;
                }
            }
        }
    }
    return subDigital;
}
