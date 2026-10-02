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
#include <security/pam_misc.h>
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

int is_user_local(const char *username)
{
    FILE *fp;
    fp = fopen("/etc/passwd", "r");
    if (fp == NULL) {
        return 0;
    }
    char buf[1024], name[128];
    while (!feof(fp)) {
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            break;
        }
        sscanf(buf, "%[^:]", name);
        if (strcmp(name, username) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

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
    int length = data ? strnlen(data, 1024) : -1;
    writeData(fd, &length, sizeof(length));
    if (data)
        writeData(fd, data, sizeof(char) * length);
}

int AuthPamThread::readData(int fd, void *buf, size_t count)
{
    ssize_t nRead = 0;
    while (true) {
        // 在每次循环开始时检查停止标志
        if (!m_isAuthenticating) {
            break;
        }
        nRead = read(fd, buf, count);
        if (!m_isAuthenticating) {
            break;
        }
        if (nRead < 0) {
            if (errno == EAGAIN) {
                // 分段睡眠，每隔一段时间检查停止标志
                for (int i = 0; i < 10; i++) {
                    if (!m_isAuthenticating) {
                        break;
                    }
                    QThread::msleep(10); // 每次睡眠10ms，总共最多100ms
                }
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
        *resp = response;
        return PAM_CONV_ERR;
    }

    // 检查是否正在停止认证
    if (!pData->isAuthenticating()) {
        qDebug() << "authentication stopped in conversation callback";
        *resp = response;
        return PAM_CONV_ERR;
    }

    int authComplete = 0;
    pData->writeData(pData->m_fdWrite, (const void *)&authComplete, sizeof(authComplete));
    pData->writeData(pData->m_fdWrite, (const void *)&msgLength, sizeof(msgLength));

    // 再次检查停止标志
    if (!pData->isAuthenticating()) {
        *resp = response;
        return PAM_CONV_ERR;
    }

    // 发送pam消息
    for (int i = 0; i < msgLength; i++) {
        const struct pam_message *m = msg[i];
        // 二进制类型，适配服务器多因子认证
        if (m->msg_style == PAM_BINARY_PROMPT) {
            pamc_bp_t binary_prompt = NULL;

            if (!m->msg) {
                qDebug() << "msg is null!";
            }

            binary_prompt = (pamc_bp_t)m->msg;

            pData->writeData(pData->m_fdWrite, (const void *)&m->msg_style, sizeof(m->msg_style));
            int length = sizeof(binary_prompt);
            pData->writeData(pData->m_fdWrite, (const void *)&length, sizeof(length));
            pData->writeData(pData->m_fdWrite, (const void *)&binary_prompt, length);
            continue;
        }

        pData->writeData(pData->m_fdWrite, (const void *)&m->msg_style, sizeof(m->msg_style));
        pData->writeString(pData->m_fdWrite, m->msg);
        // 在每次发送后检查停止标志
        if (!pData->isAuthenticating()) {
            *resp = response;
            return PAM_CONV_ERR;
        }
    }
    // 读取响应
    for (int i = 0; i < msgLength; i++) {
        // 检查停止标志
        if (!pData->isAuthenticating()) {
            *resp = response;
            return PAM_CONV_ERR;
        }
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
        return;
    }

    authRet = pam_authenticate(pamh, 0);

    if (authRet == PAM_SUCCESS) {
        /*检测账户有效性，即使密码认证通过，如果账户锁定或无效，也无法解锁*/
        int ret = pam_acct_mgmt(pamh, 0);

        /*如果密码过期，pam_acct_mgmt会返回请求修改口令，由于锁屏不支持修改
         * 密码，会导致无法解锁，因此这里要判断返回值是否为修改口令*/
        if (ret != PAM_NEW_AUTHTOK_REQD)
            authRet = ret;

        /*对于域用户，因为密码不是保持在shadow文件里，所以可以修改密码,因此
         * 这里发起修改密码的请求*/
        if (userName && !is_user_local(userName) && ret == PAM_NEW_AUTHTOK_REQD) {
            authRet = pam_chauthtok(pamh, PAM_CHANGE_EXPIRED_AUTHTOK);
        }
    }

    ret = pam_get_item(pamh, PAM_USER, (const void **)&newUser);
    if (ret != PAM_SUCCESS) {
        pam_end(pamh, 0);
        qDebug() << "failed to get username";
        return;
    }

    if (authRet != PAM_SUCCESS) {
        qDebug() << "failed to acct mgmt " << pam_strerror(NULL, authRet);
    }

    if (m_isAuthenticating) {
        // 发送认证结果
        int authComplete = 1;
        writeData(m_fdWrite, (const void *)&authComplete, sizeof(authComplete));
        writeData(m_fdWrite, (const void *)&authRet, sizeof(authRet));
    }
    pam_end(pamh, 0);
    qDebug() << "authentication result: " << authRet;
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

bool AuthPamThread::stopAuthPam()
{
    qDebug() << "stopAuthPam begin!";
    m_isAuthenticating = false;
    if (!isRunning()) {
        qDebug() << "stopAuthPam end (thread not running)";
        return true;
    }

    // 请求线程退出
    quit();

    // 使用超时等待，避免无限阻塞
    // 如果pam_authenticate阻塞在PAM模块内部（如网络认证），线程无法响应quit信号
    // 这里设置1秒超时，超时后强制终止线程
    bool finished = wait(1000);

    if (!finished) {
        qInfo() << "Thread did not finish within timeout, force terminating";
        // 强制终止线程（这是最后手段，因为无法中断阻塞的PAM调用）
        terminate();
        wait(500);
    }

    if (isRunning()) {
        qWarning() << "auth thread is still running!!";
        return false;
    }
    qDebug() << "stopAuthPam end";
    return true;
}
