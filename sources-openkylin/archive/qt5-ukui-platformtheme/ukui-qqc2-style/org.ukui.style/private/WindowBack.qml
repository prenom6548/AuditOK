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

    Window {
        id: popupwindow
        width: windowLabel.implicitWidth + 2 * tooltip.padding
        height: windowLabel.implicitHeight + 2 * tooltip.padding
        visible: controlRoot.visible
        color: "transparent"
        property T.ToolTip controlRoot
        property alias handle: popHandle
        //            margins: tooltip.margins
        //padding: tooltip.padding
//        flags: Qt.NoFocus

        ParseInterface{
            id: parseInterface
        }
        StylePrivate.APPParameter{
            id: appP
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

        onVisibleChanged: {
            if(visible){
            var p = popHandle.posByCursor(popupwindow.width, popupwindow.height)
            if(popupwindow.x !== p.x)
                popupwindow.x = p.x
            if(popupwindow.y !== p.y)
                popupwindow.y = p.y
            }
        }

        StylePrivate.PopupHandle{
            id: popHandle
            blur: true
            titlebarVisible: false
            anchors.fill: parent
            posFollwMouse: true
            visible: parent.visible
            radius: tooltip.radius
            windowStaysOnTopHint: true
        }

        Rectangle{
            id: rec
            visible: parent.visible
            anchors.fill: parent
            // radius: tooltip.radius
            color: {
                var baccolor = getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor)
                return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, (controlRoot ? controlRoot._opacity: 1.0))
            }
            // border.color: getStartColor(isTransaprent ? tooltip.backBorderTransparentColor : tooltip.backBorderColor)
            // border.width: 1

        }
        Controls.Label {
            id: windowLabel
            anchors.fill: rec
            anchors.leftMargin: tooltip.padding
            anchors.rightMargin: tooltip.padding
            anchors.topMargin: tooltip.padding
            anchors.bottomMargin: tooltip.padding


            anchors.centerIn: rec
            visible: rec.visible
            text: controlRoot ? controlRoot.text : ""
            wrapMode: controlRoot ? controlRoot.mode : Text.NoWrap
            font: controlRoot ? controlRoot.font : appP.font
            color: getStartColor(tooltip.textColor)
            textFormat: controlRoot ? controlRoot._textFormat : Qt.PlainText

            onTextChanged: {
                if(controlRoot)
                {
                    controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                    if(controlRoot._maxWidth != 0)
                        if((windowLabel.implicitWidth +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                            popupwindow.width = windowLabel.implicitWidth +  + 2 * tooltip.padding
                        else
                            popupwindow.width  = controlRoot._maxWidth
                    //                console.log("textchangeddd 111", popupwindow.width )

                    controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                }
            }

            onContentWidthChanged: {
                if(controlRoot){
                    controlRoot.implicitWidth = Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1)
                    controlRoot.implicitHeight = Math.max(windowLabel.implicitHeight+ 2 * tooltip.padding, 1)
                    if(controlRoot._maxWidth != 0 && (windowLabel.implicitWidth +  + 2 * tooltip.padding) < controlRoot._maxWidth){
                        popupwindow.width = windowLabel.implicitWidth +  + 2 * tooltip.padding
                    }
                }

            }
            onContentHeightChanged: {

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
    }


