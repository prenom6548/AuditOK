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
    \qmltype DtThemeButton
    \inqmlmodule org.ukui.quick.items
    \inherits MouseArea
    \brief 提供DT主题风格的按钮组件.

    DtThemeButton是一个使用DT主题系统的按钮组件，
    支持焦点指示和鼠标交互。按钮包含图标和背景。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    DtThemeButton {
        width: 48
        height: 48
        icon.source: "list-add"
        onClicked: console.log("Button clicked")
    }
    \endqml
*/
MouseArea {
    /*!
        \qmlproperty Icon DtThemeButton::icon
        \readonly
        按钮的图标对象。
    */
    readonly property alias icon: iconBase

    /*!
        \qmlproperty DtThemeBackground DtThemeButton::background
        \readonly
        按钮的背景对象。
    */
    readonly property alias background: backgroundBase

    hoverEnabled: true
    activeFocusOnTab: true

    DtThemeBackground {
        id: backgroundBase
        anchors.fill: parent

        border.width: parent.activeFocus ? 2 : 0
        backgroundColor: GlobalTheme.buttonActive
        borderColor: GlobalTheme.highlightActive
        useStyleTransparency: false
        alpha: 1

        Icon {
            id: iconBase
            anchors.centerIn: parent
            width: parent.width / 2
            height: parent.height / 2
        }
    }
}
