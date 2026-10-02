/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "pamauthenticate.h"
#include "../../src/common/global_utils.h"
#include "authpamthread.h"
#include <security/pam_misc.h>
#include <unistd.h>
#include <wait.h>
#include <sys/prctl.h>
#include <QSocketNotifier>
#include <QDebug>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QJsonArray>
#include <QJsonParseError>

PamAuthenticate::PamAuthenticate(QSharedPointer<LightDMHelper> helper, QObject *parent)
    : QObject(parent), m_lightdmHelper(helper)
{
    m_fdToParent[0] = -1;
    m_fdToParent[1] = -1;
    m_fdToChild[0] = -1;
    m_fdToChild[1] = -1;

    connect(
        m_lightdmHelper.get(),
        SIGNAL(showMessage(QString, QLightDM::Greeter::MessageType)),
        this,
        SLOT(onLDMShowMessage(QString, QLightDM::Greeter::MessageType)));
    connect(
        m_lightdmHelper.get(),
        SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)),
        this,
        SLOT(onLDMShowPrompt(QString, QLightDM::Greeter::PromptType)));
    connect(m_lightdmHelper.get(), SIGNAL(authenticationComplete()), this, SIGNAL(authenticationComplete()));
}

bool PamAuthenticate::inAuthentication() const
{
    if (m_isOtherUser)
        return m_lightdmHelper->inAuthentication();
    else {
        return m_isInAuthentication;
    }
}

bool PamAuthenticate::isAuthenticated() const
{
    if (m_isOtherUser)
        return m_lightdmHelper->isAuthenticated();
    else {
        return m_isAuthenticated;
    }
}

QString PamAuthenticate::authenticationUser() const
{
    if (m_isOtherUser)
        return m_lightdmHelper->authenticationUser();
    else {
        return m_strUserName;
    }
}

QString PamAuthenticate::autologinUserHint() const
{
    return m_lightdmHelper->autologinUserHint();
}

void PamAuthenticate::authenticate(const QString &username)
{
    m_isOtherUser = (getenv("USER") != username);

    if (username == "*guest") {
        m_lightdmHelper->authenticateAsGuest();
        hasSendBiometricPam = false;
    } else if (m_isOtherUser) {
        m_lightdmHelper->authenticate(username);
        hasSendBiometricPam = false;
    } else {
        cancelAuthentication();
        m_strUserName = username;
        if (pipe(m_fdToParent) || pipe(m_fdToChild))
            qDebug() << "create pipe failed: " << strerror(errno);
        m_threadAuthPam = new AuthPamThread();
        m_threadAuthPam->startAuthPam(m_fdToChild[0], m_fdToParent[1], username);
        m_isInAuthentication = true;
        m_socketNotifier = new QSocketNotifier(m_fdToParent[0], QSocketNotifier::Read);
        connect(m_socketNotifier, &QSocketNotifier::activated, this, &PamAuthenticate::onSockRead);
    }
}

void PamAuthenticate::respond(const QString &response)
{
    if (m_isOtherUser) {
        if (response == BIOMETRIC_IGNORE && !hasSendBiometricPam) {
            return;
        }
        m_lightdmHelper->respond(response);
    } else {
        m_nPrompts--;
        m_responseList.push_back(response);

        if (m_nPrompts == 0) {
            // 发送响应到子进程
            int j = 0;
            PAM_RESPONSE *resp = (PAM_RESPONSE *)calloc(m_messageList.size(), sizeof(PAM_RESPONSE));
            // 响应的数量和消息的数量一致，如果消息类型不是PROMPT，则响应是空的
            for (int i = 0; i < m_messageList.size(); i++) {
                struct pam_message message = m_messageList[i];
                PAM_RESPONSE *r = &resp[i];
                if (message.msg_style == PAM_PROMPT_ECHO_OFF || message.msg_style == PAM_PROMPT_ECHO_ON
                    || (message.msg_style == PAM_BINARY_PROMPT && j < m_responseList.count())) {
                    int respLength = m_responseList[j].length() + 1;
                    r->resp = (char *)malloc(sizeof(char) * respLength);
                    memcpy(r->resp, m_responseList[j].toLocal8Bit().data(), respLength);
                    j++;
                }
            }
            _respond(resp);
            free(resp);
            m_messageList.clear();
            m_responseList.clear();
        } else if (m_nPrompts < 0) {
            // 当m_nPrompts小于0时，需重置为0。否则在下一次收到pam消息时，会马上发送一个空响应，导致密码为空错误提示
            m_nPrompts = 0;
	    m_responseList.clear();
        }
    }
}

