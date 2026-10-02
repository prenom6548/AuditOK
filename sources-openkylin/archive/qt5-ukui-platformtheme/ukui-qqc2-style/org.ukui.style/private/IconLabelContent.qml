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
    property int labelSpace: 0

//    property var labelColor: controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor

    property bool imageVisible : image.visible || image1.visible
    property var textColor: labelColor()
    property var space: {
        if(controlRoot.display === AbstractButton.TextOnly || controlRoot.display === AbstractButton.IconOnly)
            return 0
        return (label.text != "" && imageVisible) ? btn.space : 0
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
        if(controlRoot.display !== AbstractButton.TextUnderIcon){
            var textwidth = (label.text !== "" && controlRoot.display !== AbstractButton.IconOnly) ? label.implicitWidth : 0
            var imagewidth = (!imageVisible) ? 0 : image.width
            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
                return imagewidth + margin;
            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textwidth + margin, imagewidth + margin)
            return textwidth + imagewidth + space + margin + leftSpace
        }
        var textwidth1 = label1.text != "" ? label1.implicitWidth : 0
        var imagewidth1 = imageVisible ? 0 : image1.implicitWidth

        return textwidth1 + imagewidth1 + space + margin

    }

    function adjustHeight(margin){//margin表示边距
        if(controlRoot.display !== AbstractButton.TextUnderIcon) {
            var textheight = label1.text != "" ? label1.height : 0
            var imageheight = !imageVisible ? 0 : image1.height
            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textheight + margin, imageheight + margin)
            return textheight + space + margin, imageheight + space + margin
        }
        var textheight1 = label1.text != "" ? label1.height : 0
        var imageheight1 = !imageVisible ? 0 : image1.height

        var h = textheight1 + imageheight1 + space + margin
