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
 */

import QtQuick 2.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

//PC侧边栏顶部显示
Item {
    id: sidebarTips;
    property bool notificationEmpty: false
    signal clearClick();

    UkuiItems.DtThemeText {
        anchors.verticalCenter: parent.verticalCenter;
        anchors.left: parent.left;
        verticalAlignment: Text.AlignVCenter;
        font.bold: true;
        text: qsTr("Notification Center");
        textColor: Platform.GlobalTheme.kFontPrimary;
    }

    UkuiItems.DtThemeButton {
        id: sidebarAllClear

        Accessible.role: Accessible.Button
        Accessible.description: "This is a clear all button on notification center"
        Accessible.name: "notification_center_clear_all_button"

        width: 30
        height: 30
        visible: !notificationEmpty

        anchors.right: sidebarSystemSetting.left
        anchors.rightMargin: 2
        anchors.verticalCenter: parent.verticalCenter

        icon.width: 16
        icon.height: 16
        icon.source: "ukui-rubbish-symbolic"
        icon.mode: UkuiItems.Icon.AutoHighlight

        background.backgroundColor: containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                  : containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                  : Platform.GlobalTheme.kGrayAlpha0
        background.radius: Platform.GlobalTheme.kRadiusNormal

        onClicked: {
            sidebarTips.clearClick();
        }

        UkuiItems.Tooltip {
            anchors.fill: parent
            posFollowCursor: true
            mainText: qsTr("Clear all")
        }
    }

    UkuiItems.DtThemeButton {
        id: sidebarSystemSetting

        Accessible.role: Accessible.Button
        Accessible.description: "This is a system setting button on notification center"
        Accessible.name: "notification_center_system_setting_button"

        width: 30
        height: 30

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        icon.width: 16
        icon.height: 16
        icon.source: "system-settings-symbolic"
        icon.mode: UkuiItems.Icon.AutoHighlight

        background.backgroundColor: containsPress ? Platform.GlobalTheme.kComponentAlphaClick
                                                  : containsMouse ? Platform.GlobalTheme.kComponentAlphaHover
                                                                  : Platform.GlobalTheme.kGrayAlpha0
        background.radius: Platform.GlobalTheme.kRadiusNormal

        onClicked: {
            appManager.launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", ["-m", "Notice"], "ukui-control-center");
        }

        UkuiItems.Tooltip {
            anchors.fill: parent
            posFollowCursor: true
            mainText: qsTr("Notification setting")
        }
    }
}
