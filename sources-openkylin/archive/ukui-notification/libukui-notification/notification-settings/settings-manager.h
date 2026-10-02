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


#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include "settings-properties.h"

namespace UkuiNotification {

class SettingsManagerPrivate;
class  SettingsManager : public QObject
{
    Q_OBJECT
public:
    static SettingsManager *self();

    ~SettingsManager() = default;

    /**
     * 获取全局配置
     */
    QJsonObject getGlobalSettings();

    /**
     * @brief getAllAppSettings
     * 获取当前用户全部应用通知相关配置信息。
     */
    QJsonObject getAllAppSettings();

    /**
     * @brief getAppSettings
     * 获取当前用户某个应用通知相关配置信息。
     * @param appDesktopName 传"default"获取配置默认值
     */
    QJsonObject getAppSettings(const QString &appDesktopName);

    /**
     * 获取应用通知设置默认值
     * @return
     */
    QJsonObject getAppDefaultSettings();

    /**
     * 获取默认应用列表
     * @return
     */
    void getAppList(QStringList &defaultAppList, QStringList &otherAppList);

    bool setGlobalSettings(SettingsProperty::Property key, const QVariant &value);
    bool setGlobalSettings(const QList<QPair<SettingsProperty::Property, QVariant>> &values);
    bool setAppSettings(const QString &appDesktopName, SettingsProperty::Property key, const QVariant &value);
    bool addAppSettings(const QString &appDesktopName, const QString &configurationSource = "Default");
    bool deleteAppSettings(const QString &appDesktopName);
    bool hasAppSettings(const QString &appDesktopName);
    bool isInAllowList(const QString &appDesktopName);
    bool deleteAppSettingsByConfigurationSource(const QString &configurationSource);
    QStringList getAppListByConfigurationSource(const QString &configurationSource) const;
    QStringList getAllApps() const;

Q_SIGNALS:
    void settingsDataChanged();
    void newAppInstalled(const QString &desktopEntry);
    void appUninstalled(const QString &desktopEntry);

private:
    SettingsManager(QObject *parent = nullptr);
    SettingsManagerPrivate * d = nullptr;

};
}

#endif
