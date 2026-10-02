/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.ukui.menu.extension 1.0
import AppControls2 1.0 as AppControls2

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiMenuExtension {
    property bool isVisible: mainWindow.visible

    Component.onCompleted: {
        recentFileView.model = extensionData.recentFilesModel
        extensionData.recentFilesModel.updateData()
    }

    onIsVisibleChanged: {
        if (isVisible) {
            extensionData.recentFilesModel.updateData();
        }
    }

    Item {
        anchors.fill: parent

        UkuiItems.DtThemeText {
            width: implicitWidth
            height: implicitHeight
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 3

            visible:  recentFileView.count === 0
            textColor: Platform.GlobalTheme.kFontSecondary
            font.weight: Font.Normal

            text: qsTr("No apps in recent files")
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onContainsMouseChanged: {
                if (containsMouse) {
                    scrollBar.visible = true
                }
                else {
                    scrollBar.visible = false
                }
            }

            onClicked: {
                if (mouse.button === Qt.LeftButton) {
                    var data = {};
                    data["action"] = "closeMenu";
                    send(data);
                }
            }

            onPressed: {
                if (mouse.button === Qt.RightButton) {
                    parent.forceActiveFocus();
                    var data = {};
                    data["action"] = "emptyAreaContextMenu";
                    send(data);
                }
            }

            UkuiItems.DtThemeBackground {
                anchors.top: parent.top
                width: parent.width; height: 1
                useStyleTransparency: false
                backgroundColor: Platform.GlobalTheme.kContainAlphaClick
                visible: recentFileView.contentY > 0
                z: 1
            }

            ListView {
                id: recentFileView
                anchors.fill: parent
                anchors.leftMargin: 12
                spacing: 4
                focus: true
                highlightMoveDuration: 0
                onActiveFocusChanged: {
                    if (activeFocus) {
                        currentIndex = 0;
                    } else {
                        currentIndex = -1;
                    }
                }
                onCountChanged: currentIndex = -1
                keyNavigationWraps: true
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: AppControls2.ScrollBar {
                    id: scrollBar
                    Accessible.role: Accessible.ScrollBar
                    Accessible.name: "recentFile-vertical-scrollbar"
                    Accessible.description: "a scroll bar, which allows the user to scroll the visible area"
                    visible: false
                    width: 14
                    height: recentFileView.height
                }

                delegate: UkuiItems.DtThemeBackground {
                    id: delegateItem
                    Accessible.role: Accessible.Button
                    Accessible.name: "recentFile-Button"
                    Accessible.description: "This is a recentFile Button"
                    width: ListView.view.width - 18
                    height: 40

                    useStyleTransparency: false
                    backgroundColor: itemArea.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                            : itemArea.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                                     : Platform.GlobalTheme.kContainGeneralAlphaNormal

                    radius: Platform.GlobalTheme.kRadiusMax
                    focus: true

                    states: State {
                        when: index === recentFileView.currentIndex
                        PropertyChanges {
                            target: delegateItem
                            borderColor: Platform.GlobalTheme.kBrandFocus
                            border.width: 2
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 10
                        spacing: 8

                        UkuiItems.Icon {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            Layout.alignment: Qt.AlignVCenter
                            source: model.icon
                        }

                        UkuiItems.DtThemeText {
                            id: fileText
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: model.name
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            textColor: Platform.GlobalTheme.kFontPrimary
                        }
                        UkuiItems.DtThemeText {
                            Layout.preferredWidth: contentWidth
                            Layout.fillHeight: true

                            text: model.date
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            textColor: Platform.GlobalTheme.kFontPrimaryDisable
                        }
                    }

                    Keys.onPressed: {
                        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            var data = {
                                "url": model.uri,
                                "index": model.index
                            }
                            data["action"] = "openFile"
                            send(data)
                        }
                    }

                    MouseArea {
                        id: itemArea
                        property bool hovered
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            hovered = true
                        }
                        onExited: {
                            hovered = false
                        }

                        onClicked: {
                            if (mouse.button === Qt.LeftButton) {
                                var data = {
                                    "url": model.uri,
                                    "index": model.index
                                }
                                data["action"] = "openFile";
                                send(data)
                            }
                        }

                        onPressed: {
                            if (mouse.button === Qt.RightButton) {
                                var data = {
                                    "url": model.uri,
                                    "index": model.index
                                }
                                data["action"] = "contextMenu";
                                send(data)
                            }
                        }

                        UkuiItems.Tooltip {
                            anchors.fill: parent
                            posFollowCursor: true
                            mainText: !fileText.truncated ? qsTr("Path: ") + model.path
                                                          : model.name + "\n" + qsTr("Path: ") + model.path
                            visible: itemArea.containsMouse
                        }
                    }
                }
            }
        }
    }
}
