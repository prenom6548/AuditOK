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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <kysdk/diagnosetest/libkydatacollect.h>
#include <QString>

class DataCollector
{
private:
    /**
     * 将panel的位置转化为英文字符串上下左右  上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
     * @param pos panel的位置
     */
    static QString positionToStr(const int &pos);

public:
    /**
     * 发送panel位置改变事件的数据锚点
     * @param pos panel的位置
     */
    static void setPanelPositionEvent(const int &pos);
    /**
     * 发送panel大小改变事件的数据锚点描点
     * @param panelSize panel的大小
     */
    static void setPanelSizeEvent(const int &panelSize);
};

#endif // DATACOLLECTOR_H
