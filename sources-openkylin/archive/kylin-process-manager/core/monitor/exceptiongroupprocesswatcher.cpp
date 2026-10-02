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

#include "exceptiongroupprocesswatcher.h"
#include "loghelper.h"
#include "systemddbusinterface.h"
#include <fstream>
#include <iostream>
#include <sstream>

void WatcherThread::run()
{
    m_running = true;
    int sleepDuration = 1;

    while (m_running) {
        for (auto &groupInfos : m_watcher->m_watchedGroupPaths) {
            auto pids = m_watcher->readFileToPids(groupInfos.second.path + "/cgroup.procs");
            auto exceptionPids = m_watcher->findExceptionPids(groupInfos.second.pids, pids);
            m_watcher->handleExceptionPids(exceptionPids);
            groupInfos.second.pids = pids;
        }

        QMutexLocker locker(&m_mutex);
        m_condition.wait(&m_mutex, sleepDuration * 1000);
        if (!m_running)
            break;

        sleepDuration = sleepDuration > 20 ? 20 : sleepDuration + 1;
    }
}

void WatcherThread::stop()
{
    m_running = false;
    {
        QMutexLocker locker(&m_mutex);
        m_condition.wakeAll(); // 唤醒可能正在等待的线程
    }
}

ExceptionGroupProcessWatcher::ExceptionGroupProcessWatcher(
    const std::shared_ptr<ProcessInfoManager> &processInfoManager, QObject *parent)
    : QObject{parent}
    , m_processInfoManager(processInfoManager)
    , m_watcherRunning(false)
{
    qRegisterMetaType<std::string>("std::string");
    initPaths();
}

ExceptionGroupProcessWatcher::~ExceptionGroupProcessWatcher()
{
    PRINT_NOTICE("~ExceptionGroupProcessWatcher\n");
    stopWatcher();
}

void ExceptionGroupProcessWatcher::stopWatcher()
{
    if (m_watcherThread) {
        m_watcherThread->stop();
        m_watcherThread->wait(2000);

        if (m_watcherThread->isRunning()) {
            m_watcherThread->terminate();
            m_watcherThread->wait();
        }

        delete m_watcherThread;
        m_watcherThread = nullptr;
    }
}

void ExceptionGroupProcessWatcher::startWatcher()
{
    if (m_watcherThread) {
        return;
    }

    m_watcherThread = new WatcherThread(this);
    m_watcherThread->start();
}

std::vector<int> ExceptionGroupProcessWatcher::readFileToPids(const std::string &file) const
{
    std::vector<int> pids;
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        //std::cout << "open file failed" << std::endl;
        PRINT_DBG("ExceptionGroupProcessWatcher::readFileToPids: open file failed\n");
        return pids;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        std::stringstream ss(line);
        int pid;
        ss >> pid;
        pids.push_back(pid);
    }
    ifs.close();
    return pids;
}

std::vector<int> ExceptionGroupProcessWatcher::findExceptionPids(const std::vector<int> &oldPids, const std::vector<int> &newPids) const
{
    std::vector<int> res;
    std::unordered_map<int, int> mp;
    for (int i = 0; i < oldPids.size(); ++i) {
        mp[oldPids[i]]++;
    }
    for (int i = 0; i < newPids.size(); ++i) {
        if (mp.find(newPids[i]) == mp.end() || mp[newPids[i]] == 0) {
            res.push_back(newPids[i]);
        } else {
            mp[newPids[i]]--;
        }
    }
    return res;
}

void ExceptionGroupProcessWatcher::handleExceptionPids(const std::vector<int> &pids)
{
    for (const auto &pid : pids) {
        auto appType = m_processInfoManager->getAppTypeByPid(pid);
        if(appType != sched_policy::AppType::Session) {
            continue;
        }

        auto desktop = m_processInfoManager->syncGetDesktopFileByPid(pid, false);
        if(desktop.empty()) {
            PRINT_DBG("dont find desktop by pid: %d\n", pid);
            continue;
        } 
        // only watch session scope group
        PRINT_DBG("exceptionSessionAppCaught %s %d\n", desktop.c_str(), pid);
        m_processInfoManager->handleExceptionGroupSessionApp(desktop, pid);
    }
}

void ExceptionGroupProcessWatcher::addPath(std::string path)
{
    if (m_watchedGroupPaths.find(path) != m_watchedGroupPaths.end()) {
        // qDebug() << QString("path '%1' already exists").arg(QString::fromStdString(path));
        PRINT_NOTICE("path '%s' already exists\n", path.c_str());
        return;
    }
    m_watchedGroupPaths[path] = {path};
}

void ExceptionGroupProcessWatcher::initPaths()
{
    static SystemdDbusInterface systemdInterface;
    static auto currentUserSlice = systemdInterface.currentUserSliceName();
    static auto currentSessionScope = systemdInterface.currentSessionScopeName();
    static std::string systemdPath = "/sys/fs/cgroup/systemd/";
    static std::string userSlicePrefix = "user.slice/";

    addPath(systemdPath + userSlicePrefix + currentUserSlice + "/" + currentSessionScope);
}
