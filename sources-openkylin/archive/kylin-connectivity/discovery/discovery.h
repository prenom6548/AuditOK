//
// Created by sqp on 2022/4/26.
//

#ifndef KYLIN_CONNECTIVITY_DISCOVERY_H
#define KYLIN_CONNECTIVITY_DISCOVERY_H

#include <QUdpSocket>
#include <QLocalServer>
#include <QList>
#include <QTimer>

#include <thread>
#include "util.h"
#include "discdeviceinfo.h"
#include "unix-util.h"

namespace KDiscovery
{
class Discovery : public QObject
{
    Q_OBJECT
public:
    Discovery(QObject *parent = nullptr);
    ~Discovery();
    void startDiscovery();
    void stopDiscovery();
    void sendRequest(RequestType type);

Q_SIGNALS:
    void sigAllDeviceInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &deviceInfo);

private Q_SLOTS:
    void onReadyRead();

private:
    void getClientInfo();
    void getDeviceInfo(char *buf, qint64 len);


private:
    QLocalSocket *m_socket = nullptr;

    QString m_clientIp;
    quint16 m_clientport; //端口号
    QString m_deviceName;
    QString m_uuid;
    quint16 m_devType;
    QList<QString> m_currentDeviceIpList;
    QMap<QString, KCommon::DiscoveryDeviceInfo> m_discoveryDeviceInfo;
};
} // namespace KDiscovery

#endif // KYLIN_CONNECTIVITY_DISCOVERY_H
