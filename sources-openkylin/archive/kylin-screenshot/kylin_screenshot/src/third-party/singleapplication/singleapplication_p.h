/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#ifndef SINGLEAPPLICATION_P_H
#define SINGLEAPPLICATION_P_H

#include <QtCore/QSharedMemory>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include "singleapplication.h"

struct InstancesInfo {
    bool primary;
    quint32 secondary;
    qint64 primaryPid;
};

class SingleApplicationPrivate : public QObject {
Q_OBJECT
public:
    enum ConnectionType : quint8 {
        InvalidConnection = 0,
        NewInstance = 1,
        SecondaryInstance = 2,
        Reconnect = 3
    };
    Q_DECLARE_PUBLIC(SingleApplication)

    SingleApplicationPrivate( SingleApplication *q_ptr );
     ~SingleApplicationPrivate();

    void genBlockServerName( int msecs );
    void startPrimary( bool resetMemory );
    void startSecondary();
    void connectToPrimary(int msecs, ConnectionType connectionType );
    qint64 primaryPid();

#ifdef Q_OS_UNIX
    void crashHandler();
    static void terminate( int signum );
#endif

    QSharedMemory *memory;
    SingleApplication *q_ptr;
    QLocalSocket *socket;
    QLocalServer *server;
    quint32 instanceNumber;
    QString blockServerName;
    SingleApplication::Options options;

public Q_SLOTS:
    void slotConnectionEstablished();
    void slotDataAvailable( QLocalSocket*, quint32 );
    void slotClientConnectionClosed( QLocalSocket*, quint32 );
};

#endif // SINGLEAPPLICATION_P_H
