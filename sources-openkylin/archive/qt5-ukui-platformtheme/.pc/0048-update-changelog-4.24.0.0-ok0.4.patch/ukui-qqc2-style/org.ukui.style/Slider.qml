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

import QtQuick 2.14
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtGraphicalEffects 1.14
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Slider {
    id: control

    Accessible.role: Accessible.Slider
    Accessible.name: qsTr("Slider")

    implicitWidth: control.horizontal ? (implicitBackgroundWidth + leftInset + rightInset) :
                   Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             (touchSlider ? slider.touchHandleHeight : slider.handleHeight)  + topPadding + bottomPadding)

    padding: slider.padding

    hoverEnabled: true
    palette: app.palette
    stepSize: 0.1
    property var focusEnable: app.focusEnable
    property var focusBorderWidth: app.focusBorderWidth
    property var touchSlider : control.hasOwnProperty("sliderType") ? (control.sliderType === "touchSlider" ? true : false): false


//    property var handleColor: !control.enabled ? slider.disableHandleColor : handleClick ? slider.clickHandleColor :
//                               handleHover ?slider.hoverHandleColor : slider.normalHandleColor
//    property var handleBorderColor: !control.enabled ? slider.disableHandleBorderColor : handleClick ? slider.clickHandleBorderColor :
//                                    handleHover ? slider.hoverHandleBorderColor : slider.normalHandleBorderColor

