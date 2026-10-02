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

import ".."
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Window {
    id: window

    property bool isCentered: true
    property string selectedName: ""
    property int currentIndex: -1
    property int layoutMargins: 24

    function cancelAction() {
        listView.model = null;
        searchImModel.filterText = "";
        window.destroy();
    }

    function saveAction() {
        if (selectedName !== "") {
            imModel.addInputMethod(selectedName);
            imModel.updateCurrentGroupInputMethodList();
        }
        listView.model = null;
        searchImModel.filterText = "";
        window.destroy();
    }

    width: 480
    height: 520
    minimumWidth: 480
    minimumHeight: 520
    maximumWidth: 480
    maximumHeight: 520
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint
    title: qsTr("Add")

    UkuiItems.DtThemeBackground {
        id: addInputMethodBackground

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        MouseArea {
            anchors.fill: parent
            enabled: searchTextInput.searchInputFocusActive
            onClicked: searchTextInput.searchInputFocus = false
        }

        ColumnLayout {
            id: imsColumn

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 8
            Layout.fillHeight: true
            spacing: 16

            SearchTextInput {
                id: searchTextInput

                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins
                Layout.fillWidth: true
                Layout.preferredHeight: 32
                onSearchTextChanged: {
                    searchImModel.filterText = text;
                }
            }

            UkuiItems.DtThemeBackground {
                id: imListBackground

                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins
                Layout.fillWidth: true
                height: 334
                Layout.preferredHeight: 334
                backgroundColor: GlobalTheme.kContainGeneralNormal
                borderColor: GlobalTheme.kLineNormal
                border.width: 1
                borderAlpha: 1
                clip: true

                ListView {
                    id: listView

                    anchors.fill: parent
                    model: searchImModel
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1

                    delegate: ItemDelegate {
                        width: listView.width - scrollBar.width
                        height: 36
                        hoverEnabled: true

                        Text {
                            height: parent.height
                            width: parent.width
                            text: model.name
                            Accessible.name: model.name
                            elide: Qt.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            color: index === currentIndex ? GlobalTheme.highlightedTextActive.pureColor : GlobalTheme.textActive.pureColor
                            leftPadding: 8
                        }

                        background: Rectangle {
                            id: background

                            radius: GlobalTheme.kRadiusNormal
                            color: {
                                if (index === currentIndex)
                                    return GlobalTheme.highlightActive.pureColor;
                                else if (hovered)
                                    return GlobalTheme.midLightInactive.pureColor;
                                else
                                    index % 2 ? GlobalTheme.buttonActive.pureColor : "transparent";
                            }

                            MouseArea {
                                id: mouseArea

                                anchors.fill: parent
                                onClicked: {
                                    currentIndex = index;
                                    selectedName = model.uniqueName;
                                }
                            }

                        }

                    }

                    ScrollBar.vertical: ScrollBar {
                        id: scrollBar
                        Accessible.name: qsTr("InputMethods List ScrollBar")
                        hoverEnabled: true
                        active: hovered || pressed
                        orientation: Qt.Vertical
                        size: listView.height / listView.contentHeight
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        visible: listView.contentHeight > listView.height
                    }

                }

            }

            RowLayout {
                spacing: 0
                Layout.fillWidth: true
                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins

                CheckBox {
                    id: checkBox
                    Accessible.name: label.text
                    opacity: searchTextInput.text === "" ? 1 : 0
                    Layout.topMargin: 2
                    implicitWidth: 16
                    implicitHeight: 16
                    checked: searchImModel.onlyShowCurrentLanguage
                    onCheckedChanged: {
                        searchImModel.onlyShowCurrentLanguage = checked;
                    }
                }

                UkuiItems.DtThemeText {
                    id: label

                    opacity: searchTextInput.text === "" ? 1 : 0
                    text: qsTr("Only Show Current Language")
                    Layout.leftMargin: 8
                }

                Item {
                    Layout.fillWidth: true
                }

            }

            UkuiItems.DtThemeBackground {
                Layout.fillWidth: true
                height: 1
                Layout.preferredHeight: 1
                backgroundColor: GlobalTheme.kContainGeneralNormal
                borderColor: GlobalTheme.kLineNormal
                border.width: 1
                borderAlpha: 1
                useStyleTransparency: false
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins
                spacing: 8

                Item {
                    Layout.fillWidth: true
                }

                CustomButton {
                    labelText: qsTr("Cancel")
                    onCustomButtonClicked: cancelAction()
                }

                CustomButton {
                    enableHighlight: true
                    labelText: qsTr("OK")
                    Layout.alignment: Qt.AlignRight
                    onCustomButtonClicked: saveAction()
                }

            }

        }

    }

}
