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
import QtQuick.Layouts 1.2
import QtQuick.Templates 2.5 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate
import QtQuick.Controls 2.5

T.MenuItem {
    id: controlRoot

    palette: app.palette
    font: app.font


    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            image.sourceSize.width + label.implicitWidth + labelLeftSpace + leftPadding + rightPadding +
                            (hasChecked ? /*arrow.implicitWidth*/menuItem.imageWidth + menuItem.imageSpace : 0) +
                            (controlRoot.subMenu != null ? /*indicator.implicitWidth*/menuItem.imageWidth + menuItem.imageSpace : 0) + itemLeftSpace)
    implicitHeight: visible ? Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                       Math.max(implicitContentHeight,
                                      (controlRoot.subMenu ? indicator.implicitHeight : 0) + topPadding + bottomPadding), menuItem.normalHeight) : 0

    baselineOffset: contentItem.y + contentItem.baselineOffset

    padding: menuItem.topBottomPadding

    onCheckedChanged: {
        // console.log("checked changedddddddd", checked, controlRoot.text)
//        itemHasChecked = controlRoot.checkable && controlRoot.checked;
        //itemLeftSpace = (!(itemHasChecked)/* && !controlRoot.subMenu*/) ? controlRoot.leftPadding : menuItem.imageWidth + 4 + controlRoot.leftPadding
        // console.log("checked changedd aaaaaaaaaaaaa", checked, controlRoot.text, itemHasChecked, itemLeftSpace, menuItem.imageWidth)

        image.source = contentImageSource();
        indicatorImage.source = indicatorImageSource()
        arrowImage.source = arrowImageSource()
        controlRoot.update();

        controlRoot.update()

    }

    property var focusEnable: app.focusEnable

    property bool itemHasChecked: controlRoot.checkable && controlRoot.checked
    property bool hasChecked: false

    property int gridUnit: fontMetrics.height
    property int labelLeftSpace: image.visible ? 8 : 0
    property int itemLeftSpace: (!(itemHasChecked)/* && !controlRoot.subMenu*/) ? controlRoot.leftPadding : menuItem.imageWidth + 8 + controlRoot.leftPadding

    property var isTransaprent:{
        if(controlRoot.hasOwnProperty("transparent"))
            return controlRoot.transparent
        else
            return true
    }
    property var isHightModel:{
        if(controlRoot.hasOwnProperty("type")){
            if(controlRoot.type === "import")
                return true
        }
        return false
    }
    property var isKeyBorderSelect: hovered //键盘选中 或者hover
    property var borderHColor: !controlRoot.enabled ? menuItem.disableCheckedHBorderColor : (controlFocusReason() && controlRoot.focus && focusEnable) ? app.focusBorderColor : controlRoot.pressed ? menuItem.clickedCheckedHBorderColor :
                               isKeyBorderSelect ? menuItem.hoveredCheckedHBorderColor : menuItem.normalCheckedHBorderColor
    property var backHColor: !controlRoot.enabled ? menuItem.disableCheckedHBC : controlRoot.pressed ? menuItem.clickedCheckedHBC :
                             isKeyBorderSelect ? menuItem.hoveredCheckedHBC : menuItem.normalCheckedHBC

    property var borderColor: !controlRoot.enabled ? (isTransaprent ? menuItem.disableTransparentBorderColor : menuItem.disableBorderColor) :
                               (controlFocusReason() && controlRoot.focus && focusEnable) ? app.focusBorderColor :
                               controlRoot.pressed ? (isTransaprent ? menuItem.clickTransparentBorderColor : menuItem.clickBorderColor) :
                               isKeyBorderSelect ? (isTransaprent ? menuItem.hoverTransparentBorderColor : menuItem.hoverBorderColor) :
                                                 (isTransaprent ? menuItem.normalTransparentBorderColor  : menuItem.normalBorderColor)
    property var backColor: !controlRoot.enabled ? (isTransaprent ? menuItem.disableTransparentBC : menuItem.disableBC) :
                             controlRoot.pressed ? (isTransaprent ? menuItem.clickedTransparentBC : menuItem.clickedBC) :
                             (isKeyBorderSelect || (controlRoot.subMenu && controlRoot.subMenu.visible)) ? (isTransaprent ? menuItem.hoveredTransparentBC : menuItem.hoveredBC) :
                             (isTransaprent ? menuItem.normalTransparentBC  : menuItem.normalBC)
