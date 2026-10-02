/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */

#include "localserver.h"

#include <QTime>
#include <QTimer>

LocalServer* LocalServer::_self = nullptr;

LocalServer* LocalServer::self(QObject *parent)
{
    QMutex mutex;
    mutex.lock();
    if(!_self)
        _self = new LocalServer(parent);
    mutex.unlock();
    return _self;
}

LocalServer::LocalServer(QObject *parent)
    :QObject(parent)
    ,m_localServer(nullptr)
{
    init();
    initConnect();
}

LocalServer::~LocalServer()
{
    if(m_localServer){
        m_localServer->close();
        delete m_localServer;
    }
}

void LocalServer::init()
{
    m_localServer = new QLocalServer;
    m_localServer->setSocketOptions(QLocalServer::SocketOption::WorldAccessOption);
}

void LocalServer::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &LocalServer::onNewConnectionHandler);
}

void LocalServer::runServer()
{
    // qDebug() << "Run Input Gather Server!";
    QLocalServer::removeServer(INPUT_GATHER_SERVER_NAME);
    if(m_localServer->listen(INPUT_GATHER_SERVER_NAME))
    {
        qDebug() << "info: [LocalServer][runServer]:" << "Listen " << INPUT_GATHER_SERVER_NAME << " Success!";
    }else
    {
        qDebug() << "info: [LocalServer][runServer]:" << "Listen " << INPUT_GATHER_SERVER_NAME << " Failed!";
    }
}

void LocalServer::send(QByteArray msg)
{
    // qDebug() << "LocalServer send message:" << msg;
    for(auto it = m_clientSockets.begin(); it != m_clientSockets.end();)
    {
        QLocalSocket* clientSocket = *it;
        if(!clientSocket || !clientSocket->isValid()){
            m_clientSockets.erase(it);
            continue;
        }
        const QByteArray &info = msg;
        clientSocket->write(info);
        clientSocket->flush();
        ++it;
    }
}

void LocalServer::send(Event tevent)
{
    QByteArray eventData;
    eventData.append((char *)&tevent, sizeof(tevent));
    send(eventData);
}

void LocalServer::onNewConnectionHandler()
{
    qDebug() << "info: [LocalServer][onNewConnectionHandler]: 检测到client建立到server的链接";
    QLocalSocket *clientSocket = new QLocalSocket(this);
    clientSocket = m_localServer->nextPendingConnection();
    connect(clientSocket, &QLocalSocket::disconnected, clientSocket, &QLocalSocket::deleteLater);
    // connect(clientSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onSocketError(QLocalSocket::LocalSocketError)));

    m_clientSockets.push_back(clientSocket);
}

void LocalServer::onSocketClose()
{

}

void LocalServer::onSocketError(QLocalSocket::LocalSocketError error)
{
    qDebug() << "info: [LocalServer][onSocketError]: " << error2Str(error);
}

QString LocalServer::error2Str(QLocalSocket::LocalSocketError error)
{
    switch (error) {
    case QLocalSocket::LocalSocketError::ConnectionRefusedError: return "ConnectionRefusedError";
    case QLocalSocket::LocalSocketError::PeerClosedError: return "PeerClosedError";
    case QLocalSocket::LocalSocketError::ServerNotFoundError: return "ServerNotFoundError";
    case QLocalSocket::LocalSocketError::SocketAccessError: return "SocketAccessError";
    case QLocalSocket::LocalSocketError::SocketTimeoutError: return "SocketTimeoutError";
    case QLocalSocket::LocalSocketError::DatagramTooLargeError: return "DatagramTooLargeError";
    case QLocalSocket::LocalSocketError::ConnectionError: return "ConnectionError";
    case QLocalSocket::LocalSocketError::UnsupportedSocketOperationError: return "UnsupportedSocketOperationError";
    case QLocalSocket::LocalSocketError::UnknownSocketError: return "UnknownSocketError";
    case QLocalSocket::LocalSocketError::OperationError: return "OperationError";
    default: return "LocalSocketErrorEnum";
    }
}
