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
#include "pamauthsessionclient.h"

#include "../../src/common/definetypes.h"
#include <QSocketNotifier>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace {
int readData(int fd, void *buf, size_t count)
{
    ssize_t nRead = read(fd, buf, count);
    if (nRead < 0) {
        qWarning() << "read pam pipe failed:" << strerror(errno);
    }
    return nRead;
}
}

PamAuthSessionClient::PamAuthSessionClient(QObject *parent) : QObject(parent) {}

PamAuthSessionClient::~PamAuthSessionClient()
{
    stop();
}

bool PamAuthSessionClient::start(int fd)
{
    stop();
    if (fd < 0) {
        qWarning() << "[PamAuthSessionClient] start failed, invalid fd:" << fd;
        return false;
    }
    m_fd = fd;
    m_inAuthentication = true;
    qInfo() << "[PamAuthSessionClient] start with fd:" << m_fd;
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this]() { onPipeRead(); });
    return true;
}

void PamAuthSessionClient::stop()
{
    qInfo() << "[PamAuthSessionClient] stop begin, fd:" << m_fd;
    if (m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
    m_inAuthentication = false;
    qInfo() << "[PamAuthSessionClient] stop end";
}

bool PamAuthSessionClient::inAuthentication() const
{
    return m_inAuthentication;
}

bool PamAuthSessionClient::isActive() const
{
    return m_fd >= 0;
}

bool PamAuthSessionClient::sendPacket(int cmdId, const QString &content)
{
    if (m_fd < 0) {
        qWarning() << "[PamAuthSessionClient] sendPacket failed, inactive fd, cmdId:" << cmdId;
        return false;
    }
    QJsonObject obj;
    obj["CmdId"] = cmdId;
    if (!content.isNull()) {
        obj["Content"] = content;
    }
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    int len = data.size();
    qDebug() << "[PamAuthSessionClient] sendPacket cmdId:" << cmdId << "len:" << len;
    if (write(m_fd, &len, sizeof(len)) != sizeof(len)) {
        qWarning() << "[PamAuthSessionClient] sendPacket write length failed, cmdId:" << cmdId;
        return false;
    }
    if (len > 0 && write(m_fd, data.constData(), len) != len) {
        qWarning() << "[PamAuthSessionClient] sendPacket write payload failed, cmdId:" << cmdId;
        return false;
    }
    return true;
}

void PamAuthSessionClient::onPipeRead()
{
    int payloadLen = 0;
    if (readData(m_fd, &payloadLen, sizeof(payloadLen)) <= 0 || payloadLen <= 0) {
        qWarning() << "[PamAuthSessionClient] onPipeRead invalid payload length:" << payloadLen;
        return;
    }
    qDebug() << "[PamAuthSessionClient] onPipeRead payload length:" << payloadLen;
    QByteArray payload(payloadLen, '\0');
    if (readData(m_fd, payload.data(), payloadLen) <= 0) {
        qWarning() << "[PamAuthSessionClient] onPipeRead read payload failed";
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[PamAuthSessionClient] onPipeRead parse json failed:" << parseErr.errorString();
        return;
    }
    QJsonObject obj = doc.object();
    int cmdId = obj.value("CmdId").toInt(-1);
    qDebug() << "[PamAuthSessionClient] onPipeRead cmdId:" << cmdId;
    switch (cmdId) {
        case LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE:
            if (onShowMessage) {
                onShowMessage(obj.value("Message").toString(), obj.value("MsgType").toInt());
            }
            break;
        case LOCK_CMD_ID_PAMAUTH_SHOWPROMPT:
            if (onShowPrompt) {
                onShowPrompt(obj.value("Prompt").toString(), obj.value("PromptType").toInt());
            }
            break;
        case LOCK_CMD_ID_PAMAUTH_SHOWPBINARYROMPT:
            if (onShowBinaryPrompt) {
                onShowBinaryPrompt(obj.value("BinaryVal").toVariant(), obj.value("BinaryPromptType").toInt());
            }
            break;
        case LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED:
            m_inAuthentication = false;
            if (onAuthCompleted) {
                onAuthCompleted();
            }
            break;
        default:
            qWarning() << "[PamAuthSessionClient] unknown cmdId:" << cmdId;
            break;
    }
}
