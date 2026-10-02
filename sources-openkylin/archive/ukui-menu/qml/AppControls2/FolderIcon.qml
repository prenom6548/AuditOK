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
import org.ukui.menu.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.DtThemeBackground {
    property alias icons: iconGrid.icons
    property alias rows: iconGrid.rows
    property alias columns: iconGrid.columns
    property alias padding: iconGrid.padding
    property alias spacing: iconGrid.spacing

    backgroundColor: Platform.GlobalTheme.textActive
    useStyleTransparency: false

    Grid {
        id: iconGrid
        anchors.fill: parent
        property var icons
        property int cellWidth: Math.floor((width - padding*2 - spacing*(columns - 1)) / columns)
        property int cellHeight: Math.floor((height - padding*2 - spacing*(rows - 1)) / rows)
        Repeater {
            model: icons.slice(0, rows*columns)
            delegate: UkuiItems.Icon {
                width: iconGrid.cellWidth
                height: iconGrid.cellHeight
                source: modelData.icon
                addUnifiedIconBox: true
            }
        }
    }
}
