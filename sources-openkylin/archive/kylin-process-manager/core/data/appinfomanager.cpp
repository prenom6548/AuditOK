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

#include "appinfomanager.h"
#include "appinfohelper.h"
#include "configmanager.h"
#include "loghelper.h"
#include "misc.h"
#include "processinfohelper.h"
#include "singleton.h"
#include "timewheel.h"
#include "waylanddisplay.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <fstream>
#include <kwindowinfo.h>
#include <kwindowsystem.h>

AppInfoManager::AppInfoManager(const std::shared_ptr<ConfigManager> &configManager)
    : m_configManager(configManager)
    , m_backgroundToCachedStateInterval(1800)
{
}

AppInfoManager::~AppInfoManager()
{
    PRINT_NOTICE("~AppInfoManager\n");
    Singleton<timewheel::TimeWheel>::instance().stop();
    auto appIds = appIdsWithTimer();
    for (const auto &appId : appIds) {
        deleteStateChangeTimerId(appId);
    }
}

void AppInfoManager::handleWindowAdded(const std::string &wid)
{
    const auto window = appinfo_helper::getWindowInfoByWid(wid);
    int pid = window ? window->pid : -1;

    if (pid <= 0) {
        PRINT_NOTICE("Get pid failed by wid %s.\n", wid.c_str());
        return;
    }

    // existed app
    auto appExistsAndIt = findAppInfoWithPid(pid);
    if (std::get<0>(appExistsAndIt)) {
        addWidToAppInfo(std::get<1>(appExistsAndIt), wid);
        return;
    }

    // new app
    createAppInstanceWithWid(wid);
}

void AppInfoManager::handleWindowRemoved(const std::string &wid)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("Window %s removed, but can not find the app info.\n", wid.c_str());
        return;
    }

    AppInfoIterator it = std::get<1>(appExistsAndIt);
    it->removeWid(wid);

    // wait for process and windows finished.
    auto desktopFilePid = it->desktopFilePid();
    Singleton<timewheel::TimeWheel>::instance().addTimer(1, [this, desktopFilePid]() {
        auto appExistsAndIt = findAppInfoWithDesktopFilePid(desktopFilePid);
        if (!std::get<0>(appExistsAndIt)) {
            return;
        }
        auto it = std::get<1>(appExistsAndIt);
        if (it->wids().empty()) {
            updateAppInfoWithEmptyWindows(it);
            return;
        }

        // 激活窗口因为会有延时过滤的操作，必须在接受到事件之后处理
        // 不进行任何的主动处理，保证入口的统一
        if (!it->isHasActiveWindow()) {
            updateAppStateWithoutEmptyWindows(it);
        }
    });
}

void AppInfoManager::handleWindowMinimized(const std::string &wid, bool isMinimized)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("Window %s minimized, but can not find the app info.\n", wid.c_str());
        return;
    }

    auto it = std::get<1>(appExistsAndIt);
    it->setMinimized(wid, isMinimized);
    if (it->isAllWinsMinimize()) {
        setAppState(sched_policy::AppState::Background, it);
    }
}

void AppInfoManager::handleWindowKeepAbove(const std::string &wid, bool isKeepAbove)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("Window %s keep above, but can not find the app info.\n", wid.c_str());
        return;
    }

    AppInfoIterator it = std::get<1>(appExistsAndIt);
    it->setKeepAbove(wid, isKeepAbove);
}

void AppInfoManager::handleWindowMaximized(const std::string &wid, bool isMaximized)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("Window %s keep above, but can not find the app info.\n", wid.c_str());
        return;
    }

    AppInfoIterator it = std::get<1>(appExistsAndIt);
    it->setMaximized(wid, isMaximized);
}

void AppInfoManager::handleWindowFullscreen(const std::string &wid, bool isFullscreen)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("Window %s keep above, but can not find the app info.\n", wid.c_str());
        return;
    }

    AppInfoIterator it = std::get<1>(appExistsAndIt);
    it->setFullscreen(wid, isFullscreen);
}

