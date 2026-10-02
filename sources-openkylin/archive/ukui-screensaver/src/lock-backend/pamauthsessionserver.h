/*
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef PAMAUTHSESSIONSERVER_H
#define PAMAUTHSESSIONSERVER_H

#include <QObject>
#include <QDBusUnixFileDescriptor>
#include <QJsonObject>

class PamAuthenticate;
class QSocketNotifier;

class PamAuthSessionServer : public QObject
{
public:
    explicit PamAuthSessionServer(PamAuthenticate *pamAuth, QObject *parent = nullptr);
    ~PamAuthSessionServer() override;

    QDBusUnixFileDescriptor startSession(const QString &username);
    void stopSession();

    bool writePacket(int cmdId, const QJsonObject &payload = QJsonObject());

private:
    bool createPipe();
    void onPipeRead();

private:
    PamAuthenticate *m_pamAuth = nullptr;
    int m_backendFd = -1;
    int m_dialogFd = -1;
    QSocketNotifier *m_pipeNotifier = nullptr;
};

#endif // PAMAUTHSESSIONSERVER_H
