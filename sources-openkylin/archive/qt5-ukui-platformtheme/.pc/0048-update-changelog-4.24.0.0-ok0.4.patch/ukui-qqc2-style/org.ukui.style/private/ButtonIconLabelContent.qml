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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item{
    id:root
    property T.AbstractButton controlRoot: root.parent

    property int normalWidth: btn.normalWidth
    property int normalHeight: btn.normalHeight

    property string layout: "center"
    property int leftSpace: 0
    property int labelSpace: 4

//    property var labelColor: controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor

    property bool imageVisible : image.visible || image1.visible
    property var space: {
        if(controlRoot.display === AbstractButton.TextOnly || controlRoot.display === AbstractButton.IconOnly)
            return 0
        return (label.text != "" && imageVisible) ? btn.space : 0
    }

    property var iconWidth: controlRoot.icon.width > 0 ? controlRoot.icon.width : 16
    property var iconHeight: controlRoot.icon.height > 0 ? controlRoot.icon.height : 16

    property var menuRightSpace: 0
    property var menuLeftSpace: label.visible ? 4 : 10

    onIconHeightChanged: {
        if(controlRoot.display !== AbstractButton.TextOnly && (controlRoot.icon.source.toString() !== "" || controlRoot.icon.name.toString() !== ""))
            image.sourceSize.height = iconHeight
    }
    onIconWidthChanged: {
        if(controlRoot.display !== AbstractButton.TextOnly && (controlRoot.icon.source.toString() !== "" || controlRoot.icon.name.toString() !== ""))
            image.sourceSize.width = iconWidth
    }

    onImageVisibleChanged: {
        adjust()
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

    function adjustWidth(margin){//margin表示边距
        if(controlRoot.hasOwnProperty("maxWidth") && controlRoot._maxWidth > 0){

            return controlRoot._maxWidth - 2 * controlRoot.leftPadding + margin ;
        }
        var menuImageWidth = menuImage.visible ? (menuImage.sourceSize.width + menuRightSpace + menuLeftSpace) : 0

        if(controlRoot.display !== AbstractButton.TextUnderIcon){
            var textwidth = (label.text !== "" && controlRoot.display !== AbstractButton.IconOnly) ? label.width : 0
            var imagewidth = (!imageVisible) ? 0 : image.sourceSize.width
            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
                return imagewidth + margin + menuImageWidth;
            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textwidth + margin + menuImageWidth, imagewidth + margin + menuImageWidth, normalWidth)
            return Math.max(normalWidth, textwidth + imagewidth + space + margin + menuImageWidth)
        }
        var textwidth1 = label1.text != "" ? label1.width : 0
        var imagewidth1 = imageVisible ? 0 : image1.sourceSize.width

        return Math.max(normalWidth, textwidth1 + imagewidth1 + space + margin +　menuImageWidth)

    }

    function adjustHeight(margin){//margin表示边距
        if(controlRoot.display !== AbstractButton.TextUnderIcon) {
            var textheight = label1.text != "" ? label1.height : 0
            var imageheight = !imageVisible ? 0 : image1.height

            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textheight + margin, imageheight + margin, normalHeight)
            return Math.max(normalHeight, textheight + space + margin, imageheight + space + margin)
        }
        var textheight1 = label1.text != "" ? label1.height : 0
        var imageheight1 = !imageVisible ? 0 : image1.height

        var h = Math.max(normalHeight, textheight1 + imageheight1 + space + margin)
        return h
    }

    function labelColor(){
        if(controlRoot.hasOwnProperty("disableLabelColor") && !controlRoot.enabled)
            return controlRoot.disableLabelColor
        else if(controlRoot.hasOwnProperty("clickedLabelColor") && controlRoot.enabled)
            return controlRoot.clickedLabelColor
        else if(controlRoot.hasOwnProperty("hoverLabelColor") && controlRoot.enabled)
            return controlRoot.hoverLabelColor
        else if(controlRoot.hasOwnProperty("normalLabelColor") && controlRoot.enabled)
            return controlRoot.normalLabelColor

            var closeBtn = false;
            var windowBtn = false;
            var importBtn = false;
            if(controlRoot.hasOwnProperty("buttonType")){
                closeBtn = (controlRoot.buttonType === "closeButton" || controlRoot.buttonType === "windowTitleBarCloseButton");
                windowBtn = (controlRoot.buttonType === "windowButton" || controlRoot.buttonType === "windowTitleBarWindowButton");
                importBtn = (controlRoot.buttonType === "importButton");
            }

            if(windowBtn){
//                    if(controlRoot.pressed)
//                        labelColor = appP.highlightedTextColor
//                    else if(controlRoot.hovered)
//                        labelColor = appP.brightTextColor
//                    else
                    return (controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor)

            }
            else if(closeBtn &&(controlRoot.pressed || controlRoot.hovered)){
                return btn.highlightTextColor;
            }
            else if(importBtn || controlRoot.highlighted){
                if(controlRoot.flat)
                    return (controlRoot.enabled ? (controlRoot.pressed || controlRoot.hovered) ? btn.highlightTextColor : btn.normalTextColor : btn.disableTextColor)
                else
                    return (controlRoot.enabled ? btn.highlightTextColor : btn.highlightTextDisableColor)

            }
            else{
                return (controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor)
            }

    }

    function adjust()
    {
        root.implicitWidth = adjustWidth(0);
        root.implicitHeight = adjustHeight(0);
        controlRoot.implicitWidth = adjustWidth(2 * btn.leftRightMargin)
        controlRoot.implicitHeight = adjustHeight(2 * btn.upDownMargin)
    }


    StylePrivate.UKUIButton{
        id: btn

    }
    StylePrivate.APPParameter{
        id: appP
        onFontChanged:{
            adjust()
        }
        onIconThemeChanged: {
            iconUpdate();
        }
        onParametryChanged: {
            iconUpdate();
        }
    }
    function iconUpdate(){
        image.source = ""
        menuImage.source = ""
        updateImage();
    }
    anchors.centerIn: controlRoot

    Item{
        id:item
        visible: controlRoot.display !== AbstractButton.TextUnderIcon
        implicitWidth: {

            if(controlRoot.hasOwnProperty("maxWidth") && controlRoot._maxWidth > 0){

                return controlRoot._maxWidth - controlRoot.leftPadding * 2;
            }

            var menuImageWidth = menuImage.visible ? (menuImage.sourceSize.width + menuRightSpace + menuLeftSpace) : 0

            var textwidth = (controlRoot.display !== AbstractButton.IconOnly && label.text !== "") ? label.width : 0
            var imagewidth = imageVisible ? image.sourceSize.width : 0
            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
                return imagewidth + menuImageWidth;

            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textwidth, imagewidth) + menuImageWidth;
            return Math.max( textwidth + imagewidth + space) + menuImageWidth;
        }

        implicitHeight: {
            var textheight = label.visible ? (label.text != "" ? label.height : 0) : 0
            var imageheight = imageVisible ? image.height : 0
            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
                return imageheight;

            if(controlRoot.display === AbstractButton.TextUnderIcon)
            return Math.max(textheight + imageheight + space, normalHeight);
                return Math.max(textheight, imageheight, normalHeight)

        }
        anchors.left: layout == "left" ? parent.left : undefined
        anchors.leftMargin: layout == "left" ? leftSpace : undefined
        anchors.horizontalCenter: layout !== "left" ? parent.horizontalCenter : undefined
        anchors.verticalCenter: parent.verticalCenter

