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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.12
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.DtThemeBackground {
    property alias area: notAddArea
    signal changePosition()
    backgroundColor: Platform.GlobalTheme.baseActive
    radius: Platform.GlobalTheme.kRadiusWindow
    focus: true

    onChangePosition: index => {
                          notAddArea.flowId.changePosition();
                      }

    ColumnLayout {
        id: panelLayout
        width: parent.width
        height: parent.height
        spacing: 16

        anchors.top: parent.top
        anchors.topMargin: 14
        anchors.bottomMargin: 24

        Item {
            Layout.preferredWidth: parent.width - 40
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                anchors.fill: parent

                UkuiItems.DtThemeText {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter

                    text: qsTr("No add components")
                    textColor: Platform.GlobalTheme.kFontPrimary
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }

        EditShortcutArea {
            id: notAddArea
            editType: PluginState.NotAdd
            Layout.preferredWidth: flowId.width
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
