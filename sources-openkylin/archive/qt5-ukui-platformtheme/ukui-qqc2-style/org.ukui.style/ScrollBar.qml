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
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.ScrollBar {
    id: control

    palette: app.palette
    font: app.font


    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: true

    padding: scrollbar.padding
    visible: control.policy !== ScrollBar.AlwaysOff
    property var isTransaprent:{
        if(control.hasOwnProperty("transparent"))
            return control.transparent
        else
            return true
    }
    property var itemColor: !control.enabled ? (isTransaprent ? scrollbar.disableTransparentColor : scrollbar.disableColor) :
                             control.pressed ? (isTransaprent ? scrollbar.clickTransparentColor : scrollbar.clickColor) :
                             control.hovered ? (isTransaprent ? scrollbar.hoverTransparentColor : scrollbar.hoverColor) :
                                               (isTransaprent ? scrollbar.normalTransparentColor : scrollbar.normalColor)
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

    contentItem: Rectangle {
        // 区分横向/纵向：纵向改宽度，横向改高度
        implicitWidth: control.orientation === Qt.Vertical
            ? (control.interactive ? (control.hovered || control.pressed ? 8 : 4) : 2)
            : 0  // 横向宽度由滚动区域决定
        implicitHeight: control.orientation === Qt.Horizontal
            ? (control.interactive ? (control.hovered || control.pressed ? 8 : 4) : 2)
            : 0  // 纵向高度由滚动区域决定

        radius: control.orientation === Qt.Vertical ? width / 2 : height / 2  // 圆角随动态尺寸变化
        color: gradientRec.visible ? "transparent" : getStartColor(itemColor)

        // 根据方向设置动画：纵向动画宽度，横向动画高度
        Behavior on implicitWidth {
            NumberAnimation {
                duration: 260
                easing.type: Easing.OutExpo
            }
        }
        Behavior on implicitHeight {
            NumberAnimation {
                duration: 260
                easing.type: Easing.OutExpo
            }
        }

        Rectangle{
            id: gradientRec
            anchors.fill: parent
            radius: parent.radius
//            border.width: parent.border.width
//            border.color: parent.border.color
            gradient:  Gradient {
                GradientStop { position: 0.0; color: getStartColor(itemColor) }

                GradientStop { position: 1.0; color: getEndColor(itemColor)}
            }
            visible: !parseInterface.isSolidPattern(itemColor)
        }

        states: State {
            name: "active"
            when: /*control.policy === ScrollBar.AlwaysOn || */(control.active && (control.hovered || control.pressed) && control.size < 1.0)
            PropertyChanges { target: control.contentItem; implicitWidth:8; implicitHeight:8}
        }
    }

    StylePrivate.UKUIScrollBar{
        id: scrollbar
    }

    StylePrivate.APPParameter{
        id: app
    }

    WheelHandler{
        onWheel: {
            var delta = event.angleDelta.y
            if(control.mirrored || event.inverted){
                delta = -1 * delta;
            }

            if(delta < 0){
                control.increase()
            }
            else if(delta > 0){
                control.decrease()
            }
        }

    }
}


