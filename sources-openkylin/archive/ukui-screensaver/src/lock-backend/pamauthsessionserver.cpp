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
#include "pamauthsessionserver.h"

#include "pamauthenticate.h"
#include "../../src/common/definetypes.h"
#include <QSocketNotifier>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <QDebug>

PamAuthSessionServer::PamAuthSessionServer(PamAuthenticate *pamAuth, QObject *parent)
    : QObject(parent), m_pamAuth(pamAuth)
{
}

PamAuthSessionServer::~PamAuthSessionServer()
{
    stopSession();
}

bool PamAuthSessionServer::createPipe()
{
    int pipeFds[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipeFds) != 0) {
        qWarning() << "PamAuthSessionServer createPipe failed:" << strerror(errno);
        return false;
    }
    m_backendFd = pipeFds[0];
    m_dialogFd = pipeFds[1];
    m_pipeNotifier = new QSocketNotifier(m_backendFd, QSocketNotifier::Read, this);
    connect(m_pipeNotifier, &QSocketNotifier::activated, this, [this]() { onPipeRead(); });
    return true;
}

void PamAuthSessionServer::stopSession()
{
    if (m_pipeNotifier) {
        delete m_pipeNotifier;
        m_pipeNotifier = nullptr;
    }
    if (m_backendFd >= 0) {
        close(m_backendFd);
        m_backendFd = -1;
    }
    if (m_dialogFd >= 0) {
        close(m_dialogFd);
        m_dialogFd = -1;
    }
}

QDBusUnixFileDescriptor PamAuthSessionServer::startSession(const QString &username)
{
    stopSession();
    if (m_pamAuth) {
        m_pamAuth->cancelAuthentication();
    }
    if (!createPipe()) {
        return QDBusUnixFileDescriptor();
    }
    if (m_pamAuth) {
        m_pamAuth->authenticate(username);
    }
    QDBusUnixFileDescriptor fd(m_dialogFd);
    close(m_dialogFd);
    m_dialogFd = -1;
    return fd;
}

bool PamAuthSessionServer::writePacket(int cmdId, const QJsonObject &payload)
{
    if (m_backendFd < 0) {
        return false;
    }
    QJsonObject obj(payload);
    obj["CmdId"] = cmdId;
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    int len = data.size();
    if (write(m_backendFd, &len, sizeof(len)) != sizeof(len)) {
        qWarning() << "writePacket failed:" << cmdId;
        return false;
    }
    if (len > 0 && write(m_backendFd, data.constData(), len) != len) {
        qWarning() << "writePacket failed:" << cmdId << len;
        return false;
    }
    return true;
}

void PamAuthSessionServer::onPipeRead()
{
    int len = 0;
    if (read(m_backendFd, &len, sizeof(len)) != sizeof(len) || len <= 0) {
        return;
    }
    QByteArray data(len, '\0');
    if (read(m_backendFd, data.data(), len) != len) {
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }
    QJsonObject obj = doc.object();
    int cmdId = obj.value("CmdId").toInt(-1);
    if (cmdId == LOCK_CMD_ID_PAMAUTH_RESPOND && m_pamAuth) {
        m_pamAuth->respond(obj.value("Content").toString());
    } else if (cmdId == LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL && m_pamAuth) {
        m_pamAuth->cancelAuthentication();
    }
}
