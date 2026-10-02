/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Item {

    id: iconWindow

    //自定义属性
    property int windowSize: 92
    property int backgroundSize: 72
    property int iconSize: 48


    //给定属性，不然设置完部分窗口属性会出现bug
    width: windowSize
    height: windowSize
    anchors.centerIn: parent

    //子类显示内容
    Rectangle{
        id: background

        visible: true

        width: backgroundSize
        height: backgroundSize


        color: dataManager.m_currentBackgroundColor

        //设置位置在窗体中心
        anchors.centerIn: parent

        //设置圆角
        radius: 12

        //设置不透明度
        opacity: dataManager.m_opacity

    }

    Image{
        id: iconImage

        width: iconSize
        height: iconSize

        anchors.centerIn: parent

        source: dataManager.m_iconImageSource
    }

}
