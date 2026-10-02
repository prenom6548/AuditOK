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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

import QtQuick 2.12
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype DtThemeText
    \inqmlmodule org.ukui.quick.items
    \inherits Text
    \brief 提供DT主题风格的文本组件.

    DtThemeText是一个使用DT主题系统的文本组件，
    自动使用DT主题的字体和颜色设置。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    DtThemeText {
        text: "Hello UKUI"
        textColor: GlobalTheme.textActive
        pointSizeOffset: 2
        alpha: 0.9
    }
    \endqml
*/
Text {
    /*!
        \qmlproperty var DtThemeText::textColor
        DT主题字体颜色对象。
    */
    //DT主题字体颜色
    property var textColor: GlobalTheme.textActive;

    /*!
        \qmlproperty int DtThemeText::pointSizeOffset
        字体大小偏移量。

        在DT主题字体大小基础上增加或减少的点数。
    */
    property int pointSizeOffset: 0

    /*!
        \qmlproperty real DtThemeText::alpha
        文本颜色的透明度，取值范围为[0, 1]。
    */
    property real alpha: 1.0;
    font {
        pointSize: {
            if ((GlobalTheme.fontSize + pointSizeOffset) < 0 ) {
                return GlobalTheme.fontSize;
            } else {
                return GlobalTheme.fontSize + pointSizeOffset;
            }
        }
        family: GlobalTheme.fontFamily
    }
    color: textColor.colorType === DtColorType.Pure ? textColor.setAlphaF(textColor.pureColor, alpha) : textColor.mixBackGroundColor(textColor.gradientStart, textColor.gradientBackground, alpha)
}
