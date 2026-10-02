/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cgroupmanager.h"
#include "cgroupmanagerservice.h"
#include "efficiencymode.h"
#include "efficiencymodeservice.h"
#include "loghelper.h"
#include "processmanager.h"
#include "processmanagerservice.h"
#include "resourcelimit.h"
#include "resourcelimitservice.h"
#include <QCommandLineParser>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CGroupManager cgroupManager;
    CGroupManagerService cgroupManagerService(&cgroupManager);

    ResourceLimit resourceLimit(&cgroupManager);
    ResourceLimitService resourceLimitService(&resourceLimit);

    EfficiencyMode efficiencyMode(&cgroupManager);
    EfficiencyModeService efficiencyModeService(&efficiencyMode);

    ProcessManager processManager(&cgroupManager);
    ProcessManagerService processManagerService(&processManager);

    QDBusConnection connection = QDBusConnection::systemBus();

    if (!connection.isConnected()) {
        PRINT_NOTICE("Failed to connect to system bus: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerService("com.kylin.ProcessManagerDaemon")) {
        PRINT_NOTICE("register dbus service failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManagerDaemon", &processManager)) {
        PRINT_NOTICE("register dbus object failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManagerDaemon/CgroupControl", &cgroupManager)) {
        PRINT_NOTICE("register dbus object CgroupControl failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManagerDaemon/ResourceLimit", &resourceLimit)) {
        PRINT_NOTICE("register dbus object ResourceLimit failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManagerDaemon/EfficiencyMode", &efficiencyMode)) {
        PRINT_NOTICE("register dbus object EfficiencyMode failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    return a.exec();
}
