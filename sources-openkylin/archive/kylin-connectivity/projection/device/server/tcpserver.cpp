#include "tcpserver.h"
#include "videosocket.h"
#include "inputsocket.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent) {}

TcpServer::~TcpServer() {}

void TcpServer::incomingConnection(qintptr handle)
{
    if (m_isVideoSocket) {
        VideoSocket *socket = new VideoSocket();
        connect(socket, &VideoSocket::disconnected, this, &TcpServer::sigSocketDisconnect);
        socket->setSocketDescriptor(handle);
        addPendingConnection(socket);

        // next is control socket
        m_isVideoSocket = false;
    } else if (m_isControlConnect) {
        InputSocket *socket = new InputSocket();
        socket->setSocketDescriptor(handle);
        addPendingConnection(socket);
    } else {
        QTcpSocket *socket = new QTcpSocket();
        socket->setSocketDescriptor(handle);
        addPendingConnection(socket);
        m_isControlConnect = true;
    }
}
