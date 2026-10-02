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

#include "resourcemanagerinterface.h"
#include "loghelper.h"
#include "misc.h"
#include "processinfohelper.h"
#include <QDebug>

void ResourceManagerInterface::reclaimProcessGroups(const std::vector<std::string> &groupNames)
{
    m_daemonDbus.reclaimProcessGroups(groupNames);
}

void ResourceManagerInterface::openPSR()
{
    PRINT_DBG("ResourceManagerInterface::openPSR()\n");
    m_daemonDbus.openPSR();
}

void ResourceManagerInterface::closePSR()
{
    PRINT_DBG("ResourceManagerInterface::closePSR()\n");
    m_daemonDbus.closePSR();
}

void ResourceManagerInterface::setVRRRefreshRate(std::string mode) 
{
    PRINT_DBG("ResourceManagerInterface::setVRRRefreshRate()\n");
    m_daemonDbus.setVRRRefreshRate(mode);
}

void ResourceManagerInterface::reverseVRRRefreshRate()
{
    PRINT_DBG("ResourceManagerInterface::reverseVRRRefreshRate()\n");
    m_daemonDbus.reverseVRRRefreshRate();
}

void ResourceManagerInterface::setSystemdUnitRun(const std::string &unitName, bool start, bool mode)
{
    if (mode) {
        m_daemonDbus.setSystemdUnitRun(unitName, start);
    } else {
        m_systemdDbus.setSystemdUnitRun(QString::fromStdString(unitName), start, 0);
    }
}

int ResourceManagerInterface::getSystemdUnitMainpid(const std::string &unitName, bool mode)
{
    return m_systemdDbus.getServiceMainpid(QString::fromStdString(unitName), mode);
}

void ResourceManagerInterface::setCurrentScene(const std::vector<std::string> &currScenes)
{
    return m_daemonDbus.setCurrentScene(currScenes);
}

void ResourceManagerInterface::SetSystemCpuFreq(bool cpufreq)
{
    PRINT_DBG("ResourceManagerInterface::SetSystemCpuFreq()\n");
    m_daemonDbus.setSystemCpuFreq(cpufreq);
}

void ResourceManagerInterface::SetSamplingRate(bool samplingrate)
{
    PRINT_DBG("ResourceManagerInterface::SetSamplingRate()\n");
    m_daemonDbus.SetSamplingRate(samplingrate);
}

void ResourceManagerInterface::SetKernelSched(bool kernelsched)
{
    PRINT_DBG("ResourceManagerInterface::SetKernelSched()\n");
    m_daemonDbus.SetKernelSched(kernelsched);
}

void ResourceManagerInterface::SetDirty(bool dirty)
{
    PRINT_DBG("ResourceManagerInterface::SetDirty()\n");
    m_daemonDbus.SetDirty(dirty);
}

void ResourceManagerInterface::SetNetParams(bool netparams)
{
    PRINT_DBG("ResourceManagerInterface::SetNetParams()\n");
    m_daemonDbus.SetNetParams(netparams);
}

void ResourceManagerInterface::SetCpuLimit(bool cpulimit)
{
    PRINT_DBG("ResourceManagerInterface::SetCpuLimit()\n");
    m_daemonDbus.SetCpuLimit(cpulimit);
}

bool ResourceManagerInterface::setCurrentUserProperty()
{
    return m_cgroupControlInterface.setCurrentUserProperty(m_systemdDbus.currentUserUID(), m_systemdDbus.currentUserSliceName(), m_systemdDbus.currentUserServiceName(), m_systemdDbus.currentSessionScopeName());
}

void ResourceManagerInterface::setGroupPath(std::string name, std::string path)
{
    m_cgroupControlInterface.setGroupPath(m_systemdDbus.currentUserUID(), name, path);
}

void ResourceManagerInterface::setGroupProperty(std::string name, std::string node, std::string value)
{
    m_cgroupControlInterface.setGroupProperty(m_systemdDbus.currentUserUID(), name, node, value);
}

void ResourceManagerInterface::createGroupPath(std::string name)
{
    m_cgroupControlInterface.createGroupPath(m_systemdDbus.currentUserUID(), name);
}

void ResourceManagerInterface::removeGroupPath(std::string name)
{
    m_cgroupControlInterface.removeGroupPath(m_systemdDbus.currentUserUID(), name);
}

void ResourceManagerInterface::limitProcessByGroup(std::string name, int pid)
{
    m_cgroupControlInterface.limitProcessByGroup(name, pid);
}

void ResourceManagerInterface::setGroupEnable(bool enable)
{
    m_cgroupControlInterface.setGroupEnable(enable);
}

void ResourceManagerInterface::insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid)
{
    m_daemon1Dbus.insertTopTask(pid, topType, topPrio, topSlice, topTypeByTgid);
    m_efficiencyModeInterface.setPidFilter(pid, true);
}

void ResourceManagerInterface::eraseTopTask(int pid)
{
    m_daemon1Dbus.eraseTopTask(pid);
    m_efficiencyModeInterface.setPidFilter(pid, false);
}

void ResourceManagerInterface::enable()
{
    m_daemon1Dbus.enable();
}

void ResourceManagerInterface::disable()
{
    m_daemon1Dbus.disable();
}

void ResourceManagerInterface::clearTopList()
{
    m_daemon1Dbus.clearTopList();
}