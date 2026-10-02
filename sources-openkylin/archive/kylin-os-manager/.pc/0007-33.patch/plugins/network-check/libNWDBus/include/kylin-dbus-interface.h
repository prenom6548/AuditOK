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

#ifndef KYLINDBUSINTERFACE_H
#define KYLINDBUSINTERFACE_H

#include <QObject>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QVariant>
#include <QVariantMap>
#include <QGSettings/QGSettings>
#include <QTimer>
#include <QThread>

#define WIFI_CONNECTING 1
#define WIFI_CONNECTED 2
#define WIFI_DISCONNECTED 3

class MainWindow;
class Utils;

class KylinDBus : public QObject
{
    Q_OBJECT
public:
    explicit KylinDBus(MainWindow *mw = 0, QObject *parent = nullptr);
    ~KylinDBus();

    typedef enum {
        NM_DEVICE_TYPE_UNKNOWN = 0,
        NM_DEVICE_TYPE_ETHERNET = 1,
        NM_DEVICE_TYPE_WIFI = 2,
        NM_DEVICE_TYPE_UNUSED1 = 3,
        NM_DEVICE_TYPE_UNUSED2 = 4,
        NM_DEVICE_TYPE_BT = 5, /* Bluetooth */
        NM_DEVICE_TYPE_OLPC_MESH = 6,
        NM_DEVICE_TYPE_WIMAX = 7,
        NM_DEVICE_TYPE_MODEM = 8,
        NM_DEVICE_TYPE_INFINIBAND = 9,
        NM_DEVICE_TYPE_BOND = 10,
        NM_DEVICE_TYPE_VLAN = 11,
        NM_DEVICE_TYPE_ADSL = 12,
        NM_DEVICE_TYPE_BRIDGE = 13,
        NM_DEVICE_TYPE_GENERIC = 14,
        NM_DEVICE_TYPE_TEAM = 15,
        NM_DEVICE_TYPE_TUN = 16,
        NM_DEVICE_TYPE_IP_TUNNEL = 17,
        NM_DEVICE_TYPE_MACVLAN = 18,
        NM_DEVICE_TYPE_VXLAN = 19,
        NM_DEVICE_TYPE_VETH = 20,
        NM_DEVICE_TYPE_MACSEC = 21,
        NM_DEVICE_TYPE_DUMMY = 22,
        NM_DEVICE_TYPE_PPP = 23,
        NM_DEVICE_TYPE_OVS_INTERFACE = 24,
        NM_DEVICE_TYPE_OVS_PORT = 25,
        NM_DEVICE_TYPE_OVS_BRIDGE = 26,
        NM_DEVICE_TYPE_WPAN = 27,
        NM_DEVICE_TYPE_6LOWPAN = 28,
        NM_DEVICE_TYPE_WIREGUARD = 29,
        NM_DEVICE_TYPE_WIFI_P2P = 30,
    } NMDeviceType;

    void getObjectPath();
    void getBlockNeededDevice();
    QStringList getUnvisibleDeviceNames()
    {
        return m_unvisibleDeviceNames;
    }
    QStringList getUnvisibleDevicePaths()
    {
        return m_unvisibleDevicePaths;
    }

    int getAccessPointsNumber();
    int getWiredNetworkNumber();
    QStringList getWifiSsidList();
    QString checkHasWifiConfigFile(QString wifiName);
    void showDesktopNotify(QString message);
    void initConnectionInfo();
    QList<QString> getAtiveLanSsidUuidState();
    void disConnectWiredConnect();
    QString getActiveWifiUuid();
    QList<QString> getAtiveWifiBSsidUuid(QStringList wifilist);
    void reConnectWiredNet(QString netUuid);
    bool toConnectWiredNet(QString netUuid, QString netIfName);
    void getConnectNetIp(QString netUuid);
    void getLanIpChanged();
    void onWiredSettingNumChanged();

    int getTaskBarPos(QString str);
    int getTaskBarHeight(QString str);
    void initTaskbarGsetting();
    int getTaskbarHeight();
    int getTaskbarPos();

    void getWifiSwitchState();
    bool getSwitchStatus(QString key);
    void setWifiSwitchState(bool signal);
    void setWifiCardState(bool signal);

    void initTransparentState();
    double getTransparentData();
    int checkWifiConnectivity();
    bool checkNetworkConnectivity();
    QStringList getActiveLan();
    QStringList getActiveWlanAndWifiSignal();
    QString getWifiSsid(QString accessPointPath);
    void toGetWifiList();

    int getNetworkConectivity();
    int getLanConnectivity(QDBusObjectPath path);
    int getWlanConnectivity(QDBusObjectPath path);
    int getLanOrWlanConnectivity(QDBusObjectPath path);

