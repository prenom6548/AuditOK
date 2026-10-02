/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef AUTHPAMTHREAD_H
#define AUTHPAMTHREAD_H

#include <QThread>

class AuthPamThread : public QThread
{
    Q_OBJECT
public:
    AuthPamThread(QObject *parent = nullptr);
    virtual ~AuthPamThread();
    void startAuthPam(int fdRead, int fdWrite, QString strUserName);
    bool stopAuthPam();

    void writeData(int fd, const void *buf, ssize_t count);
    void writeString(int fd, const char *data);
    int readData(int fd, void *buf, size_t count);
    char *readString(int fd);
    bool isAuthenticating() const { return m_isAuthenticating; }

protected:
    void run();

private:
    void _authenticate(const char *userName);

public:
    int m_fdRead = -1;
    int m_fdWrite = -1;
    QString m_strUserName = "";

private:
    bool m_isAuthenticating = false;
};

#endif // AUTHPAMTHREAD_H
