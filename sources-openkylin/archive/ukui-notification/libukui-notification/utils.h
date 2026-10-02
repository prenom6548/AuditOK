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

#ifndef UKUI_NOTIFICATION_UTILS_H
#define UKUI_NOTIFICATION_UTILS_H
#include <QString>
#include <QFileInfo>
#include <ukui-search/application-info.h>
namespace UkuiNotification {
/**
 * @short 提供一些方便的接口.
 * @author iaom
 */
namespace Utils {
    /**
     * @brief 获取全局UkuiSearch::ApplicationInfo 实例
     * @return
     */
    UkuiSearch::ApplicationInfo& applicationInfo();

    /**
     * @brief 根据进程号@p pid 获取应用的desktop文件，isFullPath 是否获取全路径的标志位。
     * @return isFullPath==true  获取desktop文件全路径， 否则获取desktop文件名 （不含后缀）
     */
    QString desktopEntryFromPid(uint pid, bool isFullPath = false);
    /**
     * @brief 根据进程号@p pid 获取应用所在的DISPLAY。
     * @return DISPLAY的值
     */
    QString displayFromPid(uint pid);
    /**
     * @brief 根据desktop文件名（不带后缀）获取全路径。
     * @return desktop文件全路径
     */
    QString desktopEntryFromName(QString desktopFileName);

    /**
     * @brief 从desktop文件全路径截取desktop文件名（不含后缀）
     * @param desktop文件全路径
     * @return desktop 文件名
     */
    inline QString desktopEntry2Name(const QString& desktopFileName)
    {
        if (desktopFileName.endsWith(".desktop")) {
            return QFileInfo(desktopFileName).completeBaseName();
        }
        return QFileInfo(desktopFileName).fileName();
    };

    /**
     * @brief 获取应用名称， 通过desktop全路径
     * @return
     */
    QString appNameForDesktopEntry(const QString& desktopFileName);

    /**
 *
 * @return 返回a中存在b中不存在的元素, 求差集
 */
#define StringListDiff \
    ([](const QStringList& a, const QStringList& b) { \
    QSet<QString> setA(a.begin(), a.end()); \
    QSet<QString> setB(b.begin(), b.end()); \
    return QStringList(setA.subtract(setB).values()); \
    })
}
}
#endif //UKUI_NOTIFICATION_UTILS_H