void PamAuthenticate::cancelAuthentication()
{
    if (m_isOtherUser)
        m_lightdmHelper->cancelAuthentication();
    else {
        if (m_threadAuthPam) {
            m_messageList.clear();
            m_responseList.clear();
            m_isAuthenticated = false;
            m_isInAuthentication = false;
            m_nPrompts = 0;
            if (m_socketNotifier) {
                disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &PamAuthenticate::onSockRead);
                delete m_socketNotifier;
                m_socketNotifier = nullptr;
            }
            if (m_fdToParent[1] >= 0) {
                close(m_fdToParent[1]);
                m_fdToParent[1] = -1;
            }
            if (m_fdToChild[1] >= 0) {
                close(m_fdToChild[1]);
                m_fdToChild[1] = -1;
            }
            m_threadAuthPam->stopAuthPam();
            delete m_threadAuthPam;
            m_threadAuthPam = nullptr;
            if (m_fdToParent[0] >= 0) {
                close(m_fdToParent[0]);
                m_fdToParent[0] = -1;
            }
            if (m_fdToChild[0] >= 0) {
                close(m_fdToChild[0]);
                m_fdToChild[0] = -1;
            }
        }
    }
}

static void writeData(int fd, const void *buf, ssize_t count)
{
    if (write(fd, buf, count) != count)
        qDebug() << "write to parent failed: " << strerror(errno);
}

static void writeString(int fd, const char *data)
{
    int length = data ? strnlen(data, 1024) : -1;
    writeData(fd, &length, sizeof(length));
    if (data)
        writeData(fd, data, sizeof(char) * length);
}

static int readData(int fd, void *buf, size_t count)
{
    ssize_t nRead = read(fd, buf, count);
    if (nRead < 0)
        qDebug() << "read data failed: " << strerror(errno);
    return nRead;
}

static char *readString(int fd)
{
    int length;

    if (readData(fd, &length, sizeof(length)) <= 0)
        return NULL;
    if (length <= 0)
        length = 0;

    char *value = (char *)malloc(sizeof(char) * (length + 1));
    readData(fd, value, length);
    value[length] = '\0';

    return value;
}

