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
import "childwindow"
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

UkuiItems.DtThemeBackground {
    property int dragAreaHeight: 40
    property bool isActive: true

    anchors.fill: parent
    useStyleTransparency: isActive
    Component.onCompleted: {
        shortcutController.init();
    }
    Accessible.name: qsTr("Input Method")

    Connections {
        function onActiveStateChanged(state) {
            console.debug("Active state changed:", state);
            isActive = state;
        }

        target: Window
    }

    CustomHeader {
        id: customHeader

        width: parent.width
        height: dragAreaHeight
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 4
        headerTitle: qsTr("Input Method")
        z: 1
        onDragStarted: {
            Window.startSystemMove();
        }
        onCloseClicked: {
            Window.close();
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: dragAreaHeight
            Layout.preferredWidth: 200
            Layout.maximumWidth: 200
            Layout.fillHeight: true
            color: "transparent"
            anchors.topMargin: 8

            NavigationBar {
                id: navigationBar

                anchors.top: parent.top
                anchors.topMargin: 8
            }

        }

        UkuiItems.DtThemeBackground {
            useStyleTransparency: false
            Layout.fillHeight: true
            Layout.preferredWidth: 760
            Layout.maximumWidth: 760
            Layout.alignment: Qt.AlignRight
            backgroundColor: GlobalTheme.kContainGeneralNormal

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    id: contentArea

                    Layout.fillHeight: true
                    color: "transparent"
                    Layout.preferredWidth: 760
                    Layout.maximumWidth: 760
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: dragAreaHeight

                    Loader {
                        id: pageLoader

                        anchors.fill: parent
                        source: navigationBar.currentPageSource
                    }

                }

            }

        }

    }

}
