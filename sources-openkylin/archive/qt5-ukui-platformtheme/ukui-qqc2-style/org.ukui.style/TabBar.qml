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
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.TabBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: 1
    palette: app.palette
    ParseInterface{
        id: parseInterface
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }
    contentItem: ListView {
        Accessible.name: qsTr("Tabs")
        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

//        highlightMoveDuration: 0
//        highlightRangeMode: ListView.ApplyRange
//        preferredHighlightBegin: 40
//        preferredHighlightEnd: width - 40
    }

    background: Rectangle {
        id: back
        anchors.fill: control
        color: "transparent"
    }

    StylePrivate.UKUITabBar{
        id: tabbar
    }

    StylePrivate.APPParameter{
        id: app
    }
}
