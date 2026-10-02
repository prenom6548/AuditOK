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

#include "processinfomanager.h"

#include "eventbus.h"
#include "loghelper.h"
#include "registrinfo.h"

#include <string>
#include <mutex>
#include <unordered_map>

namespace {
std::mutex g_winChangedRegMutex;
std::mutex g_focusRegMutex;

std::unordered_map<std::string, RegistrAppInfo> g_winChangedRegInfo;
std::unordered_map<std::string, RegistrProcessInfo> g_focusRegInfo;
}

ProcessInfoManager::ProcessInfoManager(const std::shared_ptr<ConfigManager> &configManager, 
    const std::shared_ptr<StateManager> &stateManager)
    : m_appInfoManager(configManager)
    , m_stateManager(stateManager)
    , m_currFoucsUuid("")
{
    initAppInfoChanged();
}

ProcessInfoManager::~ProcessInfoManager()
{
    PRINT_NOTICE("~ProcessInfoManager\n");
    for (auto &timer : m_timers) {
        timer.second.stop();
    }
    m_timers.clear();
    for (auto &conn : m_conns) {
        QObject::disconnect(conn);
        conn = {};
    }
    m_conns.clear();
}

void ProcessInfoManager::handleWindowAdded(const std::string &wid)
{
    m_appInfoManager.handleWindowAdded(wid);
}

void ProcessInfoManager::handleWindowRemoved(const std::string &wid)
{
    m_appInfoManager.handleWindowRemoved(wid);
}
void ProcessInfoManager::handleWindowActive(const std::string &wid)
{
    m_appInfoManager.handleWindowActive(wid);
}

void ProcessInfoManager::setWindowMinimized(const std::string &wid, bool isMinimized)
{
    m_appInfoManager.handleWindowMinimized(wid, isMinimized);
}

void ProcessInfoManager::setWindowKeepAbove(const std::string &wid, bool isKeepAbove)
{
    m_appInfoManager.handleWindowKeepAbove(wid, isKeepAbove);
}

void ProcessInfoManager::setWindowMaximized(const std::string &wid, bool isMaximized)
{
    m_appInfoManager.handleWindowMaximized(wid, isMaximized);
}

void ProcessInfoManager::setWindowFullscreen(const std::string &wid, bool isFullscreen)
{
    m_appInfoManager.handleWindowFullscreen(wid, isFullscreen);
}

void ProcessInfoManager::handleExceptionGroupSessionApp(const std::string &desktopFile, int pid)
{
    m_appInfoManager.handleExceptionGroupSessionApp(desktopFile, pid);
}

void ProcessInfoManager::handleMprisDbusAdded(int pid, const std::string &service)
{
    m_appInfoManager.handleMprisDbusAdded(pid, service);
}

void ProcessInfoManager::handleStatusNotifierItemRegistered(int pid)
{
    m_appInfoManager.handleStatusItemNotifierItemRegistered(pid);
}

void ProcessInfoManager::createAppInstance(
    const std::string &desktopFile, const std::vector<std::string> &args, int pid)
{
    m_appInfoManager.createAppInstance(desktopFile, args, pid);
}

void ProcessInfoManager::setAppGroupName(const std::string &desktopFilePid, const std::string &groupName)
{
    m_appInfoManager.setAppGroupName(desktopFilePid, groupName);
}

void ProcessInfoManager::forceChangCachedStateTo(sched_policy::AppState state)
{
    m_appInfoManager.forceChangCachedStateTo(state);
}

bool ProcessInfoManager::forceChangeAppStateByPid(int pid, sched_policy::AppState state)
{
    return m_appInfoManager.forceChangeAppStateByPid(pid, state);
}

void ProcessInfoManager::setBackgroundToCachedStateInterval(int interval)
{
    m_appInfoManager.setBackgroundToCachedStateInterval(interval);
}

sched_policy::AppType ProcessInfoManager::getAppTypeByPid(int pid)
{
    return m_appInfoManager.getAppTypeByPid(pid);
}

