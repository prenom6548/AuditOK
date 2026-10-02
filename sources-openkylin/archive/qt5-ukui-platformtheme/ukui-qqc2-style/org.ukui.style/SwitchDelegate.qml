/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.SwitchDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + implicitIndicatorWidth + spacing + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: ukuiSwitch.padding
    spacing: 8
    hoverEnabled: true
    font: app.font
    palette: app.palette

    // 焦点处理（与 Switch 一致）：Tab/Backtab 显焦点框，空格键激活保留，鼠标点击消失
    property bool focusEnable: false
    property bool showFocusBorder: app.focusEnable && focusEnable
    property bool keyboardPressing: false
    onVisualFocusChanged: focusEnable = control.visualFocus
    Keys.onPressed: {
        if (event.key === Qt.Key_Space)
            control.keyboardPressing = true
        event.accepted = false
    }
    onPressedChanged: {
        if (control.pressed && !control.keyboardPressing)
            focusEnable = false
        if (!control.pressed)
            control.keyboardPressing = false
    }

    // 添加动画运行状态标志
    property bool animationRunning: false

    // 动效相关颜色属性
    property var normalColor: ukuiSwitch.normalColor
    property var tansparentNormalColor: ukuiSwitch.normalTransparentColor
    property var tansparentNormalCheckColor: ukuiSwitch.checkedNormalColor

    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }

    property var backColor: control.checked ? (!control.enabled ? ukuiSwitch.checkedDisableColor :
                                              control.pressed ? ukuiSwitch.checkedClickColor :
                                              // 动画运行时忽略hover
                                              (animationRunning ? tansparentNormalCheckColor :
                                              control.hovered ? ukuiSwitch.checkedHoverColor : tansparentNormalCheckColor))
                                            : (!control.enabled ? (isTransaprent ? ukuiSwitch.disableTransparentColor : ukuiSwitch.disableColor) :
                                              control.pressed ? (isTransaprent ? ukuiSwitch.clickTransparentColor : ukuiSwitch.clickColor) :
                                              // 动画运行时忽略hover
                                              (animationRunning ? (isTransaprent ? tansparentNormalColor : normalColor) :
                                              control.hovered ? (isTransaprent ? ukuiSwitch.hoverTransparentColor : ukuiSwitch.hoverColor) :
                                                                (isTransaprent ? tansparentNormalColor : normalColor))
                                            )

    property var borderColor: control.checked ? (!control.enabled ? ukuiSwitch.checkedDisableBorderColor :
                                                control.pressed ? ukuiSwitch.checkedClickBorderColor :
                                                // 动画运行时忽略hover
                                                (animationRunning ? ukuiSwitch.checkedNormalBorderColor :
                                                control.hovered ? ukuiSwitch.checkedHoverBorderColor : ukuiSwitch.checkedNormalBorderColor))
                                              : (!control.enabled ? (isTransaprent ? ukuiSwitch.disableTransparentBorderColor : ukuiSwitch.disableBorderColor) :
                                                  control.pressed ? (isTransaprent ? ukuiSwitch.clickTransparentBorderColor : ukuiSwitch.clickBorderColor) :
                                                  // 动画运行时忽略hover
                                                  (animationRunning ? (isTransaprent ? ukuiSwitch.normalTransparentBorderColor : ukuiSwitch.normalBorderColor) :
                                                  control.hovered ? (isTransaprent ? ukuiSwitch.hoverTransparentBorderColor : ukuiSwitch.hoverBorderColor) :
                                                                    (isTransaprent ? ukuiSwitch.normalTransparentBorderColor : ukuiSwitch.normalBorderColor))
                                              )

    StylePrivate.UKUISwitch {
        id: ukuiSwitch
    }
    StylePrivate.UKUILabel {
        id: label
    }
    StylePrivate.APPParameter{
    id: app
    }
    ParseInterface{
        id: parseInterface
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }


    indicator: Rectangle {
        implicitWidth: ukuiSwitch.recWidth
        implicitHeight: ukuiSwitch.recHeight

        x: control.text ? (control.mirrored ? control.leftPadding : control.width - width - control.rightPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2

        radius: implicitHeight/2
        color: gradientRec.visible ? "transparent" : getStartColor(backColor)

        ColorAnimation {
            id: tansparentColorAnim
            target: control
            property: "tansparentNormalCheckColor"
            from: ukuiSwitch.normalTransparentColor
            to: ukuiSwitch.checkedNormalColor
            duration: 200
            easing.type: Easing.Linear
            running: false
            onRunningChanged: {
                control.animationRunning = running;
                if (!running) control.update()
            }
        }

        ColorAnimation {
            id: tansparentColorReturnAnim
            target: control
            property: "tansparentNormalColor"
            from: ukuiSwitch.checkedNormalColor
            to: ukuiSwitch.normalTransparentColor
            duration: 200
            easing.type: Easing.Linear
            running: false
            onRunningChanged: {
                control.animationRunning = running;
                if (!running) control.update()
            }
        }

        ColorAnimation {
            id: normalColorAnim
            target: control
            property: "tansparentNormalCheckColor"
            from: ukuiSwitch.normalColor
            to: ukuiSwitch.checkedNormalColor
            duration: 200
            easing.type: Easing.Linear
            running: false
            onRunningChanged: {
                control.animationRunning = running;
                if (!running) control.update()
            }
        }

        ColorAnimation {
            id: normalColorReturnAnim
            target: control
            property: "normalColor"
            from: ukuiSwitch.checkedNormalColor
            to: ukuiSwitch.normalColor
            duration: 200
            easing.type: Easing.Linear
            running: false
            onRunningChanged: {
                control.animationRunning = running;
                if (!running) control.update()
            }
        }

        Rectangle {
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: getStartColor(backColor) }
                GradientStop { position: 1.0; color: getEndColor(backColor) }
            }
            visible: !parseInterface.isSolidPattern(backColor)
        }
        Rectangle{
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: getStartColor(showFocusBorder ? app.focusBorderColor : borderColor)
            border.width: showFocusBorder ? app.focusBorderWidth : ukuiSwitch.borderWidth
        }
        Rectangle {
            x: Math.max(ukuiSwitch.indicatorLeftRightPadding, Math.min(parent.width - width - ukuiSwitch.indicatorLeftRightPadding, control.visualPosition * parent.width - (width / 2) - ukuiSwitch.indicatorLeftRightPadding))
            anchors.verticalCenter: parent.verticalCenter
            width: ukuiSwitch.indicatorWidth
            height: ukuiSwitch.indicatorWidth
            radius: height/2
            color: getStartColor(!control.enabled ? ukuiSwitch.disableIndicatorColor : ukuiSwitch.normalIndicatorColor)

            Behavior on x {
                enabled: !control.down
                NumberAnimation {
                    duration: 260
                    easing.type: Easing.OutExpo
                }
            }
        }
    }

    contentItem: IconLabelContent {
        id: iconLabel
        anchors.left: control.left
        anchors.right: control.right
        anchors.leftMargin: control.mirrored ? control.indicator.width + control.spacing : control.leftPadding
        anchors.rightMargin: !control.mirrored ? control.indicator.width + control.spacing : control.rightPadding
        controlRoot: control
        implicitWidth: iconLabel.adjustWidth(control.leftPadding + control.rightPadding)
    }

    background: Item {
        anchors.fill: parent
    }

    onCheckedChanged: {
        tansparentColorAnim.stop()
        tansparentColorReturnAnim.stop()
        normalColorAnim.stop()
        normalColorReturnAnim.stop()

        if (checked) {
            isTransaprent ? tansparentColorAnim.start() : normalColorAnim.start()
        } else {
            isTransaprent ? tansparentColorReturnAnim.start() : normalColorReturnAnim.start()
        }
    }
}
