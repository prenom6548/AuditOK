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

    property int dragAreaHeight: 40
    property int itemHeight: 435
    property int itemCheckBoxHeight: 20
    property int itemLineEditHeight: 36
    property int itemSwitchHeight: 24
    property int itemSpacing: 16
    property bool isCentered: true
    property bool enableBackground: false
    property string windowTitle: ""
    readonly property int calculatedHeight: {
        if (!mainLayout || !borderLine || !buttonRowLayout)
            return 0;

        var contentHeight = mainLayout.anchors.topMargin + externalConfigPage.implicitHeight;
        var separatorHeight = borderLine.height + borderLine.anchors.bottomMargin;
        var buttonsHeight = buttonRowLayout.implicitHeight + buttonRowLayout.anchors.bottomMargin;
        return contentHeight + separatorHeight + buttonsHeight;
    }

    function restoreAction() {
        configManager.restoreDefaultExternalConfig();
    }

    function cancelAction() {
        externalConfigPage.configGroups = null;
        configManager.releaseExternalConfigResource();
        window.destroy();
    }

    function saveAction() {
        configManager.saveExternalConfig();
        externalConfigPage.configGroups = null;
        configManager.releaseExternalConfigResource();
        window.destroy();
    }

    onClosing: {
        externalConfigPage.configGroups = null;
        configManager.releaseExternalConfigResource();
        window.destroy();
    }
    width: 480
    minimumWidth: 480
    minimumHeight: 380
    maximumHeight: 520
    height: Math.min(520, Math.max(280, calculatedHeight))
    title: windowTitle + "-" + qsTr("Config")
    maximumWidth: 480
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint

    Connections {
        function onImConfigurationChanged() {
        }

        target: configManager
    }

    UkuiItems.DtThemeBackground {
        id: externalBackGround

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: mainLayout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: borderLine.top
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            anchors.leftMargin: 0
            anchors.rightMargin: 0
            spacing: 0
            clip: true

            ConfigPageTemplate {
                id: externalConfigPage

                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: implicitHeight
                width: window.width
                itemSpacing: window.itemSpacing
                itemCheckBoxHeight: window.itemCheckBoxHeight
                itemLineEditHeight: window.itemLineEditHeight
                itemSwitchHeight: window.itemSwitchHeight
                autoSave: false
                enableBackground: window.enableBackground
                configGroups: configManager.externalConfig
                enableListOption: true
            }

        }

        UkuiItems.DtThemeBackground {
            id: borderLine

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonRowLayout.top
            anchors.bottomMargin: 16
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
            visible: !window.autoSave

            CustomButton {
                Layout.alignment: Qt.AlignLeft
                labelText: qsTr("Restore")
                onCustomButtonClicked: restoreAction()
            }

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

}
