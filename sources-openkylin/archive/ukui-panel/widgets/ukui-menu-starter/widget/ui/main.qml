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

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.ukui.quick.widgets 1.0
import org.ukui.menu.starter 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal
    property real ratio: parent.ratio ? parent.ratio : 1

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? height - mouseArea.anchors.topMargin -  mouseArea.anchors.bottomMargin : width
    Layout.preferredHeight: isHorizontal ? height : width - mouseArea.anchors.leftMargin -  mouseArea.anchors.rightMargin
    property bool isMiniLayout: isHorizontal && (height <= 36)

    Widget.active: StartMenuButton.currentScreenValid
                   && StartMenuButton.isCurrentScreen(Widget.container.screen) ?
                       (StartMenuButton.menuWindowVisible || StartMenuButton.contextMenuVisible) : false
    //  更新tooltip
    Widget.tooltip: Widget.tooltip

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        Accessible.role: Accessible.Button
        Accessible.description: "This is a start menu button of ukui-panel"
        Accessible.name: "ukui-menu-starter"
        anchors {
            leftMargin: root.isHorizontal? 0 : 4 * root.ratio
            topMargin: isMiniLayout ? 2 * root.ratio : root.isHorizontal? 4 * root.ratio : 0
            rightMargin: root.isHorizontal? 0 : 4 * root.ratio
            bottomMargin: isMiniLayout ? 2 * root.ratio : root.isHorizontal? 4 * root.ratio : 0
        }

        DtThemeBackground {
            anchors.fill: parent
            radius: GlobalTheme.kRadiusNormal
            useStyleTransparency: false
            backgroundColor: mouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                                        : mouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                             : GlobalTheme.kContainGeneralAlphaNormal
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
                margin: root.isHorizontal ? 8 + mouseArea.anchors.topMargin : 8 + mouseArea.anchors.leftMargin
            }
        }

        Icon {
            anchors.fill: parent
            anchors.margins: isMiniLayout ? 2 * root.ratio : 4 * root.ratio
            source: "kylin-startmenu"
            scale: parent.containsPress ? 0.8 : 1.0
            Behavior on scale {
                NumberAnimation { duration: 100 }
            }
        }

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton && !StartMenuButton.disableMouseLeftButton) {
                StartMenuButton.openStartMenu()
            }
            if (mouse.button === Qt.RightButton && !StartMenuButton.disableMouseRightButton) {
                 StartMenuButton.openContextMenu(root)
            }
        }
    }
}
