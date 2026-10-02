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
    property int itemWidth: 480
    property int itemHeight: 435
    property int itemCheckBoxHeight: 20
    property int itemLineEditHeight: 36
    property int itemSwitchHeight: 24
    property int itemSpacing: 16
    property bool isCentered: true
    property bool enableBackground: false
    property string windowTitle: ""
    property string uniqueName: ""

    function restoreAction() {
        // 重置时清空临时映射
        if (shortcutController)
            shortcutController.clearTempFcitxShortcutMap();

        configManager.restoreDefaultImConfig();
    }

    function cancelAction() {
        // 取消时清空临时映射
        if (shortcutController)
            shortcutController.clearTempFcitxShortcutMap();

        imConfigPage.configGroups = null;
        configManager.releaseImConfigResource();
        window.destroy();
    }

    function saveAction() {
        updatePendingShortcutMaps();
        configManager.saveImConfig();
        imConfigPage.configGroups = null;
        configManager.releaseImConfigResource();
        window.destroy();
    }

    function addPendingShortcutUpdate(name, oldShortcut, newShortcut) {
        // 更新ShortcutController中的临时映射
        if (shortcutController) {
            var fixedUniqueName = uniqueName === "" ? name : uniqueName + "_" + name;
            shortcutController.updateTempFcitxShortcutMap(fixedUniqueName, oldShortcut, newShortcut);
        }
    }

    function updatePendingShortcutMaps() {
        // 保存时，将临时映射合并到真实映射
        if (shortcutController)
            shortcutController.mergeTempFcitxShortcutMap();

    }

    title: windowTitle + "-" + qsTr("Config")
    onClosing: {
        // 关闭时清空临时映射
        if (shortcutController)
            shortcutController.clearTempFcitxShortcutMap();

        imConfigPage.configGroups = null;
        configManager.releaseImConfigResource();
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

    Connections {
        function onImConfigurationChanged() {
        }

        target: configManager
    }

    UkuiItems.DtThemeBackground {
        id: imBackGround

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ConfigPageTemplate {
            id: imConfigPage

            function openExternalConfig(uri, name) {
                configManager.getExternalConfig(uri);
                var externalConfigGroups = configManager.externalConfig;
                var isEmpty = !externalConfigGroups || externalConfigGroups.length === 0;
                if (isEmpty) {
                    var emptyDialog = emptyConfigDialogComponent.createObject(parent);
                    emptyDialog.show();
                    configManager.releaseExternalConfigResource();
                } else {
                    var externalSubConfig = externalComponent.createObject(parent);
                    externalSubConfig.windowTitle = name;
                    externalSubConfig.show();
                }
            }

            source: uniqueName
            anchors.topMargin: 8
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 435
            width: window.width
            itemSpacing: window.itemSpacing
            itemCheckBoxHeight: window.itemCheckBoxHeight
            itemLineEditHeight: window.itemLineEditHeight
            itemSwitchHeight: window.itemSwitchHeight
            autoSave: false
            enableBackground: window.enableBackground
            groupTitlebold: false
            configGroups: configManager.imConfig
            openExternalFunction: openExternalConfig
            delayedShortcutUpdateHandler: addPendingShortcutUpdate
        }

        UkuiItems.DtThemeBackground {
            id: borderLine

            useStyleTransparency: false
            height: 1
            anchors.top: imConfigPage.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.preferredHeight: 1
            backgroundColor: GlobalTheme.kContainGeneralNormal
            borderColor: GlobalTheme.kLineNormal
            border.width: 1
            borderAlpha: 1
        }

        RowLayout {
            anchors.top: borderLine.bottom
            anchors.topMargin: 16
            width: parent.width
            spacing: 8
            visible: !window.autoSave

            CustomButton {
                Layout.leftMargin: 24
                Layout.alignment: Qt.AlignLeft
                labelText: qsTr("Restore")
                onCustomButtonClicked: restoreAction()
            }

            Item {
                Layout.fillWidth: true
            }

            CustomButton {
                labelText: qsTr("Cancel")
                Layout.alignment: Qt.AlignRight
                onCustomButtonClicked: cancelAction()
            }

            CustomButton {
                enableHighlight: true
                labelText: qsTr("OK")
                Layout.rightMargin: 24
                Layout.alignment: Qt.AlignRight
                onCustomButtonClicked: saveAction()
            }

        }

    }

    Component {
        id: externalComponent

        ExternalSubConfig {
            id: externalSubConfig
        }

    }

    Component {
        id: emptyConfigDialogComponent

        EmptyConfigWarning {
        }

    }

}
