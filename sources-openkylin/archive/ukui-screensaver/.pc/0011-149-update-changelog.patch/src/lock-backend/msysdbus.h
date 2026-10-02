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
#ifndef MSYSDBUS_H
#define MSYSDBUS_H

#include <QDBusConnection>
#include <QObject>
#include <QByteArray>
#include "rsac.h"
class DbusUpperInterface;

class MSysDbus : public QObject
{
    Q_OBJECT
public:
    explicit MSysDbus(DbusUpperInterface *interface, QObject *parent = nullptr);

public Q_SLOTS:
    Q_SCRIPTABLE bool sendPassword(const QString username, QByteArray password);
    Q_SCRIPTABLE QString getPublicEncrypt();
Q_SIGNALS:
    void closed();

private:
    void registerUniauth();
    void sendSignalLoginFinished(QString username, bool res);

    DbusUpperInterface *m_interface = nullptr;
};

#endif
