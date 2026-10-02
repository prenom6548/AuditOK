#include "serverstatus.h"

#include <QTcpSocket>
#include <QThread>
#include <QUrl>

const int CONNECT_NUM_MAX = 10;

ServerStatus::ServerStatus(QUrl url)
{
    m_url = url;
}
void ServerStatus::startServer()
{
    bool state = isConnect();
    Q_EMIT sigServerState(state);
}

bool ServerStatus::isConnect()
{
    bool isConnect = false;
    int connectionTimes = 0;
    QTcpSocket *tcpSocket = new QTcpSocket;
    while (1) {
        tcpSocket->connectToHost(m_url.host(), m_url.port());
        isConnect = tcpSocket->waitForReadyRead();
        if (isConnect) {
            break;
        }
        connectionTimes++;
        if (connectionTimes > CONNECT_NUM_MAX) {
            // 尝试连接服务器十次
            break;
        }
        QThread::msleep(500);
    }
    qInfo() << "Server status: " << isConnect;
    tcpSocket->deleteLater();
    return isConnect;
}
