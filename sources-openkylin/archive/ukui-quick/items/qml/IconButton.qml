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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 */

import QtQuick 2.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype IconButton
    \inqmlmodule org.ukui.quick.items
    \inherits DtThemeBackground
    \brief 提供图标按钮组件.

    IconButton是一个基于DtThemeBackground的图标按钮组件，
    支持高亮状态、禁用状态和鼠标交互效果。
    按钮背景和边框颜色会根据状态自动变化。

    使用示例：
    \qml
    import org.ukui.quick.items 1.0

    IconButton {
        width: 48
        height: 48
        radius: 8
        iconSource: "list-add"
        isHighLight: false
        onClicked: console.log("Button clicked")
    }
    \endqml
*/
DtThemeBackground {
    /*!
        \qmlproperty bool IconButton::isHighLight
        是否处于高亮状态。

        当设置为true时，按钮将使用高亮配色方案。
    */
    property bool isHighLight: false

    /*!
        \qmlproperty bool IconButton::disable
        是否禁用按钮。
    */
    property bool disable: false

    /*!
        \qmlproperty string IconButton::iconSource
        图标源。

        可以是主题图标名称或文件路径。
    */
    property string iconSource: ""

    /*!
        \qmlsignal IconButton::clicked()
        按钮被点击时发出的信号。
    */
    signal clicked()

    useStyleTransparency: false

    backgroundColor: isHighLight ? highLightBackGroundColor()
                                   : normalBackGroundColor()
    border.width: 1
    borderColor: isHighLight ? highLightBorderColor()
                               : normalBorderColor()

    function highLightBackGroundColor() {
        return mouseArea.containsPress ? GlobalTheme.kBrandClick
                                       : mouseArea.containsMouse ? GlobalTheme.kBrandHover
                                                                 : GlobalTheme.highlightActive
    }

    function normalBackGroundColor() {
        return mouseArea.containsPress ? GlobalTheme.kComponentAlphaClick
                                       : mouseArea.containsMouse ? GlobalTheme.kComponentAlphaHover
                                                                 : GlobalTheme.kComponentAlphaNormal
    }

    function highLightBorderColor() {
        return mouseArea.containsPress ? GlobalTheme.kLineBrandClick
                                       : mouseArea.containsMouse ? GlobalTheme.kLineBrandHover
                                                                 : GlobalTheme.kLineBrandNormal
    }

    function normalBorderColor() {
        return mouseArea.containsPress ? GlobalTheme.kLineComponentClick
                                       : mouseArea.containsMouse ? GlobalTheme.kLineComponentHover
                                                                 : GlobalTheme.kLineComponentNormal
    }

    Icon {
        width: parent.width/2
        height: parent.width/2
        anchors.centerIn: parent
        source: parent.iconSource
        mode: parent.isHighLight ? Icon.Highlight : Icon.AutoHighlight
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
            parent.clicked();
        }
    }
}
