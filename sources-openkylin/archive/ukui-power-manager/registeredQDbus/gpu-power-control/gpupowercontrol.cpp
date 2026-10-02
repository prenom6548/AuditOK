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

#include "gpupowercontrol.h"

#include "../upmcommonfile.h"
#include "../upmcustomhardwareconfig.h"

enum system_policy
{
    system_policy_performance = 0,
    system_policy_balance,
    system_policy_powersave,
    system_policy_max
};

GpuPowerControl::GpuPowerControl(QObject *parent) : QObject(parent)
{
    initGpuFrequencyState();

    initGpuFrequencyLevel();
}

GpuPowerControl::~GpuPowerControl() {}

void GpuPowerControl::initGpuFrequencyState()
{
    QString config;
    if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                UPM_KEY_EXTEND_GPU_FREQUENCY_STATE, config)) {
        QStringList gpuPowerDpmState = config.split("-");
        if (system_policy_max == gpuPowerDpmState.size()) {
            m_gpuPowerDpmState[system_policy_performance] = gpuPowerDpmState[system_policy_performance];
            m_gpuPowerDpmState[system_policy_balance] = gpuPowerDpmState[system_policy_balance];
            m_gpuPowerDpmState[system_policy_powersave] = gpuPowerDpmState[system_policy_powersave];
        }
    }
}

void GpuPowerControl::initGpuFrequencyLevel()
{
    QString config;
    if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                UPM_KEY_EXTEND_GPU_FREQUENCY_LEVEL, config)) {
        QStringList gpuPowerDpmLevel = config.split("-");
        if (system_policy_max == gpuPowerDpmLevel.size()) {
            m_gpuPowerDpmLevel[system_policy_performance] = gpuPowerDpmLevel[system_policy_performance];
            m_gpuPowerDpmLevel[system_policy_balance] = gpuPowerDpmLevel[system_policy_balance];
            m_gpuPowerDpmLevel[system_policy_powersave] = gpuPowerDpmLevel[system_policy_powersave];
        }
    }
}

void GpuPowerControl::setGpuFrequencyMode(const int policy)
{
    if (policy < system_policy_performance || policy > system_policy_powersave) {
        return ;
    }

    if (true == UpmCommonFile::exists(GPU_DEVICES_PATH, GPU_POWER_DPM_STATE)) {
        UpmCommonFile::write(GPU_DEVICES_PATH, GPU_POWER_DPM_STATE, m_gpuPowerDpmState[policy]);
    }
    if (true == UpmCommonFile::exists(GPU_DEVICES_PATH, GPU_POWER_DPM_LEVEL)) {
        UpmCommonFile::write(GPU_DEVICES_PATH, GPU_POWER_DPM_LEVEL, m_gpuPowerDpmLevel[policy]);
    }
}