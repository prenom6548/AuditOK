/*
 * libkysdk-appmanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KAPPMANAGER_H
#define KAPPMANAGER_H

#include <QStringList>
#include "appinfo.h"

namespace kdk {

/**
 * @defgroup ApplicationsModule
 */

/**
 * @brief 应用行为控制类，提供应用行为控制功能如设置开机自启，获取/设置默认应用等
 * @since 3.0.1.0
 */

class KAppManager
{
public:
    enum KAppType
    {
        AUDIO,
        VIDEO,
        IMAGE,
        TEXT,
        BROWSER,
        EMAIL
    };

public:
    KAppManager();

public:
    
    /**
     * @brief 获取系统所有应用程序
     * 
     * @return QList<AppInfo*> 应用程序列表
     * @since 3.0.1.0
     */
    QList<AppInfo*> getApplications();
    
    /**
     * @brief 根据mime类型查找可用应用程序列表
     * 
     * @param mimeType mime类型
     * @return QList<AppInfo*> 应用程序列表
     * @since 3.0.1.0
     */
    QList<AppInfo*> getApplicationByMime(const QString &mimeType);

    /**
     * @brief 获取开机自启的应用程序列表
     * 
     * @return QList<AppInfo*> 应用程序列表
     * @since 3.0.1.0
     */
    QList<AppInfo *> getAutoStartApplications();

    /**
     * @brief 添加开机自启动应用
     * 
     * @param desktopFullPath desktop文件绝对路径
     * @return bool 成功返回true
     * @since 3.0.1.0
     */
    bool           addAutoStartApplication(const QString &desktopFullPath);

    /**
     * @brief 删除开机自启动应用
     * 
     * @param desktop desktop文件，不带路径。如:atril.desktop
     * @return bool 成功返回true
     * @since 3.0.1.0
     */
    bool           removeAutoStartApplication(const QString &desktop);

    /**
     * @brief 按照指定的mime类型获取默认应用
     * 
     * @param mimeType mime类型
     * @return AppInfo* 应用程序信息
     * @since 3.0.1.0
     */
    AppInfo*       getDefaultApplicationByMime(const QString &mimeType);

    /**
     * @brief 根据指定应用枚举类型获取默认应用
     * 
     * @param type 应用类型枚举AppType的值
     * @return AppInfo* 应用信息
     * @since 3.0.1.0
     */
    AppInfo*       getDefaultApplicationByType(int type);

    /**
     * @brief 根据指定mime类型设置默认应用
     * 
     * @param desktopFullPath desktop文件绝对路径
     * @param mimeType mime类型
     * @return bool 成功返回true
     * @since 3.0.1.0
     */
    bool           setDefaultApplicationByMime(const QString &desktopFullPath, const QString &mimeType);

    /**
     * @brief 根据应用类型枚举设置默认应用
     * 
     * @param desktopFullPath desktop文件绝对路径
     * @param type 应用类型枚举AppType的值
     * @return bool 成功返回true
     * @since 3.0.1.0
     */
    bool           setDefaultApplicationByType(const QString &desktopFullPath, int type);

private:
    QString        findDesktopFile(const QStringList &locations, const QString &desktop);
};
}

#endif // KAPPMANAGER_H
