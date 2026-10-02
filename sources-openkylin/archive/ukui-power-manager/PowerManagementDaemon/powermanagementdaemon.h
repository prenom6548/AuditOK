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


#ifndef POWERMANAGEMENTDAEMON_H
#define POWERMANAGEMENTDAEMON_H

#include <QSettings>
#include <QFile>
#include "idle/idlenesswatcher.h"
#include "lidwatcher/lidwatcher.h"
#include "acwatcher/acwatcher.h"
#include "eventwatcher.h"
#include "gsettingwatcher/gsettingwatcher.h"
#include "lowpowerwatcher/lowpowerwatcher.h"
#include "upmcustomdefaultgsettings.h"
#include "cloudplatform.h"
#include "upmexternalinterface.h"

class PowerManagementDaemon : public QObject
{
    Q_OBJECT

public:
    PowerManagementDaemon();
    ~PowerManagementDaemon();

    void initPowerManagementDaemon();

private:
    IdlenessWatchcer *idlenesswatcher = nullptr;
    LidWatcher *lidwatcher = nullptr;
    EventWatcher *eventwatcher = nullptr;
    GsettingWatcher *gsettingwatcher = nullptr;
    LowPowerWatcher *lowpowerwatcher = nullptr;
    UpmExternalInterface *m_externalInterface = nullptr;
};
#endif // POWERMANAGEMENTDAEMON_H
