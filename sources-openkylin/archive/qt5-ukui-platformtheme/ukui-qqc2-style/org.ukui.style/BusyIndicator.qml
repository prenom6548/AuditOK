/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */


import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.BusyIndicator {
    id: controlRoot

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    hoverEnabled: true
    property var index: 0
    property var iconWidth: controlRoot.width > 0 ? controlRoot.width : 16
    property var iconHeight:  controlRoot.height > 0 ? controlRoot.height : 16

    onIndexChanged: {
        image.source = iconSource()
        controlRoot.update();

        // console.log("index changeddd", index)
    }
    NumberAnimation{
        id: animation
        target: controlRoot
        property: "index"
        from: 0
        to: 7
        duration: 1000      // 动画时长（毫秒）
        loops: Animation.Infinite
        running: controlRoot.running
    }

    contentItem: Image {
        id: image
        source: iconSource()
        sourceSize.width: iconWidth
        sourceSize.height: iconHeight
        opacity: controlRoot.running ? 1 : 0
        smooth: true
        anchors.fill: controlRoot
        cache: false
    }

    StylePrivate.APPParameter{
        id: app
         onParametryChanged:{
            image.source = ""
            image.source = iconSource()
            controlRoot.update();
         }
        onIconThemeChanged: {
            image.source = ""
            image.source = iconSource()
            controlRoot.update();
        }
    }
    function iconSource(){
        if(app.isDark)
            return "image://imageProvider/ukui-loading-" + Math.round(index).toString() + "-symbolic/highlight"
        else
            return "image://imageProvider/ukui-loading-" + Math.round(index).toString() + "-symbolic"
    }
}
