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

import QtQuick 2.15
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0

Item {
    property alias text: textInput.text;
    StyleBackground {
        anchors.fill: parent
        alpha: 0.04
        useStyleTransparency: false
        paletteRole: Platform.Theme.Text
        border.width: 1
        borderAlpha: textInput.activeFocus ? 1 : 0.1
        borderColor: textInput.activeFocus ? Platform.Theme.Highlight : Platform.Theme.BrightText
        z: 0
    }
    TextInput {
        id: textInput
        clip: true
        anchors.fill: parent
        selectByMouse: true
        verticalAlignment: TextInput.AlignVCenter
        font.pointSize: Platform.Theme.fontSize
        activeFocusOnPress: true;
        leftPadding: 2
        rightPadding: 2
        activeFocusOnTab: true;
        z: 1

        function updateTextInputColor() {
            color = Platform.Theme.text();
            selectionColor = Platform.Theme.highlight();
        }

        Platform.Theme.onPaletteChanged: {
            updateTextInputColor();
        }

        Component.onCompleted: {
            updateTextInputColor();
        }
    }
}