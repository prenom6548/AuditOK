#include "wifimanager.h"

#include <QObject>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#include <QVariant>
#include <QVariantMap>
#include <QThread>

WifiManager::~WifiManager()
{
}

void WifiManager::onPropertiesChanged(QString str, QVariantMap map)
{
    Q_UNUSED(str);
    Q_UNUSED(map);

    QString wifiIP = checkWifiConnectivity();
    if (!m_wifiIP.isEmpty()) {
        if (wifiIP.isEmpty()) {
            qInfo() << m_wifiIP << "disconnect";
            m_wifiIP.clear();
            Q_EMIT sigWifiDisconnected();
        }
    } else {
        while (checkWifiConnectivity().isEmpty()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }
            QThread::msleep(100);
            m_count++;
            if (m_count > 20) {
                break;
            }
        }
        m_wifiIP = wifiIP;
        qInfo() << m_wifiIP << "connect";
        Q_EMIT sigWifiConnected(m_wifiIP);
    }
}

QString WifiManager::checkWifiConnectivity()
{
    QDBusInterface *interface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                                   "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    if (!interface->isValid()) {
        qCritical() << "Failed to get NetworkManager!";
        interface->deleteLater();
        return QString("");
    }

    QVariant var = interface->property("ActiveConnections");
    if (!var.isValid()) {
        qCritical() << "Failed to get ActiveConnections!";
        interface->deleteLater();
        return QString("");
    }
    QList<QDBusObjectPath> msg;
    if (var.canConvert(QMetaType::QVariantList)) {
        msg = var.value<QList<QDBusObjectPath>>();
        for (auto path : msg) {
            QDBusObjectPath wifiIPPath = getConnectedWiFiIPPath(path);
            if (!wifiIPPath.path().isEmpty()) {
                QString wifiIP = getConnectedWiFiIP(wifiIPPath);
                if (!wifiIP.isEmpty()) {
                    interface->deleteLater();
                    return wifiIP;
                }
            }
        }
    }

    interface->deleteLater();
    return QString("");
}

QDBusObjectPath WifiManager::getConnectedWiFiIPPath(QDBusObjectPath objPath)
{
    QDBusInterface *interfaceType = new QDBusInterface("org.freedesktop.NetworkManager", objPath.path(),
                                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    QDBusReply<QVariant> replyType =
        interfaceType->call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Type");
    if (replyType.value().toString() == "wifi" || replyType.value().toString() == "802-11-wireless") {
        QDBusReply<QVariant> replyState =
            interfaceType->call("Get", "org.freedesktop.NetworkManager.Connection.Active", "State");
        int wifiState = replyState.value().toUInt();  // 正在连接的状态是1，连接上的状态是2
        if (wifiState == 2) {
            QDBusMessage replyIp4 =
                interfaceType->call("Get", "org.freedesktop.NetworkManager.Connection.Active", "Ip4Config");
            QList<QVariant> outArgsIp4 = replyIp4.arguments();
            if (!outArgsIp4.isEmpty()) {
                QVariant firstIp4 = outArgsIp4.at(0);
                QDBusVariant dbvFirstIp4 = firstIp4.value<QDBusVariant>();
                QVariant vFirstIp4 = dbvFirstIp4.variant();
                QDBusObjectPath dbusPathIp4 = vFirstIp4.value<QDBusObjectPath>();
                interfaceType->deleteLater();
                return dbusPathIp4;
            }
        }
    }

    interfaceType->deleteLater();
    return QDBusObjectPath();
}

QString WifiManager::getConnectedWiFiIP(QDBusObjectPath objPath)
{
    QDBusInterface *interfaceIpv4 = new QDBusInterface("org.freedesktop.NetworkManager", objPath.path(),
                                                       "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusMessage replyIpv4 = interfaceIpv4->call("Get", "org.freedesktop.NetworkManager.IP4Config", "AddressData");

    QList<QVariant> outArgsIpv4 = replyIpv4.arguments();
    if (!outArgsIpv4.isEmpty()) {
        QVariant firstIpv4 = outArgsIpv4.at(0);
        QDBusVariant dbvFirstIpv4 = firstIpv4.value<QDBusVariant>();
        QVariant vFirstIpv4 = dbvFirstIpv4.variant();

        const QDBusArgument &dbusArgIpv4 = vFirstIpv4.value<QDBusArgument>();
        QList<QVariantMap> mDatasIpv4;
        dbusArgIpv4 >> mDatasIpv4;
        if (!mDatasIpv4.isEmpty()) {
            for (QVariantMap mDataIpv4 : mDatasIpv4) {
                QString wifiIP = mDataIpv4.value("address").toString();
                if (!wifiIP.isEmpty()) {
                    interfaceIpv4->deleteLater();
                    return wifiIP;
                }
            }
        }
    }

    interfaceIpv4->deleteLater();
    return QString("");
}

void WifiManager::startWifiInfo()
{
    m_wifiIP = checkWifiConnectivity();
    if (!m_wifiIP.isEmpty()) {
        qInfo() << m_wifiIP << "connect";
        Q_EMIT sigWifiConnected(m_wifiIP);
    } else {
        qInfo() << "There is currently no WiFi connection!";
        Q_EMIT sigWifiDisconnected();
    }
    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                         "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap)));
}
