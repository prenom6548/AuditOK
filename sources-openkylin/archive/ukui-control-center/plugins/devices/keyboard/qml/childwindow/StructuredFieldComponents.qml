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
import "../configitem"
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property var window: null

    function getStringFieldComponent() {
        return structuredStringFieldComp;
    }

    function getBooleanFieldComponent() {
        return structuredBooleanFieldComp;
    }

    function getEnumFieldComponent() {
        return structuredEnumFieldComp;
    }

    function getStringListFieldComponent() {
        return structuredStringListFieldComp;
    }

    function getStructuredEditorComponent() {
        return structuredEditorComp;
    }

    Component {
        id: structuredStringFieldComp

        ConfigLineEdit {
            property var field
            property var windowObj: root.window

            Layout.fillWidth: true
            Layout.preferredHeight: 40
            layoutMargins: 0
            labelText: field && field.desc ? field.desc : (field ? field.name : "")
            value: windowObj ? windowObj.getStructuredFieldValue(field) : ""
            onLineTextChanged: function(text) {
                if (windowObj)
                    windowObj.updateStructuredField(field.name, text);

            }

            Connections {
                function onStructuredValueChanged() {
                    value = windowObj ? windowObj.getStructuredFieldValue(field) : "";
                }

                target: windowObj
            }

        }

    }

    Component {
        id: structuredBooleanFieldComp

        ConfigSwitch {
            property var field
            property var windowObj: root.window

            Layout.fillWidth: true
            Layout.preferredHeight: 40
            layoutMargins: 0
            labelText: field && field.desc ? field.desc : (field ? field.name : "")
            value: windowObj ? windowObj.getStructuredFieldValue(field) : false
            onSwitchChanged: function(state) {
                if (windowObj) {
                    windowObj.updateStructuredField(field.name, state);
                }
            }

            Connections {
                function onStructuredValueChanged() {
                    value = windowObj ? windowObj.getStructuredFieldValue(field) : false;
                }

                target: windowObj
            }

        }

    }

    Component {
        id: structuredEnumFieldComp

        ConfigComboBox {
            property var field
            property var windowObj: root.window
            property var enumOptions: field ? field.enumOptions : []

            Layout.fillWidth: true
            Layout.preferredHeight: 40
            layoutMargins: 0
            labelText: field && field.desc ? field.desc : (field ? field.name : "")
            texts: enumOptions.map(function(item) {
                return item.text;
            })
            currentIndex: {
                if (!windowObj || enumOptions.length === 0)
                    return 0;

                var currentValue = windowObj.getStructuredFieldValue(field);
                for (var i = 0; i < enumOptions.length; ++i) {
                    if (enumOptions[i].value === currentValue)
                        return i;

                }
                return 0;
            }
            onComboBoxIndexChanged: function(comboBoxIndex, itemIndex) {
                if (windowObj && comboBoxIndex >= 0 && comboBoxIndex < enumOptions.length)
                    windowObj.updateStructuredField(field.name, enumOptions[comboBoxIndex].value);

            }

            Connections {
                function onStructuredValueChanged() {
                    if (!windowObj || enumOptions.length === 0)
                        return ;

                    var value = windowObj.getStructuredFieldValue(field);
                    for (var i = 0; i < enumOptions.length; ++i) {
                        if (enumOptions[i].value === value) {
                            currentIndex = i;
                            return ;
                        }
                    }
                    currentIndex = 0;
                }

                target: windowObj
            }

        }

    }

    Component {
        id: structuredStringListFieldComp

        Item {
            property var field
            property var windowObj: root.window
            property var listAdapter: {
                if (!windowObj || !field)
                    return null;

                var subtype = field.type && field.type.startsWith("List|") ? field.type.slice(5) : "String";
                return {
                    "subType": subtype,
                    "subDescription": field.desc || field.name || "",
                    "subFields": [],
                    "subEnumOptions": field.enumOptions || [],
                    "defaultSubValue": "",
                    "model": listModel,
                    "getItem": function(index) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        return (index >= 0 && index < arr.length) ? arr[index] : "";
                    },
                    "addItem": function(value) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        arr.push(value);
                        windowObj.updateStructuredField(field.name, arr);
                        syncModel();
                    },
                    "editItem": function(index, value) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        if (index >= 0 && index < arr.length) {
                            arr[index] = value;
                            windowObj.updateStructuredField(field.name, arr);
                            syncModel();
                        }
                    },
                    "removeItem": function(index) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        if (index >= 0 && index < arr.length) {
                            arr.splice(index, 1);
                            windowObj.updateStructuredField(field.name, arr);
                            syncModel();
                        }
                    },
                    "moveUp": function(index) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        if (index > 0 && index < arr.length) {
                            var temp = arr[index];
                            arr[index] = arr[index - 1];
                            arr[index - 1] = temp;
                            windowObj.updateStructuredField(field.name, arr);
                            syncModel();
                        }
                    },
                    "moveDown": function(index) {
                        var values = windowObj.getStructuredFieldValue(field);
                        var arr = Array.isArray(values) ? values : [];
                        if (index >= 0 && index < arr.length - 1) {
                            var temp = arr[index];
                            arr[index] = arr[index + 1];
                            arr[index + 1] = temp;
                            windowObj.updateStructuredField(field.name, arr);
                            syncModel();
                        }
                    },
                    "prepareEditValue": function(rawValue) {
                        return (rawValue === undefined || rawValue === null) ? "" : rawValue.toString();
                    },
                    "serializeEditValue": function(editValue) {
                        return (editValue === undefined || editValue === null) ? "" : editValue.toString();
                    }
                };
            }

            function syncModel() {
                var values = windowObj ? windowObj.getStructuredFieldValue(field) : null;
                var arr = Array.isArray(values) ? values : [];
                listModel.clear();
                for (var i = 0; i < arr.length; ++i) {
                    listModel.append({
                        "display": arr[i]
                    });
                }
            }

            Layout.fillWidth: true
            implicitHeight: configList.implicitHeight
            Component.onCompleted: syncModel()

            ListModel {
                id: listModel
            }

            Connections {
                function onStructuredValueChanged() {
                    syncModel();
                }

                target: windowObj
            }

            ConfigList {
                id: configList

                anchors.fill: parent
                layoutMargins: 0
                listModel: listModel
                itemData: listAdapter
                labelText: field && field.desc ? field.desc : (field ? field.name : "")
            }

        }

    }

    Component {
        id: structuredEditorComp

        ScrollView {
            id: scrollView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.vertical.policy: columnLayout.implicitHeight > scrollView.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            contentWidth: scrollView.width
            contentHeight: columnLayout.implicitHeight

            ColumnLayout {
                id: columnLayout

                width: scrollView.contentWidth
                spacing: 6

                Repeater {
                    model: root.window ? root.window.fieldDefinitions : []

                    delegate: Loader {
                        property var fieldData: modelData
                        property bool isListType: fieldData && fieldData.type && fieldData.type.startsWith("List|")

                        Layout.fillWidth: true
                        Layout.preferredHeight: isListType ? (item ? item.implicitHeight : 200) : 40
                        sourceComponent: root.window ? root.window.structuredComponentFor(fieldData) : null
                        onLoaded: {
                            if (item) {
                                if (item.hasOwnProperty("field"))
                                    item.field = fieldData;

                                if (item.hasOwnProperty("layoutMargins"))
                                    item.layoutMargins = 0;

                            }
                        }
                        onItemChanged: {
                            if (item && isListType)
                                Layout.preferredHeight = item.implicitHeight > 0 ? item.implicitHeight : 200;

                        }
                    }

                }

            }

        }

    }

}
