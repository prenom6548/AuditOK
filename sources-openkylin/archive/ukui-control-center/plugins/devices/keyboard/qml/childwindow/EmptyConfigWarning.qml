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
    id: emptyConfigDialog

    property int layoutMargins: 24
    property int layoutSpacing: 8

    function acceptAction() {
        emptyConfigDialog.destroy();
    }

    onClosing: acceptAction()
    width: 400
    height: warningLayout.implicitHeight + 32
    minimumWidth: 400
    minimumHeight: 152
    maximumWidth: 400
    maximumHeight: 200
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | Qt.WindowCloseButtonHint
    title: " "

    UkuiItems.DtThemeBackground {
        useStyleTransparency: false
        anchors.fill: parent
        clip: true
        backgroundColor: GlobalTheme.kContainGeneralNormal

        ColumnLayout {
            id: warningLayout

            anchors.fill: parent
            anchors.topMargin: 16
            anchors.bottomMargin: 16
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
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("The Config is Empty")
                }

            }

            Item {
                Layout.fillWidth: true
                height: layoutSpacing * 2
            }

            UkuiItems.DtThemeBackground {
                Layout.fillWidth: true
                Layout.leftMargin: 0
                Layout.rightMargin: 0
                Layout.preferredHeight: 1
                height: 1
                backgroundColor: GlobalTheme.kContainGeneralNormal
                borderColor: GlobalTheme.kLineNormal
                border.width: 1
                borderAlpha: 1
                useStyleTransparency: false
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
                    labelText: qsTr("OK")
                    Layout.alignment: Qt.AlignRight
                    onCustomButtonClicked: acceptAction()
                }

            }

        }

    }

}
