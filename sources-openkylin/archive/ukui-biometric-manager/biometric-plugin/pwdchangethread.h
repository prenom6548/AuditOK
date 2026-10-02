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
#ifndef PWDCHANGETHREAD_H
#define PWDCHANGETHREAD_H

#include <QObject>
#include <QThread>


class PwdChangeThread : public QThread
{
    Q_OBJECT

public:
    explicit PwdChangeThread();
    ~PwdChangeThread();

public:
    void setArgs(QString &currentpwd, QString &pwd);
    void run();

private:
    QString curpwd;
    QString newpwd;

Q_SIGNALS:
    void complete(QString result);
};

#endif // PWDCHANGETHREAD_H
