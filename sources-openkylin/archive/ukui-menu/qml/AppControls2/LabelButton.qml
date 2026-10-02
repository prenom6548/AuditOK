/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

MouseArea {
    property bool isHighlight: false
    property bool iconForceRound: false
    property int layoutSpacing: 8
    property var sourceData: {"icon": "", "text": "", "tooltip": ""}

    readonly property alias icon: iconBase
    readonly property alias background: backgroundBase

    acceptedButtons: Qt.LeftButton | Qt.RightButton
    hoverEnabled: true

    UkuiItems.DtThemeBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        backgroundColor: parent.containsPress ? Platform.GlobalTheme.kContainAlphaClick
            : parent.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                : Platform.GlobalTheme.kContainGeneralAlphaNormal
        borderColor: Platform.GlobalTheme.kBrandFocus
        useStyleTransparency: false
        radius: Platform.GlobalTheme.kRadiusMin

        RowLayout {
            anchors.fill: parent
            spacing: layoutSpacing

            UkuiItems.Icon {
                id: iconBase
                Layout.preferredWidth: iconForceRound ? 24 : parent.height / 2
                Layout.preferredHeight: iconForceRound ? 24 : parent.height / 2
                Layout.leftMargin: (parent.height - iconBase.height) / 2
                Layout.rightMargin: 0
                Layout.alignment: Qt.AlignHCenter

                mode: isHighlight ? UkuiItems.Icon.Highlight : UkuiItems.Icon.AutoHighlight
                forceRound: iconForceRound
                source: sourceData.icon
            }

            UkuiItems.DtThemeText {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignHCenter

                textColor: isHighlight ? Platform.GlobalTheme.highlightedTextActive : Platform.GlobalTheme.textActive
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                text: sourceData.text
            }
        }
    }
    UkuiItems.Tooltip {
        anchors.fill: parent
        mainText: sourceData.tooltip
        posFollowCursor: true
        margin: 6
        visible: sourceData.tooltip !== ""
    }
}
