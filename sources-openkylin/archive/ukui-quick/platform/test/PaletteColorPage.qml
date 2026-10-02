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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.0
import org.ukui.quick.platform 1.0
import org.ukui.quick.items 1.0
import QtQuick.Layouts 1.15
ColumnLayout {
    id: views
    spacing: 10
    width: parent.width
    height: parent.height
    GridView {
        id: activeView
        Layout.fillWidth: true
        Layout.preferredHeight: contentHeight
        cellWidth: 140
        model: paletteModel
        delegate: PaletteColor{
            paletteRole: model.role
            paletteGroup: Theme.Active
            colorText: model.name
        }
        clip: true
    }

    GridView {
        id: disabledView
        Layout.fillWidth: true
        Layout.preferredHeight: contentHeight
        cellWidth: 140
        model: paletteModel
        delegate: PaletteColor{
            paletteRole: model.role
            paletteGroup: Theme.Disabled
            colorText:model.name
        }
        clip: true
    }
    GridView {
        id: inactiveView
        model: paletteModel
        Layout.fillWidth: true
        Layout.preferredHeight: contentHeight
        cellWidth: 140
        delegate: PaletteColor{
            paletteRole: model.role
            paletteGroup: Theme.Inactive
            colorText:model.name
        }
        clip: true
    }
    ListModel {
        id: paletteModel
        Component.onCompleted: {
            append({role: Theme.Window, name: "Window"})
            append({role: Theme.WindowText, name: "WindowText"})
            append({role: Theme.Base, name: "Base"})
            append({role: Theme.Text, name: "Text"})
            append({role: Theme.AlternateBase, name: "AlternateBase"})
            append({role: Theme.Button, name: "Button"})
            append({role: Theme.ButtonText, name: "ButtonText"})
            append({role: Theme.Light, name: "Light"})
            append({role: Theme.MidLight, name: "MidLight"})
            append({role: Theme.Dark, name: "Dark"})
            append({role: Theme.Mid, name: "Mid"})
            append({role: Theme.Shadow, name: "Shadow"})
            append({role: Theme.Shadow, name: "Shadow"})
            append({role: Theme.Highlight, name: "Highlight"})
            append({role: Theme.HighlightedText, name: "HighlightedText"})
            append({role: Theme.BrightText, name: "BrightText"})
            append({role: Theme.Link, name: "Link"})
            append({role: Theme.LinkVisited, name: "LinkVisited"})
            append({role: Theme.ToolTipBase, name: "ToolTipBase"})
            append({role: Theme.ToolTipText, name: "ToolTipText"})
            append({role: Theme.PlaceholderText, name: "PlaceholderText"})
        }
    }
}

