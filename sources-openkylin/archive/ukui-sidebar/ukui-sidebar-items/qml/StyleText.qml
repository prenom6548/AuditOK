/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

import QtQuick 2.0
import org.ukui.sidebar.items 1.0

Text {
    //颜色枚举值，从调色板中取得
    property int paletteRole: PaletteRole.ButtonText;
    property int pointSizeOffset: 0
    property real alpha: 1.0;

    function updateColor() {
        color = themePalette.paletteColorWithCustomTransparency(paletteRole, PaletteRole.Active, alpha);
    }

    function updateFontSize() {
        if ((themePalette.fontSize() + pointSizeOffset) < 0 ) {
            font.pointSize = themePalette.fontSize();
        } else {
            font.pointSize = themePalette.fontSize() + pointSizeOffset;
        }
    }

    function updateFont() {
        font.family = themePalette.themeFont();
    }

    onPaletteRoleChanged: {
        updateColor();
    }

    onAlphaChanged: {
        updateColor();
    }

    Component.onCompleted: {
        updateFont();
        updateColor();
        updateFontSize();
        themePalette.fontChanged.connect(updateFont);
        themePalette.styleColorChanged.connect(updateColor);
        themePalette.fontSizeChanged.connect(updateFontSize);
    }

    Component.onDestruction: {
        themePalette.fontChanged.disconnect(updateFont);
        themePalette.styleColorChanged.disconnect(updateColor);
        themePalette.fontSizeChanged.disconnect(updateFontSize);
    }
}
