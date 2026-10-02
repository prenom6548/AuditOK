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

import QtQuick 2.12
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype StyleBackground
    \inqmlmodule org.ukui.quick.items
    \inherits Rectangle
    \brief 提供支持主题样式的背景组件.

    StyleBackground是一个增强的Rectangle组件，自动从主题调色板中获取颜色。
    支持跟随系统主题变化，可以设置是否使用主题透明度或自定义透明度。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0
    import org.ukui.quick.platform 1.0

    StyleBackground {
        width: 200
        height: 100
        radius: 8
        paletteRole: Theme.Window
        useStyleTransparency: true
        border.width: 1
        borderColor: Theme.Base
    }
    \endqml
*/
Rectangle {
    /*!
        \qmlproperty bool StyleBackground::useStyleTransparency
        是否跟随系统主题颜色的透明度。

        当设置为true时，使用主题的透明度；否则使用alpha属性指定的透明度。
    */
    //是否跟随系统主题颜色的透明度
    property bool useStyleTransparency: true;

    /*!
        \qmlproperty int StyleBackground::paletteRole
        背景颜色的调色板角色。

        从主题调色板中获取对应角色的颜色。
    */
    //背景颜色枚举值，从调色板中取得
    property int paletteRole: Theme.Window;

    /*!
        \qmlproperty int StyleBackground::paletteGroup
        背景颜色的调色板组。

        指定使用的颜色组（Active、Inactive等）。
    */
    property int paletteGroup: Theme.Active;

    /*!
        \qmlproperty real StyleBackground::alpha
        背景的透明度。

        仅在useStyleTransparency为false时生效，取值范围为[0, 1]。
    */
    property real alpha: 1.0;

    /*!
        \qmlproperty real StyleBackground::borderAlpha
        边框的透明度，取值范围为[0, 1]。
    */
    property real borderAlpha: 1.0;

    /*!
        \qmlproperty int StyleBackground::borderColor
        边框的调色板颜色角色。
    */
    property int borderColor: Theme.Base;

    border.width: 0;
    clip: true;

    function updateColor() {
        if (useStyleTransparency) {
            color = Theme.colorWithThemeTransparency(paletteRole, paletteGroup);
        } else {
            color = Theme.colorWithCustomTransparency(paletteRole, paletteGroup, alpha);
        }
    }

    function updateBorderColor() {
        border.color = Theme.colorWithCustomTransparency(borderColor, Theme.Active, borderAlpha);
    }

    //监听系统主题变化
    Theme.onPaletteChanged: {
        updateColor();
        updateBorderColor();
    }
    Theme.onThemeTransparencyChanged: updateColor();

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
    }
}
