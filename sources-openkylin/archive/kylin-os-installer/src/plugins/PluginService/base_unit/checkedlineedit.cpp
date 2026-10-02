/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "checkedlineedit.h"
#include <QRegExp>
#include "../PluginService/sysInfo/kpasswordcheck.h"

namespace KInstaller{

using namespace KServer;

CheckState KCheckUserName(const QString &username)
{
    if(username.isEmpty()) {
        return CheckState::EmptyError;
    } else if(username < 1) {
        return CheckState::TooShortError;
    } else if(username.length() > 32) {
        return CheckState::TooLongError;
    }
    QString pattern = "[a-z][a-zA-Z0-9_-]*";
    const QRegExp reg(pattern);
    if(!reg.exactMatch(username)) {
        return CheckState::InvalidCharError;
    }
    return CheckState::OK;
}

CheckState KCheckHostName(const QString &hostname)
{
    if(hostname.isEmpty()) {
        return CheckState::EmptyError;
    } else if(hostname.length() < 1) {
        return CheckState::TooShortError;
    } else if(hostname.length() > 64) {
        return CheckState::TooLongError;
    }

    const QRegExp reg("[a-zA-Z0-9_-]*");
    if(!reg.exactMatch(hostname)) {
        return CheckState::InvalidCharError;
    }
    return CheckState::OK;
}

CheckState KCheckPassWD(const QString &passwd, QString& strerror, QString user)
{
    if(passwd.isEmpty()) {
        return CheckState::EmptyError;
    }
//    const QRegExp reg("[\"\\ ]");
//    if(reg.exactMatch(passwd)) {
//        return CheckState::InvalidCharError;
//    }
    QByteArray bpwd = passwd.toLatin1();
    if(bpwd.isEmpty()) {
        return CheckState::EmptyError;
    }
    int ret = KServer::KPasswordCheck::getInstance()->passwdCheck(passwd, strerror, user);
    if(ret < 0) {
        return CheckState::TooWeakError;
    }

    return CheckState::OK;
}

}
