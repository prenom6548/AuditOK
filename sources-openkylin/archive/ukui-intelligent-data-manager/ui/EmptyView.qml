/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.4
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

Item {
    property int controlSpacing: 8
    Layout.fillWidth: true
    Layout.fillHeight: true

    Icon {
        mode: Icon.Disabled
        source: "docviewer-app-symbolic"
        width: 128
        height: 128
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.verticalCenter
        anchors.bottomMargin: controlSpacing
    }

    Text {
        id: hintText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: controlSpacing
        width: parent.width * 2/3
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: qsTr("The files in system would be displayed in the space, while they are existing in system and match the space's conditions")
    }

    RowLayout {
        anchors.top: hintText.bottom
        anchors.topMargin: controlSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            text: qsTr("You can manually")
        }

        LinkButton {
            text: qsTr("Add")

            onTriggered: {
                modelController.requestAddUserAddedFileToSapce(currentView.proxyModel);
            }
        }

        Text {
            text: qsTr("files, or")
        }

        LinkButton {
            text: qsTr("Edit Space Conditions")

            onTriggered: {
                modelController.editSpaceCondition(currentView.spaceId);
            }
        }
    }
}
