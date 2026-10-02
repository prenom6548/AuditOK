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
#include "authpamthread.h"

#include <security/pam_appl.h>
#include <unistd.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <QDebug>

#define PAM_SERVICE_NAME "ukui-screensaver-qt"

static void writeData(int fd, const void *buf, ssize_t count);
static void writeString(int fd, const char *data);
static int readData(int fd, void *buf, size_t count);
static char *readString(int fd);
static int pam_conversation(int msgLength, const struct pam_message **msg, struct pam_response **resp, void *appData);

AuthPamThread::AuthPamThread(QObject *parent) : QThread(parent) {}

AuthPamThread::~AuthPamThread() {}

void AuthPamThread::writeData(int fd, const void *buf, ssize_t count)
{
    if (!m_isAuthenticating) {
        return;
    }
    if (write(fd, buf, count) != count)
        qDebug() << "write to parent failed: " << strerror(errno);
}

void AuthPamThread::writeString(int fd, const char *data)
{
    int length = data ? strlen(data) : -1;
    writeData(fd, &length, sizeof(length));
    if (data)
        writeData(fd, data, sizeof(char) * length);
}

int AuthPamThread::readData(int fd, void *buf, size_t count)
{
    ssize_t nRead = 0;
    while (true) {
        nRead = read(fd, buf, count);
        if (!m_isAuthenticating) {
            break;
        }
        if (nRead < 0) {
            if (errno == EAGAIN) {
                usleep(100 * 1000);
                continue;
            } else {
                qDebug() << "read data failed: " << strerror(errno) << errno;
            }
        }
        break;
    }
    return nRead;
}

char *AuthPamThread::readString(int fd)
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

static int pam_conversation(int msgLength, const struct pam_message **msg, struct pam_response **resp, void *appData)
{
    struct pam_response *response = (struct pam_response *)calloc(msgLength, sizeof(struct pam_response));
    AuthPamThread *pData = (AuthPamThread *)appData;
    if (!pData || pData->m_fdRead < 0 || pData->m_fdWrite < 0) {
        return PAM_CONV_ERR;
    }

    int authComplete = 0;
    pData->writeData(pData->m_fdWrite, (const void *)&authComplete, sizeof(authComplete));
    pData->writeData(pData->m_fdWrite, (const void *)&msgLength, sizeof(msgLength));
    //发送pam消息
    for (int i = 0; i < msgLength; i++) {
        const struct pam_message *m = msg[i];
        pData->writeData(pData->m_fdWrite, (const void *)&m->msg_style, sizeof(m->msg_style));
        pData->writeString(pData->m_fdWrite, m->msg);
    }
    //读取响应
    for (int i = 0; i < msgLength; i++) {
        struct pam_response *r = &response[i];
        if (pData->readData(pData->m_fdRead, &r->resp_retcode, sizeof(r->resp_retcode)) < 0) {
            break;
        }
        r->resp = pData->readString(pData->m_fdRead);
    }
    *resp = response;
    return PAM_SUCCESS;
}

void AuthPamThread::_authenticate(const char *userName)
{
    qDebug() << "authenticate " << userName;

    pam_handle_t *pamh = NULL;
    char *newUser = NULL;
    int ret;
    int authRet;
    struct pam_conv conv;

    conv.conv = pam_conversation;
    conv.appdata_ptr = this;

    ret = pam_start(PAM_SERVICE_NAME, userName, &conv, &pamh);
    if (ret != PAM_SUCCESS) {
        qDebug() << "failed to start PAM: " << pam_strerror(NULL, ret);
    }

    authRet = pam_authenticate(pamh, 0);

    ret = pam_get_item(pamh, PAM_USER, (const void **)&newUser);
    if (ret != PAM_SUCCESS) {
        pam_end(pamh, 0);
        qDebug() << "failed to get username";
    }

    if (authRet == PAM_SUCCESS) {
        /*检测账户有效性，即使密码认证通过，如果账户锁定或无效，也无法解锁*/
        int ret = pam_acct_mgmt(pamh, 0);

        /*如果密码过期，pam_acct_mgmt会返回请求修改口令，由于锁屏不支持修改
         * 密码，会导致无法解锁，因此这里要判断返回值是否为修改口令*/
        if (ret != PAM_NEW_AUTHTOK_REQD)
            authRet = ret;
    }

    if (authRet != PAM_SUCCESS) {
        qDebug() << "failed to acct mgmt " << pam_strerror(NULL, authRet);
    }

    if (newUser) {
        free(newUser);
        newUser = NULL;
    }
    fprintf(stderr, "authentication result: %d\n", authRet);

    // 发送认证结果
    int authComplete = 1;
    writeData(m_fdWrite, (const void *)&authComplete, sizeof(authComplete));
    writeData(m_fdWrite, (const void *)&authRet, sizeof(authRet));
    qDebug() << "--- 认证完成";
}

void AuthPamThread::startAuthPam(int fdRead, int fdWrite, QString strUserName)
{
    if (!isRunning()) {
        qDebug() << "startAuthPam ----";
        m_isAuthenticating = true;
        int nFlags = fcntl(fdRead, F_GETFL);
        nFlags = nFlags | O_NONBLOCK;
        fcntl(fdRead, F_SETFL, nFlags);
        m_fdRead = fdRead;
        m_fdWrite = fdWrite;
        m_strUserName = strUserName;
        start();
    } else {
        qDebug() << "AuthPamThread is running!!";
    }
}

void AuthPamThread::run()
{
    if (m_fdRead >= 0 && m_fdWrite >= 0 && !m_strUserName.isEmpty()) {
        _authenticate(m_strUserName.toLocal8Bit().data());
    } else {
        qDebug() << "AuthPamThread param error:" << m_fdRead << m_fdWrite << m_strUserName;
    }
}

void AuthPamThread::stopAuthPam()
{
    qDebug() << "stopAuthPam begin!";
    m_isAuthenticating = false;
    if (isRunning()) {
        quit();
        wait();
    }
    qDebug() << "stopAuthPam end";
}
