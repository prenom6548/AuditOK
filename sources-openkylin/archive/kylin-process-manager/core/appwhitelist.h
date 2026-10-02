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

#ifndef APPWHITELIST_H
#define APPWHITELIST_H

#include "configmanager.h"
#include <QObject>

class AppWhitelist : public QObject
{
    Q_OBJECT
public:
    explicit AppWhitelist(ConfigManager &configManager);

    // dbus service interfaces
    /**
     * @brief AddApp 将指定的应用程序添加到白名单中。
     * @param desktopFile 应用程序的 .desktop 文件路径。
     * @return 如果添加成功返回 true，否则返回 false。
     */
    bool AddApp(const QString &desktopFile);
    /**
     * @brief RemoveApp 从白名单中移除指定的应用程序。
     * @param desktopFile 应用程序的 .desktop 文件路径。
     * @return 如果移除成功返回 true，否则返回 false。
     */
    bool RemoveApp(const QString &desktopFile);
    /**
     * @brief AppList 获取当前白名单中的所有应用程序列表。
     * @return 返回一个包含所有白名单应用程序 .desktop 文件路径的字符串列表。
     */
    QStringList AppList() const;

private:
    ConfigManager &m_configManager;
};

#endif // APPWHITELIST_H
