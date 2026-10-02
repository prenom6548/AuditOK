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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Rectangle {
    id: additionalComponent

    width: 760
    height: 580
    color: "transparent"

    Text {
        id: contenentText

        text: "UI configuration"
        width: 712
        height: 24
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 24
        font.pointSize: 16
        color: "black"
    }

    Rectangle {
        width: 712
        height: 182
        radius: 10
        anchors.left: parent.left
        anchors.top: contenentText.bottom
        anchors.leftMargin: 24
        anchors.topMargin: 8
        clip: true

        Rectangle {
            width: parent.width
            height: 60
            color: "lightgrey"
            anchors.centerIn: parent
        }

    }

}
