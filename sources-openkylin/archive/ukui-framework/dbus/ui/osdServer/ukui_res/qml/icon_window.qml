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

    property int windowSize: 92
    property int backgroundSize: 72
    property int iconSize: 48

    width: windowSize
    height: windowSize
    anchors.centerIn: parent
    opacity: 1.0

    // [新增] 定义信号
    signal animationFinished()

    // [新增] 启动动画函数
    function startHideAnimation() {
        fadeOutAnim.start()
    }

    Rectangle {
        id: background
        visible: true
        width: backgroundSize
        height: backgroundSize
        color: dataManager.m_currentBackgroundColor
        anchors.centerIn: parent
        radius: 12
        opacity: dataManager.m_opacity
    }

    Image {
        id: iconImage
        width: iconSize
        height: iconSize
        anchors.centerIn: parent
        source: dataManager.m_iconImageSource
    }

    // [新增] 渐隐动画
    NumberAnimation {
        id: fadeOutAnim
        target: iconWindow
        property: "opacity"
        from: 1.0
        to: 0.0
        duration: 300
        easing.type: Easing.OutQuad

        onFinished: {
            animationFinished()
        }
    }
}

