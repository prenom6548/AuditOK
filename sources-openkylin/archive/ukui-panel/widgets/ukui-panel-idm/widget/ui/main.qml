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
 * Authors: iaom <zhangmengfei@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.widgets 1.0
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

    //  更新tooltip
    Widget.tooltip: Widget.tooltip
    MouseArea {
        id: mouseArea
        Accessible.name: "ukui-panel_idm_widget"
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

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
                margin: 6
            }
        }

        Icon {
            anchors.fill: parent
            anchors.margins: isMiniLayout ? 2 * root.ratio : 4 * root.ratio
            source: Widget.icon
            scale: parent.containsPress ? 0.8 : 1.0
            Behavior on scale {
                NumberAnimation { duration: 100 }
            }
        }

        onClicked: {
            // 使其右键不显示父窗口菜单
            if (mouse.button === Qt.RightButton) {
                return;
            }
            AppLauncher.launchAppWithArguments("/usr/share/applications/peony.desktop", ["idm:///"]);
        }
    }
}
