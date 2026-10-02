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

    property int dragAreaHeight: 40
    property int itemHeight: 36
    property bool enableBackground: false

    function cancelAction() {
        window.destroy();
    }

    function saveAction() {
        if (layoutComboBox.currentIndex >= 0 && variantComboBox.currentIndex >= 0) {
            var layout = layoutManager.layout(layoutComboBox.currentIndex, variantComboBox.currentIndex);
            var imName = "keyboard-%0".arg(layout);
            console.log("layout:" + layout + ", imName:" + imName + ",current key:" + layoutManager.imEntries[0].key);
            layoutManager.defaultLayout = layout;
            if (layoutManager.imEntries.length === 0 || imName !== layoutManager.imEntries[0].key) {
                imContent.imName = imName;
                imContent.showWarning = true;
            }
        }
        window.destroy();
    }

    title: qsTr("System Keyboard Layout")
    width: 480
    height: 248
    minimumWidth: 480
    minimumHeight: 248
    maximumWidth: 480
    maximumHeight: 248
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint

    UkuiItems.DtThemeBackground {
        id: layoutBackGround

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: columnList

            anchors.top: parent.top
            anchors.topMargin: 16
            width: parent.width
            spacing: 16

            LayoutComboBox {
                id: languageComboBox

                width: parent.width
                height: itemHeight
                enableBackground: false
                labelText: qsTr("Language")
                texts: layoutManager.languageModel
                currentIndex: 0
                onComboBoxIndexChanged: (value, idx) => {
                    layoutManager.layoutModel.language = texts.language(value);
                    layoutComboBox.currentIndex = 0;
                    layoutManager.setVariantInfo(layoutManager.layoutModel.layoutInfo(layoutComboBox.currentIndex));
                    variantComboBox.currentIndex = 0;
                }
            }

            LayoutComboBox {
                id: layoutComboBox

                width: parent.width
                height: itemHeight
                enableBackground: false
                texts: layoutManager.layoutModel
                labelText: qsTr("Layout")
                currentIndex: 0
                onComboBoxIndexChanged: (value, idx) => {
                    if (currentIndex < 0)
                        return ;

                    layoutManager.setVariantInfo(texts.layoutInfo(currentIndex));
                    variantComboBox.currentIndex = 0;
                }
            }

            LayoutComboBox {
                id: variantComboBox

                width: parent.width
                height: itemHeight
                enableBackground: false
                texts: layoutManager.variantModel
                currentIndex: 0
                labelText: qsTr("Variant")
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
                Layout.leftMargin: 24
                Layout.rightMargin: 24
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

    Connections {
        function onLayoutChanged(currentLayout) {
            console.log("onLayoutChanged: " + currentLayout);
            var dashPos = currentLayout.indexOf("-");
            var layout, variant;
            if (dashPos >= 0) {
                variant = currentLayout.substring(dashPos + 1);
                layout = currentLayout.substring(0, dashPos);
            } else {
                layout = currentLayout;
            }
            languageComboBox.currentIndex = 0;
            layoutComboBox.currentIndex = layoutManager.layoutIndex(layout);
            if (variant === "")
                variantComboBox.currentIndex = 0;
            else
                variantComboBox.currentIndex = layoutManager.variantIndex(variant);
        }

        target: layoutManager
    }

}
