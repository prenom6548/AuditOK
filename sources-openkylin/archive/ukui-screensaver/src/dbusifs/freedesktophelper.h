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
#ifndef FREEDESKTOPHELPER_H
#define FREEDESKTOPHELPER_H

#include <QtDBus>
#include <QDBusAbstractInterface>
#include <QDBusUnixFileDescriptor>
#include <mutex>
#include "../common/definetypes.h"

class FreedesktopHelper : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static FreedesktopHelper* getInstance(bool bSession = true, QObject *parent = nullptr);
    void asyncStartService(const QString &serviceName);

public Q_SLOTS:
    bool NameHasOwner(const QString &strService);
    bool isServiceActivable(const QString &strService);
    void onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

Q_SIGNALS:
    void serviceStatusChanged(QString strName, bool isActive);
    void NameOwnerChanged(QString, QString, QString);

private:
    explicit FreedesktopHelper(bool bSession = true, QObject *parent = nullptr);

    static std::unique_ptr<FreedesktopHelper> m_instanceSystem;
    static std::unique_ptr<FreedesktopHelper> m_instanceSession;
    static std::once_flag m_initFlagSystem;
    static std::once_flag m_initFlagSession;
};

#endif // FREEDESKTOPHELPER_H
