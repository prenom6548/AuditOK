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
import QtQuick.Templates 2.5 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Popup {
    id: control

    palette: app.palette
    font: app.font

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentWidth > 0 ? contentWidth + leftPadding + rightPadding : 0)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentWidth > 0 ? contentHeight + topPadding + bottomPadding : 0)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    padding: popup.padding
    clip: false
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
//    StylePrivate.APPParameter{
//        id: appP
//    }
//    StylePrivate.UKUIPopup{
//        id: popup
//    }
//    ParseInterface{
//        id: parseInterface
//    }

//    function getStartColor(dtcolor)
//    {
//        return parseInterface.pStartColor(dtcolor)
//    }
//    function getEndColor(dtcolor)
//    {
//        return parseInterface.pEndColor(dtcolor)
//    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            easing.type: Easing.InOutQuad
            duration: 250
        }
    }

    contentItem: Item { }

    background: Rectangle {
        radius: popup.radius
        color: gradientRec.visible ? "transparent" : StylePrivate.ParseColorInterface.startColor(isTransaprent ? popup.transparentBC : popup.backColor)
        border.color: StylePrivate.ParseColorInterface.startColor(isTransaprent ? popup.transparentBorderColor : popup.backBorderColor)
        layer.enabled: true

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            gradient:  Gradient {
                id:gradient
                GradientStop { position: 0.0; color: StylePrivate.ParseColorInterface.startColor(isTransaprent ? popup.transparentBC : popup.backColor) }

                GradientStop { position: 1.0; color: StylePrivate.ParseColorInterface.endColor(isTransaprent ? popup.transparentBC : popup.backColor)}
            }
            visible: !StylePrivate.ParseColorInterface.isSolidPattern(isTransaprent ? popup.transparentBC : popup.backColor)
        }


        layer.effect: DropShadow {
            transparentBorder: true
            radius: popup.radius
            samples: 16
            horizontalOffset: 0
            verticalOffset: 0
            width: background.width + 8
            height: background.height + 8
            color: StylePrivate.ParseColorInterface.startColor(popup.shadowColor)
        }
    }

    StylePrivate.UKUIPopup{
        id: popup
    }

    StylePrivate.APPParameter{
        id: app
    }
}
