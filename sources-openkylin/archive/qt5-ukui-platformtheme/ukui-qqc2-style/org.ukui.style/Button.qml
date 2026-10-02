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

import QtQuick.Controls 1.4

import QtQuick 2.15
import QtQuick.Window 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.14
import QtQuick 2.6
import QtQuick.Templates 2.5 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate


T.Button {
    id: controlRoot
    Accessible.role: Accessible.Button
    Accessible.name: (controlRoot.text && controlRoot.text.length > 0) ? controlRoot.text : qsTr("Button")
    implicitWidth: {
        if(controlRoot.hasOwnProperty("maxWidth"))
            return controlRoot.maxWidth
        var contentwidth = implicitContentWidth + leftPadding + rightPadding
        return Math.max(implicitBackgroundWidth + leftInset + rightInset,
                        contentwidth, btn.normalWidth)
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding, btn.normalHeight)

    leftPadding: StylePrivate.UKUIButton.leftRightMargin
    rightPadding: StylePrivate.UKUIButton.leftRightMargin


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

    property var mIsLoadBtn: {
        if(controlRoot.hasOwnProperty("isLoadBtn")){
            return controlRoot.isLoadBtn
        }
        else
            return false
    }
    property var mIsLoading: {
        if(controlRoot.hasOwnProperty("isLoadBtn") && controlRoot.isLoadBtn && controlRoot.hasOwnProperty("isLoading")){
            return controlRoot.isLoading
        }
        else
            return false
    }

    property bool focusEnable: app.focusEnable

    font: app.font

    palette: app.palette

    onEnabledChanged: {
        content.updateImage()
        back.changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, back.showFocusBorder,
                                     (controlRoot.checkable && controlRoot.checked))
    }
    onPressedChanged: content.updateImage()
    // 键盘 Space 时设背景的 keyboardPressing=true（鼠标点击不触发 Keys；AbstractButton 仅 Space 激活）；
    // 抑制逻辑集中在 BackGroundRectangle.onPressedChanged。只打标，一律不 accept，避免吞键。
    Keys.onPressed: {
        if (event.key === Qt.Key_Space)
            back.keyboardPressing = true
        event.accepted = false
    }
    property bool hasWindowActive: Qt.application.active
    onHasWindowActiveChanged:{
        content.updateImage()
    }
    StylePrivate.UKUIButton{
        id: btn
    }
    StylePrivate.APPParameter{
        id: app

    }

    contentItem: ButtonIconLabelContent {
        id:content
        controlRoot: controlRoot
        anchors.centerIn: controlRoot
        //onWidthChanged: console.log(controlRoot.text, width)
    }
    background: BackGroundRectangle{
        id: back
        implicitHeight: controlRoot.implicitHeight
        implicitWidth: controlRoot.implicitWidth
        controlRoot: controlRoot
        _radius: btn.radius
        //Component.onCompleted: console.log("BackGroundRectangle radius", StylePrivate.UKUIButton.radius, StylePrivate.UKUIButton.normalBC)
    }
}