//    property var borderCheckedColor: !controlRoot.enabled ? (isTransaprent ? menuItem.disableCheckedTransparentBorderColor : menuItem.disableBorderCheckedColor) :
//                                      controlRoot.pressed ? (isTransaprent ? menuItem.clickedCheckedTransparentBorderColor : menuItem.clickBorderCheckedColor) :
//                                      isKeyBorderSelect ? (isTransaprent ? menuItem.hoveredCheckedTransparentBorderColor : menuItem.hoverBorderCheckedColor) :
//                                                        (isTransaprent ?  menuItem.normalCheckedTransparentBorderColor : menuItem.normalBorderCheckedColor)
//    property var backCheckedColor: !controlRoot.enabled ? (isTransaprent ? menuItem.disableCheckedTransparentBC : menuItem.disableCheckedBC) :
//                                    controlRoot.pressed ? (isTransaprent ? menuItem.clickedCheckedTransparentBC : menuItem.clickedCheckedBC) :
//                                    isKeyBorderSelect ? (isTransaprent ? menuItem.hoveredCheckedTransparentBC : menuItem.hoveredCheckedBC) :
//                                    (isTransaprent ?  menuItem.normalCheckedTransparentBC : menuItem.normalCheckedBC)

    property var bacDtColor: !controlRoot.enabled ? menuItem.disableBC :
                                                    controlRoot.pressed ? menuItem.clickedBC :
                                                                            isKeyBorderSelect ? menuItem.hoveredBC : menuItem.normalBC

    leftPadding: menuItem.leftRightPadding/* : Math.floor(gridUnit/4)*2*/
    rightPadding: menuItem.leftRightPadding// Math.floor(gridUnit/4)*2
    hoverEnabled: true

    onItemHasCheckedChanged: {
        itemLeftSpace = (!(itemHasChecked)/* && !controlRoot.subMenu*/) ? controlRoot.leftPadding : menuItem.imageWidth + 8 + controlRoot.leftPadding
    }

    onHasCheckedChanged: {
        implicitWidth = Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                 image.sourceSize.width + label.implicitWidth + labelLeftSpace + leftPadding + rightPadding +
                                 (hasChecked ? /*arrow.implicitWidth*/menuItem.imageWidth + menuItem.imageSpace : 0) +
                                 (controlRoot.subMenu != null ? /*indicator.implicitWidth*/menuItem.imageWidth + menuItem.imageSpace : 0) + itemLeftSpace)

    }

    StylePrivate.APPParameter{
        id: appP
    }

    NormalControlColor{
    id:ncColor
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
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))

        onActivated: {
            if (controlRoot.checkable) {
                controlRoot.toggle();
            } else {
                controlRoot.clicked();
            }
        }
    }

    contentItem:Item{
//        x: itemLeftSpace + controlRoot.leftPadding
        anchors.left: parent.left
        anchors.leftMargin: controlRoot.mirrored ? controlRoot.leftPadding : itemLeftSpace
        anchors.right: parent.right
        anchors.rightMargin: controlRoot.mirrored ? itemLeftSpace : controlRoot.rightPadding
        implicitHeight: Math.max(image.height, label.height)
        Image {
            id: image
            source: contentImageSource()//controlRoot.icon.source
            cache: false
            anchors.verticalCenter: parent.verticalCenter
            sourceSize.width:  (controlRoot.display !== AbstractButton.TextOnly && source !== "" ) ? menuItem.imageWidth : 0
            sourceSize.height: (controlRoot.display !== AbstractButton.TextOnly && source !== "" ) ? menuItem.imageWidth : 0
            visible: controlRoot.display !== AbstractButton.TextOnly && source !== "" && status != Image.Null
            anchors.left: controlRoot.mirrored ? undefined : parent.left
            anchors.right: controlRoot.mirrored ? parent.right : undefined

        }
        Text {
            id:label
            text: controlRoot.text
            font:controlRoot.font
            elide: Text.ElideRight
            color: getStartColor(controlRoot.enabled ? (isHightModel ? menuItem.normalHTextColor : menuItem.normalTextColor) : (isHightModel ? menuItem.disableHTextColor : menuItem.disableTextColor))
            visible: controlRoot.display !== AbstractButton.IconOnly && text !== ""

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: controlRoot.mirrored ? parent.left : (image.visible ? image.right : parent.left)
            anchors.leftMargin: controlRoot.mirrored ? undefined : labelLeftSpace//(image.visible && image.status !== Image.Null) ? 4 : labelLeftSpace
            anchors.right: controlRoot.mirrored ? (image.visible ? image.left : parent.right) : parent.right
            anchors.rightMargin: controlRoot.mirrored ? labelLeftSpace : undefined
        }

    }

    indicator: Image {
        id:indicatorImage
        anchors.left: controlRoot.mirrored ?controlRoot.right : controlRoot.left
        anchors.leftMargin: controlRoot.mirrored ? - controlRoot.rightPadding -  menuItem.imageWidth : controlRoot.leftPadding

        anchors.verticalCenter: controlRoot.verticalCenter

        visible: controlRoot.checked
        source: indicatorImageSource()
        sourceSize.width: menuItem.imageWidth
        sourceSize.height: menuItem.imageWidth
        cache: false
    }

    arrow: Image {
        id: arrowImage
        anchors.left: controlRoot.mirrored ?controlRoot.left : controlRoot.right
        anchors.leftMargin: controlRoot.mirrored ? controlRoot.leftPadding : - controlRoot.rightPadding -  menuItem.imageWidth
        anchors.verticalCenter: parent.verticalCenter
        sourceSize.width: menuItem.imageWidth
        sourceSize.height: menuItem.imageWidth

        visible: controlRoot.subMenu
        mirror: controlRoot.mirrored
        source: arrowImageSource()
        cache: false
    }

    background:
        Rectangle {
        id: backrect
        width: controlRoot.width
        height: controlRoot.height
        radius: menuItem.radius
        //opacity: (isHightModel || isKeyBorderSelect || controlRoot.pressed || controlRoot.checked) ? 1 : 0
        border.width: controlRoot.focus ? 2 :  menuItem.borderWidth
        border.color: getStartColor(isHightModel ? borderHColor : borderColor)
        color: gradientRec.visible ? "transparent" : getStartColor(isHightModel ? backHColor : backColor)

//        property var borderWidth: backrect.border.width

//        onBorderWidthChanged: console.log("border changedddd", border.width, border.color, controlRoot.text)

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            gradient:  Gradient {
                id:gradient
                GradientStop { position: 0.0; color: getStartColor(isHightModel ? backHColor : backColor) }

                GradientStop { position: 1.0; color: getEndColor(isHightModel ? backHColor : backColor)}
            }
            visible: !parseInterface.isSolidPattern(isHightModel ? backHColor : backColor)
        }
    }

    onPressedChanged: {
        focusEnable = false;
        stateChange()
    }
    onHoveredChanged: {
        stateChange()
    }
    onFocusChanged: {
        if(controlFocusReason())
            controlRoot.focusEnable = true;
        else
            controlRoot.focusEnable = false;
        stateChange()
    }
    onFocusReasonChanged:{
        if(controlFocusReason())
            controlRoot.focusEnable = true;
        else
            controlRoot.focusEnable = false;
        stateChange()
    }

    Component.onCompleted: {
        itemLeftSpace = (!(itemHasChecked)) ? controlRoot.leftPadding : menuItem.imageWidth + 8 + controlRoot.leftPadding
        stateChange()
        image.source = contentImageSource();
        indicatorImage.source = indicatorImageSource()
        arrowImage.source = arrowImageSource()
    }
    function stateChange()
    {
        bacDtColor = !controlRoot.enabled ? menuItem.disableBC :
                                                            controlRoot.pressed ? menuItem.clickedBC :
                                                                                    isKeyBorderSelect ? menuItem.hoveredBC : menuItem.normalBC

    backrect.border.color = getStartColor(!controlRoot.enabled ? menuItem.disableBorderColor : (controlFocusReason() && controlRoot.focus && focusEnable) ? app.focusBorderColor :
                                                  controlRoot.pressed ? menuItem.clickBorderColor :
                                                  isKeyBorderSelect ? menuItem.hoverBorderColor : menuItem.normalBorderColor);
    controlRoot.update()
    }
    property variant fontMetrics: TextMetrics {
        text: "M"
        function roundedIconSize(size) {
            if (size < 16) {
                return size;
            } else if (size < 22) {
                return 16;
            } else if (size < 32) {
                return 22;
            } else if (size < 48) {
                return 32;
            } else if (size < 64) {
                return 48;
            } else {
                return size;
            }
        }
    }

    StylePrivate.UKUIMenuItem{
        id: menuItem
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

    onEnabledChanged: {
        iconUpdate();
    }

    function iconUpdate(){
        image.source = ""
        image.source = contentImageSource()
        indicatorImage.source = ""
        indicatorImage.source = indicatorImageSource()
        arrowImage.source = ""
        arrowImage.source = arrowImageSource()
        controlRoot.update();
    }

    function contentImageSource()
    {
        if((controlRoot.icon.source.toString() === "" && controlRoot.icon.name.toString() === ""))
            return controlRoot.icon.source

        //var model = !controlRoot.enabled ? "disenable" : (appP.isDark || isHightModel) ? "highlight" : /*controlRoot.pressed ? "highlight" : isKeyBorderSelect ? "highlight" :*/ "normal"
        var model = "normal";
        if(!controlRoot.enabled){
            if(appP.isDark || isHightModel)
                model = "highDisbale"
            else
                model = "disenable"
        }
        else if(appP.isDark || isHightModel){
            model = "highlight"
        }

        let s = controlRoot.icon.source.toString()
        if(s === "")
            s = controlRoot.icon.name.toString()
        return "image://imageProvider/" + s + "/" + model;
    }

    function indicatorImageSource(){
        //            return "image://imageProvider/file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/object-select-symbolic.svg/normal"

        var model = "normal"
        if(!controlRoot.enabled){
            if(appP.isDark || isHightModel)
                model = "highDisbale"
            else
                model = "disenable"
        }
        else if(appP.isDark || isHightModel){
            model = "highlight"
        }
        var icon = "object-select-symbolic";
        if(controlRoot.checkable){
            if(!app.themeHasIcon(icon))
                icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/object-select-symbolic.svg"
            return "image://imageProvider/" + icon + "/" + model
        }
        else
            return ""
    }

    function arrowImageSource(){
        var model = "normal"
        if(!controlRoot.enabled){
            if(appP.isDark || isHightModel)
                model = "highDisbale"
            else
                model = "disenable"
        }
        else if(appP.isDark || isHightModel){
            model = "highlight"
        }
        var icon = "ukui-end-symbolic";
        if(!app.themeHasIcon(icon))
            icon = "file:///usr/share/icons/ukui-icon-theme-default/scalable/actions/ukui-end-symbolic.svg"
        return "image://imageProvider/" + icon + "/" + model
    }
    function controlFocusReason(){
        return app.focusEnable && (controlRoot.focusReason === Qt.TabFocusReason || controlRoot.focusReason === Qt.BacktabFocusReason)
    }
}





