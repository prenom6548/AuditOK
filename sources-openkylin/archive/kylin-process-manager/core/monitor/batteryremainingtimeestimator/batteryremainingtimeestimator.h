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

#ifndef BATTERYREMAININGTIMEESTIMATOR_H
#define BATTERYREMAININGTIMEESTIMATOR_H

#include "consumptionmonitor/processgroupconsumptionmonitor.h"
#include "systemenery/systemenergy.h"
#include <QTimer>
#include <functional>
#include <memory>

class BatteryRemainingTimeEstimator
{
public:
    using RemainingTimeChangedCallback = std::function<void(int)>;
    using FrozenProcessGroupIdsGetter = std::function<std::vector<std::string>()>;

    BatteryRemainingTimeEstimator(
        std::shared_ptr<SystemEnergy> systemEnergy,
        std::shared_ptr<ProcessGroupConsumptionMonitor> processGroupConsumptionMonitor,
        int estimateInterval,
        FrozenProcessGroupIdsGetter frozenProcessGroupIdsGetter);

    ~BatteryRemainingTimeEstimator();

    void setIncreasedRemainTimeChangedCallback(RemainingTimeChangedCallback callback);

    void startEstimate();
    void stopEstimate();

private:
    void estimateRemainingTime();
    double calculateFrozenGroupsEnergyRate(const std::vector<std::string> &groupIds);
    void setIncreasedRemainingTime(int time);

private:
    std::shared_ptr<SystemEnergy> m_systemEnergy;
    std::shared_ptr<ProcessGroupConsumptionMonitor> m_processGroupConsumptionMonitor;
    int m_estimateInterval;
    QTimer m_timer;
    QMetaObject::Connection m_conn;
    RemainingTimeChangedCallback m_increasedRemainingTimeChangedCallback;
    FrozenProcessGroupIdsGetter m_frozenProcessGroupIdsGetter;
};

#endif // BATTERYREMAININGTIMEESTIMATOR_H
