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

#ifndef RESOURCEMANAGERINTERFACE_H
#define RESOURCEMANAGERINTERFACE_H

#include "cgroupcontrolinterface.h"
#include "daemon1dbusinterface.h"
#include "daemondbusinterface.h"
#include "efficiencymodeinterface.h"
#include "systemddbusinterface.h"

class ResourceManagerInterface
{
public:
    void reclaimProcessGroups(const std::vector<std::string> &groupNames);

    void openPSR();
    void closePSR();   
    void SetSystemCpuFreq(bool cpufreq);
    void SetSamplingRate(bool samplingrate);
    void SetKernelSched(bool kernelsched);
    void SetDirty(bool dirty);
    void SetNetParams(bool netparams);
    void SetCpuLimit(bool cpulimit);

    void setVRRRefreshRate(std::string mode);
    void reverseVRRRefreshRate();
    void setSystemdUnitRun(
        const std::string &unitName, bool start, bool mode);
    int getSystemdUnitMainpid(const std::string &unitName, bool mode);
    void setCurrentScene(const std::vector<std::string> &currScenes);

    // cgroup
    bool setCurrentUserProperty();
    void setGroupPath(std::string name, std::string path);
    void setGroupProperty(std::string name, std::string node, std::string value);
    void createGroupPath(std::string name);
    void removeGroupPath(std::string name);
    void limitProcessByGroup(std::string name, int pid);
    void setGroupEnable(bool enable);

    // priority sched
    void insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid);
    void eraseTopTask(int pid);
    void enable();
    void disable();
    void clearTopList();

private:
    DaemonDbusInterface m_daemonDbus;
    Daemon1DbusInterface m_daemon1Dbus;
    SystemdDbusInterface m_systemdDbus;
    CgroupControlInterface m_cgroupControlInterface;
    EfficiencyModeInterface m_efficiencyModeInterface;
};

#endif // RESOURCEMANAGERINTERFACE_H
