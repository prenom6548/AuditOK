/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DEBPACKAGE_H
#define DEBPACKAGE_H
#include "Package.h"
#include <QStorageInfo>
#include <QDBusReply>
#include <QDBusInterface>

#define KYLIN_OS_DBUS_SERVICE_NAME      "com.kylin.systemupgrade"
#define KYLIN_OS_DBUS_PATH              "/com/kylin/systemupgrade"
#define KYLIN_OS_DBUS_INTERFACE_NAME    "com.kylin.systemupgrade.interface"

class DebPackage : public Package
{
public:
    DebPackage(QString debPath);
private:
    QString getDebInfoFromDeb(QString debPath, QString type);
    QString getAppCnName(QString appName);
private:
    virtual void initPackageInfo() override;
    virtual void initPackageInstallStatus() override;
};

#endif
