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
import QtQuick.Controls 2.12

Item {
    id: barWindow
    width: 370
    height: 72
    anchors.centerIn: parent
    opacity: 1.0

    // [新增] 定义一个信号，动画结束后通知 C++
    signal animationFinished()

    // 这里定义 QML 端的启动函数
    function startHideAnimation() {
        fadeOutAnim.start()
    }

    Rectangle {
        id: background
        visible: true
        width: 350
        height: 52
        anchors.centerIn: parent
        color: dataManager.m_currentBackgroundColor
        radius: 12
        opacity: dataManager.m_opacity // 初始透明度由数据管理控制
    }

    Rectangle {
        id: progressBackground;
        width: 282;
        height: 16;
        anchors.verticalCenter: parent.verticalCenter;
        anchors.left: parent.left;
        anchors.leftMargin: 64;
        opacity: 0.2;
        color: dataManager.m_currentContentColor;
        radius: 8
    }

    Rectangle {
        id: progressContent;
        width: 282 * dataManager.m_progressRatio;
        height: 16;
        anchors.verticalCenter: parent.verticalCenter;
        anchors.left: parent.left;
        anchors.leftMargin: 64;
        color: dataManager.m_themeColor;
        radius: 8
    }

    Image {
        id: iconImage;
        width: 24;
        height: 24;
        anchors.verticalCenter: parent.verticalCenter;
        anchors.left: parent.left;
        anchors.leftMargin: 24;
        source: dataManager.m_barImageSource
    }

    // [新增] 渐隐动画
    NumberAnimation {
        id: fadeOutAnim
        target: barWindow // 对整个 Item 进行淡出，包括内部所有元素
        property: "opacity"
        from: 1.0
        to: 0.0
        duration: 300 // 300ms
        easing.type: Easing.OutQuad

        onFinished: {
            animationFinished() // 通知 C++ 销毁窗口
        }
    }
}

