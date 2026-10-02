/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_NOTIFICATION_APPLICATIONS_SETTINGS_H
#define UKUI_NOTIFICATION_APPLICATIONS_SETTINGS_H
#include <QObject>
#include <QJsonObject>
#include "single-application-settings.h"
#include "popup-notification.h"
namespace UkuiNotification{
typedef QMap<QString, SingleApplicationSettings*> ApplicationsSettingsMap;
class ApplicationsSettingsPrivate;
/**
 * @short 用于获取应用的设置.
 * @author iaom
 */
class ApplicationsSettings : public QObject
{
    Q_OBJECT
public:
    static ApplicationsSettings *self();
    ~ApplicationsSettings();

    /**
     * 获取某条通知的配置
     * @param notification 表示一条通知
     * @return 返回的指针不需要使用者回收
     */
    SingleApplicationSettings *creatSettings(const PopupNotification &notification);
    SingleApplicationSettings *creatSettings(QString desktopEntry);
    ApplicationsSettingsMap &getAllApplicationsSettings();
    /**
     * @brief getAppSettings
     * 获取当前用户某个应用通知相关配置信息。
     * @param appDesktopName 传"default"获取配置默认值
     */
    QJsonObject getAppSettings(const QString &appDesktopName);
    bool setAppSetting(const QString &appDesktopName, SettingsProperty::Property key, const QVariant &value);

    bool hasAppSettings(const QString &appDesktopName);
    bool hasAppSettings(const PopupNotification &notification);
    /**
     * @brief 是否在白名单内
     * @param
     * @return
     */
    bool isInAllowList(const QString &appDesktopName);
    bool isInAllowList(const PopupNotification &notification);

Q_SIGNALS:
    void applicationUninstalled(const QString &desktopEntry);
    void settingsUpdated(const QString& desktopEntry);
    void applicationInstalled(const QString &desktopEntry);
private Q_SLOTS:

private:
    ApplicationsSettings(QObject *parent = nullptr);
    void settingsDataChanged();
    void onApplicationUninstalled(const QString &desktopEntry);
    ApplicationsSettingsPrivate *d = nullptr;
};
}

#endif //UKUI_NOTIFICATION_APPLICATIONS_SETTINGS_H
