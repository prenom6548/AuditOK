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

    property string conflictName: ""
    property string conflictDescription: ""
    property string conflictSource: ""
    property string conflictShortcut: ""
    property bool showInputMethodConflictInfo: conflictSource === "InputMethod" || conflictSource === "Global"
    property int layoutMargins: 24
    property int layoutSpacing: 8

    signal continueWithAccept()
    signal restoreOriginal()

    // 取消
    function cancelAction() {
        restoreOriginal();
        window.destroy();
    }

    // 继续
    function continueAction() {
        continueWithAccept();
        window.destroy();
    }

    // 确认
    function acceptAction() {
        restoreOriginal();
        window.destroy();
    }

    onClosing: {
        restoreOriginal();
        window.destroy();
    }
    width: 452
    height: warningLayout.height
    minimumWidth: 452
    minimumHeight: 152
    maximumWidth: 452
    maximumHeight: 192
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint
    title: " "

    UkuiItems.DtThemeBackground {
        id: shortcutConflictWarnBackGround

        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: warningLayout

            width: parent.width
            spacing: layoutSpacing

            Item {
                Layout.fillWidth: true
                height: 2
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins
                spacing: layoutSpacing

                UkuiItems.Icon {
                    width: 16
                    height: 16
                    source: "dialog-warning"
                    mode: UkuiItems.Icon.AutoHighlight
                }

                UkuiItems.DtThemeText {
                    id: text

                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Conflicting key combination, please change the key combination.")
                }

            }

            UkuiItems.DtThemeText {
                id: informativeText

                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textColor: GlobalTheme.kFontSecondary
                verticalAlignment: Text.AlignVCenter
                pointSizeOffset: -1
                Layout.leftMargin: layoutMargins * 1.65
                Layout.rightMargin: layoutMargins
                text: showInputMethodConflictInfo ? qsTr("\"%1\" is already occupied by other input method functions.").arg(conflictShortcut) : qsTr("\"%1\" is already occupied by system.").arg(conflictShortcut)
            }

            Item {
                Layout.fillWidth: true
                height: layoutSpacing * 2
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: layoutMargins
                Layout.rightMargin: layoutMargins
                spacing: layoutSpacing

                Item {
                    Layout.fillWidth: true
                }

                CustomButton {
                    highlighted: true
                    text: qsTr("Ok")
                    Layout.alignment: Qt.AlignRight
                    onClicked: acceptAction()
                }

            }

            Item {
                Layout.fillWidth: true
                height: layoutSpacing * 2
            }

        }

    }

}
