/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "passwdcheckutil.h"

#include <QFile>
#include <QDebug>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#define PAM_CONF_FILE "/etc/pam.d/common-password"

PasswdCheckUtil::PasswdCheckUtil(QObject *parent) : QObject(parent)
{

}

bool PasswdCheckUtil::getCurrentPamState(){
    QFile * readFile = new QFile(PAM_CONF_FILE);
    if (!readFile->open(QIODevice::ReadOnly | QIODevice::Text)){
        readFile->close();
        qDebug() << QString("Open conf file %1 failed!").arg(PAM_CONF_FILE);
        return false;
    } else {
        QTextStream stream(readFile);
        while(!stream.atEnd()){
            QString line = stream.readLine();
            if (line.contains("pam_pwquality.so"))
                return true;
        }
        return false;
    }
}

static int palindrome(const char *newPw)
{
    int i = 0, j = 0;
    i = strlen(newPw);

    /* NOT consider one character as palindrome */
    if (i == 1)
        return 0;

    for (j = 0; j < i; j++) {
        if (newPw[i - j - 1] != newPw[j]) {
            return 0;
        }
    }

    return 1;
}

int PasswdCheckUtil::checkPassword(QString newPw, QString oldPw, QString userName)
{
    int digits = 0, digitExist = 0;
    int uppers = 0, upperExist = 0;
    int lowers = 0, lowerExist = 0;
    int others = 0, otherExist = 0;
    int totalClass = 0;
    int size;
    int i;
    enum { NONE, DIGIT, UCASE, LCASE, OTHER } prevclass = NONE;
    int sameclass = 0;
    QByteArray ba = newPw.toLatin1();
    const char* newPwd = ba.data();

    if (!newPw.isEmpty() && !oldPw.isEmpty() && newPw == oldPw) {
        return PWCHK_ERROR_SAME_PASSWORD;
    }

    for (i = 0; newPwd[i]; i++) {
        if (isdigit(newPwd[i])) {
            digits++;
            digitExist = 1;
            if (prevclass != DIGIT) {
                prevclass = DIGIT;
                sameclass = 1;
            } else
                sameclass++;
        }
        else if (isupper(newPwd[i])) {
            uppers++;
            upperExist = 1;
            if (prevclass != UCASE) {
                prevclass = UCASE;
                sameclass = 1;
            } else
                sameclass++;
        }
        else if (islower(newPwd[i])) {
            lowers++;
            lowerExist = 1;
            if (prevclass != LCASE) {
                prevclass = LCASE;
                sameclass = 1;
            } else
                sameclass++;
        }
        else {
            others++;
            otherExist = 1;
            if (prevclass != OTHER) {
                prevclass = OTHER;
                sameclass = 1;
            } else
                sameclass++;
        }
    }

    size = PWCHK_DEFAULT_MIN_LENGTH;

    if (size > i)
        return PWCHK_ERROR_MIN_LENGTH;

    totalClass = digitExist + upperExist + lowerExist + otherExist;

    if (totalClass < PWCHK_DEFAULT_MIN_CLASS)
        return PWCHK_ERROR_MIN_CLASSES;

    // 回文
    if (palindrome(newPwd)) {
        return PWCHK_ERROR_PALINDROME;
    }

    if (!newPw.isEmpty() && !oldPw.isEmpty()) {
        if (newPw.compare(oldPw, Qt::CaseInsensitive) == 0) {
            return PWCHK_ERROR_CASE_CHANGES_ONLY;
        } else if (oldPw.toLower().contains(newPw.toLower())) {
            return PWCHK_ERROR_TOO_SIMILAR;
        }
    }

    return PWCHK_ERROR_SUCCESS;
}

QString PasswdCheckUtil::checkPasswordStrerr(int nResult)
{
    QString strResult = "";
    switch (nResult){
    case PWCHK_ERROR_MIN_LENGTH:
        strResult = QString(tr("The password is shorter than %1 characters")).arg(PWCHK_DEFAULT_MIN_LENGTH);
        break;
    case PWCHK_ERROR_MIN_CLASSES:
        strResult = QString(tr("The password contains less than %1 character classes")).arg(PWCHK_DEFAULT_MIN_CLASS);
        break;
    case PWCHK_ERROR_SAME_PASSWORD:
        strResult = tr("The password is the same as the old one");
        break;
    case PWCHK_ERROR_USER_CHECK:
        strResult = tr("The password contains the user name in some form");
        break;
    case PWCHK_ERROR_CASE_CHANGES_ONLY:
        strResult = tr("The password differs with case changes only");
        break;
    case PWCHK_ERROR_TOO_SIMILAR:
        strResult = tr("The password is too similar to the old one");
        break;
    case PWCHK_ERROR_PALINDROME:
        strResult = tr("The password is a palindrome");
        break;
    default:
        break;
    }
    return strResult;
}