void AppInfoManager::handleWindowActive(const std::string &wid)
{
    m_currentActiveWid = wid;
    doHandleActiveWindowChanged();
}

void AppInfoManager::handleExceptionGroupSessionApp(const std::string &desktopFile, int pid)
{
    if (desktopFile.empty() || pid <= 0) {
        return;
    }

    AppInfo appInfo(desktopFile, std::string(), pid, 0, sched_policy::AppType::Session,
                    sched_policy::AppState::Focus, nullptr);

    if (m_appInfos.find(appInfo.desktopFileId()) == m_appInfos.end()) {
        m_appInfos[appInfo.desktopFileId()].emplace_back(appInfo);
    }

    if (m_appStartedCallback) {
        m_appStartedCallback(appInfo);
    }
}

void AppInfoManager::handleMprisDbusAdded(int pid, const std::string &service)
{
    auto appExistsAndIt = findAppInfoWithPid(pid);
    if (std::get<0>(appExistsAndIt)) {
        std::get<1>(appExistsAndIt)->setMrprisDbusService(service);
    } else {
        saveMprisServiceToCache(pid, service);
    }
}

void AppInfoManager::handleStatusItemNotifierItemRegistered(int pid)
{
    auto appExistsAndIt = findAppInfoWithPid(pid);
    if (std::get<0>(appExistsAndIt)) {
        auto it = std::get<1>(appExistsAndIt);
        it->setIsSystemTrayIconApp(true);
        return;
    }

    // 可能还没匹配到该应用,先保存到缓存
    saveSystemTrayIconAppToCache(pid);
}

void AppInfoManager::createAppInstance(const std::string &desktopFile, const std::vector<std::string> &args, int pid)
{
    AppInfo appInfo = generateAppInfo(desktopFile, pid);
    appInfo.setArgs(args);

    if (!appInfo.isAvailable()) {
        PRINT_NOTICE("Create app instance failed from desktop file: %s with pid: %d.\n", desktopFile.c_str(), pid);
        return;
    }

    if (m_appStartedCallback) {
        m_appStartedCallback(appInfo);
    }

    auto desktopFileId = appinfo_helper::generateDesktopFileId(desktopFile);
    m_appInfos[desktopFileId].emplace_back(appInfo);
}

void AppInfoManager::setAppGroupName(const std::string &desktopFilePid, const std::string &groupName)
{
    auto appExistsAndIt = findAppInfoWithDesktopFilePid(desktopFilePid);
    bool appExists = std::get<0>(appExistsAndIt);
    if (!appExists) {
        PRINT_NOTICE("Set app group name failed, can not find the desktop id %s from.\n", desktopFilePid.c_str());
        return;
    }

    AppInfoIterator it = std::get<1>(appExistsAndIt);
    it->setGroupName(groupName);
}

void AppInfoManager::setAppStartedCallback(AppChangedCallback callback)
{
    m_appStartedCallback = std::move(callback);
}

void AppInfoManager::setAppStateChangedCallback(AppChangedCallback callback)
{
    m_appStateChangedCallback = std::move(callback);
}

void AppInfoManager::setAppFinishedCallback(AppChangedCallback callback)
{
    m_appFinishedCallback = std::move(callback);
}

void AppInfoManager::setAppChildPidsUpdatedCallback(AppChangedCallback callback)
{
    m_appChildPidsUpdatedCallback = std::move(callback);
}

void AppInfoManager::setAppWinsChangedCallback(AppWinChangedCallback callback)
{
    m_appWinsChangedCallback = std::move(callback);
}

bool AppInfoManager::forceChangeAppStateByPid(int pid, sched_policy::AppState state)
{
    auto appExistsAndIt = findAppInfoWithPid(pid);
    if (!std::get<0>(appExistsAndIt)) {
        PRINT_DBG("can not find\n");
        return false;
    }
    auto appInfoIt = std::get<1>(appExistsAndIt);
    if (!canChangeState(*appInfoIt) || appInfoIt->appState() == state) {
        return true;
    }

    appInfoIt->setAppState(state);
    m_appStateChangedCallback(*appInfoIt);
    return true;
}

