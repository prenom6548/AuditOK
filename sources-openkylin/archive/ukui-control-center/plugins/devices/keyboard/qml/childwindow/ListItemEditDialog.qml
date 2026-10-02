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
import QtQuick.Window 2.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Window {
    id: window

    property var structuredFields: null
    property var simpleEditors: null
    property bool isEditMode: false
    property var initialValue: null
    property string labelText: qsTr("Value")
    property int itemIndex: -1
    property var itemData: null
    property string valueType: itemData && itemData.subType ? itemData.subType : ""
    property var fieldDefinitions: itemData ? itemData.subFields : []
    property bool isStructuredMode: fieldDefinitions && fieldDefinitions.length > 0
    property var defaultValue: itemData ? itemData.defaultSubValue : null
    property var workingValue: null
    property var enumOptions: itemData ? itemData.subEnumOptions : []
    property string displayLabel: {
        if (itemData && itemData.subDescription && itemData.subDescription.length > 0)
            return itemData.subDescription;

        return labelText;
    }
    property var structuredValue: ({
    })
    readonly property int calculatedHeight: {
        if (!columnLayout || !borderLine || !buttonRowLayout)
            return 0;

        var contentHeight = columnLayout.anchors.topMargin + columnLayout.implicitHeight + columnLayout.anchors.bottomMargin;
        var borderHeight = borderLine.height + borderLine.anchors.bottomMargin;
        var buttonHeight = buttonRowLayout.implicitHeight + buttonRowLayout.anchors.bottomMargin;
        return contentHeight + borderHeight + buttonHeight;
    }

    signal confirmed(var value, int index)
    signal cancelled()

    function cancelAction() {
        cancelled();
        window.destroy();
    }

    function saveAction() {
        if (!canSave() || !itemData || typeof itemData.serializeEditValue !== "function")
            return ;

        var valueToSave = isStructuredMode ? structuredValue : workingValue;
        var serialized = itemData.serializeEditValue(valueToSave);
        confirmed(serialized, itemIndex);
        window.destroy();
    }

    function canSave() {
        if (isStructuredMode)
            return isStructuredDataValid();

        if (valueType === "" || valueType === "String")
            return workingValue && workingValue.toString().trim() !== "";

        return workingValue !== undefined && workingValue !== null;
    }

    function isStructuredDataValid() {
        if (!structuredValue || !fieldDefinitions)
            return false;

        for (var i = 0; i < fieldDefinitions.length; ++i) {
            var field = fieldDefinitions[i];
            var value = getStructuredFieldValue(field);
            if ((field.type === "String" || field.type === "Key") && (!value || value.toString().trim() === ""))
                return false;

        }
        return true;
    }

    function updateWorkingValue() {
        if (!itemData || typeof itemData.prepareEditValue !== "function")
            return ;

        var prepared = itemData.prepareEditValue(initialValue);
        if (prepared !== undefined && prepared !== null) {
            workingValue = prepared;
            if (valueType === "Enum" && enumOptions.length > 0) {
                var found = false;
                for (var i = 0; i < enumOptions.length; ++i) {
                    if (enumOptions[i].value === workingValue) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    workingValue = enumOptions[0].value;

            }
        }
    }

    function editorComponentForType() {
        if (!simpleEditors)
            return null;

        if (valueType === "Boolean")
            return simpleEditors.getBooleanEditor();

        if (valueType === "Enum" && enumOptions.length > 0)
            return simpleEditors.getEnumEditor();

        return simpleEditors.getStringEditor();
    }

    function initializeStructuredValue() {
        if (!isStructuredMode) {
            structuredValue = {
            };
            return ;
        }
        if (!itemData || typeof itemData.prepareEditValue !== "function") {
            structuredValue = {
            };
            return ;
        }
        var prepared = itemData.prepareEditValue(initialValue);
        structuredValue = (prepared && typeof prepared === "object") ? prepared : {
        };
    }

    function getStructuredFieldValue(field) {
        if (!field || !field.name)
            return "";

        var value = structuredValue && structuredValue[field.name];
        if (value === undefined || value === null)
            value = field.defaultValue;

        return value;
    }

    function updateStructuredField(name, value) {
        if (!structuredValue)
            structuredValue = {
        };

        if (structuredValue[name] === value)
            return ;

        var copy = {
        };
        for (var key in structuredValue) {
            copy[key] = structuredValue[key];
        }
        copy[name] = value;
        structuredValue = copy;
    }

    function structuredComponentFor(field) {
        if (!structuredFields)
            return null;

        if (field.type === "Boolean")
            return structuredFields.getBooleanFieldComponent();

        if (field.type === "Enum")
            return structuredFields.getEnumFieldComponent();

        if (field.type === "List|String")
            return structuredFields.getStringListFieldComponent();

        return structuredFields.getStringFieldComponent();
    }

    function initializeComponents() {
        if (!structuredFields) {
            structuredFields = Qt.createComponent("qrc:/qml/childwindow/StructuredFieldComponents.qml").createObject(window);
            if (structuredFields)
                structuredFields.window = window;

        }
        if (!simpleEditors) {
            simpleEditors = Qt.createComponent("qrc:/qml/childwindow/SimpleEditorComponents.qml").createObject(window);
            if (simpleEditors) {
                simpleEditors.window = window;
                simpleEditors.editorComponentForType = editorComponentForType;
            }
        }
        updateWorkingValue();
        initializeStructuredValue();
    }

    title: {
        var base = displayLabel && displayLabel.trim && displayLabel.trim() !== "" ? displayLabel.trim() : qsTr("Config");
        return base + "-" + (isEditMode ? qsTr("Edit") : qsTr("Add"));
    }
    width: 480
    height: {
        var totalHeight = calculatedHeight;
        if (isStructuredMode)
            return Math.min(Math.max(totalHeight, 300), 600);
        else
            return Math.max(totalHeight, 180);
    }
    minimumWidth: 480
    minimumHeight: isStructuredMode ? 300 : 180
    maximumWidth: 480
    maximumHeight: isStructuredMode ? 600 : 400
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint
    Component.onCompleted: {
        initializeComponents();
    }

    UkuiItems.DtThemeBackground {
        id: dialogBackground

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: columnLayout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 12
            anchors.bottom: borderLine.top
            anchors.bottomMargin: 38
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            spacing: 8
            clip: true

            UkuiItems.DtThemeText {
                id: label

                visible: !isStructuredMode
                text: window.displayLabel
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.preferredWidth: 80
                Layout.preferredHeight: isStructuredMode ? 0 : implicitHeight
            }

            Loader {
                id: editorLoader

                Layout.alignment: isStructuredMode ? Qt.AlignTop : (Qt.AlignRight | Qt.AlignVCenter)
                Layout.fillWidth: true
                Layout.preferredWidth: isStructuredMode ? -1 : 260
                sourceComponent: {
                    if (isStructuredMode)
                        return structuredFields ? structuredFields.getStructuredEditorComponent() : null;
                    else
                        return simpleEditors ? simpleEditors.getSimpleEditorWrapper() : null;
                }
            }

        }

        UkuiItems.DtThemeBackground {
            id: borderLine

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonRowLayout.top
            anchors.bottomMargin: 13
            height: 1
            backgroundColor: GlobalTheme.kContainGeneralNormal
            borderColor: GlobalTheme.kLineNormal
            border.width: 1
            borderAlpha: 1
            useStyleTransparency: false
        }

        RowLayout {
            id: buttonRowLayout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            anchors.bottomMargin: 24
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
                enabled: canSave()
                hoverEnabled: canSave()
                Layout.alignment: Qt.AlignRight
                onCustomButtonClicked: saveAction()
            }

        }

    }

}
