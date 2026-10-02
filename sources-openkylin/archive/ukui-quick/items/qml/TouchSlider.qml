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
import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0


/*!
    \qmltype TouchSlider
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \inherits QtQuick.Templates.Slider
    \brief 提供 DT 主题风格的滑块组件，支持触摸和键盘操作。

    TouchSlider 是一个符合 DT 设计规范的滑块组件，支持水平和垂直方向。
    提供键盘导航、鼠标滚轮支持，以及透明模式和非透明模式两种视觉风格。

    \section1 主要特性
    \list
        \li 支持水平和垂直方向
        \li 支持键盘方向键操作
        \li 支持鼠标滚轮调节
        \li 支持透明模式和非透明模式
        \li 进度条和滑块使用 DT 主题颜色
        \li 支持从右到左布局（RTL）
        \li 自动禁用状态样式
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    TouchSlider {
        width: 200
        height: 32
        from: 0
        to: 100
        value: 50
        onValueChanged: {
            console.log("当前值:", value)
        }
    }
    \endqml

    垂直滑块：
    \qml
    TouchSlider {
        width: 32
        height: 200
        orientation: Qt.Vertical
        from: 0
        to: 100
        value: 30
    }
    \endqml

    非透明模式：
    \qml
    TouchSlider {
        width: 200
        height: 32
        transparentMode: false
        stepSize: 10
    }
    \endqml

    \sa QtQuick.Controls.Slider, DtThemeBackground
*/


T.Slider {
    id: control
    /*!
        \qmlproperty bool TouchSlider::transparentMode

        控制滑块是否使用透明模式。

        \list
            \li true - 使用半透明背景色（GlobalTheme.kComponentAlpha 系列）
            \li false - 使用不透明背景色（GlobalTheme.kComponent 系列）
        \endlist

        默认值为 false。
    */
    property bool transparentMode: false
    /*!
        \qmlproperty var TouchSlider::backgroundColor

        滑块背景的颜色。

        该属性根据 transparentMode 和 enabled 状态自动计算。
    */
    property var backgroundColor : getBackgroundColor()
    /*!
        \qmlproperty var TouchSlider::borderColor

        滑块边框的颜色。

        根据 enabled 状态自动选择：
        \list
            \li 启用时：GlobalTheme.kLineComponentNormal
            \li 禁用时：GlobalTheme.kLineComponentDisable
        \endlist
    */
    property var borderColor : enabled ? GlobalTheme.kLineComponentNormal : GlobalTheme.kLineComponentDisable

    wheelEnabled: true

    Keys.onPressed: {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Down) {
            pressed = true
            if(mirrored) {
                increase()
            } else {
                decrease()
            }
            event.accepted = true;
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Up) {
            pressed = true
            if(mirrored) {
                decrease()
            } else {
                increase()
            }
            event.accepted = true;
        }
    }
    Keys.onReleased: {
        pressed = false
        event.accepted = false;
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
            if (wheel.angleDelta.x == 0) {
                wheel.accepted = false;
                return;
            }
            if (wheel.angleDelta.x > 0) {
                decrease();
            } else {
                increase();
            }
        }
    }

    function getBackgroundColor() {
        if(transparentMode) {
            return enabled ? GlobalTheme.kComponentAlphaNormal : GlobalTheme.kComponentAlphaDisable
        } else {
            return enabled ? GlobalTheme.kComponentNormal : GlobalTheme.kComponentDisable
        }
    }

    //进度条
    background: DtThemeBackground {
        id: sliderBackground
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        width: control.horizontal ? control.availableWidth : control.width
        height: control.horizontal ? control.height : control.availableHeight
        implicitWidth: control.width
        implicitHeight: control.height
        radius: control.horizontal ? height / 2 : width / 2
        useStyleTransparency: false
        backgroundColor: control.backgroundColor
        borderColor: control.borderColor
        border.width: 1
        scale: control.horizontal && control.mirrored ? -1 : 1
        DtThemeBackground {
            id: top
            x: control.horizontal ? 0 : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : control.visualPosition * (control.availableHeight - radius * 2)
            width: control.horizontal ? control.position * (control.availableWidth - radius * 2) + radius * 2 : control.width
            height: control.horizontal ? control.height : control.position * (control.availableHeight - radius * 2) + radius * 2
            radius: sliderBackground.radius
            useStyleTransparency: false
            backgroundColor: enabled ? GlobalTheme.kBrandNormal : GlobalTheme.kBrandDisable
            borderColor: enabled ? GlobalTheme.kLineBrandNormal : GlobalTheme.kLineBrandDisable
        }
    }

    //滑块
    handle: Item {
        id: sliderHandel
        x: control.horizontal ? control.visualPosition * (control.availableWidth - width) : control.availableWidth / 2 - width / 2
        y: control.horizontal ? control.availableHeight / 2 - height / 2 : control.visualPosition * (control.availableHeight - height)
        implicitWidth: control.horizontal ? control.height : control.width
        implicitHeight: control.horizontal ? control.height : control.width

        DtThemeBackground {
            id: whiteSpot
            width: control.horizontal ? (control.height / 16) * 4 : (control.width / 16) * 6
            height: control.horizontal ? (control.height / 16) * 6 : (control.width / 16) * 4
            radius: control.horizontal ? width / 2 : height / 2
            useStyleTransparency: false
            backgroundColor: GlobalTheme.kFontWhite
            anchors.centerIn: parent
        }
    }

}
