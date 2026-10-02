/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
import QtQuick.Controls 2.12
import org.ukui.menu.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    property var header: null
    property string title: ""

    clip: true
    visible: header !== null && header.visible

    onFocusChanged: {
        if (focus) actionsItem.view.forceActiveFocus();;
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 5

        UkuiItems.DtThemeText {
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
            font.weight: Font.Bold
            text: root.title === "" ? header.title : root.title

            UkuiItems.Tooltip {
                anchors.fill: parent
                mainText: parent.text
                posFollowCursor: true
                visible: parent.truncated
            }
        }

        AppListActions {
            id: actionsItem
            Layout.preferredWidth: width
            Layout.preferredHeight: 36
            Layout.topMargin: 5
            Layout.bottomMargin: 5
            Layout.alignment: Qt.AlignVCenter

            actions: header.actions
        }
    }
}
