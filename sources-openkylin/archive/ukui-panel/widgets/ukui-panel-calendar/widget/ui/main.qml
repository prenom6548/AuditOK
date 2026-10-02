/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *          zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.panel.calendar 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation === Types.Horizontal
    property real ratio: parent.ratio ? parent.ratio : 1
    property bool isInTopBar: isHorizontal && (root.height <= 36)

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? layout.width : width
    Layout.preferredHeight: isHorizontal ? height : layout.height

    onWidthChanged: {
        if (!isHorizontal) {
            updateText();
        }
    }

    onHeightChanged: {
        if (isHorizontal) {
            updateText();
        }
    }
    onIsInTopBarChanged: {
        updateText();
    }

    Component.onCompleted: {
        updateText();
        //isClearRedpoint();
    }

    Calendar {
        id: calendar

        onTimeUpdated: {
            updateText();
        }
    }

    function updateText() {
        //单行显示
        if (isInTopBar) {
            dateText.text = textInOneline();
            return;
        }

        //多行显示
        let text = calendar.time;

        //time ap
        let s = text;
        if (isOverFlow(s)) {
            if (s.match(" ")) {
                var parts = text.split(" ");
                text = parts[0];
                for (var i = 1; i < parts.length; ++ i) {
                    text += "\n" + parts[i];
                }
            } else {
                var match = s.match(/[0-9]/);
                if (match) {
                    text = s.slice(0, match.index) + "\n" + s.slice(match.index)
                }
            }
        }

        // week
        text += (isHorizontal ? " " : "\n") + calendar.week;

        // year
        text += ("\n" + calendar.year);

        // month
        s = calendar.year + " " + calendar.month;
        text += (isOverFlow(s) ? "\n" : calendar.separator) + calendar.month;

        dateText.text = text;
    }

    function isOverFlow(string) {
        if (isHorizontal) {
            return false;
        }

        let temp = tempText.createObject(root, {text: string});
        let tempWidth = temp.contentWidth;
        temp.destroy();
        return tempWidth >= (root.width - 4);
    }

    Component {
        id: tempText
        Text {
            width: root.width - 4
            height: contentHeight
            fontSizeMode: Text.HorizontalFit
            opacity: 0
        }
    }

    function textInOneline() {
        let text = calendar.time + " " + calendar.week + " " + calendar.year + calendar.separator + calendar.month;
        return text;
    }

    GridLayout {
        id: layout
        height: {
            if (isHorizontal) return parent.height;
        }
        width: {
            if (!isHorizontal) return parent.width;
        }
        rows: isHorizontal? 1 : 2
        columns: isHorizontal ? 2 : 1
        columnSpacing: 0
        rowSpacing: 0


        DtThemeBackground {
            id: backGround
            Accessible.role: Accessible.Button
            Accessible.description: "This is a calendar button of ukui-panel"
            Accessible.name: "ukui-panel_calendar_button"
            Layout.fillWidth: !isHorizontal
            Layout.fillHeight: isHorizontal
            Layout.preferredWidth: isHorizontal ? dateText.contentWidth + 8 : width
            Layout.preferredHeight: isHorizontal ? height : dateText.contentHeight + 4
            Layout.rightMargin: isHorizontal ? 0 : 4 * ratio
            Layout.leftMargin: isHorizontal ? 0 : 4 * ratio
            Layout.topMargin: isInTopBar ? 2 * ratio : isHorizontal ? 4 * ratio : 0
            Layout.bottomMargin: isInTopBar ? 2 * ratio : isHorizontal ? 4 * ratio : 0

            clip: true

            useStyleTransparency: false
            backgroundColor: mouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                                        : mouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                             : GlobalTheme.kContainGeneralAlphaNormal

            radius: iconBackGround.radius

            DtThemeText {
                id: dateText
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                width: isHorizontal ? contentWidth : parent.width - 4
                height: isHorizontal ? parent.height - 4 : contentHeight
                fontSizeMode: isHorizontal ? Text.VerticalFit : Text.HorizontalFit
                minimumPointSize: 1
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                Tooltip {
                    anchors.fill: parent
                    mainText: calendar.tooltipText
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
                    margin: isHorizontal ? 8 + backGround.Layout.topMargin : 8 + backGround.Layout.leftMargin
                }

                Menu {
                    id: calendarMenu
                    transientParent: parent
                    content: [
                        MenuItem {
                            icon: "system-settings-symbolic"
                            text: qsTr("Set Time and Date")
                            onClicked: function () {
                                AppLauncher.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", ["-m", "Date"]);
                            }
                        }
                    ]
                }

                onClicked: {
                    // TODO: open calendar
                    if (mouse.button === Qt.RightButton && !calendar.disableMouseRightButton) {
                        calendarMenu.open();
                    } else if (mouse.button === Qt.LeftButton && !calendar.disableMouseLeftButton){
                        calendar.activeCalendar();
                    }
                }
            }
        }

        DtThemeBackground {
            id: iconBackGround
            Accessible.role: Accessible.Button
            Accessible.description: "This is a sidebar button of ukui-panel"
            Accessible.name: "ukui-panel sidebar button"
            Layout.fillWidth: !isHorizontal
            Layout.fillHeight: isHorizontal
            Layout.rightMargin: isHorizontal ? 0 : 4 * ratio
            Layout.leftMargin: isHorizontal ? 0 : 4 * ratio
            Layout.topMargin: isHorizontal ? 4 * ratio : 0
            Layout.bottomMargin: isHorizontal ? 4 * ratio : 0

            useStyleTransparency: false
            backgroundColor: iconMouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                                        : iconMouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                             : GlobalTheme.kContainGeneralAlphaNormal

            radius: isInTopBar ? 0 : GlobalTheme.kRadiusNormal

            // 任务栏内容区域默认高度是44
            property real iconZoom: isHorizontal ? (root.height / 44) * 8 : (root.width / 44) * 8
            Layout.preferredWidth: isInTopBar ? root.height : (isHorizontal ? Math.ceil(iconZoom * 4) : Math.ceil(iconZoom * 5))
            Layout.preferredHeight: isInTopBar ? root.height : (isHorizontal ? Math.ceil(iconZoom * 5) : Math.ceil(iconZoom * 4))
            Icon {
                id: icon
                anchors.centerIn: parent
                property bool isClearRedpoint: calendar.unreadMsgCount <= 0
                width: isInTopBar ? parent.width - 8 : Math.ceil(parent.iconZoom * 2)
                height: width
                source: isClearRedpoint ? "ukui-tool-symbolic" : "ukui-tool-box-null-symbolic"
                mode: Icon.AutoHighlight
            }

            MouseArea {
                id: iconMouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                Tooltip {
                    anchors.fill: parent
                    mainText: String(calendar.notifyIconTooltipText)
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
                    margin: isHorizontal ? 8 + iconBackGround.Layout.topMargin : 8 + iconBackGround.Layout.leftMargin
                }

                Menu {
                    id: notiMenu
                    transientParent: parent
                    content: [
                        MenuItem {
                            icon: "ukui-tool-symbolic"
                            text: qsTr("Open")
                            onClicked: calendar.openSidebar()
                        },
                        MenuItem {
                            icon: "system-settings-symbolic"
                            text: qsTr("Set up notification center")
                            onClicked: function () {
                                AppLauncher.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", ["-m", "Notice"]);
                            }
                        }
                    ]
                }

                onClicked: {
                    if (mouse.button === Qt.LeftButton && !calendar.disableMouseLeftButton) {
                        calendar.openSidebar();
                    } else if (mouse.button === Qt.RightButton && !calendar.disableMouseRightButton) {
                        notiMenu.open();
                    }
                }
            }
        }
    }
}