void PamAuthenticate::onSockRead()
{
    //    qDebug() << "has message";
    int msgLength;
    int authComplete = 0;
    readData(m_fdToParent[0], &authComplete, sizeof(authComplete));

    if (authComplete) {
        int authRet = -1;
        if (readData(m_fdToParent[0], (void *)&authRet, sizeof(authRet)) <= 0)
            qDebug() << "get authentication result failed: " << strerror(errno);
        qDebug() << "result: " << authRet;
        m_isAuthenticated = (authRet == PAM_SUCCESS);
        m_isInAuthentication = false;
        if (m_socketNotifier) {
            m_socketNotifier->deleteLater();
            m_socketNotifier = nullptr;
        }
        Q_EMIT authenticationComplete();
    } else {
        readData(m_fdToParent[0], &msgLength, sizeof(msgLength));
        //        qDebug() << "message length: " << msgLength;

        for (int i = 0; i < msgLength; i++) {
            // 读取message
            struct pam_message message;
            pamc_bp_t binary_prompt;
            const char *auth_mode;
            int idata;
            int pic_len;
            const unsigned char *pic_buf;
            const char *str_data;
            int msgType = -1;
            QString binaryString;
            int binaryNum;
            QByteArray binaryArray;
            QPair<int, QVariant> pair;

            readData(m_fdToParent[0], &message.msg_style, sizeof(message.msg_style));
            if (message.msg_style == PAM_BINARY_PROMPT) {
                int length;
                if (readData(m_fdToParent[0], &length, sizeof(length)) <= 0) {
                    continue;
                }
                if (length <= 0)
                    length = 0;
                readData(m_fdToParent[0], &binary_prompt, length);
                switch (binary_prompt->control) {
                    case 0x10: {
                        msgType = BinaryPromptAuthType;
                        auth_mode = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(auth_mode);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    case 0x11: {
                        msgType = BinaryPromptNumber;
                        idata = *(int *)PAM_BP_RDATA(binary_prompt);
                        binaryNum = idata;
                        pair.first = msgType;
                        pair.second = binaryNum;
                        break;
                    }
                    case 0x12: {
                        msgType = BinaryPromptMessage;
                        str_data = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(str_data);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    case 0x13: {
                        msgType = BinaryPromptImage;
                        pic_len = binary_prompt->length - PAM_BP_MIN_SIZE;
                        pic_buf = PAM_BP_RDATA(binary_prompt);
                        binaryArray = QByteArray(reinterpret_cast<const char *>(pic_buf), pic_len);
                        pair.first = msgType;
                        pair.second = (QString)binaryArray.toBase64();
                        break;
                    }
                    case 0x14: {
                        msgType = BinaryPromptQrcodeStr;
                        str_data = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(str_data);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    case 0x15: {
                        msgType = BinaryPromptPromptMessage;
                        str_data = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(str_data);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    case 0x16: {
                        msgType = BinaryPromptErrorMessage;
                        str_data = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(str_data);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    case 0x17: {
                        msgType = BinaryPromptEmergMsg;
                        str_data = (const char *)PAM_BP_RDATA(binary_prompt);
                        binaryString = QString::fromUtf8(str_data);
                        pair.first = msgType;
                        pair.second = binaryString;
                        break;
                    }
                    default: {
                        break;
                    }
                }
            } else {
                message.msg = readString(m_fdToParent[0]);
                qDebug() << message.msg;
                m_messageList.push_back(message);
            }

            switch (message.msg_style) {
                case PAM_PROMPT_ECHO_OFF:
                    m_nPrompts++;
                    Q_EMIT showPrompt(message.msg, PamAuth::PromptTypeSecret);
                    break;
                case PAM_PROMPT_ECHO_ON:
                    m_nPrompts++;
                    Q_EMIT showPrompt(message.msg, PamAuth::PromptTypeQuestion);
                    break;
                case PAM_ERROR_MSG:
                    Q_EMIT showMessage(message.msg, PamAuth::MessageTypeError);
                    break;
                case PAM_TEXT_INFO:
                    Q_EMIT showMessage(message.msg, PamAuth::MessageTypeInfo);
                    break;
                case PAM_BINARY_PROMPT: {
                    m_nPrompts++;
                    m_messageList.push_back(message);
                    Q_EMIT showBinaryPrompt(pair.second, pair.first);
                    break;
                }
            }
        }

        if (m_nPrompts == 0) {
            // 不需要响应，发送一个空的
            PAM_RESPONSE *response = (PAM_RESPONSE *)calloc(m_messageList.size(), sizeof(PAM_RESPONSE));
            _respond(response);
            free(response);
            m_messageList.clear();
        }
    }
}

void PamAuthenticate::_respond(const PAM_RESPONSE *response)
{
    for (int i = 0; i < m_messageList.size(); i++) {
        const PAM_RESPONSE *resp = &response[i];
        writeData(m_fdToChild[1], (const void *)&resp->resp_retcode, sizeof(resp->resp_retcode));
        writeString(m_fdToChild[1], resp->resp);
    }
}

void PamAuthenticate::onLDMShowMessage(QString strMsg, QLightDM::Greeter::MessageType type)
{
    Q_EMIT showMessage(strMsg, type);
}

void PamAuthenticate::onLDMShowPrompt(QString strPrompt, QLightDM::Greeter::PromptType type)
{
    if (strPrompt == BIOMETRIC_PAM) {
        hasSendBiometricPam = true;
    } else {
        hasSendBiometricPam = false;
    }

    if (isJsonString(strPrompt)) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(strPrompt.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            if (jsonObj.contains("type")) {
                int type = jsonObj["type"].toInt();
                switch (type) {
                    case BinaryPromptNumber: {
                        if (jsonObj.contains("binaryInt")) {
                            int number = jsonObj["binaryInt"].toInt();
                            Q_EMIT showBinaryPrompt(number, type);
                            return;
                        }
                        break;
                    }
                    case BinaryPromptAuthType:
                    case BinaryPromptMessage:
                    case BinaryPromptQrcodeStr:
                    case BinaryPromptErrorMessage:
                    case BinaryPromptEmergMsg:
                    case BinaryPromptPromptMessage: {
                        if (jsonObj.contains("binaryStr")) {
                            QString str = jsonObj["binaryStr"].toString();
                            Q_EMIT showBinaryPrompt(str, type);
                            return;
                        }
                        break;
                    }
                    case BinaryPromptImage: {
                        qDebug() << "binaryPrompt image";
                        Q_EMIT showBinaryPrompt("", type);
                        return;
                    }
                    default:
                        break;
                }
            }
        }
    }

    Q_EMIT showPrompt(strPrompt, type);
}

bool PamAuthenticate::isJsonString(const QString &str)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &error);
    return (error.error == QJsonParseError::NoError) && !doc.isNull();
}
