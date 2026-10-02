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

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    // mirrored 属性，qml自动切换
    property bool mirrored: Qt.application.layoutDirection === Qt.RightToLeft

//    padding: textfield.leftRightPadding

    // leftPadding: padding + 4
    // rightPadding: (cleanBtn.visible && eyeBtn.visible ? cleanBtn.width + eyeBtn.width + 8 :
    //               cleanBtn.visible ? cleanBtn.width + 8 : eyeBtn.visible ? eyeBtn.width + 8 : textfield.leftRightPadding)
    
    
    leftPadding: control.mirrored ? 
                 (cleanBtn.visible && eyeBtn.visible ? cleanBtn.width + eyeBtn.width + 8 :
                  cleanBtn.visible ? cleanBtn.width + 8 : 
                  eyeBtn.visible ? eyeBtn.width + 8 : textfield.leftRightPadding) : 
                 (textfield.leftRightPadding + 4)
    rightPadding: control.mirrored ? 
                  (textfield.leftRightPadding + 4) :
                  (cleanBtn.visible && eyeBtn.visible ? cleanBtn.width + eyeBtn.width + 8 :
                   cleanBtn.visible ? cleanBtn.width + 8 : eyeBtn.visible ? eyeBtn.width + 8 : textfield.leftRightPadding)
    

    color: getStartColor(!control.enabled ? textfield.disableTextColor : textfield.normalTextColor)
    selectionColor: getStartColor(textfield.focusBorderColor)
    selectedTextColor: control.palette.highlightedText
    placeholderTextColor: getStartColor(!control.enabled ? textfield.placeHolderDisableTextColor : textfield.placeHolderNormalTextColor)
    verticalAlignment: TextInput.AlignVCenter

    // 让placeholder 文本, 镜像模式下右对齐
    horizontalAlignment: control.mirrored ? TextInput.AlignRight : TextInput.AlignLeft

    font: app.font
    hoverEnabled: true
    selectByMouse: true
    palette: app.palette
    property var isPassMode:{
        if(control.hasOwnProperty("passMode"))
            return control.passMode
        else
            return false
    }
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var backColor: !control.enabled ? (isTransaprent ? textfield.disableTransparentBC : textfield.disableBC) :
                             (control.activeFocus) ? (isTransaprent ? textfield.inputNormalTransparentBC : textfield.inputNormalBC) :
                             click ? (isTransaprent ? textfield.clickedTransparentBC : textfield.clickedBC) :
                             control.hovered ? (isTransaprent ? textfield.hoveredTransparentBC : textfield.hoveredBC) :
                                               (isTransaprent ? textfield.normalTransparentBC : textfield.normalBC)
    property var backBorderColor: !control.enabled ? textfield.disableBorderColor : (control.activeFocus) ? textfield.focusBorderColor :
                                                                                                                           control.hovered ? textfield.hoverBorderColor : textfield.normalBorderColor

