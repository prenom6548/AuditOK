/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
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

#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include <QObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>

class DbusClient : public QObject
{
    Q_OBJECT
public:
    explicit DbusClient(QObject *parent = 0);
    ~DbusClient();

public slots:
    void initDbusData();
    void getSystemProp(int type, const QString& field);

signals:
    void initFinished();
    void dockerStopped(const QString &name);
    void systemprop(const QString &value);

private:
    QDBusInterface* mSystemBusInterface;
    QDBusInterface* mSessionBusInterface;
    QDBusInterface* mPrefBusInterface;
    QDBusInterface* mSougouInterfaceDbus;
};

#endif // DBUSCLIENT_H
