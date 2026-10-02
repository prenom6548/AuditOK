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
#ifndef ACCOUNTSERVICEHELPER_H
#define ACCOUNTSERVICEHELPER_H

#include <QObject>
#include <QDBusInterface>
#include <QMap>
#include <QDBusObjectPath>

class AccountsHelper : public QObject
{
    Q_OBJECT
public:
    explicit AccountsHelper(QObject *parent = nullptr);

    virtual ~AccountsHelper();

    QString getUserLanguageByName(QString strUserName);

    QString getUserBackgroundByName(QString strUserName);

    QString getUserSessionByName(QString strUserName);

    QString getAccountBackground(uid_t uid);

    QStringList getUserList();

Q_SIGNALS:
    void UserAdded(const QDBusObjectPath &path);
    void UserRemoved(const QDBusObjectPath &path);

private:
    void initData();
    void initConnections();

private:
    QDBusInterface *m_accountsInterface = nullptr;
};

#endif // ACCOUNTSERVICEHELPER_H
