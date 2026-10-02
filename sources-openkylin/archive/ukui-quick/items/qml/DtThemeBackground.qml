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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

import QtQuick 2.15
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype DtThemeBackground
    \inqmlmodule org.ukui.quick.items
    \inherits ShadowedRectangle
    \brief 提供支持DT主题的背景组件.

    DtThemeBackground是一个基于ShadowedRectangle的背景组件，
    支持DT主题系统的渐变色和纯色背景。可以自动跟随系统主题变化，
    支持阴影效果和边框设置。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0
    import org.ukui.quick.platform 1.0

    DtThemeBackground {
        width: 200
        height: 100
        radius: 8
        backgroundColor: GlobalTheme.windowActive
        useStyleTransparency: true
        shadow {
            size: 10
            color: "black"
            yOffset: 2
        }
    }
    \endqml
*/
ShadowedRectangle {
    /*!
        \qmlproperty bool DtThemeBackground::useStyleTransparency
        是否跟随系统主题颜色的透明度。

        当设置为true时，背景透明度将使用GlobalTheme.transparency；
        否则使用alpha属性指定的透明度。
    */
    //是否跟随系统主题颜色的透明度
    property bool useStyleTransparency: true;

    /*!
        \qmlproperty var DtThemeBackground::backgroundColor
        背景颜色对象。

        使用GlobalTheme中的颜色对象，支持纯色和渐变色。
    */
    //背景颜色
    property var backgroundColor: GlobalTheme.windowActive;

    /*!
        \qmlproperty var DtThemeBackground::borderColor
        边框颜色对象。
    */
    property var borderColor: GlobalTheme.baseActive

    /*!
        \qmlproperty real DtThemeBackground::borderAlpha
        边框的透明度，取值范围为[0, 1]。
    */
    property real borderAlpha: 1.0;

    /*!
        \qmlproperty real DtThemeBackground::alpha
        背景的透明度，取值范围为[0, 1]。

        仅在useStyleTransparency为false时生效。
    */
    property real alpha: 1.0

    /*!
        \qmlproperty var DtThemeBackground::shadowData
        阴影数据对象。

        包含blurRadius（模糊半径）、spread（扩散）、xOffset（X偏移）、
        yOffset（Y偏移）和shadowColor（阴影颜色）等属性。
    */
    //阴影
    property var shadowData

    border.width: 0;
    border.color : borderColor.setAlphaF(borderColor.pureColor,borderAlpha);
    color:backgroundColor.setAlphaF(backgroundColor.pureColor, useStyleTransparency ? GlobalTheme.transparency : alpha);
    pureColor: backgroundColor.colorType === DtColorType.Pure;
    startColor: backgroundColor.mixBackGroundColor(backgroundColor.gradientStart, backgroundColor.gradientBackground, useStyleTransparency ? GlobalTheme.transparency : alpha)
    endColor: backgroundColor.mixBackGroundColor(backgroundColor.gradientEnd, backgroundColor.gradientBackground, useStyleTransparency ? GlobalTheme.transparency : alpha)
    angle: backgroundColor.gradientDeg

    onShadowDataChanged: {
        if (shadowData === undefined) {
            shadow.size = 0;
            shadow.xOffset = 0.0;
            shadow.yOffset = 0.0;
            shadow.color = "black";
        } else {
            shadow.size = shadowData.blurRadius + shadowData.spread;
            shadow.xOffset = shadowData.xOffset;
            shadow.yOffset = shadowData.yOffset;
            shadow.color = shadowData.shadowColor;
        }
    }
}