std::string ProcessInfoManager::syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable)
{
    return m_appInfoManager.syncGetDesktopFileByPid(pid, needUpDateDesktopTable);
}

std::vector<AppInfo> ProcessInfoManager::getAllAppInfos()
{
    return m_appInfoManager.getAllAppInfos();
}

std::vector<AppInfo> ProcessInfoManager::getAppInfosWithState(sched_policy::AppState appState)
{
    return m_appInfoManager.getAppInfosWithState(appState);
}

AppInfo ProcessInfoManager::getLatestAppInfoByDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    return m_appInfoManager.getLatestAppInfoByDesktopFile(desktopFile, args);
}

AppInfo ProcessInfoManager::getLatestAppInfoByCmdline(const std::string &cmdline)
{
    return m_appInfoManager.getLatestAppInfoByCmdline(cmdline);
}

AppInfo ProcessInfoManager::getAppInfo(const std::string &appId)
{
    return m_appInfoManager.getAppInfo(appId);
}

std::string ProcessInfoManager::getDesktopFileByPid(int pid)
{
    return m_appInfoManager.getDesktopFileByPid(pid);
}

void ProcessInfoManager::registerAppWinChangedSpellWatcher(const std::string &uuid, const RegistrAppInfo &process, int spell)
{
    g_winChangedRegInfo[uuid] = process;
    QTimer &timer = m_timers[uuid];
    timer.setSingleShot(true);
    timer.setInterval(spell * 1000);
    m_conns.emplace_back(QObject::connect(&timer, &QTimer::timeout, [this, uuid]() {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::AppWinChangedSpell));
        app_win_changed_spell.insert(uuid);
        m_stateManager->enableTarget(StateTarget::AppWinChangedSpell);
    }));
}

void ProcessInfoManager::unregisterAppWinChangedSpellWatcher(const std::string &uuid)
{
    if (g_winChangedRegInfo.find(uuid) != g_winChangedRegInfo.end()){
        g_winChangedRegInfo.erase(uuid);
    }
}

void ProcessInfoManager::registerAppFocusSpellWatcher(const std::string &uuid, const RegistrProcessInfo &process, int spell)
{
    g_focusRegInfo[uuid] = process;
    QTimer &timer = m_timers[uuid];
    timer.setSingleShot(true);
    timer.setInterval(spell * 1000);
    m_conns.emplace_back(QObject::connect(&timer, &QTimer::timeout, [this, uuid]() {
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::AppFocusSpell));
        app_focus_spell.insert(uuid);
        m_stateManager->enableTarget(StateTarget::AppFocusSpell);
    }));
}

void ProcessInfoManager::unregisterAppFocusSpellWatcher(const std::string &uuid)
{
    if (g_focusRegInfo.find(uuid) != g_focusRegInfo.end()){
        g_focusRegInfo.erase(uuid);
    }
}

void ProcessInfoManager::initAppInfoChanged()
{
    m_appInfoManager.setAppStartedCallback([this](const AppInfo &appInfo){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ProcessInfo));
        setAppChangeType(appInfo.desktopFilePid(), sched_policy::AppChangeType::AppStateChanged);
        handleAppInfoChanged(appInfo);
        EventBus::instance()->notifyAddProcessGroup(appInfo.desktopFilePid(), appInfo.pids());
    });

    m_appInfoManager.setAppStateChangedCallback([this](const AppInfo &appInfo){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ProcessInfo));
        setAppChangeType(appInfo.desktopFilePid(), sched_policy::AppChangeType::AppStateChanged);
        handleAppStateChanged(appInfo);
        handleAppInfoChanged(appInfo);
    });

    m_appInfoManager.setAppChildPidsUpdatedCallback([this](const AppInfo &appInfo){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ProcessInfo));
        setAppChangeType(appInfo.desktopFilePid(), sched_policy::AppChangeType::AppPidsChanged);
        handleAppInfoChanged(appInfo);
    });

    m_appInfoManager.setAppFinishedCallback([this](const AppInfo &appInfo){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ProcessInfo));
        setAppChangeType(appInfo.desktopFilePid(), sched_policy::AppChangeType::AppStateChanged);
        if (all_app_infos.find(appInfo.desktopFilePid()) != all_app_infos.end()) {
            all_app_infos.erase(appInfo.desktopFilePid());
        }
        m_stateManager->enableTarget(StateTarget::ProcessInfo);
    });

    m_appInfoManager.setAppWinsChangedCallback([this](const AppInfo &appInfo, const std::string &wid){
        std::lock_guard<FairMutex>(m_stateManager->fairMutex(StateTarget::ProcessInfo));
        setAppChangeType(appInfo.desktopFilePid(), sched_policy::AppChangeType::AppWinsChanged);
        handleWindowChanged(appInfo, wid);
        handleAppInfoChanged(appInfo);
    });
}

