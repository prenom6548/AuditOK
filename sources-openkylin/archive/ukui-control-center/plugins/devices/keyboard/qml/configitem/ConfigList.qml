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
import "../childwindow"
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Item {
    id: configList

    property int currentIndex: -1
    property alias labelText: label.text
    property var listModel: null
    property var itemData: null
    property int itemIndex: -1
    property bool enableBackground: true
    property int layoutMargins: 0
    readonly property int itemHeight: 36
    readonly property int maxItemCount: 6

    function execEditDialog(initialValue, index) {
        if (!itemData)
            return ;

        var subFields = itemData.subFields || [];
        var validValue = initialValue;
        if (validValue === undefined || validValue === null)
            validValue = itemData.defaultSubValue || "";

        var preparedValue = itemData.prepareEditValue(validValue);
        var isStructured = subFields.length > 0;
        var component = Qt.createComponent(isStructured ? "../childwindow/ListItemEditDialog.qml" : "../childwindow/ListSimpleValueDialog.qml");
        if (component.status !== Component.Ready) {
            console.error("Error loading dialog:", component.errorString());
            return ;
        }
        var dialogProps = {
            "isEditMode": index >= 0,
            "initialValue": preparedValue,
            "itemIndex": index,
            "itemData": itemData
        };
        if (!isStructured) {
            dialogProps.valueType = itemData.subType || "String";
            dialogProps.enumOptions = itemData.subEnumOptions || [];
            dialogProps.labelText = itemData.subDescription || itemData.desc || "";
        } else {
            dialogProps.labelText = itemData.subDescription || itemData.desc || "";
        }
        var dialog = component.createObject(configList, dialogProps);
        dialog.confirmed.connect(function(value, idx) {
            if (!itemData)
                return ;

            var serialized = itemData.serializeEditValue(value);
            if (idx >= 0)
                itemData.editItem(idx, serialized);
            else
                itemData.addItem(serialized);
        });
        dialog.show();
    }

    function openAddDialog() {
        if (!itemData)
            return ;

        execEditDialog(itemData.defaultSubValue || "", -1);
    }

    function openEditDialog() {
        if (!itemData || currentIndex < 0 || currentIndex >= listView.count)
            return ;

        var currentValue = itemData.getItem(currentIndex);
        execEditDialog(currentValue, currentIndex);
    }

    implicitHeight: columnLayout.implicitHeight

    ColumnLayout {
        id: columnLayout

        spacing: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: layoutMargins
        anchors.rightMargin: layoutMargins

        UkuiItems.DtThemeText {
            id: label
        }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true

            UkuiItems.DtThemeBackground {
                id: imListBackground

                Layout.fillWidth: true
                Layout.preferredHeight: maxItemCount * itemHeight
                backgroundColor: GlobalTheme.kContainGeneralNormal
                borderColor: GlobalTheme.kLineNormal
                border.width: 1
                borderAlpha: 1
                clip: true

                ListView {
                    id: listView

                    anchors.fill: parent
                    model: listModel
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1

                    delegate: ItemDelegate {
                        width: listView.width - (scrollBar.visible ? scrollBar.width : 0)
                        height: itemHeight
                        hoverEnabled: true

                        Text {
                            height: parent.height
                            width: parent.width
                            text: model.display
                            Accessible.name: model.display
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
                                }
                            }

                        }

                    }

                    ScrollBar.vertical: ScrollBar {
                        id: scrollBar

                        Accessible.name: qsTr("Config List ScrollBar")
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

            ColumnLayout {
                spacing: 8
                Layout.alignment: Qt.AlignTop

                Button {
                    id: addButton

                    Accessible.name: qsTr("Add")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                    hoverEnabled: true
                    ToolTip.text: qsTr("Add")
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered
                    onClicked: {
                        openAddDialog();
                    }

                    icon {
                        name: "list-add-symbolic"
                    }

                }

                Button {
                    id: editButton

                    enabled: currentIndex >= 0 && currentIndex < listView.count
                    Accessible.name: qsTr("Edit")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                    hoverEnabled: true
                    ToolTip.text: qsTr("Edit")
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered
                    onClicked: {
                        if (itemData && currentIndex >= 0)
                            openEditDialog();

                    }

                    icon {
                        name: "edit-symbolic"
                    }

                }

                Button {
                    id: upButton

                    enabled: currentIndex > 0 && currentIndex < listView.count
                    Accessible.name: qsTr("Up")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                    hoverEnabled: true
                    ToolTip.text: qsTr("Up")
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered
                    onClicked: {
                        if (itemData && currentIndex > 0) {
                            itemData.moveUp(currentIndex);
                            currentIndex = currentIndex - 1;
                        }
                    }

                    icon {
                        name: "go-up-symbolic"
                    }

                }

                Button {
                    id: downButton

                    enabled: currentIndex >= 0 && currentIndex < listView.count - 1
                    Accessible.name: qsTr("Down")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                    hoverEnabled: true
                    ToolTip.text: qsTr("Down")
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered
                    onClicked: {
                        if (itemData && currentIndex >= 0 && currentIndex < listView.count - 1) {
                            itemData.moveDown(currentIndex);
                            currentIndex = currentIndex + 1;
                        }
                    }

                    icon {
                        name: "go-down-symbolic"
                    }

                }

                Button {
                    id: removeButton

                    enabled: currentIndex >= 0 && currentIndex < listView.count
                    Accessible.name: qsTr("Remove")
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                    hoverEnabled: true
                    ToolTip.text: qsTr("Remove")
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    ToolTip.visible: hovered
                    onClicked: {
                        if (itemData && currentIndex >= 0) {
                            itemData.removeItem(currentIndex);
                            if (currentIndex >= listView.count)
                                currentIndex = listView.count - 1;

                        }
                    }

                    icon {
                        name: "list-remove-symbolic"
                    }

                }

            }

        }

    }

}
