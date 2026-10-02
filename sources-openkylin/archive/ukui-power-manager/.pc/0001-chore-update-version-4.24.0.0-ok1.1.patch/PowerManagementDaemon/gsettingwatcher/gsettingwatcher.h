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

#ifndef GSETTINGWATCHER_H
#define GSETTINGWATCHER_H

#include <QFile>
#include "eventwatcher.h"
#include "acwatcher/acwatcher.h"
#include "lowpowerwatcher/lowpowerwatcher.h"

class GsettingWatcher : public EventWatcher
{
    Q_OBJECT
public:
    GsettingWatcher();
    ~GsettingWatcher();

    void initGsettingWatcher();

public slots:
    void afterSyncGlobalConfig(){
        qInfo() << "afterSyncGlobalConfig" << mAcWatcher.mPowerState;
        if(mAcWatcher.mPowerState == false){
            resetPowerPolicyBatteryBackup();
        }
    };

private:
    enum policy
    {
        Performance = 0,
        Balance,
        EnergySaving,
        Unknow,
    };

    void initSettingsConnect();

    void policyAC();

    void policyBattery();

    int mPowerPolicyAc;

    int mPowerPolicyBattery;

    int mLowBatteryAutoSave;

    bool mLowPrecentState = false;

    AcWatcher mAcWatcher;

    int m_powerPolicyBatteryBackup = Unknow;

    int m_batteryPercentage;

    int m_percentageLow;

    bool m_isIntel = false;

    bool m_isTlp = false;

    QTimer *m_ptlpTimer = nullptr;
private slots:

    void AcWatcherChanged(const bool &onBattery);

    void setLowPowerState(bool lowPowerState);

    void initLowPowerAutoSave();

    void resetPowerPolicyBatteryBackup();

    void dealBatteryPropertiesMessage(QDBusMessage msg);

    void setTlp();
private:

    QString getCpuInfo();

    void delayStartTlp(bool state);
};

#endif // GSETTINGWATCHER_H
