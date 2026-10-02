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


T.RoundButton {
    id: controlRoot
    Accessible.role: Accessible.Button
    Accessible.name: (controlRoot.text && controlRoot.text.length > 0) ? controlRoot.text : qsTr("Button")
    implicitWidth: {
    var contentwidth = implicitContentWidth + leftPadding + rightPadding
        return Math.max(implicitBackgroundWidth + leftInset + rightInset,
                        contentwidth, 36)
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding, 36)

    leftPadding: StylePrivate.UKUIButton.margin
    rightPadding: StylePrivate.UKUIButton.margin

    hoverEnabled: true //Qt.styleHints.useHoverEffects TODO: how to make this work in 5.7?


    font: app.font
    radius: height/2
    palette: app.palette

    NormalControlColor{
    id:ncColor
    }
    StylePrivate.APPParameter{
        id: app

    }
    onEnabledChanged: {
         content.updateImage()
        back.changeColor(controlRoot.highlighted, controlRoot.enabled, controlRoot.pressed, controlRoot.hovered, back.showFocusBorder,
                                     (controlRoot.checkable && controlRoot.checked))
    }
    onPressedChanged: content.updateImage()
    // 键盘 Space 设背景 keyboardPressing=true；抑制逻辑在 BackGroundRectangle。只打标，一律不 accept。
    Keys.onPressed: {
        if (event.key === Qt.Key_Space)
            back.keyboardPressing = true
        event.accepted = false
    }

    contentItem: ButtonIconLabelContent {
        id: content
        controlRoot: controlRoot
        anchors.centerIn: controlRoot
//        onWidthChanged: console.log(controlRoot.text, width)
    }
    background: BackGroundRectangle{
        id: back
        controlRoot: controlRoot
        _radius: controlRoot.radius
        anchors.fill: parent

    }
}
