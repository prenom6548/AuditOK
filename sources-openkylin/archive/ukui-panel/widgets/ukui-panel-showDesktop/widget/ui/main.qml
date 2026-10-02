/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.widgets 1.0
import org.ukui.panel.showDesktop 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal
    property real ratio: parent.ratio ? parent.ratio : 1
    property real inset: 2 * ratio

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredHeight: isHorizontal ? height : (15 * ratio + inset)
    Layout.preferredWidth: isHorizontal ? (15 * ratio + inset) : width
    //使其右键不显示父窗口的菜单
    MouseArea {
        Accessible.name: "ukui-panel_showDesktop_widget"
        anchors.fill: root
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button === Qt.RightButton) {
                return;
            }
        }
    }

    MouseArea {
        Accessible.role: Accessible.Button
        Accessible.description: "This is a showDesktop button of ukui-panel"
        Accessible.name: "show desktop"
        anchors.fill: parent
        anchors.leftMargin: isHorizontal ? inset : 0
        anchors.topMargin: isHorizontal ? 0 : inset
        hoverEnabled: true

        DtThemeBackground {
            anchors.fill: parent
            useStyleTransparency: false
            backgroundColor: parent.containsPress ? GlobalTheme.kComponentAlphaClick
                                                  : parent.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                       : GlobalTheme.kComponentAlphaNormal
        }

        ShowDesktop {
            id: showDesktop
        }

        onClicked: {
            //使其右键不显示父窗口的菜单
            if (mouse.button === Qt.RightButton) {
                return;
            } else if (mouse.button === Qt.LeftButton && !showDesktop.disableMouseLeftButton) {
                showDesktop.showDesktop();
            }
        }

        Tooltip {
            id: tooltip
            anchors.fill: parent
            mainText: Widget.tooltip
            posFollowCursor: false
            location: {
                switch(Widget.container.position) {
                    case Types.Bottom:
                        return Dialog.BottomEdge;
                    case Types.Left:
                        return Dialog.LeftEdge;
                    case Types.Top:
                        return Dialog.TopEdge;
                    case Types.Right:
                        return Dialog.RightEdge;
                }
            }
            margin: 8
        }
    }
}