void AppInfoManager::forceChangCachedStateTo(sched_policy::AppState state)
{
    for (auto &appInfoPair : m_appInfos) {
        for (auto &appInfo : appInfoPair.second) {
            if (!canChangeState(appInfo)) {
                continue;
            }

            if (appInfo.appState() == sched_policy::AppState::Cached) {
                appInfo.setAppState(state);
                m_appStateChangedCallback(appInfo);
            }
        }
    }
}

sched_policy::AppType AppInfoManager::getAppTypeByDesktopFile(const std::string &desktopFile)
{
    std::ifstream file(desktopFile);
    if (!file.good()) {
        PRINT_NOTICE("Get desktop file type failed from desktop file: %s.\n", desktopFile.c_str());
        return sched_policy::AppType::Unknown;
    }

    if (appinfo_helper::isTopApp(desktopFile,
                                 misc::qt2stl::qstringList2StdVectorString(m_configManager->defaultAppsInTopGroup()))) {
        return sched_policy::AppType::Top;
    }

    auto fileName = appinfo_helper::getDesktopFileName(desktopFile);
    auto availableDesktopFiles = m_desktopFileManager.desktopFilesWithName(fileName);
    if (appinfo_helper::isSessionApp(desktopFile, availableDesktopFiles)) {
        return sched_policy::AppType::Session;
    }

    return sched_policy::AppType::Normal;
}

sched_policy::AppType AppInfoManager::getAppTypeByPid(int pid)
{
    auto desktopFile = m_desktopFileManager.syncGetDesktopFileByPid(pid);
    return getAppTypeByDesktopFile(desktopFile);
}

std::string AppInfoManager::syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable)
{
    return m_desktopFileManager.syncGetDesktopFileByPid(pid, needUpDateDesktopTable);
}

std::vector<AppInfo> AppInfoManager::getAllAppInfos()
{
    std::vector<AppInfo> appInfoVector;
    for (auto &appInfos : m_appInfos) {
        appInfoVector.insert(
            appInfoVector.end(), appInfos.second.begin(), appInfos.second.end());
    }
    return appInfoVector;
}

std::vector<AppInfo> AppInfoManager::getAppInfosWithState(sched_policy::AppState appState)
{
    std::vector<AppInfo> appInfos;
    for (const auto &appInfoPair : m_appInfos) {
        for (const auto &appInfo : appInfoPair.second) {
            if (appInfo.appState() == appState) {
                appInfos.emplace_back(appInfo);
            }
        }
    }

    return appInfos;
}

AppInfo AppInfoManager::getLatestAppInfoByDesktopFile(
    const std::string &desktopFile, const std::vector<std::string> &args)
{
    std::string desktopFileId = appinfo_helper::generateDesktopFileId(desktopFile);
    if (m_appInfos.count(desktopFileId) == 0) {
        return AppInfo();
    }

    auto appInfos = m_appInfos.at(desktopFileId);
    if (appInfos.empty()) {
        return AppInfo();
    }

    auto it = std::find_if(appInfos.rbegin(), appInfos.rend(), [args](const AppInfo &appInfo) {
        return appInfo.args() == args && !appInfo.pids().empty();
    });

    if (it != appInfos.rend()) {
        return *it;
    }

    auto cmdline = appinfo_helper::getCmdlineFromDesktopFile(desktopFile, args);

    return getLatestAppInfoByCmdline(cmdline);
}

AppInfo AppInfoManager::getLatestAppInfoByCmdline(const std::string &cmdline)
{
    auto predicate = [cmdline](const AppInfo &appInfo) {
        return appInfo.cmdline() == cmdline;
    };
    auto appExistsAndIt = findAppInfo(predicate);

    if (std::get<0>(appExistsAndIt)) {
        auto it = std::get<1>(appExistsAndIt);
        return *it;
    }

    return AppInfo();
}

AppInfo AppInfoManager::getAppInfo(const std::string &appId)
{
    auto predicate = [appId](const AppInfo &appInfo) {
        return appInfo.desktopFilePid() == appId;
    };
    auto appExistsAndIt = findAppInfo(predicate);

    if (std::get<0>(appExistsAndIt)) {
        auto it = std::get<1>(appExistsAndIt);
        return *it;
    }

    return AppInfo();
}

