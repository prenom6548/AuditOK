/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Rectangle {
    property int dragAreaHeight: 40
    property alias headerTitle: styleText.text

    signal dragStarted()
    signal closeClicked()

    color: "transparent"
    width: parent.width
    height: dragAreaHeight

    MouseArea {
        id: dragArea

        anchors.fill: parent
        onPressed: {
            dragStarted();
        }
    }

    RowLayout {
        anchors.fill: parent

        Rectangle {
            id: iconArea

            width: 24
            height: 24
            color: "transparent"

            UkuiItems.Icon {
                anchors.fill: parent
                source: "ukui-control-center"
            }

        }

        Rectangle {
            id: titleRect

            width: styleText.width
            height: 40
            color: "transparent"
            Layout.leftMargin: 4

            UkuiItems.DtThemeText {
                id: styleText

                Accessible.name: text
                anchors.centerIn: parent
            }

        }

        Item {
            Layout.fillWidth: true
            height: 24
        }

        Button {
            id: closeButton

            property var buttonType: "closeButton"

            Accessible.name: qsTr("Close")
            Layout.preferredHeight: 30
            hoverEnabled: true
            ToolTip.text: qsTr("Close")
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.visible: hovered
            onClicked: {
                closeClicked();
            }

            icon {
                name: "edit-clear-symbolic"
            }

        }

    }

}
