/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RFKILLSTATE_H
#define RFKILLSTATE_H

#include <QObject>
#include "QGSettings/qgsettings.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
class RfkillState : public QObject
{
    Q_OBJECT
public:
    explicit RfkillState(QObject *parent = nullptr);
    ~RfkillState();
    static RfkillState* self();

    bool isSupportWlan();
    bool getWlanState();
    bool setWlanState(bool state);

    int getFlightState() const;
    void setFlightState(bool state);

    int getBluetooth();
    void setBluetooth(bool state);

private:
    void initialization();
    //全局配置文件读写rfkill
    QVariant getGlobalRfkillState();
    void setGlobalRfkillState(int state);
    void setSettingsState(int state);
    int getSettingsState() const;

Q_SIGNALS:
public Q_SLOTS:
    void doSettingsChangeAction(const QString& key);
    void onUserActive(bool state);
private:
    QGSettings* m_rfkillSettings = nullptr;
};

#endif // RFKILLSTATE_H