std::string AppInfoManager::getDesktopFileByPid(int pid)
{
    auto appExistsAndIt = findAppInfoWithPid(pid);
    if (std::get<0>(appExistsAndIt)) {
        return std::get<1>(appExistsAndIt)->desktopFile();
    }

    return std::string();
}

void AppInfoManager::setBackgroundToCachedStateInterval(int interval)
{
    if (m_backgroundToCachedStateInterval == interval) {
        return;
    }

    m_backgroundToCachedStateInterval = interval;
    resetBackgroundStateTimers();
}

void AppInfoManager::doHandleActiveWindowChanged()
{
    if (m_currentActiveWid == m_preActiveWid) {
        return;
    }

    if (!appinfo_helper::isFromSameProcess(m_currentActiveWid, m_preActiveWid)) {
        appBecomeActiveWithWid(m_currentActiveWid);
        appBecomeInactiveWithWid(m_preActiveWid);
    } else {
        auto appExistsAndIt = findAppInfoWithWid(m_currentActiveWid);

        if (std::get<0>(appExistsAndIt)) {
            auto it = std::get<1>(appExistsAndIt);
            it->setActive(m_currentActiveWid, true);
            it->setActive(m_preActiveWid, false);
        }
    }

    m_preActiveWid = m_currentActiveWid;
}

void AppInfoManager::addWidToAppInfo(AppInfoIterator appInfoIt, const std::string &wid)
{
    if (appInfoIt->wids().empty()) {
        updateAppCmdlineByWid(appInfoIt, wid);
    }

    appInfoIt->appendWid(wid);
}

void AppInfoManager::updateAppCmdlineByWid(AppInfoIterator appInfoIt, const std::string &wid)
{
    const auto window = appinfo_helper::getWindowInfoByWid(wid);
    int pid = window ? window->pid : -1;
    appInfoIt->setCmdline(process_info_helper::cmdline(pid));
}

void AppInfoManager::createAppInstanceWithWid(const std::string &wid)
{
    AppInfo appInfo = std::move(generateAppInfoByWid(wid));
    if (appInfo.desktopFilePid().empty()) {
        PRINT_DBG("Generate app info failed with the wid %s.\n", wid.c_str());
        return;
    }

    m_appInfos[appInfo.desktopFileId()].emplace_back(std::move(appInfo));

    if (m_appStartedCallback) {
        m_appStartedCallback(appInfo);
    }
}

void AppInfoManager::appBecomeActiveWithWid(const std::string &wid)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);

    if (std::get<0>(appExistsAndIt)) {
        auto it = std::get<1>(appExistsAndIt);
        setAppState(sched_policy::AppState::Focus, it);
        it->setActive(wid, true);
    } else {
        handleWindowAdded(wid);
    }
}

void AppInfoManager::appBecomeInactiveWithWid(const std::string &wid)
{
    auto appExistsAndIt = findAppInfoWithWid(wid);
    if (!std::get<0>(appExistsAndIt)) {
        return;
    }

    auto it = std::get<1>(appExistsAndIt);
    auto state = it->isAllWinsMinimize() ? sched_policy::AppState::Background : sched_policy::AppState::Foreground;
    setAppState(state, it);
    it->setActive(wid, false);
}

void AppInfoManager::updateAppStateWithoutEmptyWindows(AppInfoIterator appInfoIt)
{
    sched_policy::AppState
        state = appInfoIt->isAllWinsMinimize() ? sched_policy::AppState::Background : sched_policy::AppState::Foreground;
    setAppState(state, appInfoIt);
}

void AppInfoManager::updateAppInfoWithEmptyWindows(AppInfoIterator appInfoIt)
{
    if (appInfoIt->pids().empty()) {
        auto &appInfoVector = m_appInfos[appInfoIt->desktopFileId()];
        m_appFinishedCallback(*appInfoIt);
        appInfoVector.erase(std::remove(appInfoVector.begin(), appInfoVector.end(), *appInfoIt));
    } else {
        // tray icon app
        appInfoIt->setIsSystemTrayIconApp(true);
        setAppState(sched_policy::AppState::WinsNone, appInfoIt);
    }
}

