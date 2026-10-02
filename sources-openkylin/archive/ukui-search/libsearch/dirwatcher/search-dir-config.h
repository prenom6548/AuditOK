/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: maomao <baijunjie@kylinos.cn>
 *
 */

#ifndef UKUI_SEARCH_SEARCHDIRCONFIG_H
#define UKUI_SEARCH_SEARCHDIRCONFIG_H

#include <QObject>

namespace UkuiSearch {

class SearchDirConfig : public QObject
{
    Q_OBJECT
public:
    SearchDirConfig(QObject *parent = nullptr);

    /**
     * @brief SearchDirConfig::getBlockDirOfUser
     * 获取用户自定义的屏蔽目录
     * @return QStringList: 屏蔽目录列表
     */
    QStringList getBlockDirOfUser();

Q_SIGNALS:
    /**
     * @brief SearchDirConfig::blockItemAdded
     * 新增屏蔽目录
     * @param blockDir 新增的屏蔽目录
     */
    void blockItemAdded(const QString& blockDir);

    /**
     * @brief SearchDirConfig::blockItemRemoved
     * 删除屏蔽目录
     * @param blockDir 删除的屏蔽目录
     */
    void blockItemRemoved(const QString& blockDir);
};

} // UkuiSearch

#endif //UKUI_SEARCH_SEARCHDIRCONFIG_H
