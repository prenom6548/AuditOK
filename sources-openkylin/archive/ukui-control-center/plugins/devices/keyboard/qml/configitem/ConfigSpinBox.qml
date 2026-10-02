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
    id: spinbox

    property alias labelText: label.text
    property int itemIndex: -1
    property bool enableBackground: true
    property int minValue: 0
    property int maxValue: 100
    property int value: 0
    property int step: 1
    property int layoutMargins: 0

    signal spinValueChanged(int value, int index)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.forceActiveFocus();
        }
    }

    RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.leftMargin: layoutMargins
        anchors.rightMargin: layoutMargins

        UkuiItems.DtThemeText {
            id: label

            elide: Qt.ElideRight
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
        }

        SpinBox {
            id: configSpinBox

            Accessible.name: label.text
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: 240
            from: minValue
            to: maxValue
            value: spinbox.value
            onValueChanged: {
                spinValueChanged(value, spinbox.itemIndex);
            }
        }

    }

}
