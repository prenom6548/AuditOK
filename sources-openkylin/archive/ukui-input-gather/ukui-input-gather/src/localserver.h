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

#ifndef INPUTGATHERSERVER_H
#define INPUTGATHERSERVER_H

#include "event.h"

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QVector>
#include <QMap>
#include <QMutex>

#define INPUT_GATHER_SERVER_NAME "InputGatherServer"

/*!
 * \brief 负责将事件向客户端进行分发
 * \details 基于QLocalServer QLocalSocket实现
 */

class LocalServer : public QObject
{
    Q_OBJECT
public:
    static LocalServer* self(QObject *parent = nullptr);
    ~LocalServer();

    void runServer();
    void send(Event event);
    void send(QByteArray msg);

private slots:
    void onNewConnectionHandler();
    void onSocketClose();
    void onSocketError(QLocalSocket::LocalSocketError);

private:
    LocalServer(QObject *parent = nullptr);
    void init();
    void initConnect();
    QString error2Str(QLocalSocket::LocalSocketError error);

    static LocalServer* _self;
    QLocalServer *m_localServer;
    QVector<QLocalSocket*> m_clientSockets;
    QMap<QMetaObject::Connection, QLocalSocket*> m_connectMapSocket;
};

#endif // INPUTGATHERSERVER_H