//        console.log("heigh0000000000....", controlRoot.text, textheight1, imageheight1, h, space, margin)

        return h
    }

    function labelColor(){
            var closeBtn = false;
            var windowBtn = false;
            var importBtn = false;
            if(controlRoot.hasOwnProperty("buttonType")){
                closeBtn = (controlRoot.buttonType === "closeButton");
                windowBtn = (controlRoot.buttonType === "windowButton");
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
            else if(importBtn){
                if(controlRoot.flat)
                    return (controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor)
                else
                    return (controlRoot.enabled ? btn.highlightTextColor : btn.highlightTextDisableColor)

            }
            else{
                return (controlRoot.enabled ? btn.normalTextColor : btn.disableTextColor)
            }

    }

    function iconSource()
    {
        return parseInterface.iconSource(controlRoot)
    }
    property var iconWidth: 16
    property var iconHeight: 16


    StylePrivate.UKUIButton{
        id: btn

    }
    StylePrivate.APPParameter{
        id: appP
        onFontChanged:{
//            controlRoot.width = adjustWidth(2 * btn.leftRightMargin)
//            controlRoot.height = adjustHeight(2 * btn.upDownMargin)
        }

        onIconThemeChanged: {
            iconUpdate()
        }

        onParametryChanged: {
            iconUpdate()
        }
    }
    function iconUpdate()
    {
        var source0 = iconSource()
        image.source = ""
        image.source = source0
        image1.source = ""
        image1.source = source0
    }
//    anchors.centerIn: controlRoot

    Item{
        id:item
        visible: controlRoot.display !== AbstractButton.TextUnderIcon
//        implicitWidth: {
//            var textwidth = (controlRoot.display !== AbstractButton.IconOnly && label.text !== "") ? label.width : 0
//            var imagewidth = imageVisible ? image.width : 0
//            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
//                return imagewidth;

//            if(controlRoot.display !== AbstractButton.TextBesideIcon)
//                return Math.max(textwidth, imagewidth)
//            return textwidth + imagewidth + space
//        }

        implicitHeight: {
            var textheight = label.visible ? (label.text != "" ? label.height : 0) : 0
            var imageheight = imageVisible ? image.height : 0
            if(controlRoot.display === AbstractButton.IconOnly || label.text === "")
                return imageheight;

            if(controlRoot.display === AbstractButton.TextUnderIcon)
            return Math.max(textheight + imageheight + space, normalHeight);
                return Math.max(textheight, imageheight, normalHeight)

        }
        anchors.left: parent.left
        anchors.leftMargin: {

              return leftSpace;

        }
        anchors.right: parent.right
        anchors.rightMargin: leftSpace
        anchors.verticalCenter: parent.verticalCenter

        Image {
            id: image
            source: iconSource()//controlRoot.icon.source
            anchors.verticalCenter: parent.verticalCenter
            sourceSize.width: (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconWidth : 0
            sourceSize.height: (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconHeight : 0
            visible: controlRoot.display !== AbstractButton.TextOnly && source !== "" && status != Image.Null
            cache: false
            anchors.left: controlRoot.mirrored ? undefined : parent.left
            anchors.right: controlRoot.mirrored ? parent.right : undefined

        }
        Text {
            id:label
            text: controlRoot.text
            font: controlRoot.font
            color: getStartColor(textColor)
            visible: controlRoot.display !== AbstractButton.IconOnly && text !== ""
            elide: Text.ElideRight

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: controlRoot.mirrored ? undefined : image.right
            anchors.leftMargin: controlRoot.mirrored ? undefined : ((image.visible && image.status !== Image.Null) ? labelSpace : 0)
            anchors.right: controlRoot.mirrored ? image.left : parent.right
//            anchors.rightMargin: controlRoot.mirrored ? ((image.visible && image.status !== Image.Null) ? labelSpace : 0) : 0
        }

    }
    Item{
        id:item1
        visible: controlRoot.display === AbstractButton.TextUnderIcon
        implicitWidth: {
            var textwidth1 = label1.text != "" ? label1.width : 0
            var imagewidth1 = controlRoot.icon.source === "" ? 0 : image1.width
            if(controlRoot.display === AbstractButton.TextUnderIcon)
            return Math.max(textwidth1, imagewidth1);
            if(controlRoot.display !== AbstractButton.TextBesideIcon)
                return Math.max(textwidth1, imagewidth1)
            return textwidth1 + imagewidth1 + space
        }
        implicitHeight: {
            var textheight1 = label1.text != "" ? label1.height : 0
            var imageheight1 = controlRoot.icon.source === "" ? 0 : image1.height
            if(display === AbstractButton.TextUnderIcon)
            return textheight1 + imageheight1 + space
                return Math.max(textheight1, imageheight1);

        }
        anchors.centerIn: parent
        Image {
            id: image1
            source: image.source//controlRoot.icon.source
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.width: (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconWidth : 0
            sourceSize.height: (controlRoot.display !== AbstractButton.TextOnly && source !== "") ? iconHeight : 0
            visible: controlRoot.display !== AbstractButton.TextOnly && source !== "" /*&& status != Image.Null*/
            cache: false
        }
        Text {
            id:label1
            text: controlRoot.text
            font:controlRoot.font
            color: getStartColor(textColor)
            visible: controlRoot.display !== AbstractButton.IconOnly && text !== ""

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: image1.bottom
            anchors.topMargin: space
        }

    }
    Component.onCompleted: {
        if(controlRoot.icon.width > 0 )
            iconWidth = controlRoot.icon.width
        if(controlRoot.icon.height > 0 )
            iconHeight = controlRoot.icon.height

        implicitWidth = adjustWidth(0);
        implicitHeight = adjustHeight(0);
    }

    onVisibleChanged: {
        implicitWidth = adjustWidth(0);
        implicitHeight = adjustHeight(0);
    }
    Connections {
        target: controlRoot
        // function onEnabledChanged(){
        //     image.source = iconSource()
        //     image1.source = iconSource()
        // }
    }

}
