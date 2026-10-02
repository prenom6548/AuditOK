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
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.SpinBox {
    id: control
    hoverEnabled: true

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset//Math.max(implicitBackgroundWidth + leftInset + rightInset,
                   //         contentItem.implicitWidth + 2 * padding +
                   //         up.implicitIndicatorWidth)
    implicitHeight: implicitContentHeight + topPadding + bottomPadding//Math.max(implicitContentHeight + topPadding + bottomPadding,
                  //         implicitBackgroundHeight,
                    //         up.implicitIndicatorHeight + down.implicitIndicatorHeight)

    padding: spinbox.padding
//    leftPadding: padding + (control.mirrored ? (up.indicator ? up.indicator.width : 0) : (down.indicator ? down.indicator.width : 0))
    rightPadding: padding + (control.mirrored ? (down.indicator ? down.indicator.width : 0) : (up.indicator ? up.indicator.width : 0))

    font: app.font
    palette: app.palette

    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property bool upHover: control.up.hovered
    property bool upClick: control.up.pressed
    property bool downHover: control.down.hovered
    property bool downClick: control.down.pressed
    property bool controlPress: false
    property var backColor: !isTransaprent ? (!control.enabled ? spinbox.disableColor : ((control.pressed || control.activeFocus)? spinbox.clickColor :
                                            ((control.hovered && !control.upHover && !control.downHover) ? spinbox.hoverColor : spinbox.normalColor))) :
                                            (!control.enabled ? spinbox.disableTransparentBC :
                                            ((control.pressed || control.activeFocus)? spinbox.clickedTransparentBC : ((control.hovered && !control.upHover && !control.downHover) ? spinbox.hoveredTransparentBC : spinbox.normalTransparentBC)))
    property var borderColor: !isTransaprent ? (!control.enabled ? spinbox.disableBorderColor :
                                              ((control.activeFocus) ? spinbox.clickBorderColor :
                                              (control.pressed ? spinbox.clickBorderColor :
                                              ((control.hovered && !control.upHover && !control.downHover) ? spinbox.hoverBorderColor : spinbox.normalBorderColor)))) :
                                              (!control.enabled ? spinbox.disableTransparentBorderColor : ((control.activeFocus) ? spinbox.clickedTransparentBorderColor :
                                              (control.pressed ? spinbox.clickedTransparentBorderColor : ((control.hovered && !control.upHover && !control.downHover) ? spinbox.hoveredTransparentBorderColor : spinbox.normalTransparentBorderColor))))

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
    onUpHoverChanged:{
        upRadiusRec.canvas.requestPaint();
    }
    onUpClickChanged:{
        upRadiusRec.canvas.requestPaint();
    }
    onDownHoverChanged:{
        downRadiusRec.canvas.requestPaint();
    }
    onDownClickChanged:{
        downRadiusRec.canvas.requestPaint();
    }
    onValueChanged: {
        iconUpdate()
    }
    onEnabledChanged: {
        iconUpdate()
    }


    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: TextInput {
        id:textInput
        z: 2
        text: control.displayText
        font: control.font
        color: getStartColor(!control.enabled ? spinbox.disableTextColor : spinbox.normalTextColor)
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        anchors.left: control.mirrored ? upRadiusRec.right : control.left
        anchors.leftMargin: StylePrivate.UKUITextFiled.leftRightPadding

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
    }

    up.indicator: RadiusRectangle {
        id:upRadiusRec
        z:3
        implicitWidth: spinbox.btnNormalWidth
        //implicitHeight: control.height/2.0
        anchors.right: control.mirrored ? undefined : control.right
        anchors.rightMargin: control.mirrored ? undefined : (borderRec.border.color.a > 0 ? borderRec.border.width : 0)
        anchors.left: control.mirrored ? control.left : undefined
        anchors.leftMargin: control.mirrored ? (borderRec.border.color.a > 0 ? borderRec.border.width : 0) : undefined
        anchors.top: parent.top
        anchors.topMargin: borderRec.border.color.a > 0 ? borderRec.border.width : 0
        anchors.bottom: control.verticalCenter

        leftTopRadius: spinbox.radius
        rightTopRadius: spinbox.radius
        leftBottomRadius: 0
        rightBottomRadius: 0
        backColor: isTransaprent ? ((!upRadiusRec.enabled || !control.enabled) ? spinbox.btnDisableTransparentColor :
                                   control.up.pressed ? spinbox.btnClickTransparentColor : control.up.hovered ?
                                   spinbox.btnHoverTransparentColor : spinbox.btnNormalTransparentColor) :
                                   ((!upRadiusRec.enabled || !control.enabled) ? spinbox.btnDisableColor : control.up.pressed ? spinbox.btnClickColor :
                    control.up.hovered ? spinbox.btnHoverColor : spinbox.btnNormalColor)
        Image {
            id: upImage
            anchors.centerIn: parent
            sourceSize.width: 16
            sourceSize.height: 16
            source: upIconSource()
            cache: false
        }
    }

    down.indicator: RadiusRectangle {
        id: downRadiusRec
        z:3
        implicitWidth: spinbox.btnNormalWidth
        //implicitHeight: control.height/2.0

        anchors.top: control.verticalCenter
        anchors.right: control.mirrored ? undefined : control.right
        anchors.rightMargin: control.mirrored ? undefined : (borderRec.border.color.a > 0 ? borderRec.border.width : 0)
        anchors.left: control.mirrored ? control.left : undefined
        anchors.leftMargin: control.mirrored ? (borderRec.border.color.a > 0 ? borderRec.border.width : 0) : undefined
        anchors.bottom: parent.bottom
        anchors.bottomMargin: borderRec.border.color.a > 0 ? borderRec.border.width : 0
        leftTopRadius: 0
        rightTopRadius: 0
        leftBottomRadius: spinbox.radius
        rightBottomRadius: spinbox.radius

        backColor: isTransaprent ? ((!upRadiusRec.enabled || !control.enabled) ? spinbox.btnDisableTransparentColor :
                                   control.down.pressed ? spinbox.btnClickTransparentColor : control.down.hovered ?
                                   spinbox.btnHoverTransparentColor : spinbox.btnNormalTransparentColor) :
                                   ((!upRadiusRec.enabled || !control.enabled) ? spinbox.btnDisableColor : control.down.pressed ? spinbox.btnClickColor :
                    control.down.hovered ? spinbox.btnHoverColor : spinbox.btnNormalColor)

        Image {
            id: downImage
            anchors.centerIn: parent
            sourceSize.width: 16
            sourceSize.height: 16
            source: downIconSource()
            cache: false
        }
    }

    MouseArea {
        anchors.fill: upRadiusRec
        acceptedButtons: Qt.LeftButton
        enabled: control.enabled
        onPressed: {
            control.forceActiveFocus()
            mouse.accepted = false
        }
    }

    MouseArea {
        anchors.fill: downRadiusRec
        acceptedButtons: Qt.LeftButton
        enabled: control.enabled
        onPressed: {
            control.forceActiveFocus()
            mouse.accepted = false
        }
    }

    background: Rectangle {
        implicitWidth: spinbox.normalWidth
        implicitHeight: spinbox.normalHeight
        radius: spinbox.radius
        color: !parseInterface.isSolidPattern(backColor) ? "transparent" : getStartColor(backColor)
        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(backColor) }

                GradientStop { position: 1.0; color: getEndColor(backColor)}
            }
            visible: !parseInterface.isSolidPattern(backColor)
        }
        Rectangle{
            id: borderRec
        anchors.fill: parent
        radius: parent.radius
        border.width: (control.activeFocus) ? spinbox.focusBorderWidth : spinbox.normalBorderWidth
        border.color: getStartColor(borderColor)
        color: "transparent"
        }
