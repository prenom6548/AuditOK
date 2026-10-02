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
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

Item {
    property string text: textField.text
    property bool aiSearch: false
    property int searchControlSpacing: controlSpacing/2
    property int searchMode: textField.focus? (aiSearch? 2: 1): 0

    TextField {
        id: textField
        anchors.fill: parent
        leftPadding: (searchIndicator.width + controlSpacing * 2)
        rightPadding: (24 + 16 + searchControlSpacing * 3)
        background: searchEditBackground
        selectByMouse: true
        text: ""
        Layout.fillWidth: true
        placeholderText: qsTr("Search")
        horizontalAlignment: focus||text.length? Text.AlignLeft: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        ThemeIcon {
            id: searchIndicator
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            x: textField.focus||text.length? controlSpacing: textField.width/2 - 90
            source: "edit-find-symbolic"
        }

        ThemeIcon {
            id: clearIndicator
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: separator.left
            anchors.rightMargin: searchControlSpacing
            visible: textField.focus && textField.text.length > 0
            source: "edit-clear-symbolic"

            MouseArea {
                anchors.fill: parent
                z: 999
                onClicked: {
                    textField.clear();
                }
            }
        }

        ThemeSeparator {
            id: separator
            width: 1
            height: 24
            anchors.right: aiSearchIndicator.left
            anchors.rightMargin: searchControlSpacing
            anchors.verticalCenter: parent.verticalCenter
        }

        Image {
            id: imageSource
            source: "qrc:/data/semanteme-symbolic.svg"
            visible: false
        }

        ColorOverlay {
            id: aiSearchIndicator
            anchors.right: parent.right
            anchors.rightMargin: searchControlSpacing
            anchors.verticalCenter: parent.verticalCenter
            width: 24
            height: 24
            source: imageSource
            color: aiSearch? Theme.highlight(Theme.Active): "transparent"
            MouseArea {
                anchors.fill: parent
                z: 999
                onClicked: {
                    aiSearch = !aiSearch;
                    console.log(aiSearch);
                }
            }
        }

        Item {
            id: searchEditBackground
            Rectangle {
                id: aiSearchBackground
                anchors.centerIn: parent
                width: parent.height
                height: parent.width
                radius: Theme.normalRadius
                rotation: 270
                gradient: aiGradient
                visible: aiSearch
                Gradient {
                    id: aiGradient
                    GradientStop { position: 0.0; color: "#E888F8" }
                    GradientStop { position: 0.3465; color: "#E888F8" }
                    GradientStop { position: 0.7402; color: "#4E75FF" }
                    GradientStop { position: 1.0; color: "#0036F5" }
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    radius: aiSearchBackground.radius > 2? aiSearchBackground.radius - 2: 0
                    color: Theme.base(Theme.Active)
                }
            }

            Rectangle {
                id: normalSearchBackground
                anchors.fill: parent
                radius: Theme.normalRadius

                color: textField.focus? Theme.base(Theme.Active): Theme.button(Theme.Active)
                border.width: 2
                border.color: textField.focus? Theme.highlight(Theme.Active): color;
                visible: !aiSearch
            }
        }
    }

    function setSearchKeyword(string) {
        textField.text = string;
    }

    Component.onCompleted: {
        console.log("search edit loadded");
    }
}