void AppInfoManager::setAppState(sched_policy::AppState appState, AppInfoIterator appInfoIt)
{
    PRINT_DBG("AppInfoManager App %s state changed to %d, App Id : %s start", appInfoIt->name().c_str(), static_cast<int>(appState), appInfoIt->desktopFilePid().c_str());
    if (appInfoIt->appState() == appState) {
        return;
    }

    // 只有当后台状态转换成缓存或者服务状态是根据持续时间自动进行转换的
    // 其他的状态均依赖应用窗口的状态
    if (appState != sched_policy::AppState::Cached && appState != sched_policy::AppState::Service) {
        deleteStateChangeTimerId(appInfoIt->desktopFilePid());
    }

    appInfoIt->setAppState(appState);
    PRINT_DBG("AppInfoManager App %s state changed to %d, App Id : %s", appInfoIt->name().c_str(), static_cast<int>(appState), appInfoIt->desktopFilePid().c_str());
    if (!canChangeState(*appInfoIt)) {
        return;
    }

    if (appState == sched_policy::AppState::Background) {
        int timerId = setBackgroundStateTimer(appInfoIt->desktopFilePid());
        addStateChangedTimerId(appInfoIt->desktopFilePid(), timerId);
    }
}

unsigned long long AppInfoManager::setBackgroundStateTimer(const std::string &appId)
{
    PRINT_DBG("AppInfoManager Set background state timer for app %s", appId.c_str());
    auto timerId = Singleton<timewheel::TimeWheel>::instance().addTimer(
        m_backgroundToCachedStateInterval, [this, appId]() {
            PRINT_DBG("AppInfoManager Set background state timer for app %s 1", appId.c_str());
            auto appExistsAndIt = findAppInfoWithDesktopFilePid(appId);
            if (!std::get<0>(appExistsAndIt)) {
                return;
            }
            PRINT_DBG("AppInfoManager Set background state timer for app %s 2", appId.c_str());
            auto it = std::get<1>(appExistsAndIt);
            auto appState = canBeCachedState(it) ? sched_policy::AppState::Cached : sched_policy::AppState::Service;
            setAppState(appState, it);
        });

    return timerId;
}

bool AppInfoManager::canBeCachedState(AppInfoIterator appInfoIt) const
{
    return !m_selectionChecker.hasSelection(appInfoIt->pids()) && !appInfoIt->isSystemTrayIconApp();
}

void AppInfoManager::resetBackgroundStateTimers()
{
    auto appIds = appIdsWithTimer();
    for (const auto &appId : appIds) {
        deleteStateChangeTimerId(appId);
        unsigned long long timerId = setBackgroundStateTimer(appId);
        addStateChangedTimerId(appId, timerId);
    }
}

std::vector<std::string> AppInfoManager::appIdsWithTimer()
{
    std::vector<std::string> appIds;
    for (const auto &appTimerId : m_appStateChangeTimerIds) {
        appIds.emplace_back(appTimerId.first);
    }
    return appIds;
}

