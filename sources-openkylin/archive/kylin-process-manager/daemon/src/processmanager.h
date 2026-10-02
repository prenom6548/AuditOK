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

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QDBusContext>
#include <QObject>

#include "cgroupmanager.h"
#include "cpuraplenergymeter.h"
#include "systemdunitmanager.h"
#include "systemresourcemanager.h"
#include "refreshratemanager.h"
#include "inputeventlistener.h"

#include <QObject>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

class ProcessManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit ProcessManager(CGroupManager *cgroupManager, QObject *parent = nullptr);

    void ClosePSR();
    void OpenPSR();

    void SetVRRRefreshRate(QString mode);
    void ReverseVRRRefreshRate();
    bool isSupportVRR();

    void SetSystemdUnitRun(const QString &unitName, bool start);
    void ReclaimProcesses(const QList<int> &pids) const;
    void ReclaimProcessGroups(const QStringList &groupNames) const;
    double GetCpuEnergyConsumption() const;
    void SetSystemCpuFreq(bool cpufreq);
    void SetSamplingRate(bool samplingrate);
    void SetKernelSched(bool kernelsched);
    void SetDirty(bool dirty);
    void SetNetParams(bool netparams);
    void SetCpuLimit(bool cpulimit);
    void initCpuFreq();
    void initSamplingRate();
    void initKernelSched();
    void initDirty();
    void initNetParams();
    void setcpuset();
    void SetCurrentScene(const QStringList &sceneids);
    QStringList GetCurrentScene();
    void LockGSettingsKey(const QString &key, bool lock);

Q_SIGNALS:
    void ResourceThresholdWarning(QString resource, int level);
    void IdleStatus(bool Status);
    void SceneChanged(const QStringList &sceneids);

private:
    void initInputIdle();

private:
    CGroupManager *m_cgroupManager;
    SystemdUnitManager m_systemdUnitManager;
    SystemResourceManager m_systemResourceManager;
    CpuRaplEnergyMeter m_cpuEnergyMeter;
    PSRStateManager m_psrStateManager;
    InputMonitor m_listener;
    QStringList m_sceneids;
    std::vector<std::string> m_cpuCores;
    // 用于存储最大和最小频率的map
    std::map<int, int> m_maxFreqMap;
    std::map<int, int> m_minFreqMap;
    std::string m_originSamplingRate;
    long long m_kernelschedoriginalvalues[2];
    long m_dirtyoriginalvalues[4];
    int m_netparamsoriginalvalues[6];

    pid_t m_pid;
    pid_t find_pid_by_name(const std::string& process_name);
    double parse_cpu_usage(const std::string& line);
    std::unique_ptr<RefreshRateManager> m_refreshRateManager;
};

#endif // PROCESSMANAGER_H
