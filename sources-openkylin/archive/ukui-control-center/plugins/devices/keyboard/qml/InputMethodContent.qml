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
import "childwindow"
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Rectangle {
    id: imContent

    property bool showWarning: false
    property string imName: ""

    onShowWarningChanged: {
        if (showWarning) {
            var warningWindow = warningComponent.createObject(imContent);
            warningWindow.show();
            showWarning = false;
        }
    }
    anchors.fill: parent
    anchors.topMargin: 8
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: imItems

            color: "transparent"
            Layout.preferredHeight: 495
            Layout.fillWidth: true
            clip: true

            DraggableList {
                anchors.fill: parent
                model: imModel.currentGroupInputMethodEntries
                onItemMoved: (from, to) => {
                }
            }

        }

        Rectangle {
            id: rect

            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true

            ColumnLayout {
                id: childColumn

                anchors.fill: parent
                spacing: 8
                anchors.topMargin: 8

                UkuiItems.DtThemeBackground {
                    Layout.preferredWidth: childColumn.width
                    height: 1
                    Layout.preferredHeight: 1
                    backgroundColor: GlobalTheme.kContainGeneralNormal
                    borderColor: GlobalTheme.kLineNormal
                    border.width: 1
                    borderAlpha: 1
                    useStyleTransparency: false
                }

                RowLayout {
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24
                    Layout.alignment: Qt.AlignTop

                    CustomButton {
                        Layout.alignment: Qt.AlignLeft
                        labelText: qsTr("Add")
                        onCustomButtonClicked: {
                            var addInputMethod = addInputMethodComponent.createObject(imContent);
                            addInputMethod.show();
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    CustomButton {
                        Layout.alignment: Qt.AlignRight
                        labelText: qsTr("Layout")
                        onCustomButtonClicked: {
                            layoutManager.getLayouts();
                            layoutManager.getCurrentLayout();
                            var keyboardLayout = keyboardLayoutComponent.createObject(imContent);
                            keyboardLayout.show();
                        }
                    }

                }

            }

        }

        Connections {
            function onAvailableInputMethodsChanged() {
            }

            target: imModel
        }

        Component {
            id: keyboardLayoutComponent

            KeyboardLayout {
                id: keyboardLayout
            }

        }

        Component {
            id: addInputMethodComponent

            AddInputMethod {
                id: addInputMethod
            }

        }

        Component {
            id: warningComponent

            IMNotMatchWarning {
                id: imNotMatch
            }

        }

    }

}