AppInfo AppInfoManager::generateAppInfoByWid(const std::string &wid)
{
    const auto window = appinfo_helper::getWindowInfoByWid(wid);
    if (!window)
        return AppInfo();

    int pid = window->pid;
    if (pid <= 0)
        return AppInfo();

    std::string desktopFile = m_desktopFileManager.syncGetDesktopFileByAppId(window->appId);

    if (desktopFile.empty()) {
        PRINT_DBG("Get desktop file failed by appId %s.\n", window->appId.c_str());
        desktopFile = m_desktopFileManager.syncGetDesktopFileByPid(pid);
    }

    if (desktopFile.empty()) {
        PRINT_DBG("Get desktop file failed by pid %d.\n", pid);
        desktopFile = m_desktopFileManager.findDesktopFileByTitle1(wid, pid);
    }
    if (desktopFile.empty()) {
        PRINT_DBG("Get desktop file failed by wid %s.\n", wid.c_str());
        return AppInfo();
    }

    auto appType = getAppTypeByDesktopFile(desktopFile);
    auto appState = sched_policy::AppState::Foreground;
    int64_t launchTimestap = QDateTime::currentSecsSinceEpoch();
    PRINT_DBG("Generate app info by wid: %s, desktopFile: %s, pid: %d, appId: %s\n", wid.c_str(), desktopFile.c_str(), pid, window->appId.c_str());
    AppInfo appInfo(desktopFile, window->appId, pid, launchTimestap, appType, appState, [this](const AppInfo &appInfo) {
        if (m_appStateChangedCallback) {
            m_appStateChangedCallback(appInfo);
        }
    });
    appInfo.setAppWinsChangedCallback([this](AppInfo &appInfo, const std::string &wid){
        if (m_appWinsChangedCallback){
            m_appWinsChangedCallback(appInfo, wid);
        }
        appInfo.clearWinsInstantState();
    });
    auto mprisService = takeMprisServiceFromCache(pid);
    int cachedPid = takeSystemTrayIconAppFromCache(pid);
    // 只有在窗口添加时（windowadded）才会生成应用信息，默认状态设置为Foreground
    // 具体真实状态依赖窗口状态改变的事件
    auto childrenPids = process_info_helper::childPids(pid);
    appInfo.appendWid(wid);
    appInfo.setMrprisDbusService(mprisService);
    appInfo.setIsSystemTrayIconApp(cachedPid > 0);
    appInfo.setCmdline(process_info_helper::cmdline(pid));

    return appInfo;
}

AppInfo AppInfoManager::generateAppInfo(const std::string &desktopFile, int pid)
{
    if (desktopFile.empty() || pid <= 0) {
        return AppInfo();
    }

    auto appType = getAppTypeByDesktopFile(desktopFile);
    auto appState = sched_policy::AppState::Foreground;
    int64_t launchTimestap = QDateTime::currentSecsSinceEpoch();
    return AppInfo{desktopFile, std::string(), pid, launchTimestap, appType, appState, [this](const AppInfo &appInfo) {
        if (m_appStateChangedCallback) {
            m_appStateChangedCallback(appInfo);
        }
    }};
}

std::tuple<bool, AppInfoManager::AppInfoIterator> AppInfoManager::findAppInfo(const FindAppInfoPredicateCallback &predicateCallback)
{
    for (auto &appInfos : m_appInfos) {
        auto it = std::find_if(appInfos.second.begin(), appInfos.second.end(),
                               [&predicateCallback](const AppInfo &appInfo) {
            return predicateCallback(appInfo);
        });
        if (it != appInfos.second.end()) {
            return {true, it};
        }
    }

    return {false, AppInfoIterator()};
}

bool AppInfoManager::hasAppInfoContainsPid(const FindAppInfoPredicateCallback &predicateCallback)
{
    for (auto &appInfos : m_appInfos) {
        auto it = std::find_if(appInfos.second.begin(), appInfos.second.end(),
                               [&predicateCallback](const AppInfo &appInfo) {
            return predicateCallback(appInfo);
        });
        if (it != appInfos.second.end()) {
            return true;
        }
    }
    return false;
}

std::tuple<bool, AppInfoManager::AppInfoIterator> AppInfoManager::findAppInfoWithWid(const std::string &wid)
{
    auto predicate = [wid](const AppInfo &appInfo) {
        return appInfo.containsWid(wid);
    };
    return findAppInfo(predicate);
}

