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
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.5 as Controls
import QtQuick.Templates 2.5 as T
import QtQuick.Window 2.15
import org.ukui.qqc2style.private 1.0 as StylePrivate

    Rectangle {
        id: popupwindow
        width: windowLabel.implicitWidth + 2 * tooltip.padding
        height: windowLabel.implicitHeight + 2 * tooltip.padding
        visible: true
        radius: tooltip.radius
        color: {
            // console.log("normalrec color:", getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor).r)
            return Qt.rgba(getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor).r,
                           getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor).g,
                           getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor).b,
                           1)
        }
        opacity: 1

        property T.ToolTip controlRoot
        //            margins: tooltip.margins
        //padding: tooltip.padding
//        flags: Qt.NoFocus
        Rectangle{
            anchors.fill: parent
            color: "transparent"
            border.color: getStartColor(isTransaprent ? tooltip.backBorderTransparentColor : tooltip.backBorderColor)
            border.width: 1
            radius: parent.radius
        }

        ParseInterface{
            id: parseInterface
        }
        StylePrivate.APPParameter{
            id: appP
            onParametryChanged: {
                // console.log("parametryyyyyyyy")
                // var c = getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor)
                // console.log("parametryyyyyyyy", c.r, c.g, c.b)
            }
        }

        StylePrivate.UKUIToolTip{
            id: tooltip
        }
        function getStartColor(dtcolor)
        {
            return parseInterface.pStartColor(dtcolor)
        }
        function getEndColor(dtcolor)
        {
            return parseInterface.pEndColor(dtcolor)
        }

        function setControlRoot(control){
            controlRoot = control
        }

        Connections{
            target: controlRoot
            function onVisibleChanged() {
                popupwindow.visible = controlRoot.visible
            }
            function onXChanged(){
                return popupwindow.x = Math.max(controlRoot.width - popupwindow.width, 0)
            }
        }
        Component.onCompleted: {
            // var c = getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor)
            // popupwindow.color = Qt.rgba(c.red, c.green, c.blue, 255)
            popupwindow.opacity = 1
            if(controlRoot){
                adjustSize()
            }
        }

        onVisibleChanged: {
            if(visible && controlRoot){
                adjustSize();
            }
        }

        // Rectangle{
        //     id: rec
        //     visible: parent.visible
        //     anchors.fill: parent
        //     radius: tooltip.radius
        //     color: getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor)
        //     border.color: getStartColor(isTransaprent ? tooltip.backBorderTransparentColor : tooltip.backBorderColor)
        //     opacity: 1
        //     border.width: 1

        // }
        TextMetrics{
            id: metrics
            font: windowLabel.font
            text: windowLabel.text
        }
        Controls.Label {
            id: windowLabel
            anchors.fill: parent
            anchors.leftMargin: tooltip.padding
            anchors.rightMargin: tooltip.padding
            anchors.topMargin: tooltip.padding
            anchors.bottomMargin: tooltip.padding

            anchors.centerIn: parent
            visible: parent.visible
            text: controlRoot ? controlRoot.text : ""
            wrapMode: controlRoot ? controlRoot.mode : Text.NoWrap
            font: controlRoot ? controlRoot.font : appP.font
            color: getStartColor(tooltip.textColor)
            textFormat: controlRoot ? controlRoot._textFormat : Qt.PlainText
            onTextChanged: {
                metrics.text = text

                controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                if(controlRoot._maxWidth != 0)
                    if((metrics.width +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                        popupwindow.width = metrics.width +  + 2 * tooltip.padding
                    else{
                        if(Window.width > 20)
                            popupwindow.width  = Math.min(controlRoot._maxWidth, Window.width - 20)
                        else
                            popupwindow.width  = controlRoot._maxWidth
                    }

                controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                adjustSize()
            }

            onWidthChanged: {
                adjustSize()
                if(controlRoot){
                    controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                    controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                    if(controlRoot._maxWidth != 0 && (metrics.width +  + 2 * tooltip.padding) < controlRoot._maxWidth){
                        popupwindow.width = metrics.width +  + 2 * tooltip.padding
                    }
                }
            }
            onHeightChanged: {
                adjustSize()
                if(controlRoot){
                    controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                    controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                }
            }
            Component.onCompleted: {
                if(controlRoot){
                    controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                    controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                }
            }
        }

        function setPosition() {
            var window = popupwindow.Window.window;
            if (!window) return;
            var contentItem = window.contentItem;
            var p = appP.posByCursor(popupwindow.width, popupwindow.height)

            var pointInContent = contentItem.mapFromGlobal(p.x, p.y);
            var pointInParent = popupwindow.parent.mapFromItem(contentItem, pointInContent.x, pointInContent.y);
            controlRoot.x = pointInParent.x;
            controlRoot.y = pointInParent.y;
        }
        function adjustSize(){
            if(visible && controlRoot){
               // var p = appP.posByCursor(popupwindow.width, popupwindow.height)
               // console.log("rec visible p", p.x, p.y)
               //  console.log("rec visible p", popupwindow.mapFromGlobal(p.x, p.y))
               //  console.log("rec visible p", windowLabel.mapFromGlobal(p.x, p.y))

               // p = popupwindow.mapFromGlobal(p.x, p.y)
               // console.log("rec visible p1", p.x, p.y)

               // controlRoot.x = p.x
               // controlRoot.y = p.y

                controlRoot.implicitWidth = popupwindow.width
                controlRoot.implicitHeight = popupwindow.height
                controlRoot.x = controlRoot.parent ? (parent.width - popupwindow.width)/2 : 0
                controlRoot.y = -popupwindow.height -3
                // console.log("rec visible controlRoot", controlRoot.parent, controlRoot.x, controlRoot.y)
                // var p = popupwindow.mapToGlobal(popupwindow.x, popupwindow.y)
                // var l = appP.posByCursor(popupwindow.width, popupwindow.height).x - (p.x)
                // console.log("lllllllllllllllllllllll", popupwindow.x, p.x, popupwindow.width, appP.posByCursor(popupwindow.width, popupwindow.height).x, l)
                // if(l > 10){
                //     popupwindow.x = l
                // }
                // setPosition()

            }


            let current = parent
            var window = Window
            while (current && current.parent) {
                current = current.parent
            }
            // 如果顶层对象是 Window 则返回
            if (current && current.hasOwnProperty("window")) {
                window = current.window
            }
            if(window.width > 0 && controlRoot.x + popupwindow.width > window.width){
                if(popupwindow.width > window.width &&  window.width > 20)
                    popupwindow.width = window.width - 20
                controlRoot.x = window.width - popupwindow.width
            }
        }
    }


