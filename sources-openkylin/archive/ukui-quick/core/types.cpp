/*
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "types.h"

/*!
    \class UkuiQuick::Types
    \inmodule UkuiQuickCore
    \inheaderfile types.h
    \brief 提供一系列枚举类型，用于描述 UI 元素的位置、方向和类型等.
 */

/*!
    \enum Types::Pos
     UI 元素的位置.
    \value NoPosition 无特定位置，由布局系统自动决定
    \value Left 紧贴容器左侧
    \value Top 紧贴容器顶部
    \value Right 紧贴容器右侧
    \value Bottom 紧贴容器底部
    \value TopLeft 容器左上角
    \value TopRight 容器右上角
    \value BottomRight 容器右下角
    \value BottomLeft 容器左下角
    \value Center 容器正中心
    \value LeftCenter 容器左侧垂直居中
    \value TopCenter 容器顶部水平居中
    \value RightCenter 容器右侧垂直居中
    \value BottomCenter 容器底部水平居中
 */

/*!
    \enum Types::Orientation
    UI 元素的布局方向，影响子元素的排列方式.

    \value Vertical 垂直方向
    \value Horizontal 水平方向
 */

/*!
    \enum Types::WidgetType
    UI 元素的类型.
    \value Widget 小部件
    \value Container 容器
 */

/*!
    \enum Types::PopupLocation
    定义了 UI 弹出元素(如对话框、菜单等)在屏幕上的定位方式

    \value Floating 自由浮动，不固定位置和层级
    \value Desktop 平铺在桌面层，全屏延伸
    \value FullScreen 全屏显示
    \value TopEdge 固定在屏幕顶部边缘
    \value BottomEdge 固定在屏幕底部边缘
    \value LeftEdge 固定在屏幕左侧边缘
    \value RightEdge 固定在屏幕右侧边缘
 */
namespace UkuiQuick {

} // global
