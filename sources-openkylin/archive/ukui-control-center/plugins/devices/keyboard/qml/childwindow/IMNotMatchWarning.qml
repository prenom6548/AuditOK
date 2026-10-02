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

    function cancelAction() {
        window.destroy();
    }

    function checkNameExists(targetName) {
        for (var i = 0; i < imModel.currentGroupInputMethodEntries.length; i++) {
            if (imModel.currentGroupInputMethodEntries[i].uniqueName === targetName)
                return true;

        }
        return false;
    }

    function saveAction() {
        if (!checkNameExists(imContent.imName)) {
            console.log(imContent.imName, "not added current im list");
            imModel.addInputMethod(imContent.imName);
            imModel.updateCurrentGroupInputMethodList();
        }
        layoutManager.joinImEntries(imContent.imName);
        layoutManager.save();
        window.destroy();
    }

    title: qsTr("System Keyboard Layout")
    width: 440
    height: 200 // 固定高度，避免循环绑定
    minimumWidth: 440
    minimumHeight: 180
    maximumWidth: 440
    maximumHeight: 300 // 设置最大高度允许文本换行
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint

    UkuiItems.DtThemeBackground {
        id: imNotMatchWarnBackGround

        width: parent.width
        height: parent.height
        anchors.fill: parent
        useStyleTransparency: false
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: buttonLayout

            anchors.fill: parent
            anchors.topMargin: 16
            anchors.bottomMargin: 16
            width: parent.width
            spacing: 16

            UkuiItems.DtThemeText {
                id: warningText

                Layout.fillWidth: true
                Layout.fillHeight: true
                wrapMode: Text.Wrap
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Your currently configured input method does not match your selected layout, do you want to add the corresponding input method for the layout?")
            }

            UkuiItems.DtThemeBackground {
                id: borderLine

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
                Layout.bottomMargin: 8
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
