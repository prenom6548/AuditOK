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

Item {
    property alias labelText: label.text
    property alias value: checkBox.checked
    property int itemIndex: -1
    property bool enableBackground: true
    property int layoutMargins: 0
    property bool enableVCenter: true

    signal checkBoxChanged(bool state, int index)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.forceActiveFocus();
        }
    }

    RowLayout {
        spacing: 8
        width: parent.width

        anchors {
            top: !enableVCenter ? parent.top : undefined
            left: parent.left
            verticalCenter: enableVCenter ? parent.verticalCenter : undefined
            leftMargin: layoutMargins
            rightMargin: layoutMargins
        }

        CheckBox {
            id: checkBox

            Accessible.name: label.text
            Layout.topMargin: 2
            implicitWidth: 16
            implicitHeight: 16
            onCheckedChanged: {
                checkBoxChanged(checked, itemIndex);
            }
        }

        UkuiItems.DtThemeText {
            id: label

            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        Item {
            Layout.fillWidth: true
        }

    }

}
