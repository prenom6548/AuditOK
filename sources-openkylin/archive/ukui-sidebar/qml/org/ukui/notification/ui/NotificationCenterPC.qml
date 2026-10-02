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

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform
import QtGraphicalEffects 1.0

Item {
    clip: true

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NotificationCenterLabel {
            Layout.fillWidth: true
            Layout.minimumHeight: 56
            Layout.preferredHeight: 56
            notificationEmpty: sidebarWindow.isNotificationEmpty

            onClearClick: {
                if (!isLiteMode) {
                    notificationGroupList.viewClearAnimation.start();
                } else {
                    sourceModel.clearAll();
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            NotificationGroupList {
                id: notificationGroupList
                width: parent.width
                height: parent.height
                visible: count > 0
            }

            UkuiItems.DtThemeText {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 16
                visible: notificationGroupList.count === 0
                text: qsTr("No new notifications")
                textColor: Platform.GlobalTheme.kFontSecondary
            }
        }
    }
}
