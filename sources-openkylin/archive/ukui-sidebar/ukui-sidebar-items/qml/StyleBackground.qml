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

Rectangle {
    //是否跟随系统主题颜色的透明度
    property bool useStyleTransparency: true;
    //背景颜色枚举值，从调色板中取得
    property int paletteRole: PaletteRole.Window;
    property int paletteGroup: PaletteRole.Active;
    property real alpha: 1.0;
    property real borderAlpha: 1.0;
    property int borderColor: PaletteRole.Base;

    border.width: 0;
    clip: true;

    function updateColor() {
        if (useStyleTransparency) {
            color = themePalette.paletteColorWithTransparency(paletteRole, paletteGroup);
        } else {
            color = themePalette.paletteColorWithCustomTransparency(paletteRole, paletteGroup, alpha);
        }
    }

    function updateBorderColor() {
        border.color = themePalette.paletteColorWithCustomTransparency(borderColor, PaletteRole.Active, borderAlpha)
    }
    //监听系统主题变化
    onUseStyleTransparencyChanged: {
        updateColor();
    }

    onPaletteGroupChanged: {
        updateColor();
    }

    onPaletteRoleChanged: {
        updateColor();
    }

    onAlphaChanged: {
        updateColor();
    }

    onBorderAlphaChanged: {
        updateBorderColor();
    }

    onBorderColorChanged: {
        updateBorderColor();
    }

    Component.onCompleted: {
        updateColor();
        updateBorderColor();
        themePalette.styleColorChanged.connect(updateColor);
        themePalette.styleColorChanged.connect(updateBorderColor);
    }

    Component.onDestruction: {
        themePalette.styleColorChanged.disconnect(updateColor);
        themePalette.styleColorChanged.disconnect(updateBorderColor);
    }
}
