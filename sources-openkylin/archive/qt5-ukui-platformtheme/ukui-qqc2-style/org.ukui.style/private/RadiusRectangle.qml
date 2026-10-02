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
import QtQuick.Controls 2.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item{
    id:control
//    property Rectangle controlRoot: control.parent

    property int leftTopRadius: 6
    property int rightTopRadius: 6
    property int leftBottomRadius: 6
    property int rightBottomRadius: 6

    property var borderColor
    property var backColor
    property int borderWidth

    property alias canvas : customButtonCanvas

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
    Canvas {
        id: customButtonCanvas
        width: control.width
        height: control.height
        anchors.centerIn: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var gradient = ctx.createLinearGradient(customButtonCanvas.x, customButtonCanvas.y, customButtonCanvas.width, customButtonCanvas.height)

            gradient.addColorStop(0, getStartColor(backColor));
            gradient.addColorStop(1, getEndColor(backColor));

            // 填充颜色
            ctx.fillStyle = gradient;

            // 开始绘制路径
            ctx.beginPath();

            // 绘制圆角矩形，只有左上角和右上角有圆角
            // 绘制第一段圆弧路径
            ctx.moveTo(0, leftTopRadius);
            ctx.arc(leftTopRadius, leftTopRadius, leftTopRadius, Math.PI, Math.PI * 3 / 2);
            // 绘制第一段直线路径
            ctx.lineTo(width - rightTopRadius, 0);
            // 绘制第二段圆弧路径
            ctx.arc(width - rightTopRadius, rightTopRadius, rightTopRadius, Math.PI * 3 / 2, Math.PI * 2);
            // 绘制第二段直线路径
            ctx.lineTo(width, height - rightBottomRadius);
            // 绘制第三段圆弧路径
            ctx.arc(width - rightBottomRadius, height - rightBottomRadius, rightBottomRadius, 0, Math.PI * 1 / 2);
            // 绘制第三段直线路径
            ctx.lineTo(leftBottomRadius, height);
            // 绘制第四段圆弧路径
            ctx.arc(leftBottomRadius, height - leftBottomRadius, leftBottomRadius, Math.PI * 1 / 2, Math.PI);
            // 绘制第四段直线路径
            ctx.lineTo(leftTopRadius);

            // 填充路径
            ctx.closePath();
            ctx.clip();
            ctx.fill();

            // 绘制边框
//            ctx.strokeStyle = getStartColor(borderColor);
//            ctx.lineWidth = borderWidth;
//            ctx.stroke();
        }

        // 鼠标点击事件（模拟按钮点击）
//        MouseArea {
//            anchors.fill: parent
//            onClicked: {
//                canvas.requestPaint()

//            }
//        }
    }
}
