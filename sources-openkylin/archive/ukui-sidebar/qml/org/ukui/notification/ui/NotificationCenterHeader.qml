/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

import QtQuick 2.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform
import  QtQuick.Layouts 1.12

ColumnLayout {
    id: root;
    spacing: 48;
    property bool haveMessage: false
    signal clearClick();

    Clock {
        id: clock;

        Layout.fillWidth: true;
        Layout.alignment: Qt.AlignHCenter;
        Layout.topMargin: 96;
    }

    // 通知中心顶部显示
    Item {
        id: rencentMessages;
        // 在通知中心，这块区域高度为48+8=56，在侧边栏内高度为：16+24+16=56
        Layout.fillHeight: true;
        Layout.fillWidth: true;
        // “通知中心”按钮 平板模式有消息时显示
        Item {
            id: notificationText;
            height: 32;
            width: notificationTextBody.width;
            anchors.verticalCenter: parent.verticalCenter;
            visible: root.haveMessage;

            UkuiItems.DtThemeText {
                id: notificationTextBody;
                anchors.verticalCenter: parent.verticalCenter;
                font.pixelSize: 32;
                verticalAlignment: Text.AlignVCenter;
                font.bold: true;
                text: qsTr("Notification Center");
                textColor: Platform.GlobalTheme.kFontWhite
            }
        }

        Item {
            id: noMessageText;
            height: 32;
            width: visible ? noMessageTextBody.width : 0;
            anchors.verticalCenter: parent.verticalCenter
            x: visible ? (parent.width - noMessageTextBody.width)/2:0;
            visible: !root.haveMessage
            clip: true;
            Behavior on width {
                NumberAnimation {
                    duration: 300;
                    easing.type: Easing.InOutSine;
                }
            }
            Behavior on x {
                NumberAnimation {
                    duration: 300;
                    easing.type: Easing.InOutSine;
                }
            }

            UkuiItems.DtThemeText {
                id: noMessageTextBody;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.left: parent.left;

                font.pixelSize: 32;
                font.bold: true;
                text: qsTr("No new notifications");
                textColor: Platform.GlobalTheme.kFontWhite
            }
        }

        UkuiItems.Icon {
            height: 24;
            width: 24;
            anchors.verticalCenter: parent.verticalCenter;
            anchors.left: noMessageText.visible ? noMessageText.right : notificationText.right;
            source: "pan-end-symbolic";
            mode: UkuiItems.Icon.ForceHighlight

            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    notificationWindow.activeNotificationCenter(false);
                    appManager.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", ["-m", "Notice"], "ukui-control-center")
                }
            }
        }

        // “清空”按钮——平板模式有消息时显示
        UkuiItems.DtThemeBackground {
            height: 48;
            width: 64;
            radius: Platform.GlobalTheme.kRadiusNormal;
            visible: root.haveMessage;

            anchors.rightMargin: 0;
            anchors.right: parent.right;
            anchors.verticalCenter: parent.verticalCenter;

            useStyleTransparency: true;
            backgroundColor: mouseArea.containsPress ? Platform.GlobalTheme.kComponentClick
                                                     : mouseArea.containsMouse ? Platform.GlobalTheme.kComponentHover
                                                                               : Platform.GlobalTheme.kGrayAlpha0;

            UkuiItems.DtThemeText {
                anchors.centerIn: parent;
                font.pixelSize: 16;
                text: qsTr("Clear All");
                textColor: mouseArea.containsMouse || mouseArea.containsPress ? Platform.GlobalTheme.kFontPrimary
                                                                              : Platform.GlobalTheme.kFontWhite;
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent;
                hoverEnabled: true;

                onClicked: {
                    root.clearClick();
                }
            }
        }

    }
}
