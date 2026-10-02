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

import QtQuick 2.6
import QtQuick.Templates 2.5 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate


T.CheckBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 6
    spacing: checkbox.space
    hoverEnabled: true
    palette: app.palette
    font: app.font
    property var normalColor:checkbox.normalIndicatorColor
    property var tansparentNormalColor: checkbox.normalTransparentIndicatorColor
    property var tansparentNormalCheckColor: checkbox.checked_normalIndicatorColor
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    // visualFocus 区分键盘/鼠标聚焦，但「Tab 聚焦后再点击」时 visualFocus 仍为 true，需要 focusEnable
    // 状态机做点击抑制：visualFocus 变 true→置 true，按下→置 false，焦点离开→置 false。
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

    // 新增：判断是否有动画正在运行（涵盖所有相关动画）
    property bool isAnimating: scaleAnim.running || opacityAnim.running
        || uncheckScaleAnim.running || uncheckOpacityAnim.running
        || tansparentColorReturnAnim.running || tansparentColorAnim.running
        || normalColorAnim.running || normalColorReturnAnim.running

    property var borderColor: control.checked || (control.tristate == true && control.checkState === Qt.PartiallyChecked)?
        (!control.enabled ? checkbox.checked_disableIndicatorBorderColor :
         control.pressed ? checkbox.checked_clickIndicatorBorderColor :
         showFocusBorder ? app.focusBorderColor :
         (!isAnimating && control.hovered) ? checkbox.checked_hoverIndicatorBorderColor :
         checkbox.checked_normalIndicatorBorderColor) :
        (!control.enabled ? checkbox.disableIndicatorBorderColor :
         control.pressed ? checkbox.clickIndicatorBorderColor :
         showFocusBorder ? app.focusBorderColor :
         (!isAnimating && control.hovered) ? checkbox.hoverIndicatorBorderColor :
         checkbox.normalIndicatorBorderColor)

    property var checkBoxColor: control.checked || (control.tristate == true && control.checkState === Qt.PartiallyChecked) ?
        (!control.enabled ? checkbox.checked_disableIndicatorColor :
         control.pressed ? checkbox.checked_clickIndicatorColor :
         (!isAnimating && control.hovered) ? checkbox.checked_hoverIndicatorColor :
         tansparentNormalCheckColor) :
        isTransaprent ?
        (!control.enabled ? checkbox.disableTransparentIndicatorColor :
         control.pressed ? checkbox.clickTransparentIndicatorColor :
         (!isAnimating && control.hovered) ? checkbox.hoverTransparentIndicatorColor :
         tansparentNormalColor) :
        (!control.enabled ? checkbox.disableIndicatorColor :
         control.pressed ? checkbox.clickIndicatorColor :
         (!isAnimating && control.hovered) ? checkbox.hoverIndicatorColor :
         normalColor)

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

    indicator: Item{
        implicitWidth: checkbox.indicatorWidth
        implicitHeight: checkbox.indicatorWidth
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2

        Rectangle {
            anchors.fill: parent
            radius: checkbox.radius

            color: gradientRec.visible ? "transparent" : getStartColor(checkBoxColor)
            border.width: showFocusBorder ? app.focusBorderWidth : checkbox.borderWidth
            border.color: getStartColor(borderColor)

            Rectangle{
                id: gradientRec
                anchors.fill: parent
                radius: parent.radius
                border.width: parent.border.width
                border.color: parent.border.color

                gradient:  Gradient {
                    id:gradient
                    GradientStop { position: 0.0; color: getStartColor(checkBoxColor) }
                    GradientStop { position: 1.0; color: getEndColor(checkBoxColor)}
                }
                visible: !parseInterface.isSolidPattern(checkBoxColor)
            }

//            Rectangle{
//                anchors.fill: parent
//                radius: checkbox.radius
//                border.width: (controlFocusReason() && control.focus && focusEnable) ? app.focusBorderWidth : checkbox.borderWidth
//                border.color: getStartColor(borderColor)
//                color: "transparent"
//            }
        }

        Image {
            id: image
            anchors.centerIn: parent
            sourceSize.width:  16
            sourceSize.height: 16
            source: iconSource()
            visible: control.checkState === Qt.Checked || (control.checkState === Qt.Unchecked && uncheckScaleAnim.running)
            cache: false
            scale: 0.2
            opacity: 0.0
            transformOrigin: Item.Center
        }

        NumberAnimation {
            id: scaleAnim
            target: image
            property: "scale"
            from: 0.2
            to: 1.0
            duration: 240
            easing.type: Easing.InOutCirc
            onRunningChanged: if (!running) control.update()
        }

        PropertyAnimation {
            id: opacityAnim
            target: image
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: 100
            easing.type: Easing.InOutQuart
            onRunningChanged: if (!running) control.update()
        }

        NumberAnimation {
            id: uncheckScaleAnim
            target: image
            property: "scale"
            from: 1.0
            to: 0.2
            duration: 240
            easing.type: Easing.InOutCirc
            onRunningChanged: if (!running) control.update()
        }

        PropertyAnimation {
            id: uncheckOpacityAnim
            target: image
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: 100
            easing.type: Easing.InOutQuint
            onRunningChanged: if (!running) control.update()
        }

        Rectangle {
            anchors.centerIn: parent
            width: 9
            height: 1
            color: {
                getStartColor(!control.enabled ? checkbox.checked_disableChildrenColor :
                             control.pressed ? checkbox.checked_clickChildrenColor:
                             // 部分选中状态处理hover与动画的关系
                             (!isAnimating && control.hovered) ? checkbox.checked_hoverChildrenColor:
                             checkbox.checked_normalChildrenColor)
            }
            visible: control.checkState === Qt.PartiallyChecked
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
        text: control.text
        font: control.font
        color: getStartColor(control.enabled ? checkbox.normalTextColor : checkbox.disableTextColor)
    }

    StylePrivate.APPParameter{
        id: app
        onParametryChanged:{
            iconUpdate();
        }
        onIconThemeChanged: {
            iconUpdate()
        }
    }
    StylePrivate.UKUICheckBox{
        id: checkbox
    }
    onEnabledChanged: iconUpdate()
    function iconUpdate(){
        image.source = ""
        image.source = iconSource()
        control.update();
    }

    ColorAnimation {
        id: tansparentColorAnim
        target: control
        property: "tansparentNormalCheckColor"
        from:checkbox.normalTransparentIndicatorColor
        to: checkbox.checked_normalIndicatorColor
        duration: 200
        easing.type: Easing.Linear
        running: false
        onRunningChanged: if (!running) control.update()
    }

    ColorAnimation {
        id: tansparentColorReturnAnim
        target: control
        property: "tansparentNormalColor"
        from: checkbox.checked_normalIndicatorColor
        to: checkbox.normalTransparentIndicatorColor
        duration: 200
        easing.type: Easing.Linear
        running: false
        onRunningChanged: if (!running) control.update()
    }
    ColorAnimation {
        id: normalColorAnim
        target: control
        property: "tansparentNormalCheckColor"
        from:checkbox.normalIndicatorColor
        to: checkbox.checked_normalIndicatorColor
        duration: 200
        easing.type: Easing.Linear
        running: false
        onRunningChanged: if (!running) control.update()
    }
    ColorAnimation {
        id: normalColorReturnAnim
        target: control
        property: "normalColor"
        from: checkbox.checked_normalIndicatorColor
        to: checkbox.normalIndicatorColor
        duration: 200
        easing.type: Easing.Linear
        running: false
        onRunningChanged: if (!running) control.update()
    }



    property int prevCheckState: Qt.Unchecked

    Component.onCompleted: {
        prevCheckState = control.checkState;
    }

    onCheckStateChanged: {
        // console.log("状态转换：", prevCheckState, "→", control.checkState)
        if (prevCheckState === Qt.Unchecked && control.checkState === Qt.PartiallyChecked) {

            if(isTransaprent){
                tansparentColorReturnAnim.stop();
                tansparentColorAnim.start();
            }else{
                normalColorReturnAnim.stop();
                normalColorAnim.start();
            }

            scaleAnim.start();
            opacityAnim.start();
        }
        else if (prevCheckState === Qt.PartiallyChecked && control.checkState === Qt.Checked) {

            if(isTransaprent){
                tansparentColorReturnAnim.stop();
                tansparentColorAnim.stop();
                tansparentNormalCheckColor = checkbox.checked_normalIndicatorColor;
            }else{
                normalColorReturnAnim.stop();
                normalColorAnim.stop();
                normalColor = checkbox.checked_normalIndicatorColor;
            }
            scaleAnim.start();
            opacityAnim.start();
        }
        else if (prevCheckState === Qt.Checked && control.checkState === Qt.Unchecked) {
            if(isTransaprent){
                tansparentColorAnim.stop();
                tansparentColorReturnAnim.start();
            }else{
                normalColorAnim.stop();
                normalColorReturnAnim.start();
            }
            uncheckScaleAnim.start();
            uncheckOpacityAnim.start();
        }
        else if (prevCheckState === Qt.Unchecked && control.checkState === Qt.Checked) {
            if(isTransaprent){
                tansparentColorReturnAnim.stop();
                tansparentColorAnim.start();
            }else{
                normalColorReturnAnim.stop();
                normalColorAnim.start();
            }
            scaleAnim.start();
            opacityAnim.start();
        }

        prevCheckState = control.checkState;
        control.update();
    }

    onCheckedChanged: {
        if (!control.tristate) {
            if (control.checked) {
                if(isTransaprent)
                    tansparentColorAnim.start();
                else
                    normalColorAnim.start();
                scaleAnim.start();
                opacityAnim.start();
            } else {
                if(isTransaprent)
                    tansparentColorReturnAnim.start();
                else
                    normalColorReturnAnim.start();
                uncheckScaleAnim.start();
                uncheckOpacityAnim.start();
            }
        }
        image.source = iconSource();
    }

    function iconSource(){
        // var model = (!control.enabled && control.checked) ? "highDisbale" :
        //             !control.enabled ? "disenable" :
        //             control.checked ? "highlight" : "normal"
        var model = "highlight"
        var icon = "object-select-symbolic";
        if(!app.themeHasIcon(icon))
            icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/object-select-symbolic.svg"
        return "image://imageProvider/" + icon + "/" + model;
    }

}
