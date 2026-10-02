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
import QtGraphicalEffects 1.15

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    palette: app.palette

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
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var isShowNormalProgressBarAnim:{
        if(control.hasOwnProperty("showNormalProgressBarAnim"))
            return control.showNormalSliderAnim
        else
            return false
    }
    property real animationSpeed: 160  // 统一速度值（像素/秒）
    property real sliderWidth: 56  // 保持滑块宽度不变
    // 用于普通模式动画的属性
    property real animationX: -sliderWidth

    // 计算进度百分比的辅助属性
    property real progressPercent: Math.min(Math.max(0, (control.value - control.from) / (control.to - control.from) * 100), 100)

    background: Rectangle {
        radius: progressbar.radius
        implicitWidth: progressbar.normalWidth
        implicitHeight: control.indeterminate ? 8 : progressbar.normalHeight

        height: control.indeterminate ? 8 : progressbar.normalHeight

        color: gradientRec.visible ? "transparent" : getStartColor(isTransaprent ? progressbar.normalTransparentColor : progressbar.normalColor)

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(isTransaprent ? progressbar.normalTransparentColor : progressbar.normalColor)}

                GradientStop { position: 1.0; color: getEndColor(isTransaprent ? progressbar.normalTransparentColor : progressbar.normalColor)}
            }
            visible: !parseInterface.isSolidPattern(isTransaprent ? progressbar.normalTransparentColor : progressbar.normalColor)
        }
        Rectangle{
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: progressbar.borderWidth
            border.color: getStartColor(isTransaprent ? progressbar.transparentborderColor : progressbar.borderColor)
        }
    }

    contentItem: Item{
        implicitHeight: background.height
        implicitWidth: background.width

        Item {
            id: staticProgress
            x: !control.mirrored ? 0 : (control.width * (1.0 - control.position))
            width: control.width * control.position
            height: parent.height
            visible: !control.indeterminate

            // 启用图层效果实现圆角剪裁
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: staticProgress.width
                    height: staticProgress.height
                    radius: progressbar.radius
                }
            }
            Rectangle {
                anchors.fill: parent
                color: gradientItem.visible ? "transparent" : getStartColor(progressbar.childrenColor)
                radius: progressbar.radius

                Rectangle{
                    id: gradientItem
                    anchors.fill: parent
                    radius: parent.radius
                    border.width: progressbar.borderWidth
                    border.color: getStartColor(progressbar.childrenColor)
                    gradient:  Gradient {
                        GradientStop { position: 0.0; color: getStartColor(progressbar.childrenColor) }
                        GradientStop { position: 1.0; color: getEndColor(progressbar.childrenColor)}
                    }
                    visible: !parseInterface.isSolidPattern(progressbar.childrenColor)
                }
            }

            // 普通模式下滑块效果
            Rectangle {
                id: normalAnimationRec
                x: animationX
                width: sliderWidth
                height: parent.height
                visible: isShowNormalProgressBarAnim && !control.indeterminate && control.value > control.from
                color: "transparent"
                radius: progressbar.radius

                // 渐变滑块效果
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    radius: parent.radius
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.02; color: "transparent" }
                        GradientStop { position: 0.4; color: Qt.rgba(1, 1, 1, 0.35) }
                        GradientStop { position: 0.6; color: Qt.rgba(1, 1, 1, 0.35) }
                        GradientStop { position: 0.98; color: "transparent" }
                    }
                }
            }
        }

        Text{
            visible: !control.indeterminate
            anchors.centerIn: parent
            text: (control.position * 100).toString() + "%"
            font: app.font
            color: control.value > (control.from + (control.to - control.from) * 1.0 / 2.0 ) ? getStartColor(progressbar.hightlightTextColor) :
                                                                                             getStartColor(StylePrivate.UKUILabel.normalColor)
        }

        // 不确定状态的滑块
        Rectangle {
            id:rec
            implicitHeight: parent.height
            implicitWidth: control.width * progressbar.indeterminateChildrenWidth / progressbar.normalWidth
            visible: false
            radius: height/2

            LinearGradient {
                anchors.fill: parent
                start: Qt.point(0, 0)
                end: Qt.point(width, 0)
                gradient:  Gradient {
                    GradientStop { position: 0.0; color: getStartColor(progressbar.indeterminateColor) }
                    GradientStop { position: 1.0; color: getEndColor(progressbar.indeterminateColor)}
                }
            }
        }

        // 圆角矩形作为遮罩
        Rectangle {
            id: mask
            anchors.fill: rec
            radius: rec.radius
            color: "white"
            visible: false
        }
        OpacityMask {
            anchors.centerIn: parent
            anchors.fill: rec
            source: rec
            maskSource: mask
            visible: control.indeterminate
        }

        // 不确定状态的动画
        Timer {
            id: slideTimer
            interval: 1
            running: control.indeterminate
            repeat: true
            onTriggered: {
                slideAnimation.start()
            }
        }

        PropertyAnimation {
            id: slideAnimation
            target: rec
            property: "x"
            to: rec.x + rec.width >= control.width ? 0 : control.width - rec.width
            duration: 500 + control.width
            easing.type: Easing.InOutQuad
        }

        // 普通模式的动画 - 固定大小滑块从左到右滑动
        SequentialAnimation {
            id: normalAnimation
            running: !control.indeterminate && control.value > control.from
            loops: Animation.Infinite

            PropertyAnimation {
                target: control
                property: "animationX"
                from: -sliderWidth
                to: control.width * (progressPercent / 100)
                duration: Math.abs(control.width * (progressPercent / 100) - (-sliderWidth)) * 1000 / animationSpeed
                easing.type: Easing.Linear
            }

            PauseAnimation { duration: 300 }

            // 直接跳回起点
            PropertyAction {
                target: control
                property: "animationX"
                value: -sliderWidth
            }
        }
    }

    StylePrivate.UKUIProgressBar{
        id: progressbar
    }

    StylePrivate.APPParameter{
        id: app
    }
}
