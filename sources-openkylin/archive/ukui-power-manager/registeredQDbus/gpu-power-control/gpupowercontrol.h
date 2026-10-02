/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef GPUPOWERCONTROL_H
#define GPUPOWERCONTROL_H

#include <QObject>

#define GPU_DEVICES_PATH        "/sys/class/drm/card0/device/"
#define GPU_POWER_DPM_STATE     "power_dpm_state"
#define GPU_POWER_DPM_LEVEL     "power_dpm_force_performance_level"

class GpuPowerControl : public QObject
{
    Q_OBJECT
public:
    explicit GpuPowerControl(QObject *parent = nullptr);
    ~GpuPowerControl();

    void setGpuFrequencyMode(const int policy);

private:
    QStringList m_gpuPowerDpmState = {"performance", "balanced", "battery"};
    QStringList m_gpuPowerDpmLevel = {"high", "auto", "low"};

    void initGpuFrequencyState();
    void initGpuFrequencyLevel();

signals:

};

#endif // GPUPOWERCONTROL_H
