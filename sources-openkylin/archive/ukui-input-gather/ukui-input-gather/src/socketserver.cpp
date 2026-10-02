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

#include "socketserver.h"

#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

#include <QMutex>
#include <QVector>
#include <QThread>
#include <QDebug>

SocketServer* SocketServer::_self = nullptr;

SocketServer* SocketServer::self(QObject *parent)
{
    static QMutex mutex;
    mutex.lock();
    if(!_self)
        _self = new SocketServer(parent);
    mutex.unlock();
    return _self;
}

SocketServer::SocketServer(QObject *parent)
    :QObject(nullptr)
{

}

SocketServer::~SocketServer()
{

}

void SocketServer::run()
{
    qDebug() << "Starting daemon server...";
    signal(SIGPIPE, processSignal);
    m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);

    if (this->m_socket == -1) {
        qCritical() << "socket create failed:" << strerror(errno);
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCKET_NAME.data(), SOCKET_NAME.size());

    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        qDebug() << "setsockopt failed:" << strerror(errno);
    }

    auto socketAddr = reinterpret_cast<sockaddr *>(&addr); // NOLINT
    if (bind(this->m_socket, socketAddr, sizeof(addr)) == -1) {
        qCritical() << "bind failed:" << strerror(errno);
        close(m_socket);
        m_socket = -1;
        return;
    }

    if (::listen(this->m_socket, 10) == -1) {
        qCritical() << "listen failed:" << strerror(errno);
        close(m_socket);
        m_socket = -1;
        return;
    }

    std::thread connectThread{[ this ]() {
        while (true) {
            struct sockaddr_un clientAddr;
            socklen_t          clientAddrLength = sizeof(clientAddr);
            auto               socketAddrIn = reinterpret_cast<sockaddr *>(&clientAddr); // NOLINT
            int                client       = accept(this->m_socket, socketAddrIn, &clientAddrLength);

            if (client != -1) {
                qDebug() << "New client connected with ID: " << client;
                this->m_clients.push_back(client);
            }
        }
    }};
    connectThread.detach();
}

void SocketServer::send(Event event)
{
    QVector<int> disconnectedClients;
    for(int client : m_clients)
    {
        int written = write(client, &event, sizeof(Event));
        if (written < 0) {
            qDebug() << "Error sending message to daemon with ID " << client << ". daemon disconnected";
            disconnectedClients.push_back(client);
        }
    }

    // Disconnect dead clients
    for (auto client : disconnectedClients) {
        m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
    }
}
