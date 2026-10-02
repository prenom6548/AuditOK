/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

MouseArea {
    id: root
    property alias icon: appIcon
    property alias text: appName
    property alias background: styleBackground
    property bool isRecentInstalled: false
    property bool hasFocus: false

    hoverEnabled: true
    activeFocusOnTab: true

    pressAndHoldInterval: 300

    UkuiItems.Tooltip {
        anchors.fill: parent
        mainText: text.text
        posFollowCursor: true
        active: text.truncated
    }

    UkuiItems.DtThemeBackground {
        id: styleBackground
        anchors.fill: parent
        radius: Platform.GlobalTheme.kRadiusMax
        useStyleTransparency: false
        backgroundColor: root.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                            : root.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                                                 : Platform.GlobalTheme.kContainGeneralAlphaNormal
        border.width: root.hasFocus ? 2 : 0
        borderColor: Platform.GlobalTheme.kBrandFocus

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 16
            anchors.bottomMargin: 16
            spacing: 8

            UkuiItems.Icon {
                id: appIcon
                Layout.preferredWidth: styleBackground.width * 0.6
                Layout.preferredHeight: width
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: appName.contentHeight
                Layout.maximumHeight: appName.contentHeight
                Layout.alignment: Qt.AlignHCenter

                UkuiItems.DtThemeBackground {
                    id: tagPoint
                    property int tagSize: isRecentInstalled ? 8 : 0
                    Layout.alignment: Qt.AlignVCenter
                    visible: isRecentInstalled
                    Layout.preferredHeight: tagSize
                    Layout.preferredWidth: tagSize
                    radius: width / 2
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.highlightActive
                }

                UkuiItems.DtThemeText {
                    id: appName
                    Layout.maximumWidth: styleBackground.width - tagPoint.width
                    Layout.preferredHeight: contentHeight
                    Layout.maximumHeight: contentHeight

                    elide: Text.ElideRight
                    textColor: Platform.GlobalTheme.textActive
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
