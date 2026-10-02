#include "networkmanagerhelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString networkService = QStringLiteral("org.freedesktop.NetworkManager");
const static QString networkPath = QStringLiteral("/org/freedesktop/NetworkManager");
const static QString networkInterface = QStringLiteral("org.freedesktop.NetworkManager");
const static QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

NetWorkManagerHelper::NetWorkManagerHelper(QObject *parent)
    : QObject{parent}
{
    m_networkService
        = new QDBusInterface(networkService, networkPath, networkInterface, QDBusConnection::systemBus(), this);

    m_iNetWorkConnectStatus = m_networkService->property("Connectivity").toInt();
    qDebug() << " m_iNetWorkConnectStatus = " << m_iNetWorkConnectStatus;
    // 监听属性变化
    QDBusConnection::systemBus().connect(
        networkService,
        networkPath,
        propertiesInterface,
        "PropertiesChanged",
        this,
        SLOT(onNetworkPropChanged(QString, QVariantMap, QStringList)));
}

int NetWorkManagerHelper::getNetWorkConnectStatus()
{
    return m_iNetWorkConnectStatus;
}

void NetWorkManagerHelper::onNetworkPropChanged(QString strInterface, QVariantMap mapVar, QStringList listValue)
{
    qDebug() << "onNetworkPropChanged:" << strInterface << mapVar << listValue;
    QVariantMap::iterator itVar = mapVar.begin();
    for (; itVar != mapVar.end(); itVar++) {
        qDebug() << " key = " << itVar.key() << "value = " << itVar.value();
        if (itVar.key() == "Connectivity") {
            QVariant varValue = itVar.value();
            m_iNetWorkConnectStatus = varValue.toInt();
            Q_EMIT netWorkConnectStateChanged(m_iNetWorkConnectStatus);
            break;
        }
    }
}
