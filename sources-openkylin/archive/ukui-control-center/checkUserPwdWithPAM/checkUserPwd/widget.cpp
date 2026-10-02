/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "widget.h"

#include "auth-pam.h"

#include <QDebug>


Widget::Widget()
{

    auth = new AuthPAM(this);

    accountlock = false;

    connect(auth, &Auth::showMessage, this, &Widget::onShowMessage);
    connect(auth, &Auth::showPrompt, this, &Widget::onShowPrompt);
    connect(auth, &Auth::authenticateComplete, this, &Widget::onAuthComplete);

}

Widget::~Widget()
{

    auth->stopAuth();

    delete auth;
}

void Widget::pwdCheck(QString userName, QString userPwd){
    auth->authenticate(userName, userPwd);
}

void Widget::onShowMessage(const QString &message, Auth::MessageType type)
{
//    qDebug() << "showMessage" << message;
    accountlock = true;
    showMsg = message;
//    printf("%s\n", message.toUtf8().data());
}

void Widget::onShowPrompt(const QString &prompt, Auth::PromptType type)
{
//    qDebug() << "prompt: " << prompt;
}

void Widget::onAuthComplete()
{
    if(!auth->isAuthenticated()) {
        if (showMsg == "") {
            printf("Failed!\n");
        } else {
            printf("%s\n", showMsg.toUtf8().data());
        }
    }

    exit(0);

}