//    property bool handleClick: control.handle.pressed
//    property bool handleHover: control.handle.hovered
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

    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }

    //opacity: control.enabled ? 1 : 0.45

    handle: Item{
        x: {
            if (control.horizontal) {
                return control.leftPadding + control.visualPosition * (control.availableWidth - 2 *　focusBorderWidth - width) + focusBorderWidth
            } else {
                return (control.availableWidth - width) / 2
            }
        }
        y: touchSlider ? (control.topPadding + (control.horizontal ?
                    (control.availableHeight - height) / 2 : (control.visualPosition * (control.availableHeight - 2 *　focusBorderWidth - 15) + focusBorderWidth))) :
                    (control.topPadding + (control.horizontal ?
                     (control.availableHeight - height) / 2 : (control.visualPosition * (control.availableHeight - 2 *　focusBorderWidth - height) + focusBorderWidth)))
        // implicitWidth: touchSlider ? (control.horizontal ? (15 + control.visualPosition * (control.availableWidth - /*slider.touchHandleWidth*/15) < slider.touchHandleWidth ?
        //                (15 + control.visualPosition * (control.availableWidth - /*slider.touchHandleWidth*/15))　:　slider.touchHandleWidth) : slider.touchHandleHeight) :
        //                              ((pressed || hovered) ? slider.handleHoverHeight : slider.handleHeight)
        // implicitHeight: touchSlider ? (control.horizontal ? slider.touchHandleHeight :
        //                 (15 + (1-control.visualPosition) * (control.availableHeight - 15) < slider.touchHandleWidth ?
        //                 (15 + (1-control.visualPosition) * (control.availableHeight - 15)) : slider.touchHandleWidth)) :
        //                               ((pressed || hovered) ? slider.handleHoverHeight : slider.handleHeight)
        property real normalWidth: touchSlider ?
            (control.horizontal ?
                    // RTL 时用 1 - visualPosition 反转尺寸变化方向
                    (15 + (control.mirrored ? (1 - control.visualPosition) : control.visualPosition) * (control.availableWidth - 2 *　focusBorderWidth - 15) > slider.touchHandleWidth ?
                     slider.touchHandleWidth : (15 + (control.mirrored ? (1 - control.visualPosition) : control.visualPosition) * (control.availableWidth - 2 *　focusBorderWidth - 15))) :
                    slider.touchHandleHeight) :
            slider.handleHeight
        property real normalHeight: touchSlider ?
            (control.horizontal ? slider.touchHandleHeight :
             (15 + (1-control.visualPosition) * (control.availableHeight - 2 *　focusBorderWidth - 15) < slider.touchHandleWidth ?
             (15 + (1-control.visualPosition) * (control.availableHeight - 2 *　focusBorderWidth - 15)) : slider.touchHandleWidth)) :
            slider.handleHeight

        implicitWidth: normalWidth
        implicitHeight: normalHeight
        property bool hovered: handleRec.hovered || mouseArea.containsMouse

        MouseArea {
            id: mouseArea
            Accessible.name: qsTr("Slider handle")
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onPressed: mouse.accepted = false
        }

        Rectangle{
        anchors.fill: handleRec
        visible: !touchSlider && !control.enabled
        color: getStartColor(slider.touchHandleBaseColor)
        radius: handleRec.radius
        }
        Rectangle {
        id: handleRec
        opacity: control.enabled ? 1 : 0.45
        anchors.fill: parent

        // x: touchSlider ? (control.leftPadding + (control.horizontal ?
        //              control.visualPosition * (control.availableWidth - slider.touchHandleWidth) : (control.availableWidth - width) / 2)) :
        //             (control.leftPadding + (control.horizontal ?
        //              control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2))
        // y: touchSlider ? (control.topPadding + (control.horizontal ?
        //             (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - 15))) :
        //             (control.topPadding + (control.horizontal ?
        //              (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height)))
        // implicitWidth: touchSlider ? (control.horizontal ? (15 + control.visualPosition * (control.availableWidth - /*slider.touchHandleWidth*/15) < slider.touchHandleWidth ?
        //                (15 + control.visualPosition * (control.availableWidth - /*slider.touchHandleWidth*/15))　:　slider.touchHandleWidth) : slider.touchHandleHeight) :
        //                              ((pressed || hovered) ? slider.handleHoverHeight : slider.handleHeight)
        // implicitHeight: touchSlider ? (control.horizontal ? slider.touchHandleHeight :
        //                 (15 + (1-control.visualPosition) * (control.availableHeight - 15) < slider.touchHandleWidth ?
        //                 (15 + (1-control.visualPosition) * (control.availableHeight - 15)) : slider.touchHandleWidth)) :
        //                               ((pressed || hovered) ? slider.handleHoverHeight : slider.handleHeight)
        radius: touchSlider ? 20 : width / 2
        color: touchSlider ? (gradientHandleRec.visible ? "transparent" : getStartColor(!control.enabled ? /*slider.disableGrooveColor*/"transparent" : slider.normalGrooveColor)) :
                             (gradientHandleRec.visible ? "transparent" : getStartColor(!control.enabled ? slider.disableHandleColor : pressed ? slider.clickHandleColor :
               hovered ? slider.hoverHandleColor : slider.normalHandleColor))
        border.width: touchSlider ? 0 : slider.handleBorderWidth
        border.color: getStartColor(!control.enabled ? slider.disableHandleBorderColor : pressed ? slider.clickHandleBorderColor :
                                    hovered ? slider.hoverHandleBorderColor : slider.normalHandleBorderColor)
        scale: 1.0
        states: [
            State {
                name: "HOVERED"
                when: !touchSlider && !control.pressed && control.hovered
                PropertyChanges { target: handleRec; scale: 1.125 }
            },
            State {
                name: "PRESSED"
                when: !touchSlider && control.pressed
                PropertyChanges {
                    target: handleRec
                    scale: 1.0
                    border.width: slider.handleBorderWidth * 1.25
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"; to: "HOVERED"
                reversible: true
                ParallelAnimation {
                    NumberAnimation { property: "scale"; duration: 100; easing.type: Easing.InOutSine }
                }
            },
            Transition {
                from: "*"; to: "PRESSED"
                reversible: true
                ParallelAnimation {
                    NumberAnimation { property: "scale"; duration: 100; easing.type: Easing.InOutSine }
                    NumberAnimation { property: "border.width"; duration: 100; easing.type: Easing.Linear }
                }
            }
        ]

        Rectangle{
            id: gradientHandleRec
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(!control.enabled ? slider.disableHandleColor : pressed ? slider.clickHandleColor :
                                                                   hovered ?slider.hoverHandleColor : slider.normalHandleColor) }

                GradientStop { position: 1.0; color: getEndColor(!control.enabled ? slider.disableHandleColor : pressed ? slider.clickHandleColor :
                                                                 hovered ?slider.hoverHandleColor : slider.normalHandleColor)}
            }
            visible: !parseInterface.isSolidPattern(!control.enabled ? slider.disableHandleColor : pressed ? slider.clickHandleColor :
                                                                                                             hovered ?slider.hoverHandleColor : slider.normalHandleColor)
        }

        Rectangle{
            anchors.left: control.mirrored ? parent.left : undefined
            anchors.leftMargin: control.mirrored ? 6 : undefined
            anchors.right: control.mirrored ? undefined : parent.right
            anchors.rightMargin: control.mirrored ? undefined : 6

            anchors.horizontalCenter: control.horizontal ? undefined : parent.horizontalCenter
            anchors.top: control.horizontal ? undefined : parent.top
            anchors.topMargin: control.horizontal ? undefined : 6
            anchors.verticalCenter: control.horizontal ? parent.verticalCenter : undefined

            visible: touchSlider
            width: control.horizontal ? 3 : 6
            height: control.horizontal ? 6 : 3
            color: slider.touchHandleColor
        }
    }
}

    background: Item{
        id: item
        implicitWidth: 2 * focusBorderWidth + (control.horizontal ? (slider.normalWidth /*+ (touchSlider ? 15 : 0)*/) : slider.normalHeight)
        implicitHeight: 2 * focusBorderWidth + (control.horizontal ? slider.normalHeight : (slider.normalWidth + (touchSlider ? 0 : 24)))

        Rectangle{
            id: focusRect
            width: control.horizontal ? parent.width: parent.width
            height:  control.horizontal ? parent.height : parent.height
            color: "transparent"
            anchors.centerIn: parent
            border.width: (controlFocusReason() && control.focus && focusEnable) ? app.focusBorderWidth : 0
            border.color: (controlFocusReason() && control.focus && focusEnable) ? getStartColor(app.focusBorderColor) : "transparent"
        }

        Rectangle {
            width: item.width - 2 * focusBorderWidth
            height: item.height - 2 * focusBorderWidth
            anchors.centerIn: parent
            // scale: control.horizontal && control.mirrored ? -1 : 1
            color: "transparent"
            opacity: control.enabled ? 1: 0.45

            Rectangle{
                id: unGroove
                x: touchSlider ? (control.horizontal ? 0 : ((parent.width - width) / 2)) : (control.horizontal ? 0 : (parent.width - width) / 2)
                y: touchSlider ? (control.horizontal ? ((parent.height - height) / 2) : 0) : (control.horizontal ? ((parent.height - height) / 2) : (0))
                width: touchSlider ? (control.horizontal ? parent.width : slider.touchGrooveHeight) : (control.horizontal ? parent.width : slider.grooveHeight)
                height: touchSlider ? (control.horizontal ? slider.touchGrooveHeight : parent.height) : (control.horizontal ? slider.grooveHeight : parent.height)

                radius: touchSlider ? 20 : (control.horizontal ? height/2 : width/2)
                border.width: slider.grooveBorderWidth
                border.color: getStartColor(!control.enabled ? slider.disableUnGrooveBorderColor : slider.normalUnGrooveBorderColor)
                color: gradientUNGrooveRec.visible ? "transparent" : getStartColor(isTransaprent ? (!control.enabled ? slider.disableTransparentUnGrooveColor : slider.normalTransparentUnGrooveColor) :
                                                                                              (!control.enabled ? slider.disableUnGrooveColor : slider.normalUnGrooveColor))

                Rectangle{
                    id: gradientUNGrooveRec
                    anchors.fill: parent
                    radius: parent.radius
                    border.width: parent.border.width
                    border.color: parent.border.color
                    gradient:  Gradient {
                        GradientStop { position: 0.0; color: getStartColor(!control.enabled ? slider.disableUnGrooveColor : slider.normalUnGrooveColor) }
                        GradientStop { position: 1.0; color: getEndColor(!control.enabled ? slider.disableUnGrooveColor : slider.normalUnGrooveColor)}
                    }
                    visible: !parseInterface.isSolidPattern(!control.enabled ? slider.disableUnGrooveColor : slider.normalUnGrooveColor)
                }
            }

            Rectangle{
                id:groove
                width: touchSlider ?
                    (control.horizontal ?
                        (control.mirrored ? (parent.width - handle.x) : (handle.x - focusBorderWidth + handle.width)) :
                        slider.touchGrooveHeight) :
                    (control.horizontal ?
                        (control.mirrored ? (parent.width - handle.x) : (handle.x - focusBorderWidth + handle.width)) :
                        slider.grooveHeight)

                height: touchSlider ? (control.horizontal ? slider.touchGrooveHeight : (parent.height - handle.y)/*((control.position * parent.height) < slider.touchHandleWidth ?
                                                   handle.height : (parent.height - handle.y))*/) : (control.horizontal ? slider.grooveHeight : (parent.height - handle.y - handle.height/2))
                radius: touchSlider ? 20 : (control.horizontal ? height/2 : width/2)

                x: touchSlider ?
                    (control.horizontal ?
                        (control.mirrored ? handle.x - focusBorderWidth : 0) :
                        ((parent.width - width) / 2)) :
                    (control.horizontal ?
                        (control.mirrored ? (parent.width - width) : 0) :
                        ((parent.width - width) / 2))
                y: touchSlider ? (control.horizontal ? ((parent.height - height) / 2) : handle.y - focusBorderWidth) : (control.horizontal ? ((parent.height - height) / 2) : (handle.y - focusBorderWidth + handle.height/2))

                border.width: slider.grooveBorderWidth
                border.color: getStartColor(!control.enabled ? slider.disableGrooveBorderColor : slider.normalGrooveBorderColor)
                color: gradientGrooveRec.visible ? "transparent" : getStartColor(!control.enabled ? slider.disableGrooveColor : slider.normalGrooveColor)
                Rectangle{
                    id: gradientGrooveRec
                    anchors.fill: parent
                    radius: parent.radius
                    border.width: parent.border.width
                    border.color: parent.border.color
                    gradient:  Gradient {
                        GradientStop { position: 0.0; color: getStartColor(!control.enabled ? slider.disableGrooveColor : slider.normalGrooveColor) }
                        GradientStop { position: 1.0; color: getEndColor(!control.enabled ? slider.disableGrooveColor : slider.normalGrooveColor)}
                    }
                    visible: !parseInterface.isSolidPattern(!control.enabled ? slider.disableGrooveColor : slider.normalGrooveColor)
                }
//                Rectangle{
//                                        anchors.right: parent.right
//                                        width: handle.width
//                                        height: handle.height
//                                        radius: handle.height/2
//                                    }
            }
//            Rectangle {
//                id: handle
//                x: control.horizontal ? (control.position * control.width - width/2) : ((item.width - width) / 2)
//                y: control.horizontal ? ((item.height - height) / 2) : (control.visualPosition * parent.height - height/2)
//                width: slider.grooveHeight
//                height: slider.grooveHeight

//                radius: slider.grooveHeight / 2
//                color: !control.enabled ? slider.disableHandleColor : slider.normalHandleColor
//                border.width: slider.handleBorderWidth
//                border.color: !control.enabled ? slider.disableHandleBorderColor : slider.normalHandleBorderColor
//            }
        }
    }

    StylePrivate.UKUISlider{
        id: slider
    }

    StylePrivate.APPParameter{
        id: app
    }

    onFocusChanged: {
        if(controlFocusReason())
            control.focusEnable = true;
        else
            control.focusEnable = false;
    }
    onPressedChanged: control.focusEnable = false;

    Keys.onPressed: {

        switch(event.key){
        case Qt.Key_PageUp:
            control.value = Math.min(control.value + 10 * control.stepSize, control.to)
            break;
        case Qt.Key_PageDown:
            control.value = Math.max(control.value - 10 * control.stepSize, control.from)
            break;
        case Qt.Key_Down:
            control.decrease()
            break;
        case Qt.Key_Up:
            control.increase()
            break;
        case Qt.Key_Home:
            control.value = control.from
            break;
        case Qt.Key_End:
            control.value = control.to
            break;

        }
    }

    WheelHandler{
        onWheel: {
            var delta = event.angleDelta.y
            if(control.mirrored || event.inverted){
                delta = -1 * delta;
            }

            if(delta > 0){
                //var r = Math.floor(control.value + Math.random() * 10)
                control.value = Math.round(Math.min(control.value + 10 * Math.random() * delta/120, control.to))
            }
            else if(delta < 0){
                control.value = Math.round(Math.max(control.value + 10 * Math.random() * delta/120, control.from))
            }


        }
    }
    function controlFocusReason(){
        return app.focusEnable && (control.focusReason === Qt.TabFocusReason || control.focusReason === Qt.BacktabFocusReason)
    }
}
