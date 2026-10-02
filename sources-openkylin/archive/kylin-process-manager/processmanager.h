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

#include <memory>
#include <QObject>
#include <QApplication>

#include "applaunchmanager.h"
#include "applaunchmanagerservice.h"
#include "appwhitelist.h"
#include "appwhitelistservice.h"

#include "eventwatcher.h"
#include "exceptiongroupprocesswatcher.h"

#include "datacontext.h"

#include "statemanager.h"

#include "scenemanager.h"

#include "cgroupcontrolinterface.h"

class ProcessManager : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    ProcessManager();
    ~ProcessManager();

    void ThawProcess(int pid);
    void ThawFrozenProcesses();

private:
    void initWatcher();
    void connectAppLauncherSignals();
    bool canFrozenProcess();

private:
    // 上下文
    std::shared_ptr<DataContext> m_dataContext;

    // 监视
    std::unique_ptr<EventWatcher> m_eventWatcher;
    std::unique_ptr<ExceptionGroupProcessWatcher> m_exceptionGroupWatcher;

    // 场景
    std::unique_ptr<SceneManager> m_sceneManager;

    CgroupControlInterface m_cgroupControlInterface;

public:
    // 白名单
    std::unique_ptr<AppWhitelist> app_whitelist;
    std::unique_ptr<AppWhitelistService> app_whitelist_service;

    // 启动
    std::shared_ptr<AppLaunchManager> app_launch_manager;
    std::unique_ptr<AppLaunchManagerService> app_launch_manager_service;

    std::vector<QMetaObject::Connection> m_conns;
};

#endif // PROCESSMANAGER_H