// AppInfo 中保存的 pid 是所有子进程的 pid
// 但是，两个应用的 pid 之间可能会存在父子关系，比如终端中打开应用
// 因此匹配 pid 的规则要稍微严格一些：
// 1. 先匹配 launcher pid，如果相同，那么肯定是同一个应用
// 2. 如果 launcher pid 不一致，可能就是存在父子关系的两个应用，先尝试找到需要匹配的应用 desktop 文件
//    如果两个 desktop 文件一致，并且 pid 存在父子关系，那么就认为是同一个应用，否则认为是两个应用
std::tuple<bool, AppInfoManager::AppInfoIterator> AppInfoManager::findAppInfoWithPid(int pid)
{
    auto predicate = [pid](const AppInfo &appInfo) {
        return appInfo.launcherPid() == pid;
    };
    auto existsAndAppInfoIt = findAppInfo(predicate);
    if (std::get<0>(existsAndAppInfoIt)) {
        // qDebug() << QString("findAppInfoWithPid: found app with launcher pid %1, appId: %2, desktopFile: %3")
        //                 .arg(pid)
        //                 .arg(QString::fromStdString(std::get<1>(existsAndAppInfoIt)->appId()))
        //                 .arg(QString::fromStdString(std::get<1>(existsAndAppInfoIt)->desktopFile()));
        PRINT_DBG("findAppInfoWithPid: found app with launcher pid %d, appId: %s, desktopFile: %s", pid, std::get<1>(existsAndAppInfoIt)->appId().c_str(), std::get<1>(existsAndAppInfoIt)->desktopFile().c_str());
        return existsAndAppInfoIt;
    }

    auto predicate3 = [pid](const AppInfo &appInfo) {
        return appInfo.containsPid(pid);
    };

    if(!hasAppInfoContainsPid(predicate3)) {
        return {false, AppInfoIterator()};
    }

    auto desktopFile = m_desktopFileManager.syncGetDesktopFileByPid(pid);
    auto predicate2 = [pid, desktopFile](const AppInfo &appInfo) {
        return appInfo.containsPid(pid) && appInfo.desktopFile() == desktopFile;
    };
    return findAppInfo(predicate2);
}

std::tuple<bool, AppInfoManager::AppInfoIterator> AppInfoManager::findAppInfoWithAppId(const std::string &appId)
{
    auto predicate = [appId](const AppInfo &appInfo) {
        return appInfo.appId() == appId;
    };
    return findAppInfo(predicate);
}

std::tuple<bool, AppInfoManager::AppInfoIterator> AppInfoManager::findAppInfoWithDesktopFilePid(const std::string &desktopFilePid)
{
    auto predicate = [desktopFilePid](const AppInfo &appInfo) {
        return appInfo.desktopFilePid() == desktopFilePid;
    };
    return findAppInfo(predicate);
}

void AppInfoManager::addStateChangedTimerId(const std::string &appId, unsigned long long timerId)
{
    m_appStateChangeTimerIds[appId] = timerId;
}

void AppInfoManager::deleteStateChangeTimerId(const std::string &appId)
{
    auto it = m_appStateChangeTimerIds.find(appId);
    if (it != m_appStateChangeTimerIds.end()) {
        Singleton<timewheel::TimeWheel>::instance().deleteTimer(m_appStateChangeTimerIds[appId]);
        m_appStateChangeTimerIds.erase(it);
    }
}

void AppInfoManager::saveMprisServiceToCache(int pid, const std::string &service)
{
    m_mprisServices[pid] = service;
}

std::string AppInfoManager::takeMprisServiceFromCache(int pid)
{
    if (m_mprisServices.find(pid) != m_mprisServices.end()) {
        return m_mprisServices[pid];
    }
    return std::string();
}

void AppInfoManager::saveSystemTrayIconAppToCache(int pid)
{
    auto it = std::find(m_systemTrayIconApps.begin(), m_systemTrayIconApps.end(), pid);
    if (it == m_systemTrayIconApps.end()) {
        m_systemTrayIconApps.emplace_back(pid);
    }
}

int AppInfoManager::takeSystemTrayIconAppFromCache(int pid)
{
    auto it = std::find(m_systemTrayIconApps.begin(), m_systemTrayIconApps.end(), pid);
    if (it != m_systemTrayIconApps.end()) {
        int pid = *it;
        m_systemTrayIconApps.erase(it);
        return pid;
    }

    return 0;
}

bool AppInfoManager::canChangeState(const AppInfo &appInfo)
{
    return appInfo.appType() == sched_policy::AppType::Normal;
}