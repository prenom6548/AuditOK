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
import "configitem"
import org.ukui.quick.items 1.0 as UkuiItems
import "utils.js" as UtilsScripts

Loader {
    id: itemLoader

    property var itemData
    property int index: -1
    property int itemWidth: 432
    property int itemHeight: 432
    property int itemCheckBoxHeight: 0
    property int itemLineEditHeight: 0
    property int itemSwitchHeight: 0
    property int itemLayoutMargins: 0
    property bool autoSave: false
    property bool useCustomTranslate: false
    property bool enableBackground: true
    property var saveHandler: null
    property var openExternalHandler: null
    property var delayedShortcutUpdateHandler: null
    property bool enableVCenter: true
    property string source: ""
    property bool enableListOption: false

    function handleValueChange() {
        if (autoSave && saveHandler)
            saveHandler();

    }

    function handleOpenExternal(uri, name) {
        if (openExternalHandler)
            openExternalHandler(uri, name);

    }

    function isExcludedName(name) {
        if (UtilsScripts.getExcludeFields().includes(name))
            return true;

        return false;
    }

    sourceComponent: {
        if (isExcludedName(itemData.name))
            return null;

        switch (itemData.type) {
        case "Boolean":
            if (itemData.name === "ActiveByDefault" || itemData.name === "CloudPinyinEnabled")
                return switchComp;

            return checkboxComp;
        case "Key":
            return shortcutComp;
        case "KeyList":
            return shortcutComp;
        case "Integer":
            return spinboxComp;
        case "Enum":
            return comboBoxComp;
        case "String":
            return lineEditComp;
        case "External":
            return externalComp;
        case "List":
            if (enableListOption)
                return listComp;

            return null;
        default:
            return null;
        }
    }
    onLoaded: {
        if (!item || !itemData)
            return ;

        item.labelText = useCustomTranslate ? UtilsScripts.getTranslation(itemData.name) || "" : itemData.desc;
        if (itemData.name === "LongWordLengthLimit")
            item.labelText = UtilsScripts.getTranslation(itemData.name) || "";

        item.itemIndex = index;
        switch (itemData.type) {
        case "Boolean":
            break;
        case "Integer":
            break;
        case "Enum":
            break;
        default:
            break;
        }
        itemData.valueChanged.connect(handleValueChange);
    }

    Component {
        id: checkboxComp

        ConfigCheckBox {
            id: checkBox

            enableVCenter: itemLoader.enableVCenter
            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemCheckBoxHeight === 0 ? itemHeight : itemCheckBoxHeight
            enableBackground: itemLoader.enableBackground
            value: itemData.value
            onCheckBoxChanged: (value, idx) => {
                if (value !== itemData.value)
                    itemData.value = value;

            }

            Connections {
                function onValueChanged() {
                    value = Qt.binding(() => {
                        return itemData.value;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: switchComp

        ConfigSwitch {
            id: switchButton

            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemCheckBoxHeight === 0 ? itemHeight : itemCheckBoxHeight
            enableBackground: itemLoader.enableBackground
            value: itemData.value
            onSwitchChanged: {
                if (value !== itemData.value)
                    itemData.value = value;

            }

            Connections {
                function onValueChanged() {
                    value = Qt.binding(() => {
                        return itemData.value;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: shortcutComp

        ConfigShortCut {
            id: shortcut

            source: itemLoader.source
            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            enableBackground: itemLoader.enableBackground
            value: itemData.value
            name: itemData.name
            allowModifierLess: itemData.allowModifierLess
            allowModifierOnly: itemData.allowModifierOnly
            onShortcutChanged: (value, idx) => {
                if (value !== itemData.value)
                    itemData.value = value;

            }
            onShortcutUpdateRequested: (name, oldValue, newValue) => {
                if (itemLoader.autoSave && shortcutController)
                    shortcutController.updateFcitxShortcutMap(name, oldValue, newValue);
                else if (itemLoader.delayedShortcutUpdateHandler)
                    itemLoader.delayedShortcutUpdateHandler(name, oldValue, newValue);
                else
                    console.warn('ConfigItemLoader: delayedShortcutUpdateHandler is null!');
            }

            Connections {
                function onValueChanged() {
                    value = Qt.binding(() => {
                        return itemData.value;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: comboBoxComp

        ConfigComboBox {
            id: comboBox

            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            enableBackground: itemLoader.enableBackground
            texts: itemData.texts
            currentIndex: itemData.index
            onComboBoxIndexChanged: (value, idx) => {
                if (value !== itemData.index)
                    itemData.index = value;

            }

            Connections {
                function onValueChanged() {
                    currentIndex = Qt.binding(() => {
                        return itemData.index;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: spinboxComp

        ConfigSpinBox {
            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            minValue: itemData.min
            maxValue: itemData.max === 0 ? 100 : itemData.max
            value: itemData.value
            enableBackground: itemLoader.enableBackground
            onSpinValueChanged: itemData.value = value

            Connections {
                function onValueChanged() {
                    value = Qt.binding(() => {
                        return itemData.value;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: lineEditComp

        ConfigLineEdit {
            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            value: itemData.value
            enableBackground: itemLoader.enableBackground
            onLineTextChanged: itemData.value = text

            Connections {
                function onValueChanged() {
                    value = Qt.binding(() => {
                        return itemData.value;
                    });
                }

                target: itemData
            }

        }

    }

    Component {
        id: externalComp

        ConfigExternal {
            layoutMargins: itemLayoutMargins
            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            enableBackground: itemLoader.enableBackground
            uri: itemData.uri
            name: itemData.desc
            launchSubConfig: itemData.launchSubConfig
            onExternalConfigChanged: {
                handleOpenExternal(uri, name);
            }
            onLaunchExternalConfig: {
                itemData.handleLaunchExtenalConfig();
            }
        }

    }

    Component {
        id: listComp

        ConfigList {
            id: listView

            layoutMargins: itemLayoutMargins
            width: itemWidth
            enableBackground: itemLoader.enableBackground
            listModel: itemLoader.itemData.model
            itemData: itemLoader.itemData

            Connections {
                function onValueChanged() {
                    if (itemLoader.itemData)
                        listModel = Qt.binding(() => {
                        return itemLoader.itemData.model;
                    });

                }

                target: itemLoader.itemData
            }

        }

    }

    Component {
        id: unknownTypeComp

        UkuiItems.DtThemeText {
            id: label

            property int itemIndex: -1
            property alias labelText: label.text
            property bool enableBackground: false

            width: itemWidth
            height: itemLineEditHeight === 0 ? itemHeight : itemLineEditHeight
            text: "Unsupported type: " + modelData.type
            color: "green"
        }

    }

}
