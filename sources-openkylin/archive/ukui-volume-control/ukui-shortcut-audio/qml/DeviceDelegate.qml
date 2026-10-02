/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems

Item {
    id: deviceDelegate
    width: deviceDelegateWidth
    height: deviceDelegateHeight

    property real deviceDelegateWidth: 396
    property real deviceDelegateHeight: deviceDelegate.visible ? 56 : 0
    property real deviceDelegateTopMargin: 0
    property string deviceDelegateBtnSource: "audio-speakers-symbolic"

    property alias deviceDelegateBtnHighLighted: deviceDelegateBtn.isHighLight
    property alias deviceDelegatePortDesTextStr: deviceDelegatePortDesText.text

    property alias deviceDelegatePortEnabled: deviceDelegate.visible

    signal activated()

    ItemDelegate {
        anchors.fill: parent

        contentItem: Item {
            anchors.fill: parent

            UkuiItems.IconButton {
                id: deviceDelegateBtn
                width: 36
                height: 36
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                Layout.alignment: Qt.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24

                radius: width / 2
                iconSource: deviceDelegateBtnSource
                isHighLight: false

                onClicked: deviceDelegate.activated()
            }

            Label {
                id: deviceDelegatePortDesText
                width: 306
                anchors.left: deviceDelegateBtn.right
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                text: qsTr("")

                ToolTip {
                    text: deviceDelegatePortDesText.text
                    visible: hoverHandler.hovered && deviceDelegatePortDesText.truncated
                    delay: 500
                    property var maxWidth: 500
                    property var wrapMode: Text.WrapAnywhere
                }

                HoverHandler {
                    id: hoverHandler
                }
            }
        }

        onClicked: deviceDelegate.activated()
    }
}
