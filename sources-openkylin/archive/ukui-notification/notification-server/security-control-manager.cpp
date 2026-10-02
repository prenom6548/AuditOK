/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "security-control-manager.h"

#include <notification-settings/settings-manager.h>

#include "security-control-config.h"
#include "utils.h"
using namespace UkuiNotification;
SecurityControlManager::SecurityControlManager(const std::unique_ptr<NotificationControlRegister>& controlRegister, QObject *parent)
    : QObject(parent)
    , m_controlRegister(controlRegister)
{
    connect(&SecurityControlConfig::instance(), &SecurityControlConfig::configChanged,
            this, &SecurityControlManager::onSecurityControlConfigChanged);
    connect(&SecurityControlConfig::instance(), &SecurityControlConfig::exitControl,
            this, &SecurityControlManager::onSecurityControlExit);
    updateSecurityControlApps();
}

bool SecurityControlManager::isNotificationAllowed(const QString& desktopEntry) const
{
    return SecurityControlConfig::instance().isNotificationAllowed(desktopEntry).value_or(true);
}

bool SecurityControlManager::isEnabledNotification() const
{
    return SecurityControlConfig::instance().isEnabledAllNotification();
}

void SecurityControlManager::onSecurityControlConfigChanged()
{
    updateSecurityControlApps();
}

void SecurityControlManager::onSecurityControlExit()
{
    unregisterSecurityControlApps();
}

void SecurityControlManager::updateSecurityControlApps()
{
    const QStringList controlledApps = SecurityControlConfig::instance().getAppList();
    //删除的域控app
    {
        //已注册的域控app列表
        const QStringList domainApps = SettingsManager::self()->getAppListByConfigurationSource(
            NotificationControlRegister::registerModeToString(NotificationControlRegister::RegisterMode::Domain));
        //需要删除的域控app列表
        const QStringList appsToDelete = StringListDiff(domainApps, controlledApps);
        for (const QString& appName : appsToDelete) {
            m_controlRegister->unRegisterNotificationControl(appName);
        }
    }

    //新增的域控app列表
    {
        //当前已注册app列表
        const QStringList currentApps = SettingsManager::self()->getAllAppSettings().keys();
        //需要注册的app列表
        const auto registeredApps = StringListDiff(controlledApps, currentApps);
        //全量app列表
        const auto allAppList = SettingsManager::self()->getAllApps();
        const auto allAppsSet = QSet<QString>(allAppList.begin(), allAppList.end());
        for (const QString& appName : registeredApps) {
            if (allAppsSet.contains(appName)) {
                m_controlRegister->registerNotificationControl(appName, NotificationControlRegister::RegisterMode::Domain);
            }
        }
    }
}

void SecurityControlManager::unregisterSecurityControlApps() const
{
    m_controlRegister->unRegisterNotificationControlByMode(NotificationControlRegister::RegisterMode::Domain);
}
