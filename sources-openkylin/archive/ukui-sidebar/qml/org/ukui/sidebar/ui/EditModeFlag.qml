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
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Loader {
    id: tag
    property bool isEdit
    property int buttonType
    property int addState
    signal clicked()

    Component {
        id: editImage
        UkuiItems.HoverButton {
            width: 24
            height: 24

            activeFocusOnTab: false
            onClicked: tag.clicked()

            icon.source: addState === PluginState.Add ? "list-remove-symbolic" : "list-add-symbolic"
        }
    }

    sourceComponent: isEdit &&
                     ((tag.buttonType === ShortcutPlugin.Icon) || (tag.buttonType === ShortcutPlugin.MenuButton)) ? editImage : null
}

