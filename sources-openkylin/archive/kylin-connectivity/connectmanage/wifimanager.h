#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QObject>
#include <QString>
#include <QtDBus/QDBusObjectPath>

class WifiManager : public QObject
{
    Q_OBJECT
public:
    WifiManager()
    {
    }
    ~WifiManager();

private:
    QString checkWifiConnectivity();

    QDBusObjectPath getConnectedWiFiIPPath(QDBusObjectPath objPath);

    QString getConnectedWiFiIP(QDBusObjectPath objPath);

Q_SIGNALS:

    void sigWifiConnected(QString wifiIP);

    void sigWifiDisconnected();

public Q_SLOTS:

    void startWifiInfo();

    void onPropertiesChanged(QString str, QVariantMap map);

private:
    QString m_wifiIP = "";
    int m_count = 0;
};

#endif  // WIFIMANAGER_H
