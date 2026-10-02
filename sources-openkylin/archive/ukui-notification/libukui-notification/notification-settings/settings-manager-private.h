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

#ifndef NotificationSettingsPrivate_H
#define NotificationSettingsPrivate_H

#include "settings-manager.h"
#include <application-info.h>
#include <QJsonObject>
#include <QFileSystemWatcher>
#include <QList>
#include "settings-properties.h"

namespace UkuiNotification {
    /**
 * @brief NOTIFICATION_SETTINGS_VERSION
 * 通知配置文件版本号，用于判断是否需要更新配置文件。
 * 1.0 -> 2.0 -> 2.1
 * 1.0版本：存储所有应用通知配置
 * 2.0版本: 清空旧版本版本配置，按需增加应用通知配置
 * 2.1版本: 增加通知配置来源属性
 */
#define NOTIFICATION_SETTINGS_VERSION "2.1"

class SettingsManagerPrivate : public  QObject
{
    friend class SettingsManager;
    Q_OBJECT
public:
    explicit SettingsManagerPrivate(QObject* parent = nullptr);
    ~SettingsManagerPrivate();
    void init();

private Q_SLOTS:
    /**
     * @brief checkAndLoad
     * 检查通知配置文件，不存在则初始化生成默认配置文件，存在则根据配置文件版本判断是否更新配置文件。
     */
    void checkAndLoad();
private:
    /**
     * @brief createSettingsFile
     * 初始化创建通知配置文件。
     */
    void createSettingsFile();

    /**
     * @brief updateSettingsFile
     * 更新通知配置文件。
     */
    bool updateSettingsFile(const QString& oldVersion, const QString& newVersion, QJsonObject& data);

    /**
     * @brief getAppInfo
     * 从搜索应用数据库服务中获取desktop信息。
     */
    QStringList getAppInfo();

    void initSettingsData(QJsonObject &data);
    void checkApplicationList(QJsonObject &data);

    QJsonObject loadAllowListData();

public Q_SLOTS:
    void desktopFileDelete(const QStringList &data);
Q_SIGNALS:
    void settingsUpdateFinished();
    void appUninstalled(const QString &desktopEntry);

private:
    void save2SettingsFile(const QJsonObject &jsonDocData);
    QJsonObject m_settingsData;
    QFileSystemWatcher *m_watcher = nullptr;
    UkuiSearch::ApplicationInfo& m_applicationInfo;
    QJsonObject m_applicationDefaultSettings;
    bool m_needCheckApplist = true;
    const QJsonObject m_allowListData;
};

}
#endif // NotificationSettingsPrivate_H
