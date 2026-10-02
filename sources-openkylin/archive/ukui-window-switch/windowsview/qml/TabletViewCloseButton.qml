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
import QtGraphicalEffects 1.0
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

UkuiItem.DtThemeBackground {
    id : root
    visible : false
    width : iconAndCloseButtonHeight
    height : width
    radius : width / 2
    state : "default"
    property alias closeButtonContainMouse: closeButtonMouseArea.containsMouse

    signal entered()
    signal exited()
    signal pressed()
    signal clicked()

    Accessible.role: Accessible.Button
    Accessible.name: "close"
    Accessible.description: "a push button, click to close the preview window"
    Accessible.onPressAction: { root.clicked() }

    states :  [
        State {
            name : "default"
            PropertyChanges { target: root; backgroundColor: UkuiPlatform.GlobalTheme.kContainGeneralAlphaNormal }
        },
        State {
            name : "hover"
            PropertyChanges { target: root; backgroundColor: UkuiPlatform.GlobalTheme.kComponentHover }
        },
        State {
            name : "press"
            PropertyChanges { target: root; backgroundColor: UkuiPlatform.GlobalTheme.kComponentClick }
        }
    ]

    UkuiItem.Icon {
        id: closeIcon
        anchors.centerIn: parent
        width: parent.width / 2
        height: parent.height / 2
        source: "window-close-symbolic"
        mode: parent.closeButtonContainMouse ? UkuiItem.Icon.AutoHighlight : UkuiItem.Icon.Highlight
    }

    MouseArea{
        id : closeButtonMouseArea
        anchors.fill : parent
        hoverEnabled : true
        onEntered: {
            root.state = "hover"
            root.entered()
        }

        onExited: {
            root.state = "default"
            root.exited()
        }

        onPressed: {
            root.state = "press"
            root.pressed()
        }

        onClicked: {
            root.clicked()
        }
    }
}
