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
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

DtThemeBackground {
    id : root
    visible : false
    state : "default"
    border.width: 1

    property string windowType : "none"

    signal entered()
    signal exited()
    signal pressed()
    signal clicked()

    Accessible.role: Accessible.Button
    Accessible.name: "close button"
    Accessible.description: "a push button, click to close the preview window or desktop"
    Accessible.onPressAction: { root.clicked() }

    states : [
        State {
            name : "default"
            PropertyChanges {
                target : root
                backgroundColor: GlobalTheme.kGrayAlpha0
                borderColor: GlobalTheme.kGrayAlpha0
            }
        },
        State {
            name : "hover"
            PropertyChanges {
                target : root
                backgroundColor: windowType == "desktop" ? GlobalTheme.kComponentAlphaNormal : GlobalTheme.kComponentNormal
                borderColor: GlobalTheme.kLineComponentNormal
            }
        },
        State {
            name : "press"
            PropertyChanges {
                target : root
                backgroundColor: windowType == "desktop" ? GlobalTheme.kComponentAlphaClick : GlobalTheme.kComponentClick
                borderColor: GlobalTheme.kLineComponentClick
            }
        }
    ]

    Icon {
        id: closeIcon
        anchors.centerIn: parent
        width: parent.width / 2
        height: parent.height / 2
        source: "window-close-symbolic"
        mode: closeButtonMouseArea.containsMouse ? Icon.AutoHighlight
                                             : windowType == "desktop" ? Icon.AutoHighlight : Icon.Highlight
    }

    MouseArea {
        id : closeButtonMouseArea
        anchors.fill : parent
        hoverEnabled : true
        preventStealing: true

        onEntered: {
            root.state = "hover"
            root.entered()
        }
        onExited: {
            root.exited()
            root.state = "default"
        }
        onPressed: {
            root.state = "press"
            root.pressed()
        }
        onReleased: {
            root.state = containsMouse ? "hover" : "default"
        }
        onClicked: {
            root.clicked()
        }
    }
}
