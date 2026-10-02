/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inputmethodchannel.h"
#include "utils.h"
#include "config/kmreenv.h"

#include <QByteArray>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

InputmethodChannel::InputmethodChannel(QObject *parent) :
    QObject(parent)
    , mSocketFd(-1)
    , mConnectReady(false)
{

}

InputmethodChannel::~InputmethodChannel()
{
    cleanSocket();
}

void InputmethodChannel::onInitSocket()
{
    //连接android端的输入法服务端 unix domain socket: kmre_ime
    struct sockaddr_un un_addr;
    memset(&un_addr, 0, sizeof(un_addr));

    std::string android_socket_path = KmreEnv::GetContainerSocketPath().toStdString() + "/kmre_ime";
    if (sizeof(android_socket_path.c_str()) > sizeof(un_addr.sun_path)) {
        syslog(LOG_ERR, "[%s] Invalid socket path! Init inputmethod socket failed!", __func__);
        return;
    }

    if ((mSocketFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0) {
        syslog(LOG_ERR, "[%s] Create inputmethod socket failed!", __func__);
        return;
    }

    un_addr.sun_family = AF_UNIX;
    strncpy(un_addr.sun_path, android_socket_path.c_str(), sizeof(un_addr.sun_path) -1);

    if (::connect(mSocketFd, reinterpret_cast<struct sockaddr*> (&un_addr), sizeof(struct sockaddr_un)) < 0) {
        syslog(LOG_WARNING, "[%s] Connect inputmethod socket failed!", __func__);
        cleanSocket();
        return;
    }

    mConnectReady = true;
}

void InputmethodChannel::cleanSocket()
{
    if (mSocketFd > 0) {
        ::close(mSocketFd);
        mSocketFd = -1;
    }
    mConnectReady = false;
}

void InputmethodChannel::onSend(QString text)
{
    if (!mConnectReady) {
        onInitSocket();
    }

    if (mConnectReady) {
        QByteArray ba = text.toUtf8();
        if (write_fully(ba.data(), ba.length() + 1) != 0) {
            syslog(LOG_ERR, "[%s] Send inputmethod text failed!", __func__);
        }
    }
}

int InputmethodChannel::write_fully(const void *buffer, size_t size)
{
    int retval = 0;

    if (mSocketFd > 0) {
        size_t res = size;

        while (res > 0) {
            ssize_t stat = ::send(mSocketFd, (const char *)buffer + (size - res), res, MSG_NOSIGNAL);
            if (stat < 0) {
                if (errno != EINTR) {
                    retval =  stat;
                    break;
                }
            } else {
                res -= stat;
            }
        }

        if (retval < 0) {
            this->cleanSocket();
        }
    }
    else {
        retval = -1;
    }

    return retval;
}
