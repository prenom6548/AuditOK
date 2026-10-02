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

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QObject>
#include <QVector>

#include "event.h"

/**
 * Unix domain socket (abstract namespace).
 */
static const std::string SOCKET_NAME{"\0/InputGatherServer", 10}; // NOLINT

class SocketServer : public QObject
{
public:
    static SocketServer* self(QObject *parent = nullptr);
    ~SocketServer();

    static void processSignal(int signo){};
    void run();
    void send(Event event);

private:
    SocketServer(QObject *parent = nullptr);

    static SocketServer *_self;

    int m_socket;
    QVector<int> m_clients;
};

#endif // SOCKETSERVER_H
