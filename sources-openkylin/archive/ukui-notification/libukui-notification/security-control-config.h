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
 * Authors: maomao <baijunjie@kylinos.cn>
 */

#ifndef UKUI_NOTIFICATION_SECURITYCONTROLCONFIG_H
#define UKUI_NOTIFICATION_SECURITYCONTROLCONFIG_H

#include <optional>
#include <QObject>
#include "control-config.h"
#include "notification-settings/settings-properties.h"
namespace UkuiNotification {
class SecurityControlConfig : public QObject
{
    Q_OBJECT
public:
    static SecurityControlConfig& instance();

    [[nodiscard]] bool isEnabledAllNotification() const;
    [[nodiscard]] std::optional<bool> isNotificationAllowed(const QString &desktopEntry) const;
    [[nodiscard]] bool isNotificationSwitchEditable(const QString &desktopEntry) const;
    [[nodiscard]] std::optional<bool> isShowNotificationOnLockScreenAllowed(const QString &desktopEntry) const;
    [[nodiscard]] bool isShowNotificationSwitchOnLockScreenEditable(const QString &desktopEntry) const;

    [[nodiscard]] QStringList getAppList() const;
public Q_SLOTS:
    void onControlConfigChanged(const QString &name);
    void onControlExited();

Q_SIGNALS:
    void configChanged(SettingsProperty::Property property);
    void exitControl();

private:
    explicit SecurityControlConfig(QObject *parent = nullptr);
    void updateControlConfig(const QString &name);

    const QMap<QString, SettingsProperty::Property> m_configKeyToPropertyMap{
        {"disableAllNotifications", SettingsProperty::ReceiveNotificationsFromApps},
        {"allowNotify", SettingsProperty::AllowNotify},
        {"showNotificationOnLockScreen", SettingsProperty::ShowNotificationOnLockScreen},
    };

    QMap<SettingsProperty::Property, std::shared_ptr<ControlConfig>> m_controlConfigMap;
    //禁用通知总开关
    bool m_disabledAllNotification{false};
};
}


#endif //UKUI_NOTIFICATION_SECURITYCONTROLCONFIG_H
