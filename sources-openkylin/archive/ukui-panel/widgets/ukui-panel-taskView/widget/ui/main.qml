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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.ukui.quick.widgets 1.0
import org.ukui.panel.taskView 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal
    property real spacing: isMiniLayout ? 2 : parent.ratio ? 4 * parent.ratio : 4
    property bool isMiniLayout: isHorizontal && (height <= 36)

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? height - mouseArea.anchors.topMargin -  mouseArea.anchors.bottomMargin : width
    Layout.preferredHeight: isHorizontal ? height : width - mouseArea.anchors.leftMargin -  mouseArea.anchors.rightMargin
    //使其右键不显示父窗口的菜单
    MouseArea {
        Accessible.name: "ukui-panel_showTaskView_widget"
        anchors.fill: root
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button === Qt.RightButton) {
                return;
            }
        }
    }

    MouseArea {
        id: mouseArea
        Accessible.role: Accessible.Button
        Accessible.description: "This is a taskview button of ukui-panel"
        Accessible.name: "ukui-panel-taskView"
        anchors.fill: parent
        hoverEnabled: true
        anchors {
            leftMargin: root.isHorizontal? 0 : spacing
            topMargin: root.isHorizontal? spacing : 0
            rightMargin: root.isHorizontal? 0 : spacing
            bottomMargin: root.isHorizontal? spacing : 0
        }

        DtThemeBackground {
            id: backGround
            anchors.fill: parent
            radius: GlobalTheme.kRadiusNormal
            backgroundColor: mouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                                                    : mouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                                         : GlobalTheme.kContainGeneralAlphaNormal
            useStyleTransparency: false
            Icon {
                id: icon
                anchors.fill: parent
                anchors.margins: spacing
                source: "ukui-taskview-black-symbolic"
                mode: Icon.AutoHighlight
                scale: mouseArea.containsPress ? 0.8 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }
        }

        TaskViewButton {
            id: taskviewButton
        }

        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton && !taskviewButton.disableMouseLeftButton) {
                AppLauncher.runCommand("ukui-window-switch --show-workspace");
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
            margin: 8 + root.spacing
        }
    }
}
