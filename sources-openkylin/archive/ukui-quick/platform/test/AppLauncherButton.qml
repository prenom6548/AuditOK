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

import QtQuick 2.12
import QtQuick.Controls 2.15 as Controls
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0


MouseArea {
    readonly property alias background: backgroundBase
    property alias text : nameText.text

    hoverEnabled: true
    activeFocusOnTab: true

    StyleBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        paletteRole: Theme.BrightText
        borderColor: Theme.Highlight
        useStyleTransparency: false
        alpha: parent.containsPress ? 0.30 : parent.containsMouse ? 0.15 : 0.0

        StyleText {
            id: nameText
            width: parent.width
            height: parent.height
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: (nameText.text)
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
            interactive: true
        }
    }
}

