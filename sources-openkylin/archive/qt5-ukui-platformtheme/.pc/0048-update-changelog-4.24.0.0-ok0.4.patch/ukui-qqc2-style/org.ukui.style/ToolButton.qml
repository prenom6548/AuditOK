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


T.ToolButton {
    id: controlRoot
    Accessible.role: Accessible.Button
    Accessible.name: (controlRoot.text && controlRoot.text.length > 0) ? controlRoot.text : qsTr("Tool button")
    implicitWidth: {
        if(controlRoot.hasOwnProperty("maxWidth"))
            return controlRoot.maxWidth
        var contentwidth = implicitContentWidth + leftPadding + rightPadding
        return Math.max(implicitBackgroundWidth + leftInset + rightInset,
                        contentwidth, btn.normalWidth)
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding, btn.normalHeight)

    leftPadding: btn.leftRightMargin
    rightPadding: btn.leftRightMargin

    hoverEnabled: true //Qt.styleHints.useHoverEffects TODO: how to make this work in 5.7?


    property var _menuToolButton:{
        if(controlRoot.hasOwnProperty("hasMenu"))
            return controlRoot.hasMenu
        else
            return false
    }

    property var _maxWidth: {
        if(controlRoot.hasOwnProperty("maxWidth")){
            return controlRoot.maxWidth
        }
        else
            return -1
    }
    property bool focusEnable: appP.focusEnable

    font: appP.font
    palette: appP.palette

    NormalControlColor{
    id:ncColor
    }

    StylePrivate.UKUIButton{
        id: btn
    }
    StylePrivate.APPParameter{
        id: appP
    }

    onEnabledChanged: {
        content.updateImage()
        back.changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, (focusEnable && controlRoot.focus && back.controlFocusReason()),
                                     (controlRoot.checkable && controlRoot.checked))
    }
    property bool hasWindowActive: Qt.application.active
    onHasWindowActiveChanged:{
        content.updateImage()
    }
    onPressedChanged: content.updateImage()
    contentItem: ButtonIconLabelContent {
        id: content
        controlRoot: controlRoot
        anchors.centerIn: controlRoot
//        onWidthChanged: console.log(controlRoot.text, width)
    }
    background: BackGroundRectangle{
        id: back
        controlRoot: controlRoot
        _radius: btn.radius
    }}