//        MouseArea{
//            id: mouseArea
//            anchors.fill: parent
//            onPressed: controlPress = true
//            onReleased: controlPress = false
//        }
    }

    StylePrivate.UKUISpinBox{
        id: spinbox
    }

    StylePrivate.APPParameter{
        id: app
        onParametryChanged:{
            iconUpdate()
        }
        onIconThemeChanged: {
            iconUpdate()
        }
    }
    function iconUpdate()
    {
        upImage.source = ""
        upImage.source = upIconSource()
        downImage.source = ""
        downImage.source = downIconSource()
        control.update();
    }

    function upIconSource(){
        var model = ((!upRadiusRec.enabled || !control.enabled) && app.isDark) ? "highDisbale" : (!upRadiusRec.enabled || !control.enabled) ? "disenable" : (app.isDark) ? "highlight" : "normal"

        var icon = "ukui-up-symbolic";
        if(!app.themeHasIcon(icon))
            icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/ukui-up-symbolic.svg"
        return "image://imageProvider/" + icon + "/" + model;
    }
    function downIconSource(){
        var model = ((!downRadiusRec.enabled || !control.enabled) && app.isDark) ? "highDisbale" : (!downRadiusRec.enabled || !control.enabled) ? "disenable" : (app.isDark) ? "highlight" : "normal"
        var icon = "ukui-down-symbolic";
        if(!app.themeHasIcon(icon))
            icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/ukui-down-symbolic.svg"
        return "image://imageProvider/" + icon + "/" + model;

    }
    
}
