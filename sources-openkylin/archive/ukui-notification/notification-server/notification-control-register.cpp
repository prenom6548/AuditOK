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

#include <QDebug>
#include <QTimer>
#include "notification-control-register.h"
#include "notification-settings/settings-manager.h"
#include "notification.h"
#include "notification-manager.h"
#include "utils.h"

using namespace NotificationServer;
NotificationControlRegister::NotificationControlRegister(QObject *parent)
    : QObject{parent}
{

}

void NotificationControlRegister::registerNotificationControl(const QString &desktopEntry, RegisterMode mode)
{
    if (desktopEntry.isEmpty()) {
        qWarning() << "registerNotificationControl filed :" << "desktop entry is empty .";
        return;
    }
    //白名单检测
    if (UkuiNotification::SettingsManager::self()->isInAllowList(desktopEntry)) {
        qDebug() << desktopEntry << "notification configuration is in allow list";
        return;
    }
    //配置存在
    if (UkuiNotification::SettingsManager::self()->hasAppSettings(desktopEntry)) {
        return;
    }

    qDebug() << "need add settings: " << desktopEntry << "notification configuration does not exist";
    //添加配置
    if (!UkuiNotification::SettingsManager::self()->addAppSettings(desktopEntry, registerModeToString(mode))) {
        qWarning() << "Failed to add app settings :" << desktopEntry;
        return;
    }

    if (mode == RegisterMode::Notice) {
        QTimer::singleShot(6000, [desktopEntry]() {
            const QString& desktopFullPath = UkuiNotification::Utils::desktopEntryFromName(desktopEntry);
            if (desktopFullPath.isEmpty()) {
                qWarning() << "Failed to query desktop files through desktop entry :" << desktopEntry;
                return;
            }
            QString appName = UkuiNotification::Utils::appNameForDesktopEntry(desktopFullPath);
            if (appName.isEmpty()) {
                qWarning() << "Failed to retrieve app name, using desktop entry." << desktopFullPath ;
                appName = desktopEntry;
            }

            auto notification = std::make_shared<Notification>();
            notification->setAppName(tr("System notification"));
            notification->setAppIcon("ukui-control-center");
            notification->setSummary(tr("Whether to allow  \"%1\"  to send system notifications").arg(appName));
            notification->setBody(tr("If not, you will no longer receive any prompt messages from the app"));

            //hints
            QVariantMap hints;
            hints.insert("urgency", 2);
            notification->setHints(hints);

            //actions
            auto* disallowAction = notification->addAction("Disallow", tr("Disallow"));
            auto* allowAction = notification->addAction("Allow", tr("Allow"));
            notification->send();

            auto actionOption = [desktopEntry] (NotificationAction* action, bool isAllow)
            {
                if (action) {
                    connect(action, &NotificationAction::actionActived, [desktopEntry, isAllow] () {
                        UkuiNotification::SettingsManager::self()->setAppSettings(desktopEntry, UkuiNotification::SettingsProperty::AllowNotify, isAllow);
                    });
                }
            };

            actionOption(disallowAction, false);
            actionOption(allowAction, true);
        });
    }
}

void NotificationControlRegister::unRegisterNotificationControl(const QString &desktopEntry)
{
    UkuiNotification::SettingsManager::self()->deleteAppSettings(desktopEntry);
}

void NotificationControlRegister::updateNotificationControl(const QString& desktopEntry, RegisterMode mode)
{
    UkuiNotification::SettingsManager::self()->setAppSettings(desktopEntry, UkuiNotification::SettingsProperty::ConfigurationSource ,registerModeToString(mode));
}

void NotificationControlRegister::unRegisterNotificationControlByMode(const RegisterMode mode) const
{
    UkuiNotification::SettingsManager::self()->deleteAppSettingsByConfigurationSource(registerModeToString(mode));
}

