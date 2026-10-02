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
    \qmltype HoverButton
    \inqmlmodule org.ukui.quick.items
    \inherits MouseArea
    \brief 提供带悬停效果的圆形按钮组件.

    HoverButton是一个圆形按钮组件，支持悬停和按下状态的视觉反馈。
    按钮带有阴影效果，背景和边框颜色会根据鼠标状态自动变化。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    HoverButton {
        width: 24
        height: 24
        icon.source: "window-close"
        onClicked: console.log("Close button clicked")
    }
    \endqml
*/
MouseArea {
    /*!
        \qmlproperty Icon HoverButton::icon
        \readonly
        按钮的图标对象。
    */
    readonly property alias icon: iconBase

    /*!
        \qmlproperty DtThemeBackground HoverButton::background
        \readonly
        按钮的背景对象。
    */
    readonly property alias background: backgroundBase
    hoverEnabled: true
    DtThemeBackground {
        id: backgroundBase
        anchors.fill: parent
        border.width: 1
        width: 24
        height: 24
        backgroundColor: getBackgroundColor()
        borderColor: getBorderdColor()
        useStyleTransparency: false
        radius: width/2
        alpha: 1
        DtDropShadow {
            anchors.fill: backgroundBase
            shadowData: GlobalTheme.kShadowMin
            shadowRadius: backgroundBase.width/2
        }
        Icon {
            id: iconBase
            width: 16
            height: 16
            anchors.centerIn: parent
            mode: Icon.AutoHighlight
        }
        function getBackgroundColor() {
                return containsPress ? GlobalTheme.kContainClick :
                                       containsMouse ? GlobalTheme.kContainHover
                                                     : GlobalTheme.kContainGeneralNormal
        }
        function getBorderdColor() {
                return containsPress ? GlobalTheme.kLineComponentClick :
                                       containsMouse ? GlobalTheme.kLineComponentHover
                                                     : GlobalTheme.kLineComponentNormal
        }
    }

}
