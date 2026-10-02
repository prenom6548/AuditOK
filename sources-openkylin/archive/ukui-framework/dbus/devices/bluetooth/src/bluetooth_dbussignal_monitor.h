/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#ifndef __BLUETOOTH_DBUSSIGNAL_MONITOR_H__
#define __BLUETOOTH_DBUSSIGNAL_MONITOR_H__


#include <QObject>
#include <QString>
#include <QMap>

#include "CSingleton.h"

class QDBusPendingCallWatcher;

class DbusSignalMonitor : public QObject
{
    Q_OBJECT
protected:
    explicit DbusSignalMonitor(QObject *parent = nullptr);
    virtual ~DbusSignalMonitor();

protected:
    int init(void);

    int fini(void);

protected slots:
    void adapterAddSignal(QMap<QString, QVariant>);
    void adapterAttrChanged(QString, QMap<QString, QVariant>);
    void adapterRemoveSignal(QString);

private:
    QDBusPendingCallWatcher * __ukuibluetooth_dbus_call(const QString & method, const QList<QVariant> & args);

private:
    QString m_defaultAdapter;

    friend class SingleTon<DbusSignalMonitor>;
};
typedef SingleTon<DbusSignalMonitor>  DBUSSINALMONITOR;

#endif