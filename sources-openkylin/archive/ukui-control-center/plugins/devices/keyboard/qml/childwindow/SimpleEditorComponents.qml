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

Item {
    id: root

    property var window: null
    property var editorComponentForType: null

    function getSimpleEditorWrapper() {
        return simpleEditorWrapper;
    }

    function getStringEditor() {
        return stringEditor;
    }

    function getBooleanEditor() {
        return booleanEditor;
    }

    function getEnumEditor() {
        return enumEditor;
    }

    Component {
        id: simpleEditorWrapper

        Loader {
            id: simpleEditorLoader

            Layout.fillWidth: true
            Layout.preferredWidth: 260
            sourceComponent: root.editorComponentForType ? root.editorComponentForType() : null
            onLoaded: {
                if (item && item.forceActiveFocus)
                    item.forceActiveFocus();

            }
        }

    }

    Component {
        id: stringEditor

        TextField {
            id: textFieldEditor

            Layout.fillWidth: true
            text: {
                if (root.window && root.window.workingValue !== null && root.window.workingValue !== undefined)
                    return root.window.workingValue.toString();

                return "";
            }
            selectByMouse: true
            focus: true
            onTextChanged: {
                if (root.window)
                    root.window.workingValue = text;

            }
            Keys.onReturnPressed: {
                if (root.window)
                    root.window.saveAction();

            }
        }

    }

    Component {
        id: booleanEditor

        Switch {
            checked: root.window ? root.window.toBool(root.window.workingValue) : false
            onToggled: {
                if (root.window)
                    root.window.workingValue = checked;

            }
        }

    }

    Component {
        id: enumEditor

        ComboBox {
            property var enumOptions: root.window ? root.window.enumOptions : []

            Layout.preferredWidth: 260
            model: {
                var texts = [];
                for (var i = 0; i < enumOptions.length; ++i) {
                    texts.push(enumOptions[i].text);
                }
                return texts;
            }
            currentIndex: {
                if (!root.window || enumOptions.length === 0)
                    return 0;

                var currentValue = root.window.workingValue;
                for (var i = 0; i < enumOptions.length; ++i) {
                    if (enumOptions[i].value === currentValue)
                        return i;

                }
                return 0;
            }
            onActivated: function(index) {
                if (root.window && index >= 0 && index < enumOptions.length)
                    root.window.workingValue = enumOptions[index].value;

            }
            onCurrentIndexChanged: {
                if (root.window && currentIndex >= 0 && currentIndex < enumOptions.length)
                    root.window.workingValue = enumOptions[currentIndex].value;

            }
        }

    }

}
