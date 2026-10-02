/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
import org.ukui.ai.assistant 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal
    property real ratio: parent.ratio ? parent.ratio : 1
    visible: assistantButton.available

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: assistantButton.available ? (isHorizontal ? height - mouseArea.anchors.topMargin -  mouseArea.anchors.bottomMargin : width) : 0
    Layout.preferredHeight: assistantButton.available ? (isHorizontal ? height : width - mouseArea.anchors.leftMargin -  mouseArea.anchors.rightMargin) : 0
    property bool isMiniLayout: isHorizontal && (height <= 36)

    //  更新tooltip
    Widget.tooltip: Widget.tooltip
    //使其右键不显示父窗口的菜单
    MouseArea {
        anchors.fill: root
        Accessible.name: "ukui-panel_ai_assistant_widget"
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
        Accessible.description: "This is an ai-assistant button of ukui-panel"
        Accessible.name: "kylin-ai-assistant"
        anchors.fill: parent
        hoverEnabled: true
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
            source: assistantButton.icon
            scale: parent.containsPress ? 0.8 : 1.0
            Behavior on scale {
                NumberAnimation { duration: 100 }
            }
        }

        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton && !assistantButton.disableMouseLeftButton) {
                assistantButton.pressed();
            }
        }
    }

    AiAssistantButton {
        id: assistantButton
    }
}