//        StylePrivate.KyIcon{
//            id: image
//            iconName: parseInterface.kyIconSource(controlRoot)
//            visible: controlRoot.display !== AbstractButton.TextOnly && iconName !== ""
//            width: (controlRoot.display !== AbstractButton.TextOnly && iconName !== "") ? iconWidth : 0
//            height: (controlRoot.display !== AbstractButton.TextOnly && iconName !== "") ? iconHeight : 0
//            anchors.verticalCenter: parent.verticalCenter

//            Rectangle{
//            anchors.fill: parent
//            color: "#30000000"
//            }
//        }

        Image {
            id: image
            source: parseInterface.iconSource(controlRoot)//controlRoot.icon.source
            anchors.verticalCenter: parent.verticalCenter

            //Image 默认没有定义 anchors.left，anchors.right

            anchors.left: controlRoot.mirrored ? undefined : parent.left 
            anchors.right: controlRoot.mirrored ? parent.right : undefined

            anchors.leftMargin: controlRoot.mirrored ? ((image.visible) ? labelSpace : 0) : undefined
            anchors.rightMargin: controlRoot.mirrored ? undefined : ((image.visible) ? labelSpace : 0)

            sourceSize.width: (controlRoot.display !== AbstractButton.TextOnly && (controlRoot.icon.source.toString() !== "" || controlRoot.icon.name.toString() !== "")) ? iconWidth : 0
            sourceSize.height: (controlRoot.display !== AbstractButton.TextOnly && (controlRoot.icon.source.toString() !== "" || controlRoot.icon.name.toString() !== "")) ? iconHeight : 0
            visible: controlRoot.display !== AbstractButton.TextOnly && source !== "" && status != Image.Null
            fillMode: Image.PreserveAspectFit
            cache: false
            onSourceChanged: {
                sourceSize.width = (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconWidth : 0
                sourceSize.height = (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconHeight : 0

                adjust()
            }
            onSourceSizeChanged: {
                updateImage()
            }
            Component.onCompleted: {
                if(source.toString() == "" && (controlRoot.icon.source.toString() !== "" || controlRoot.icon.name.toString() !== ""))
                    source = parseInterface.iconSource(controlRoot)
                imageVisible = image.visible || image1.visible
            }
        }
        Text {
            id:label
            text: controlRoot.text
            font: controlRoot.font
            color: getStartColor(labelColor())
            visible: controlRoot.display !== AbstractButton.IconOnly && text !== ""

            anchors.verticalCenter: parent.verticalCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            elide: (controlRoot.hasOwnProperty("maxWidth") && controlRoot._maxWidth > 0) ? Text.ElideRight : Text.ElideNone

            // 原始默认代码
            //anchors.left: image.right
            //anchors.leftMargin: (image.visible) ? labelSpace : 0

//            anchors.left: controlRoot.mirrored ? parent.left : undefined
//            anchors.right: controlRoot.mirrored ? undefined : parent.right

//            anchors.leftMargin: controlRoot.mirrored ? undefined : ((image.visible) ? labelSpace : 0)
//            anchors.rightMargin: controlRoot.mirrored ? ((image.visible) ? labelSpace : 0) : undefined



            anchors.left: controlRoot.mirrored ? (controlRoot.hasOwnProperty("maxWidth") && controlRoot._maxWidth > 0) ? parent.left :undefined : image.right
            anchors.right: controlRoot.mirrored ? image.left : (controlRoot.hasOwnProperty("maxWidth") && controlRoot._maxWidth > 0) ? parent.right :undefined

            anchors.leftMargin: controlRoot.mirrored ? undefined : ((image.visible) ? labelSpace : 0)
            anchors.rightMargin: controlRoot.mirrored ? ((image.visible) ? labelSpace : 0) : undefined



            onTextChanged: {
                adjust()
            }
            onFontChanged: adjust()
        }

        Image {
            id: menuImage
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: controlRoot.mirrored ? undefined : (label.text != "" ? label.right : image.right)
            anchors.leftMargin: controlRoot.mirrored ? undefined : menuLeftSpace
            anchors.right: controlRoot.mirrored ? (label.text != "" ? label.left : image.left) : undefined
            anchors.rightMargin: controlRoot.mirrored ? menuLeftSpace : undefined

            sourceSize.width: iconWidth
            sourceSize.height: iconHeight
            fillMode: Image.PreserveAspectFit
            cache: false
            source: controlRoot._menuToolButton ? parseInterface.buttonIcon(controlRoot, "ukui-down-symbolic") : ""
            Component.onCompleted: {
                if(controlRoot._menuToolButton)
                    source = parseInterface.buttonIcon(controlRoot, "ukui-down-symbolic")
            }
            visible: {
                if(controlRoot.hasOwnProperty("_menuToolButton"))
                    return controlRoot._menuToolButton
                else
                    return false
            }
        }

    }
    Item{
        id:item1
        visible: controlRoot.display === AbstractButton.TextUnderIcon
        implicitWidth: {
            var menuImageWidth = menuImage1.visible ? (menuImage1.sourceSize.width + menuRightSpace + menuLeftSpace) : 0

            var textwidth1 = label1.text != "" ? label1.width : 0
            var imagewidth1 = controlRoot.icon.source === "" ? 0 : image1.sourceSize.width
            if(controlRoot.display === AbstractButton.TextUnderIcon)
            return Math.max(textwidth1, imagewidth1, normalWidth) + menuImageWidth;
            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textwidth1, imagewidth1, normalWidth) + menuImageWidth
            return Math.max( textwidth1 + imagewidth1 + space, normalWidth) + menuImageWidth
        }
        implicitHeight: {
            var textheight1 = label1.text != "" ? label1.height : 0
            var imageheight1 = controlRoot.icon.source === "" ? 0 : image1.height
            if(display === AbstractButton.TextUnderIcon)
            return Math.max(textheight1 + imageheight1 + space, normalHeight);
                return Math.max(textheight1, imageheight1, normalHeight)

        }
        anchors.centerIn: parent
        Image {
            id: image1
            source: image.source//controlRoot.icon.source
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.width: image.sourceSize.width
            sourceSize.height: image.sourceSize.height
            visible: controlRoot.display !== AbstractButton.TextOnly && source !== "" /*&& status != Image.Null*/
            onSourceChanged: {
                sourceSize.width = (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconWidth : 0
                sourceSize.height = (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconHeight : 0

                adjust()
            }
            fillMode: Image.PreserveAspectFit
            cache: false
            Component.onCompleted: {
                imageVisible = image.visible || image1.visible
            }
        }
        Text {
            id:label1
            text: controlRoot.text
            font:controlRoot.font
            color: label.color
            visible: controlRoot.display !== AbstractButton.IconOnly && text !== ""

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: image1.bottom
            anchors.topMargin: space

            onTextChanged: {
                adjust()
            }
            onFontChanged: adjust()
        }

        Image {
            id: menuImage1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: menuRightSpace
            anchors.left: label1.right
            anchors.leftMargin: menuLeftSpace


            sourceSize.width: iconWidth
            sourceSize.height: iconHeight
            fillMode: Image.PreserveAspectFit
            cache: false
            source: menuImage.source
            visible: {
                if(controlRoot.hasOwnProperty("_menuToolButton"))
                    return controlRoot._menuToolButton
                else
                    return false
            }
        }

    }



    Component.onCompleted: {
//        console.log("iconwidth:", iconWidth, iconHeight, controlRoot.icon.width, controlRoot.icon.height)
        if(controlRoot.icon.width > 0 && controlRoot.icon.width !== iconWidth){
            iconWidth = controlRoot.icon.width
        }
        if(controlRoot.icon.height > 0 && controlRoot.icon.height !== iconHeight){
            iconHeight = controlRoot.icon.height
        }

        adjust()
        // console.log("===> Button: controlRoot.mirrored = ", controlRoot.mirrored)
    }

    onVisibleChanged: {
        if(controlRoot.icon.width > 0 && controlRoot.icon.width !== iconWidth){
            iconWidth = controlRoot.icon.width
        }
        if(controlRoot.icon.height > 0 && controlRoot.icon.height !== iconHeight){
            iconHeight = controlRoot.icon.height
        }

        if(parseInterface.iconSource(controlRoot) != "")
            imageVisible = image.visible || image1.visible

        adjust()
    }
    Connections {
        target: controlRoot
        function onClicked() {
            updateImage()
        }
        function onHoveredChanged () {
            updateImage()
        }
        function onIconChanged(){
            updateImage()
        }
    }

    function updateImage(){
        if(controlRoot){
        if(controlRoot.icon.width > 0 && controlRoot.icon.width !== iconWidth){
            iconWidth = controlRoot.icon.width
        }
        if(controlRoot.icon.height > 0 && controlRoot.icon.height !== iconHeight){
            iconHeight = controlRoot.icon.height
        }
        var source0 = parseInterface.iconSource(controlRoot)
        image.source = source0
        var source1 = parseInterface.buttonIcon(controlRoot, "ukui-down-symbolic")

        menuImage.source = controlRoot._menuToolButton ? source1 : ""
        if(parseInterface.iconSource(controlRoot) !== "")
            imageVisible = image.visible || image1.visible
        }
    }
}
