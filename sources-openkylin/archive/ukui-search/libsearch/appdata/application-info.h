/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
#ifndef APPLICATIONINFO_H
#define APPLICATIONINFO_H

#include <QObject>

#include "application-property.h"
namespace UkuiSearch {
class ApplicationInfoPrivate;
class ApplicationInfo : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationInfo(QObject *parent = nullptr);
    ~ApplicationInfo();
    /**
     * @brief ApplicationInfo::getInfo 查询单个应用的单个属性
     * @param desktopFile: 要查询的应用的desktop文件路径
     * @param property: 要查询的属性
     * @return
     */
    QVariant getInfo(const QString &desktopFile, ApplicationProperty::Property property);
    /**
     * @brief ApplicationInfo::getInfo 查询单个应用的多个属性
     * @param desktopFile: 要查询的应用的desktop文件路径
     * @param properties: 要查询的属性
     * @return
     */
    ApplicationPropertyMap getInfo(const QString &desktopFile, ApplicationProperties properties);
    /**
     * @brief ApplicationInfo::getInfo 查询所有应用的多个属性
     * @param properties: 要查询的属性
     * @return
     */
    ApplicationInfoMap getInfo(ApplicationProperties properties);

    /**
     * @brief ApplicationInfo::getInfo
     * 获取满足限制条件restrictions的指定应用属性
     * @param restrictions: 限制条件
     * @param properties: 返回的应用信息会携带这些属性
     * @return ApplicationInfoMap: 结果信息
     */
    ApplicationInfoMap getInfo(ApplicationProperties properties, ApplicationPropertyMap restrictions);

    /**
     * @brief ApplicationInfo::searchApp
     * 根据关键词搜索应用
     * @param properties: 返回的应用信息会携带这些属性
     * @param keyword: 关键词
     * @param restrictions: 限制条件
     * @return ApplicationInfoMap: 搜索结果
     */
    ApplicationInfoMap searchApp(ApplicationProperties properties, const QString &keyword, ApplicationPropertyMap restrictions);
    ApplicationInfoMap searchApp(ApplicationProperties properties, const QStringList &keywords, ApplicationPropertyMap restrictions);

    /**
     * @brief ApplicationInfo::searchAppWithOrder
     * 根据关键词搜索应用, 结果以按照相关性排序后返回
     * @param properties: 返回的应用信息会携带这些属性
     * @param keywords: 关键词
     * @param restrictions: 限制条件
     * @return QList<ApplicationInfoMap>: 搜索结果
     */
    QList<ApplicationInfoMap> searchAppWithOrder(ApplicationProperties properties, const QStringList &keywords, ApplicationPropertyMap restrictions);

    /**
     * @brief AppInfoTable::setAppToFavorites
     * 设置应用为收藏
     * @param desktopFilePath: 应用的desktop文件路径
     */
    void setAppToFavorites(const QString &desktopFilePath);

    /**
     * @brief AppInfoTable::removeAppFromFavorites
     * 取消收藏
     * @param desktopFilePath: 应用的desktop文件路径
     */
    void removeAppFromFavorites(const QString &desktopFilePath);

    /**
     * this function has been deprecated, use setAppToFavorites and removeAppFromFavorites instead
     * @brief ApplicationInfo::setFavoritesTo
     * set the favorites state of the app to num, you can also use to change the position of the app which is one of the Favorites Apps
     * @param desktopFilePath: the desktop file path of app
     * @param num: the favorites app's position(from 1). If num is 0, it will remove the app from the favorites apps
     */
    [[deprecated]] void setFavoritesOfApp(const QString &desktopFilePath, size_t num);

    /**
     * @brief ApplicationInfo::setAppToTop
     * set the app to top apps(default is at 1)
     * @param desktopFilePath: desktop file path of app
     */
    [[deprecated]] void setAppToTop(const QString &desktopFilePath);

    /**
     * @brief ApplicationInfo::setAppTopTo
     * set the top state of the app to num, you can also use to change the position of the app which is one of the Top Apps
     * @param desktopFilePath: the desktop file path of app
     * @param num: the top app's position(from 1). If num is 0, it will remove the app from the top apps
     */
    [[deprecated]] void setTopOfApp(const QString &desktopFilePath, size_t num);

    /**
     * @brief ApplicationInfo::setAppLaunchedState
     * 设置应用是否启动过(未启动过会在开始菜单中显示一个提示)
     * @param desktopFilePath
     * @param launched
     */
    void setAppLaunchedState(const QString &desktopFilePath, bool launched = true);

    /**
     * @brief ApplicationInfo::tranPid2DesktopFp
     * 根据进程号找到应用的desktop文件[obsolete]
     * @param pid: 进程号
     * @param desktopFilePath:进程号对应的desktop文件
     * @return bool:成功返回true;失败返回false
     */
    [[deprecated]] bool tranPidToDesktopFp(int pid, QString &desktopFilePath);//obsolete
    /**
    * @brief ApplicationInfo::tranPid2DesktopFp
    * 根据进程号找到应用的desktop文件
    * @param pid: 进程号
    * @param desktopFilePath:进程号对应的desktop文件
    * @return bool:成功返回true;失败返回false
    */
    bool tranPidToDesktopFp(uint pid, QString &desktopFilePath);

    /**
     * @brief ApplicationInfo::desktopFilePathFromName
     * 根据desktop文件名 (不带 .desktop后缀)找到desktop文件的全路径
     * @param desktopFileName: desktop文件名
     * @param desktopFilePath: desktop文件全路径
     * @return bool:成功返回true;失败返回false
     */
    bool desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath);

    /**
     * This function has been deprecated
     *
     * @brief ApplicationInfo::tranWinIdToDesktopFilePath
     * 根据窗口id找到对应的desktop文件,目前仅在x环境可用
     * @param winId:窗口ID(Only for X)
     * @param desktopFilePath:desktop文件全路径
     * @return bool:成功返回true;失败返回false
     */
    [[deprecated]] bool tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopFilePath);

Q_SIGNALS:
    /**
     * 数据库异常时发送,此时应用查询相关接口均不可用
     */
    void DBOpenFailed();

    /**
     * @brief ApplicationInfo::appDBItems2BUpdate
     * 数据库中部分应用更新了一部分属性时发送
     * @param ApplicationInfoMap: 更新的应用属性
     */
    void appDBItems2BUpdate(ApplicationInfoMap);

    /**
    * @brief ApplicationInfo::appDBItems2BUpdate
    * 数据库中部分应用更新了全部属性时发送
    * @param QStringList: 更新的应用列表(desktop文件全路径)
    */
    void appDBItems2BUpdateAll(QStringList);

    /**
    * @brief ApplicationInfo::appDBItems2BAdd
    * 数据库中新增应用时发送
    * @param QStringList: 新增的应用列表(desktop文件全路径)
    */
    void appDBItems2BAdd(QStringList);

    /**
     * @brief ApplicationInfo::appDBItems2BDelete
     * 数据库中删除应用时发送
     * @param QStringList: 删除的应用列表(desktop文件全路径)
     */
    void appDBItems2BDelete(QStringList);

private:
    ApplicationInfoPrivate *d = nullptr;
};

}
#endif // APPLICATIONINFO_H
