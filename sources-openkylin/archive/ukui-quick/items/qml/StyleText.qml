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
    \qmltype StyleText
    \inqmlmodule org.ukui.quick.items
    \inherits Text
    \brief 提供支持主题样式的文本组件.

    StyleText是一个增强的Text组件，自动从主题调色板中获取文本颜色，
    并使用主题字体设置。支持跟随系统主题变化。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0
    import org.ukui.quick.platform 1.0

    StyleText {
        text: "Hello UKUI"
        paletteRole: Theme.ButtonText
        pointSizeOffset: 2
        alpha: 0.8
    }
    \endqml
*/
Text {
    /*!
        \qmlproperty int StyleText::paletteRole
        文本颜色的调色板角色。

        从主题调色板中获取对应角色的颜色。
    */
    //颜色枚举值，从调色板中取得
    property int paletteRole: Theme.ButtonText;

    /*!
        \qmlproperty int StyleText::pointSizeOffset
        字体大小偏移量。

        在主题字体大小基础上增加或减少的点数。
    */
    property int pointSizeOffset: 0

    /*!
        \qmlproperty real StyleText::alpha
        文本颜色的透明度，取值范围为[0, 1]。
    */
    property real alpha: 1.0;

    font {
        pointSize: {
            if ((Theme.fontSize + pointSizeOffset) < 0 ) {
                return Theme.fontSize;
            } else {
                return Theme.fontSize + pointSizeOffset;
            }
        }
        family: Theme.fontFamily
    }

    function updateColor() {
        color = Theme.colorWithCustomTransparency(paletteRole, Theme.Active, alpha);
    }

    onPaletteRoleChanged: {
        updateColor();
    }

    onAlphaChanged: {
        updateColor();
    }

    Theme.onPaletteChanged: updateColor();

    Component.onCompleted: {
        updateColor();
    }
}
