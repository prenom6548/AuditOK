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
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

TextField {
    id: searchField

    property string placeholderTips: qsTr("Search")
    property alias searchInputFocusActive: searchField.activeFocus
    property alias searchInputFocus: searchField.focus

    signal searchTextChanged(string text)

    Accessible.name: qsTr("Search")
    Layout.fillWidth: true
    leftPadding: (parent.width - searchIcon.width - placeholderMetrics.width) / 2 + searchIcon.width + 5
    verticalAlignment: Text.AlignVCenter
    onTextChanged: {
        searchTextChanged(text);
    }
    onActiveFocusChanged: {
        if (activeFocus)
            searchField.state = "active";
        else
            searchField.state = "";
    }
    states: [
        State {
            name: "active"

            PropertyChanges {
                target: searchField
                leftPadding: searchIcon.width + 10
            }

            PropertyChanges {
                target: placeholderItem
                visible: false
            }

        },
        State {
            name: ""

            PropertyChanges {
                target: searchField
                leftPadding: (searchField.width - searchIcon.width - placeholderMetrics.width) / 2 + searchIcon.width + 5
            }

            PropertyChanges {
                target: placeholderItem
                visible: true
            }

        }
    ]

    TextMetrics {
        id: placeholderMetrics

        font: searchField.font
        text: searchField.placeholderTips
    }

    UkuiItems.Icon {
        id: searchIcon

        source: "edit-find-symbolic"
        width: 16
        height: 16
        mode: UkuiItems.Icon.AutoHighlight

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: searchField.activeFocus ? 10 : (searchField.width - placeholderMetrics.width - width) / 2
        }

        Behavior on anchors.leftMargin {
            NumberAnimation {
                duration: 50
            }

        }

    }

    UkuiItems.DtThemeText {
        id: placeholderItem

        Accessible.name: text
        text: searchField.placeholderTips
        font: searchField.font
        visible: !searchField.activeFocus
        alpha: 0.5

        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
            horizontalCenterOffset: searchIcon.width / 2 + 2
        }

    }

}
