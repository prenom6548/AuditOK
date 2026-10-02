/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

UkuiItem.DtThemeBackground {
    id : arrowButton
    width : Math.floor(48 * scaleFactor)
    height : width
    radius : UkuiPlatform.GlobalTheme.kRadiusNormal
    shadowData: UkuiPlatform.GlobalTheme.kShadowMin
    backgroundColor: buttonMouseArea.containsPress ? UkuiPlatform.GlobalTheme.kContainClick
                                                   : buttonMouseArea.containsMouse ? UkuiPlatform.GlobalTheme.kContainHover : UkuiPlatform.GlobalTheme.kContainGeneralNormal
    property var direction : "right"

    Accessible.role: Accessible.Button
    Accessible.name: "arrow button" + direction
    Accessible.description: "a push button, click to move the parent view "+ direction

    UkuiItem.Icon {
        id: arrowIcon
        anchors.centerIn: parent
        width: parent.width / 2
        height: width
        source: direction == "right" ? "ukui-end-symbolic" : "ukui-start-symbolic"
        mode: UkuiItem.Icon.AutoHighlight
    }

    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked : {
            if (direction == "left") {
                if (desktopListView.contentX + desktopListView.contentXoffset - desktopMoveDistance > 0) {
                    contentMoveAnimation.to = desktopListView.contentX - desktopMoveDistance
                    contentMoveAnimation.start()
                } else {
                    contentMoveAnimation.to = 0 - desktopListView.contentXoffset
                    contentMoveAnimation.start()
                }
            }

            if (direction == "right") {
                if (desktopListView.contentX + desktopListView.contentXoffset + desktopMoveDistance < desktopListView.contentWidth-desktopListView.width) {
                    contentMoveAnimation.to = desktopListView.contentX + desktopMoveDistance
                    contentMoveAnimation.start()
                } else {
                    contentMoveAnimation.to = desktopListView.contentWidth - desktopListView.width - desktopListView.contentXoffset
                    contentMoveAnimation.start()
                }
            }

        }
    }
}
