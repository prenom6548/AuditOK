/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#ifndef MOCK_GLOBAL_DATA_H
#define MOCK_GLOBAL_DATA_H

// 模拟GlobalSizeData类
class GlobalSizeData
{
public:
    static const int UKUILight = 0;
    static const int UKUIDark = 1;
    static int THEME_COLOR;
    
    // 模拟SDKPointMainSearch方法
    static void SDKPointMainSearch() {
        // 空实现，用于测试
    }
};

// 定义静态成员变量
int GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;

#endif // MOCK_GLOBAL_DATA_H