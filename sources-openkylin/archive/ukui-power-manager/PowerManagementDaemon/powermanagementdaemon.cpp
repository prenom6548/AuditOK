/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "powermanagementdaemon.h"

#include <QDebug>

PowerManagementDaemon::PowerManagementDaemon() {}

PowerManagementDaemon::~PowerManagementDaemon()
{
    if (nullptr != idlenesswatcher) {
        delete idlenesswatcher;
    }
    if (nullptr != lidwatcher) {
        delete lidwatcher;
    }
    if (nullptr != eventwatcher) {
        delete eventwatcher;
    }

    if (nullptr != lowpowerwatcher) {
        delete lowpowerwatcher;
    }
    if (nullptr != gsettingwatcher) {
        delete gsettingwatcher;
    }
}

void PowerManagementDaemon::initPowerManagementDaemon()
{
    UpmCustomDefaultGsettings upmCustomDefaultGsettings;
    upmCustomDefaultGsettings.setUpmCustomDefaultGsettings();

    eventwatcher = new EventWatcher;
    eventwatcher->initEventWatcher();

    if (false == CloudPlatform::self()->isCloudPlatform()) {
        idlenesswatcher = new IdlenessWatchcer;
        idlenesswatcher->initIdleWatcher();
    }

    lidwatcher = new LidWatcher;
    lidwatcher->initLidWatcher();

    lowpowerwatcher = new LowPowerWatcher;
    lowpowerwatcher->initLowPowerWatcher();
    connect(lowpowerwatcher, &LowPowerWatcher::percentageLowSignal,
            eventwatcher, &EventWatcher::percentageLowRegulateBrightness);
    gsettingwatcher = new GsettingWatcher;
    gsettingwatcher->initGsettingWatcher();

    connect(eventwatcher, &EventWatcher::allUserSyncFinished,
            gsettingwatcher, &GsettingWatcher::afterSyncGlobalConfig);

    m_externalInterface = new UpmExternalInterface(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("org.ukui.powerManager")) {
        qWarning() << "QDbus register service failed reason:" << connection.lastError();
        return;
    }

    if (!connection.registerObject("/PowerManager", m_externalInterface,
                                   QDBusConnection::ExportAllSlots |
                                   QDBusConnection::ExportAllSignals)) {
        qWarning() << "QDbus register object failed reason:" << connection.lastError();
    }
}
