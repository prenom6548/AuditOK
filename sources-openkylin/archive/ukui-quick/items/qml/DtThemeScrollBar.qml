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
import QtQuick.Templates 2.12 as T
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

/*!
    \qmltype DtThemeScrollBar
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \inherits QtQuick.Templates.ScrollBar
    \brief 提供 DT 主题风格的滚动条组件。

    DtThemeScrollBar 是一个符合 DT 设计规范的滚动条组件，支持透明模式和自动隐藏功能。
    滚动条会根据用户交互状态（悬停、按下）自动调整大小和透明度，提供流畅的视觉反馈。

    \section1 主要特性
    \list
        \li 支持透明模式和非透明模式
        \li 自动显示/隐藏动画效果
        \li 悬停和按下时自动调整大小
        \li 符合 DT 主题颜色规范
        \li 支持水平和垂直方向
    \endlist

    \section1 使用示例

    在 ScrollView 中使用：
    \qml
    ScrollView {
        width: 200
        height: 200
        ScrollBar.vertical: DtThemeScrollBar {
            visual: true
            transparentMode: true
        }
        ScrollBar.horizontal: DtThemeScrollBar {
            visual: true
            transparentMode: false
        }
    }
    \endqml

    \sa QtQuick.Controls.ScrollBar
*/

T.ScrollBar {
    id: control
    /*!
        \qmlproperty bool DtThemeScrollBar::visual

        控制滚动条是否可见。

        当设置为 true 时，滚动条会根据 policy 属性和内容大小自动显示或隐藏。
        默认值为 true。
    */
    property bool visual: true
    /*!
        \qmlproperty bool DtThemeScrollBar::transparentMode

        控制滚动条是否使用透明模式。

        \list
            \li true - 使用半透明背景色（GlobalTheme.kGrayAlpha 系列）
            \li false - 使用不透明背景色（GlobalTheme.kGray 系列）
        \endlist

        默认值为 true。
    */
    property bool transparentMode: true
    hoverEnabled: true
    implicitWidth: control.horizontal ? Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding) : 16
    implicitHeight: control.horizontal ? 16 : Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: (hovered || pressed) ? 4 : 6
    Behavior on padding {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }
    visible: control.policy !== T.ScrollBar.AlwaysOff
    minimumSize: orientation == Qt.Horizontal ? height / width : width / height

    contentItem: DtThemeBackground {
        implicitWidth: control.interactive ? 8 : 4
        implicitHeight: control.interactive ? 8 : 4
        radius: width / 2
        backgroundColor: getBackgroundColor()
        opacity: 0.0
    }

    states: State {
        name: "active"
        when: activeState()
    }

    transitions: [
        Transition {
            to: "active"
            NumberAnimation { targets: [control.contentItem]; property: "opacity"; to: 1.0 }
        },
        Transition {
            from: "active"
            SequentialAnimation {
                PropertyAction { targets: [control.contentItem]; property: "opacity"; value: 1.0 }
                NumberAnimation { targets: [control.contentItem]; property: "opacity"; to: 0.0 }
            }
        }
    ]

    function getBackgroundColor() {
        if(control.transparentMode) {
            return control.pressed ? GlobalTheme.kGrayAlpha11 :
                                     control.interactive && control.hovered ? GlobalTheme.kGrayAlpha13
                                                                            : control.enabled ? GlobalTheme.kGrayAlpha9
                                                                                              : GlobalTheme.kGrayAlpha5
        } else {
            return control.pressed ? GlobalTheme.kGray14 :
                                     control.interactive && control.hovered ? GlobalTheme.kGray13
                                                                            : control.enabled ? GlobalTheme.kGray12
                                                                                              : GlobalTheme.kGray8
        }
    }

    function activeState() {
        //Policy possible values: AsNeeded AlwaysOn AlwaysOff
        if (control.policy === T.ScrollBar.AsNeeded) {
            return (control.size < 1.0) && control.visual;
        } else {
            return (control.policy === T.ScrollBar.AlwaysOn) && control.visual;
        }
    }
}
