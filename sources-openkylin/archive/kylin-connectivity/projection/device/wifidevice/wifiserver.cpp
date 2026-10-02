#include <QCoreApplication>
#include <QTime>
#include "wifiserver.h"
#include "apputil.h"

const int MAX_CONNECT_COUNT = 30;
const int MAX_RESTART_COUNT = 1;


WifiServer::WifiServer(QObject *parent, QString ip, int port) : BaseServer(parent)
{
    m_ip = ip;
    m_port = port;
    m_videoSocket = new VideoSocket(this);
    connect(m_videoSocket, &QTcpSocket::connected, this, &WifiServer::onConnected);
    connect(m_videoSocket, &QTcpSocket::disconnected, this, &WifiServer::sigSocketDisconnect);

    m_videoSocket->connectToHost(QHostAddress(m_ip), m_port);
}

WifiServer::~WifiServer() {}

void WifiServer::stop()
{
    if (m_videoSocket) {
        m_videoSocket->close();
        m_videoSocket->deleteLater();
    }
    m_serverSocket.close();
}

void WifiServer::onConnected()
{
    m_clientIP = AppUtil::convertToIPv4Addr(m_videoSocket->peerAddress());
    if (m_videoSocket) {
        if (!m_videoSocket->isValid() || !readInfo(m_videoSocket, m_deviceName, m_deviceSize)) {
            stop();
            Q_EMIT connectToResult(false);
        } else {
            Q_EMIT connectToResult(true, m_deviceName, m_deviceSize, m_clientIP);
        }
    }
}