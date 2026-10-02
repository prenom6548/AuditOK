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

    property bool isEditMode: false
    property string valueType: "String"
    property var enumOptions: []
    property var initialValue: ""
    property string labelText: qsTr("Value")
    property int itemIndex: -1
    readonly property int calculatedHeight: {
        if (!mainColumnLayout || !borderLine || !buttonRowLayout)
            return 0;

        var contentHeight = mainColumnLayout.anchors.topMargin + mainColumnLayout.implicitHeight + mainColumnLayout.anchors.bottomMargin;
        var separatorHeight = borderLine.height + borderLine.anchors.bottomMargin;
        var buttonsHeight = buttonRowLayout.implicitHeight + buttonRowLayout.anchors.bottomMargin;
        return contentHeight + separatorHeight + buttonsHeight;
    }

    signal confirmed(var value, int index)
    signal cancelled()

    function cancelAction() {
        cancelled();
        window.destroy();
    }

    function saveAction() {
        var value = getInputValue();
        if (valueType === "String" && value.trim() === "")
            return ;

        confirmed(value, itemIndex);
        window.destroy();
    }

    function getInputValue() {
        if (valueType === "Enum") {
            if (textInputLoader.item && textInputLoader.item.currentIndex >= 0 && textInputLoader.item.currentIndex < enumOptions.length)
                return enumOptions[textInputLoader.item.currentIndex].value;

            return "";
        }
        if (textInputLoader.item)
            return textInputLoader.item.text.trim();

        return "";
    }

    title: {
        var base = labelText && labelText.trim() !== "" ? labelText.trim() : qsTr("Config");
        return base + "-" + (isEditMode ? qsTr("Edit") : qsTr("Add"));
    }
    width: 480
    height: Math.max(calculatedHeight, 180)
    minimumWidth: 480
    minimumHeight: 180
    maximumWidth: 480
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint
    Component.onCompleted: {
        if (valueType === "String" && textInputLoader.item)
            textInputLoader.item.forceActiveFocus();

    }

    UkuiItems.DtThemeBackground {
        id: dialogBackground

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: mainColumnLayout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 12
            anchors.bottom: borderLine.top
            anchors.bottomMargin: 38
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            spacing: 16
            clip: true

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                UkuiItems.DtThemeText {
                    Layout.preferredWidth: window.labelText && window.labelText.trim() !== "" ? 80 : 0
                    visible: window.labelText && window.labelText.trim() !== ""
                    text: window.labelText
                }

                Item {
                    Layout.fillWidth: window.labelText && window.labelText.trim() !== ""
                }

                Loader {
                    id: textInputLoader

                    Layout.preferredWidth: 260
                    sourceComponent: valueType === "Enum" ? enumInputComp : textInputComp
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
                onCustomButtonClicked: saveAction()
            }

        }

    }

    Component {
        id: textInputComp

        TextField {
            id: valueInput

            text: initialValue ? initialValue.toString() : ""
            selectByMouse: true
            focus: true
            onAccepted: saveAction()
        }

    }

    Component {
        id: enumInputComp

        ComboBox {
            id: enumCombo

            model: enumOptions.map(function(item) {
                return item.text;
            })
            currentIndex: {
                var idx = -1;
                for (var i = 0; i < enumOptions.length; ++i) {
                    if (enumOptions[i].value === initialValue) {
                        idx = i;
                        break;
                    }
                }
                return idx >= 0 ? idx : 0;
            }
        }

    }

}