    QDBusObjectPath wirelessPath;              //无线设备的路径
    QList<QDBusObjectPath> multiWirelessPaths; // Wireless Device的对象路径列表
    QList<QDBusObjectPath> multiWiredPaths;    // Wired Device的对象路径列表
    QList<QString> multiWiredCableState;       //多有线网卡的情况，判断有线网卡对应网线是否插入
    QList<QString> multiWiredMac;              //对应有线网卡的Mac地址
    QList<QString> multiWiredIfName;           //对应有线网的接口

    bool isWiredCableOn = false;   //是否插入了网线
    bool isWirelessCardOn = false; //是否插入了无线网卡

    QStringList dbusLanIpv4;
    QString dbusLanIpv6 = "";
    QString dbusLanIpv6Method = "";
    QString dbusWifiIpv4Method = "";
    QString dbusWifiIpv6Method = "";
    QStringList dbusActiveLanIpv4;
    QString dbusActiveLanIpv6 = "";
    QString dbusActiveWifiIpv4 = "";
    QString dbusActiveWifiIpv6 = "";
    QString dbusWifiIpv4 = "";
    QString dbusWifiIpv6 = "";
    QString dbusLanGateway = "";
    QString dbusWiFiCardName = "";
    QString dbusWifiMac = "";
    QString dbusIfName;
    QString dbusMacDefault;
    int dbusActLanDNS;
    bool oldWifiSwitchState; //上一次获取到的wifi开关状态

    //网络检测工具后添加函数及变量
    QString getPrimaryConnection();
    QString getPrimarySetting();
    QString getPrimaryConfig();
    QString getPrimaryConnUUID();
    bool isWiredConnection();
    bool isPrimaryNetDHCP();
    bool isSameVlan();
    bool isDHCPOK();
    bool isDNSSet();
    QStringList getDNS();

public slots:
    void onNewConnection(QDBusObjectPath objPath);
    void onConnectionRemoved(QDBusObjectPath objPath);
    void toCreateNewLan();
    bool getWiredCableStateByIfname(QString ifname);
    QString getConnLanNameByIfname(QString ifname);
    void onPropertiesChanged(QVariantMap qvm);
    void onAutoConnect();
    void onLanWiredPropertyChanged(QVariantMap qvm);
    void onLanPropertyChanged(QString strInfo, QVariantMap qvm, QStringList strlistInfo);
    void onWlanPropertyChanged(QString strInfo, QVariantMap qvm, QStringList strlistInfo);
    void onLanIpPropertiesChanged();
    void onWifiIpPropertiesChanged();
    void getPhysicalCarrierState(int n);
    void getLanHwAddressState();
    void getWiredCardName();
    void getWirelessCardName();
    void getLanInfo(QString uuid, bool isActNet, QString &lanName);
    void getWifiIp(QString uuid);
    QString getLanMAC(QString ifname);
    void getWifiMac(QString netName);
    void slot_timeout();
    void requestScanWifi();

private:
    MainWindow *mw;
    Utils *mUtils;
    QThread *mUtilsThread;

    bool isRunningFunction = false;
    QTimer *time = nullptr;
    QList<QDBusObjectPath> oldPaths;        //已连接网络的对象路径列表
    QList<QDBusObjectPath> oldSettingPaths; //保存之前的路径
    QStringList oldPathInfo;                //某个已连接网络对象路径对应的网络类型(ethernet or wifi)

    QGSettings *m_tastbar_gsettings = nullptr;
    QGSettings *m_gsettings = nullptr;
    QGSettings *m_transparency_gsettings = nullptr;
    QStringList m_lanPathList; //有线网dbuspath列表

    QList<QString> m_unvisibleDeviceNames; //需要屏蔽的网卡名称
    QList<QString> m_unvisibleDevicePaths; //需要屏蔽的网卡地址

    //网络检测工具后添加函数及变量
    QString m_currentPrimaryConnection = "";
    QString m_currentPrimarySetting = "";
    QString m_currentPrimaryNetName = "";
    QString m_currentPrimaryNetCard = "";
    bool m_isCurrentPrimaryIPv4DHCP = false;
    bool m_isCurrentPrimaryIPv6DHCP = false;


    QString m_currentActiveConnection = "";
    QString m_currentActiveConnectionIPV4Config = "";

    QStringList m_ipv4List;
    QString m_ipv6 = "";
    int m_DNS;

    quint32 m_currentPrimaryIPv4 = 0;
    quint32 m_currentPrimaryNetMask = 0;
    quint32 m_currentPrimaryNetGateWay = 0;

    quint32 m_DHCP4IP = 0;
    quint32 m_DHCP4NetMask = 0;
    quint32 m_DHCP4NetGateWay = 0;

    QString ipv4INT2STR(quint32 intIP);
    quint32 ipv4StringToInteger(const QString &ip);
    int subMaskStringToInteger(const QString &ip);
signals:
    void updateWiredList(int n);
    void updateWirelessList();
    void requestSendDesktopNotify(QString message);
    void newConnAdded(int type);
    void toGetWifiListFinished(QStringList slist);
};

#endif // KYLINDBUSINTERFACE_H
