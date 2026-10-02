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
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Rectangle {
    property int selectedIndex: 0
    property string currentPageSource: listModel.get(selectedIndex).pageSource

    width: 200
    height: 128
    color: "transparent"

    ListModel {
        // coming soon...
        //        ListElement {
        //            text: qsTr("Additional Component")
        //            pageSource: "AdditionalComponent.qml"
        //        }

        id: listModel

        ListElement {
            text: qsTr("Input Methods")
            pageSource: "InputMethodContent.qml"
        }

        ListElement {
            text: qsTr("Global Config")
            pageSource: "GlobalConfigContent.qml"
        }

    }

    ListView {
        id: listView

        anchors.fill: parent
        anchors.topMargin: 8
        currentIndex: 0
        model: listModel
        spacing: 2
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        delegate: ItemDelegate {
            height: 36
            width: 184
            hoverEnabled: true
            highlighted: index === selectedIndex
            text: model.text
            checkable: true
            checked: index === selectedIndex
            onClicked: {
                selectedIndex = index;
                currentPageSource = model.pageSource;
            }
        }

    }

}
