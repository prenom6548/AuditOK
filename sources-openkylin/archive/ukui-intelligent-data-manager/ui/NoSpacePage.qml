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
import org.ukui.quick.platform 1.0

Item {
    property int controlSpacing: 8

    Image {
        id: nopageindicator
        width: 263
        height: 200
        anchors.bottom: boldText.top
        anchors.bottomMargin: -controlSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:/data/nospaceindicator.png"
    }

    Text {
        id: boldText
        anchors.centerIn: parent
        font.bold: true
        font.pointSize: Theme.fontSize * 1.5
        text: qsTr("Intelligent Data Manager")
    }

    Text {
        id: hintText
        anchors.top:  boldText.bottom
        anchors.topMargin: controlSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Setting filter condition, polymerizing files dynamically.")
    }

    StyledButton {
        id: createSpaceButton
        anchors.top: hintText.bottom
        anchors.topMargin: controlSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        width: 180
        height: 36
        state: "ImportantButton"
        text: qsTr("Create Space")

        onClicked: {
            modelController.createSpace();
        }
    }
}
