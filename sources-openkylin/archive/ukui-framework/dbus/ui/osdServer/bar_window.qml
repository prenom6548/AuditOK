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

import QtQuick 2.0

Item {

    id: barWindow

    width: 370
    height: 72

    anchors.centerIn: parent

    Rectangle{
        id: background

        visible: true

        width: 350
        height: 52

        anchors.centerIn: parent

        color: dataManager.m_currentBackgroundColor

        //设置圆角
        radius: 12

        //设置不透明度
        opacity: dataManager.m_opacity

    }

    //进度条背景
    Rectangle{
        id: progressBackground

        width: 282
        height: 16

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        anchors.leftMargin: 64

        opacity: 0.2

        color: dataManager.m_currentContentColor

        radius: 8
    }

    //进度条内容
    Rectangle{

        id: progressContent

        width: 282 * dataManager.m_progressRatio
        height: 16

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        anchors.leftMargin: 64

        color: dataManager.m_themeColor

        radius: 8
    }

    //图标
    Image{

        id: iconImage

        width: 24
        height: 24

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left

        anchors.leftMargin: 24

        source: dataManager.m_barImageSource
    }

}
