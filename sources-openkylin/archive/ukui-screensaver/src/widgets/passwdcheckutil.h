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
#ifndef PASSWDCHECKUTIL_H
#define PASSWDCHECKUTIL_H

#include <QObject>

#define PWCHK_ERROR_SUCCESS              0
#define PWCHK_ERROR_MIN_LENGTH          -1
#define PWCHK_ERROR_MIN_CLASSES         -2
#define PWCHK_ERROR_SAME_PASSWORD       -3
#define PWCHK_ERROR_USER_CHECK          -4
#define PWCHK_ERROR_CASE_CHANGES_ONLY   -5
#define PWCHK_ERROR_TOO_SIMILAR         -6
#define PWCHK_ERROR_PALINDROME          -7

#define PWCHK_DEFAULT_MIN_LENGTH        8
#define PWCHK_DEFAULT_MIN_CLASS         2

class PasswdCheckUtil : public QObject
{
    Q_OBJECT
public:
    explicit PasswdCheckUtil(QObject *parent = nullptr);

    static bool getCurrentPamState();

    static int checkPassword(QString newPw, QString oldPw, QString userName);

    static QString checkPasswordStrerr(int nResult);
};

#endif // PASSWDCHECKUTIL_H