//    property bool hover: false
    property bool click: false
    property bool cleanVisible: false
    property bool _needCleanBtn:{
        if(control.hasOwnProperty("needCleanBtn"))
            return control.needCleanBtn
        else
            return true
    }

    Component.onCompleted: {
        if(isPassMode)
            control.echoMode = TextInput.Password
        imageSource()
        // console.log("===> TextField: mirrored = ", mirrored, ", control.mirrored = ",control.mirrored, ",leftPadding = ",leftPadding,",rightPadding = ",rightPadding,",placeholder.width = ",placeholder.width)
    }
    onEchoModeChanged: {
//        console.log("echomode changedee", control.echoMode)
        imageSource()
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

    PlaceholderText {
        id: placeholder
        //x: control.leftPadding
        x: control.mirrored ? control.width - control.rightPadding - width : control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: getStartColor(!control.enabled ? textfield.placeHolderDisableTextColor : textfield.placeHolderNormalTextColor)
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        
        //elide: Text.ElideRight
        elide: control.mirrored ? Text.ElideLeft : Text.ElideRight

        renderType: control.renderType
    }
    //TextField background
    background: Rectangle {
        id: backrec
        radius: textfield.radius
        implicitWidth: textfield.normalWidth
        implicitHeight: textfield.normalHeight
        color: gradientRec.visible ? "transparent" : getStartColor(backColor)

        Rectangle {
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(backColor) }

                GradientStop { position: 1.0; color: getEndColor(backColor)}
            }
            visible: !parseInterface.isSolidPattern(backColor)
        }
        Rectangle {
            id: bac
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: (control.activeFocus) ? textfield.focusBorderWidth : textfield.borderWidth
            border.color: getStartColor(backBorderColor)
        }

        RoundButton {
            id: cleanBtn
            Accessible.name: qsTr("Clear")
            width: height
            height:  control.height >= 36 ? 30 : (control.height - 6)
            // anchors.right: eyeBtn.visible ? eyeBtn.left : parent.right

            anchors.right: control.mirrored ? undefined : (eyeBtn.visible ? eyeBtn.left : parent.right)
            anchors.left: control.mirrored ? (eyeBtn.visible ? eyeBtn.right : parent.left) : undefined

            anchors.verticalCenter: parent.verticalCenter
            //anchors.rightMargin: eyeBtn.visible ? 0 : backrec.border.width

            anchors.rightMargin: control.mirrored ? 0 : (eyeBtn.visible ? 0 : bac.border.width)
            anchors.leftMargin: control.mirrored ? (eyeBtn.visible ? 0 : bac.border.width) : 0

            flat: true
            visible: _needCleanBtn && !control.readOnly && control.enabled && (control.text !== "")
//            icon.source: "window-close-symbolic"

            // onClicked: {
            // }
            onPressed: {
                control.clear()
                cleanVisible = false
                control.focus = true
            }
        }
        RoundButton{
            id: eyeBtn
            Accessible.name: qsTr("Show or hide password")
//            icon.source: "ukui-eye-hidden-symbolic"
            //anchors.right: parent.right

            anchors.right: control.mirrored ? undefined : parent.right
            anchors.left: control.mirrored ? parent.left : undefined

            anchors.verticalCenter: parent.verticalCenter
            //anchors.rightMargin: (control.activeFocus) ? textfield.focusBorderWidth : textfield.borderWidth

            anchors.rightMargin: control.mirrored ? 0 : ((control.activeFocus) ? textfield.focusBorderWidth : textfield.borderWidth)
            anchors.leftMargin: control.mirrored ? ((control.activeFocus) ? textfield.focusBorderWidth : textfield.borderWidth) : 0

            width: height
            height:  control.height >= 36 ? 30 : (control.height - 6)
            visible: control.enabled && isPassMode
            flat: true
            onPressed: {
                if(control.echoMode == TextInput.Normal)
                    control.echoMode = TextInput.Password
                else
                    control.echoMode = TextInput.Normal
                control.focus = true
            }
        }
    }

    StylePrivate.UKUITextFiled{
        id: textfield
    }

    StylePrivate.APPParameter{
        id: app

        onParametryChanged:{
            iconUpdate();
        }
        onIconThemeChanged: {
            iconUpdate();
        }
    }
    onEnabledChanged: iconUpdate()

    function iconUpdate(){
        eyeBtn.icon.source = ""
        cleanBtn.icon.source = ""
        imageSource();
    }

    function imageSource(){
        //            return "image://imageProvider/file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/object-select-symbolic.svg/normal"

        var model = "normal"
        if(!control.enabled){
            if(app.isDark)
                model = "highDisbale"
            else
                model = "disenable"
        }
        else if(app.isDark){
            model = "highlight"
        }
        //console.log("control.echoMode...", control.echoMode === TextInput.Password)
        if(isPassMode){
            if(control.echoMode === TextInput.Password)
                eyeBtn.icon.source = "ukui-eye-hidden-symbolic/" + model
            else
                eyeBtn.icon.source = "ukui-eye-display-symbolic/" + model

        }
        if(control.text !== "")
            cleanBtn.icon.source = "window-close-symbolic/" + model

    }

    onPressed: {
        click = true;
    }
    onTextChanged: {
        imageSource()
        if(control.text !== "")
            cleanVisible = true
    }
    onReleased: click = false
}
