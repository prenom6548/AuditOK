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
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import org.ukui.qqc2style.private 1.0 as StylePrivate
import QtQuick.Templates 2.5 as T

T.TabButton {
    id: controlRoot
    Accessible.role: Accessible.PageTab
    Accessible.name: (controlRoot.text && controlRoot.text.length > 0) ? controlRoot.text : qsTr("Tab")
    implicitWidth:
    {
    var contentwidth = buttonIconLabel.adjustWidth(0) + leftPadding + rightPadding
        return Math.max(implicitBackgroundWidth + leftInset + rightInset,
                        contentwidth)
    }
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

//    leftPadding: btn.margin
//    rightPadding: btn.margin

    hoverEnabled: true //Qt.styleHints.useHoverEffects TODO: how to make this work in 5.7?


    font: app.font
    palette: app.palette

    property var isTransaprent:{
        if(controlRoot.hasOwnProperty("transparent"))
            return controlRoot.transparent
        else
            return true
    }
    property var backColor: controlRoot.checked ? (isTransaprent ? tabbutton.checkedTransparentColor : tabbutton.checkedColor) : controlRoot.pressed ?
                               (isTransaprent ? tabbutton.clickedTransparentColor : tabbutton.clickedColor) : controlRoot.hovered ?
                                   (isTransaprent ? tabbutton.hoverTransparentColor : tabbutton.hoverColor) :
                                                    (isTransaprent ? tabbutton.normalTransparentColor : tabbutton.normalColor)
  //  property var backcolor: checked ? "blue" : pressed ? "green" : hovered ? "red" : "yellow"

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
    contentItem: ButtonIconLabelContent {
        id: buttonIconLabel
        controlRoot: controlRoot
        anchors.centerIn: controlRoot
        layout: "left"
        leftSpace: 10
//        labelColor: tabbutton.normalTextColor
//        onWidthChanged: console.log(controlRoot.text, width)
    }


    background: Canvas{
        id: canvas
        anchors.fill: parent
        onPaint: {
            var radius = 12;
            var ctx = getContext("2d");
            ctx.reset(); // 重置画布状态
            var gradient = ctx.createLinearGradient(canvas.x, canvas.y, canvas.width, canvas.height)

            gradient.addColorStop(0, getStartColor(backColor));
            gradient.addColorStop(1, getEndColor(backColor));
            ctx.fillStyle = gradient;

            ctx.beginPath();
            // if(controlRoot.checked || controlRoot.pressed){
                // 绘制第一段圆弧路径
                ctx.arc(x + radius, y + radius, radius, Math.PI, Math.PI * 3 / 2);
                // 绘制第一段直线路径
                ctx.lineTo(width - radius + x, y);
                // 绘制第二段圆弧路径
                ctx.arc(width - radius + x, radius + y, radius, Math.PI * 3 / 2, Math.PI * 2);
                // 绘制第二段直线路径
                ctx.lineTo(width + x, height + y);
                // 绘制第三段直线路径
                ctx.lineTo(x, height + y);
                // 绘制第四段直线路径
                ctx.lineTo(x, y + radius);
            // }
            // else{
//                // 绘制第一段圆弧路径
//                ctx.arc(x + radius, y + radius, radius, Math.PI, Math.PI * 3 / 2);
                // 绘制第一段直线路径
                // ctx.moveTo(x, y);
                // 绘制第二段直线路径
                // ctx.lineTo(x + width, y);
                // ctx.lineTo(x + width, y + height);
                // ctx.lineTo(x, y + height);
                // ctx.lineTo(x, y);

//                // 绘制第二段圆弧路径
//                ctx.arc(width - radius + x, radius + y, radius, Math.PI * 3 / 2, Math.PI * 2);
//                // 绘制第三段直线路径
//                ctx.lineTo(width + x, y);
//                // 绘制第四段直线路径
//                ctx.lineTo(width + x - radius, y);


            // }
            ctx.closePath();

            ctx.fill();

            if(getStartColor(tabbutton.focusBorderColor).alpha >= 0.0 && app.focusEnable){
                ctx.lineWidth = tabbutton.borderWidth;
                ctx.strokenStyle = getStartColor(tabbutton.focusBorderColor);

                ctx.stroke();
            }
        }
    }

    onPressedChanged: {
        canvas.requestPaint()
    }
    onHoveredChanged: {
        canvas.requestPaint()
    }
    onCheckedChanged: {
        canvas.requestPaint()
    }

    StylePrivate.UKUITabButton{
        id: tabbutton
    }

    StylePrivate.APPParameter{
        id: app
        onParametryChanged: canvas.requestPaint()
    }
}