void ProcessInfoManager::handleAppInfoChanged(const AppInfo &appInfo)
{
    if (all_app_infos.find(appInfo.desktopFilePid()) != all_app_infos.end()) {
        all_app_infos.erase(appInfo.desktopFilePid());
    }
    all_app_infos.emplace(appInfo.desktopFilePid(), appInfo);
    m_stateManager->enableTarget(StateTarget::ProcessInfo);
}

void ProcessInfoManager::handleAppStateChanged(const AppInfo &appInfo)
{
    if (appInfo.appState() == sched_policy::AppState::Focus) {
        focus_app_id = appInfo.desktopFilePid();
        std::lock_guard<std::mutex> lock(g_focusRegMutex);
        if (g_focusRegInfo.empty())   return;

        std::string uuid = "";
        for (std::unordered_map<std::string, RegistrProcessInfo>::iterator it = g_focusRegInfo.begin(); it != g_focusRegInfo.end(); ++it){
            if (it->second.name_ != "-"){
                if (appInfo.name().find(it->second.name_) != std::string::npos){
                    uuid = it->first;
                }
            } else if (it->second.cmdline_ != "-"){
                if (appInfo.cmdline().find(it->second.cmdline_) != std::string::npos){
                    uuid = it->first;
                }
            }
        }
        if (!m_currFoucsUuid.empty() && m_currFoucsUuid != uuid){
            m_timers[m_currFoucsUuid].stop();
        }
        m_currFoucsUuid = uuid;
        if (!m_currFoucsUuid.empty()){
            m_timers[m_currFoucsUuid].start();
        }
    }
}

void ProcessInfoManager::handleWindowChanged(const AppInfo &appInfo, const std::string &wid)
{
    std::lock_guard<std::mutex> lock(g_winChangedRegMutex);
    if (g_winChangedRegInfo.empty())   return;
    
    auto winInfoIt = appInfo.winInfos().find(wid);
    if (winInfoIt == appInfo.winInfos().end()){
        return;
    }
    if (winInfoIt->second.is_added || winInfoIt->second.is_removed){
        for (std::unordered_map<std::string, RegistrAppInfo>::iterator it = g_winChangedRegInfo.begin(); it != g_winChangedRegInfo.end(); ++it){
            if (it->second.process_.name_ != "-"){
                if (appInfo.name().find(it->second.process_.name_) != std::string::npos &&
                    winInfoIt->second.title_ == it->second.window_.title_){
                    m_timers[it->first].start();
                }
            } else if (it->second.process_.cmdline_ != "-"){
                if (appInfo.cmdline().find(it->second.process_.cmdline_) != std::string::npos &&
                    winInfoIt->second.title_ == it->second.window_.title_){
                    m_timers[it->first].start();
                }
            }
        }
    }
}

void ProcessInfoManager::setAppChangeType(std::string appId, sched_policy::AppChangeType changeType)
{
    static std::vector<sched_policy::AppChangeType> appChangeTypeValues = {
        sched_policy::AppChangeType::AppStateChanged,
        sched_policy::AppChangeType::AppPidsChanged,
        sched_policy::AppChangeType::AppWinsChanged
    };

    if (app_change_type.find(appId) == app_change_type.end()){
        for (const auto &value : appChangeTypeValues){
            app_change_type[appId][value] = false;
        }
    }
    app_change_type[appId][changeType] = true;
}
