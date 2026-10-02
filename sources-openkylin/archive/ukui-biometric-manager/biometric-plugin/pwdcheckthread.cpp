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
#include "pwdcheckthread.h"

#include <QDebug>

PwdCheckThread::PwdCheckThread()
{
}

PwdCheckThread::~PwdCheckThread()
{
}

void PwdCheckThread::setArgs(const QString &userName, const QString &userPwd){
    uname = userName;
    upwd = userPwd;
}

void PwdCheckThread::run(){

    FILE * stream;
    char command[128];
    char output[256];

    QString result;;

    QByteArray ba1 = uname.toLatin1();

    //
    if (upwd.contains("'")){
        snprintf(command, 128, "/usr/bin/checkUserPwd %s \"%s\"", ba1.data(), upwd.toLatin1().data());
    } else {

        snprintf(command, 128, "/usr/bin/checkUserPwd %s '%s'", ba1.data(), upwd.toLatin1().data());
    }

    if ((stream = popen(command, "r")) != NULL){

        while(fgets(output, 256, stream) != NULL){
            result = QString(output).simplified();
//            if (QString::compare(QString(output).simplified(), "Succes!") == 0){
//                result = true;
//            }

        }
        pclose(stream);
    }

    emit complete(result);

}
