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
    \qmltype Button
    \inqmlmodule org.ukui.quick.items
    \inherits MouseArea
    \brief 提供带图标和背景的按钮组件.

    Button是一个基于MouseArea的按钮组件，包含图标和背景。
    支持鼠标悬停、按下等交互状态，并提供焦点指示。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    Button {
        width: 48
        height: 48
        icon.source: "icon-name"
        onClicked: console.log("Button clicked")
    }
    \endqml
*/
MouseArea {
    /*!
        \qmlproperty ThemeIcon Button::icon
        \readonly
        按钮的图标对象，可以设置图标源、模式等属性。
    */
    readonly property alias icon: iconBase

    /*!
        \qmlproperty StyleBackground Button::background
        \readonly
        按钮的背景对象，可以设置背景颜色、圆角等属性。
    */
    readonly property alias background: backgroundBase

    hoverEnabled: true
    activeFocusOnTab: true

    StyleBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        paletteRole: Theme.Button
        borderColor: Theme.Highlight
        useStyleTransparency: false
        alpha: parent.containsPress ? 0.30 : parent.containsMouse ? 0.15 : 0.0

        ThemeIcon {
            id: iconBase
            anchors.centerIn: parent
            width: parent.width / 2
            height: parent.height / 2
        }
    }
}
