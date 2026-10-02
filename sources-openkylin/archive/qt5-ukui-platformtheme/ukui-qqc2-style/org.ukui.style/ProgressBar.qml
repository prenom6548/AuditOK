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
    StylePrivate.ParseColorInterface {
        id: parseColor
    }

    function getStartColor(dtcolor)
    {
        return parseInterface.pStartColor(dtcolor)
    }
    function getEndColor(dtcolor)
    {
        return parseInterface.pEndColor(dtcolor)
    }

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token

    /// 优先 token，否则 getDtColor；结果统一走 getStartColor/getEndColor
    function dtToken(tokenName, dtKey) {
        if (_hasToken && ukui_platformtheme_token[tokenName] !== undefined
                && ukui_platformtheme_token[tokenName] !== null)
            return ukui_platformtheme_token[tokenName]
        return parseColor.getDtColor(dtKey)
    }
    function dtStart(tokenName, dtKey) {
        return getStartColor(dtToken(tokenName, dtKey))
    }
    function dtEnd(tokenName, dtKey) {
        return getEndColor(dtToken(tokenName, dtKey))
    }
    function dtSolid(tokenName, dtKey) {
        return parseInterface.isSolidPattern(dtToken(tokenName, dtKey))
    }

    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    /// 可选：实例上声明 property bool textVisible 控制居中百分比（默认 true）
    property bool _textVisible: !control.hasOwnProperty("textVisible") || control.textVisible
    /// 可选：实例上声明 property string progressText 自定义居中文案（默认百分比）
    property string _progressText: {
        if (control.hasOwnProperty("progressText"))
            return control.progressText
        return Math.round(control.position * 100).toString() + "%"
    }
    /// 可选：实例上声明 property bool showNormalProgressBarAnim 开启普通态扫光
    property var isShowNormalProgressBarAnim:{
        if(control.hasOwnProperty("showNormalProgressBarAnim"))
            return control.showNormalProgressBarAnim
        else
            return false
    }
    /// 可选：自定义背景色（纯色）；未设置则用主题槽色
    property bool _customBg: control.hasOwnProperty("progressBackgroundColor")
                             && control.progressBackgroundColor !== undefined
                             && control.progressBackgroundColor !== null
    /// 可选：覆盖填充色；未设置则用 progressbar.childrenColor
    property var _childrenColor: {
        if (control.hasOwnProperty("progressChildrenColor") && control.progressChildrenColor)
            return control.progressChildrenColor
        return progressbar.childrenColor
    }
    /// 可选：填充渐变终点（与 progressChildrenColor 组成双色渐变）
    property var _childrenColorEnd: {
        if (control.hasOwnProperty("progressChildrenColorEnd") && control.progressChildrenColorEnd)
            return control.progressChildrenColorEnd
        return null
    }
    /// 纯色/自定义色（非 DT brush）时为 true，直接当 Color 用
    property bool _fillPlain: control.hasOwnProperty("progressFillPlain") && control.progressFillPlain
    property bool _inverted: control.hasOwnProperty("invertedAppearance") && control.invertedAppearance

    /// disable：不透明槽 kcomponent-disable；半透明槽 kcomponent-selected-alpha-disable
    property var _grooveBrush: {
        if (!control.enabled) {
            if (isTransaprent)
                return dtToken("kComponentSelectedAlphaDisable", "kcomponent-selected-alpha-disable")
            return dtToken("kComponentDisable", "kcomponent-disable")
        }
        return isTransaprent ? progressbar.normalTransparentColor : progressbar.normalColor
    }

    function fillStartColor() {
        if (!control.enabled)
            return dtStart("kBrandDisable", "kbrand-disable")
        if (_fillPlain)
            return _childrenColor
        return getStartColor(_childrenColor)
    }
    function fillEndColor() {
        if (!control.enabled)
            return dtEnd("kBrandDisable", "kbrand-disable")
        if (_childrenColorEnd)
            return _fillPlain ? _childrenColorEnd : getStartColor(_childrenColorEnd)
        if (_fillPlain)
            return _childrenColor
        return getEndColor(_childrenColor)
    }
    function fillUseGradient() {
        if (!control.enabled)
            return !dtSolid("kBrandDisable", "kbrand-disable")
        if (_childrenColorEnd)
            return true
        if (_fillPlain)
            return false
        return !parseInterface.isSolidPattern(_childrenColor)
    }

    /// 居中文字：disable 时 ≤50% 用 kfont-primary-disable，>50% 用 kfont-white-disable
    property color _centerTextColor: {
        var overHalf = control.value > (control.from + (control.to - control.from) * 1.0 / 2.0)
        if (!control.enabled) {
            if (overHalf)
                return dtStart("kFontWhiteDisable", "kfont-white-disable")
            return dtStart("kFontPrimaryDisable", "kfont-primary-disable")
        }
        if (overHalf)
            return getStartColor(progressbar.hightlightTextColor)
        return getStartColor(StylePrivate.UKUILabel.normalColor)
    }

    property real animationSpeed: 160  // 统一速度值（像素/秒）
    property real sliderWidth: 56  // 保持滑块宽度不变
    // 用于普通模式动画的属性
    property real animationX: -sliderWidth

    // 计算进度百分比的辅助属性
    property real progressPercent: Math.min(Math.max(0, (control.value - control.from) / (control.to - control.from) * 100), 100)

    background: Rectangle {
        id: bg
        radius: progressbar.radius
        implicitWidth: progressbar.normalWidth
        implicitHeight: control.indeterminate ? 8 : progressbar.normalHeight
        // 不写死 height，由控件几何铺满，保证槽与填充同尺寸、同起点

        color: {
            if (control.enabled && control._customBg)
                return control.progressBackgroundColor
            return gradientRec.visible ? "transparent" : getStartColor(control._grooveBrush)
        }

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(control._grooveBrush)}

                GradientStop { position: 1.0; color: getEndColor(control._grooveBrush)}
            }
            visible: !(control.enabled && control._customBg) && !parseInterface.isSolidPattern(control._grooveBrush)
        }

        // 进度填充：切换方向时用代码显式解锚/设锚，避免绑定 undefined 残留
        Item {
            id: staticProgress
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * control.position
            visible: !control.indeterminate

            property bool fillFromRight: control._inverted || control.mirrored

            function updateFillAnchor() {
                if (fillFromRight) {
                    anchors.left = undefined
                    anchors.right = parent.right
                } else {
                    anchors.right = undefined
                    anchors.left = parent.left
                }
            }

            Component.onCompleted: updateFillAnchor()
            onFillFromRightChanged: updateFillAnchor()

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
                color: gradientItem.visible ? "transparent" : control.fillStartColor()
                radius: progressbar.radius

                Rectangle{
                    id: gradientItem
                    anchors.fill: parent
                    radius: parent.radius
                    border.width: 0//progressbar.borderWidth
                    border.color: control.fillStartColor()
                    gradient:  Gradient {
                        GradientStop { position: 0.0; color: control.fillStartColor() }
                        GradientStop { position: 1.0; color: control.fillEndColor()}
                    }
                    visible: control.fillUseGradient()
                }
            }

            Rectangle {
                id: normalAnimationRec
                x: animationX
                width: sliderWidth
                height: parent.height
                visible: isShowNormalProgressBarAnim && control.enabled
                         && !control.indeterminate && control.value > control.from
                color: "transparent"
                radius: progressbar.radius

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

        Rectangle{
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: progressbar.borderWidth
            border.color: getStartColor(isTransaprent ? progressbar.transparentborderColor : progressbar.borderColor)
        }
    }

    contentItem: Item{
        implicitHeight: bg.implicitHeight
        implicitWidth: bg.implicitWidth

        // 居中百分比（textVisible 控制显隐）
        Text{
            visible: control._textVisible && !control.indeterminate
            anchors.centerIn: parent
            text: control._progressText
            font: app.font
            color: control._centerTextColor
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
            running: isShowNormalProgressBarAnim && control.enabled
                     && !control.indeterminate && control.value > control.from
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
