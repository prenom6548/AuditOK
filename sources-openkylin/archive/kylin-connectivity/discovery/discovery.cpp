//
// Created by sqp on 2022/4/26.
//

#include "discovery.h"

#include <QLocalSocket>
#include <QDataStream>
#include <QRandomGenerator>
#include "generatetools.h"
#include "unix-util.h"

namespace KDiscovery {
const char *SERVER_UNIX_SOCKET_ADDR = "/tmp/kylin.deviceDiscover.socket";
const int BUF_LEN = 1024;

using namespace tlv;

Discovery::Discovery(QObject *parent) : QObject(parent)
{
    m_socket = new QLocalSocket(this);
    connect(m_socket, &QLocalSocket::readyRead, this, &Discovery::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, m_socket, &QLocalSocket::deleteLater);
}

Discovery::~Discovery() noexcept
{
    qInfo() << "~Discovery()";
    //    stopDiscovery();
}

void Discovery::startDiscovery()
{
    qInfo() << "start discovery";
    m_socket->connectToServer(SERVER_UNIX_SOCKET_ADDR);
    GenerateTools::delayMs(200);
    sendRequest(START_DISCOVERY);
}

void Discovery::stopDiscovery()
{
    qInfo() << "stop discovery";

    sendRequest(STOP_DISCOVERY);

    GenerateTools::delayMs(100);

    // m_socket->close();
    m_discoveryDeviceInfo.clear();
}

void Discovery::onReadyRead()
{
    char buf[BUF_LEN];
    int len = m_socket->read(buf, BUF_LEN);
    if (len < 0) {
        qWarning() << "recv device info fail";
    }
    getDeviceInfo(buf, len);
}

void Discovery::getDeviceInfo(char *buf, qint64 len)
{
    DiscoveryDeviceInfo parsedBoxes;
    if (!parsedBoxes.Parse((unsigned char *)buf, len)) {
        qWarning() << "DiscoveryDeviceInfo Parse Failed !\n";
        return;
    }

    DiscoveryDeviceInfo parsedBox;
    if (!parsedBoxes.GetObjectValue(RESPONSE, parsedBox)) {
        qWarning() << "GetObjectValue Failed !\n";
        return;
    }

    int value;
    if (!parsedBox.GetResponseValue(RESPONSE, value)) {
        qWarning() << "GetResponseValue Failed !\n";
        return;
    }

    switch (value) {
        case DEVICE_INFO: {
            {
                char value[128];
                int length = 128;
                if (!parsedBox.GetUuidValue(DEVICE_UUID, value, length)) {
                    qWarning() << "GetUuidValue Failed !\n";
                    return;
                }
                qInfo() << "GetUuidValue Success " << value;
                m_uuid = value;
            }

            {
                int value;
                if (!parsedBox.GetDevTypeValue(DEVICE_TYPE, value)) {
                    qWarning() << "GetDevTypeValue Failed !\n";
                    return;
                }
                qInfo() << "GetDevTypeValue Success " << value;
                if (value != KCommon::DeviceType::PC && value != KCommon::DeviceType::Phone) {
                    return;
                }
                m_devType = value;
            }

            {
                char value[128];
                int length = 128;
                if (!parsedBox.GetDevNameValue(DEVICE_NAME, value, length)) {
                    qWarning() << "GetDevNameValue Failed !\n";
                    return;
                }
                qInfo() << "GetDevNameValue Success " << value;
                m_deviceName = value;
            }

            {
                char value[128];
                int length = 128;
                if (!parsedBox.GetDevIpValue(DEVICE_IP, value, length)) {
                    qWarning() << "GetDevIpValue Failed !\n";
                    return;
                }
                qInfo() << "GetDevIpValue Success " << value;
                m_clientIp = value;
            }

            {
                int value;
                if (!parsedBox.GetDevPortValue(DEVICE_PORT, value)) {
                    qWarning() << "GetDevPortValue Failed !\n";
                    return;
                }
                qInfo() << "GetDevPortValue Success " << value;
                m_clientport = value;
            }

            if (!m_currentDeviceIpList.contains(m_clientIp)) {
                getClientInfo();
            }
            break;
        }
        default:
            break;
    }
}

void Discovery::getClientInfo()
{
    KCommon::DiscoveryDeviceInfo deviceInfo;

    deviceInfo.ip = m_clientIp;
    deviceInfo.port = m_clientport;
    deviceInfo.uuid = m_uuid;
    deviceInfo.deviceType = m_devType;
    deviceInfo.deviceName = m_deviceName;

    if (!m_discoveryDeviceInfo.contains(m_clientIp)) {
        m_discoveryDeviceInfo.insert(m_clientIp, deviceInfo);
        Q_EMIT sigAllDeviceInfo(m_discoveryDeviceInfo);
    }
}

void Discovery::sendRequest(RequestType type)
{
    DiscoveryDeviceInfo requestMsg;

    requestMsg.PutRequestValue(REQUEST, type);

    if (!requestMsg.Serialize()) {
        qWarning() << "requestMsg Serialize Failed !\n";
        return;
    }

    DiscoveryDeviceInfo request;
    request.PutObjectValue(REQUEST, requestMsg);

    if (!request.Serialize()) {
        qWarning() << "REQUEST Serialize Failed !\n";
        return;
    }

    if (m_socket) {
        if ((m_socket->write((const char *)request.GetSerializedBuffer(), request.GetSerializedBytes())) < 0) {
            qWarning() << "send request fail";
        }
    }
}

}  // namespace KDiscovery
