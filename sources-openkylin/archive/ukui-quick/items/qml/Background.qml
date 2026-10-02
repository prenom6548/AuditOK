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
import QtQml 2.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype Background
    \inqmlmodule org.ukui.quick.items
    \inherits Rectangle
    \brief 提供支持颜色混合和主题绑定的背景组件.

    Background是一个增强的Rectangle组件，集成了ColorMix颜色混合功能。
    可以设置前景色和背景色，通过不同的混合模式生成最终颜色。
    颜色自动跟随系统主题变化。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0
    import org.ukui.quick.platform 1.0

    Background {
        width: 200
        height: 100
        radius: 8
        mixMode: ColorMix.Normal
        backColorRole: Theme.Base
        foreColorRole: Theme.Highlight
        foreColorAlpha: 0.3
    }
    \endqml
*/
Rectangle {
    /*!
        \qmlproperty real Background::borderAlpha
        边框的透明度，取值范围为[0, 1]。
    */
    property real borderAlpha: 1.0

    /*!
        \qmlproperty int Background::borderColor
        边框的主题颜色角色。
    */
    property int  borderColor: Theme.Base

    /*!
        \qmlproperty ColorMix Background::colorMixer
        颜色混合器对象，用于配置颜色混合。
    */
    // colorMixer
    property alias colorMixer: mixer

    /*!
        \qmlproperty ColorMix.MixMode Background::mixMode
        颜色混合模式。
    */
    property alias mixMode: mixer.mixMode

    /*!
        \qmlproperty Theme.ColorRole Background::backColorRole
        背景色的主题颜色角色。
    */
    // 背景色
    property alias backColorRole: mixer.backColorRole

    /*!
        \qmlproperty Theme.ColorGroup Background::backColorGroup
        背景色的主题颜色组。
    */
    property alias backColorGroup: mixer.backColorGroup

    /*!
        \qmlproperty real Background::backColorAlpha
        背景色的透明度。
    */
    property alias backColorAlpha: mixer.backColorAlpha

    /*!
        \qmlproperty Theme.ColorRole Background::foreColorRole
        前景色的主题颜色角色。
    */
    // 前景色
    property alias foreColorRole: mixer.foreColorRole

    /*!
        \qmlproperty Theme.ColorGroup Background::foreColorGroup
        前景色的主题颜色组。
    */
    property alias foreColorGroup: mixer.foreColorGroup

    /*!
        \qmlproperty real Background::foreColorAlpha
        前景色的透明度。
    */
    property alias foreColorAlpha: mixer.foreColorAlpha

    color: mixer.color
    ColorMix {
        id: mixer
    }
    border.width: 0
    border.color: {
        return Theme.colorWithCustomTransparency(borderColor, Theme.Active, borderAlpha);
    }

    //监听系统主题变化
    Theme.onPaletteChanged: {
        borderColorChanged();
    }
}
