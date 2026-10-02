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


T.ToolTip {
    id: controlRoot

    palette: appP.palette
    font: appP.font


    x: parent ? Math.round((parent.width - implicitWidth) / 2) : 0
    y: -implicitHeight - 3

    // Always show the tooltip on top of everything else
    z: 999

    implicitWidth: popupwindow.width
//    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    margins: tooltip.margins
    padding: tooltip.padding


    // Timeout based on text length, from QTipLabel::restartExpireTimer
    timeout: 4000
    delay: Qt.styleHints.mousePressAndHoldInterval
    Component.onDestruction: {
        if (typeof popupwindow !== "undefined" && popupwindow) {
            popupwindow.visible = false//如果window还没销毁 显隐藏
        }
    }
    property bool loaderSuc: false

    property bool showWindow: {
        if(controlRoot.hasOwnProperty("backOpacity") && controlRoot.backOpacity >= 1)
            return false
        return true;//appP.showToolTipWindow()
    }
    property var internalComponent: null
    property var _opacity:{
        if(controlRoot.hasOwnProperty("backOpacity"))
            return controlRoot.backOpacity
        return appP.menuTransparency
    }
    property var mode:{
        if(controlRoot.hasOwnProperty("wrapMode")){
            //console.log("mode:", controlRoot.wrapMode)
            return controlRoot.wrapMode
        }
        return Text.NoWrap
    }
    property var _maxWidth:{
        if(controlRoot.hasOwnProperty("maxWidth"))
            return controlRoot.maxWidth
        return 0
    }
    property var _textFormat:{
        if(controlRoot.hasOwnProperty("textFormat"))
            return controlRoot.textFormat
        return Qt.PlainText
    }
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent
    property var isTransaprent:{
        if(controlRoot.hasOwnProperty("transparent"))
            return controlRoot.transparent
        else
            return true
    }
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


    StylePrivate.APPParameter{
        id: appP
    }

    StylePrivate.UKUIToolTip{
        id: tooltip
    }
    onVisibleChanged: {
        if(visible){
            var p = popHandle.posByCursor(popupwindow.width, popupwindow.height)

            if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                popupwindow.setGeometry(Math.ceil(p.x), Math.ceil(p.y), Math.ceil(popupwindow.width), Math.ceil(popupwindow.height))
                popupwindow.visible = false
                popupwindow.visible = true
            }
        }
        popupwindow.visible = controlRoot.visible
    }

    // ========== 新增：文本测量相关属性和函数 ==========
    // 存储行数
    property int textLineCount: 0
    // 存储每行最大宽度
    property real maxLineWidth: 0


    contentItem: Item{
        Window {
            id: popupwindow
            width: Math.max(1, (controlRoot._maxWidth != 0 && controlRoot.text != "") ? Math.min(maxLineWidth > 0 ? (maxLineWidth  + 2 * tooltip.padding): controlRoot._maxWidth,
                                                                                                 Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding) :
                                                                                        (Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding))

            height: {
                if (typeof popHandle !== "undefined" && popHandle) {
                    return Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding))
                }
                return Math.max(1, Math.ceil(windowLabel.implicitHeight + 2 * tooltip.padding))
            }
            visible: controlRoot.visible
            color: "transparent"
            flags: Qt.WindowTransparentForInput

            onWidthChanged: {
                if(visible){
                    var p = popHandle.posByCursor(popupwindow.width, popupwindow.height)

                    if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                        popupwindow.setGeometry(Math.ceil(p.x), Math.ceil(p.y), Math.ceil(popupwindow.width), Math.ceil(popupwindow.height))
                        popupwindow.requestUpdate()
                    }
                }
            }

            onHeightChanged: {
                if(visible){
                    var p = popHandle.posByCursor(popupwindow.width, popupwindow.height)

                    if(popupwindow.x !== p.x || popupwindow.y !== p.y){
                        popupwindow.setGeometry(Math.ceil(p.x), Math.ceil(p.y), Math.ceil(popupwindow.width), Math.ceil(popupwindow.height))
                        popupwindow.requestUpdate()
                    }
                }
            }
            onVisibleChanged: {
                if(visible){
                   calculateTextLines()
                    var w = popupwindow.width;
                    if(controlRoot._maxWidth !== 0)
                        if(maxLineWidth > 0 )
                            w = maxLineWidth  + 2 * tooltip.padding
                        else if((windowLabel.implicitWidth +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                            w = windowLabel.implicitWidth +  + 2 * tooltip.padding
                        else{
                            w  = controlRoot._maxWidth
                        }

                    var h = popupwindow.height
                    if (typeof popHandle !== "undefined" && popHandle) {
                        h = Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding))
                        if(popHandle.getScreenSize().height < windowLabel.implicitHeight + 2 * tooltip.padding){
                            windowLabel.elide = Qt.ElideRight
                            windowLabel.maximumLineCount = 100
                        }
                    }
                    else
                    h =  Math.max(1, windowLabel.implicitHeight + 2 * tooltip.padding)

                    controlRoot.implicitWidth = Math.max(Math.ceil(windowLabel.implicitWidth + 2 * tooltip.padding), 1)
                    var p = popHandle.posByCursor(Math.ceil(w), Math.ceil(h))

                    if(popupwindow.x !== p.x || popupwindow.y !== p.y || popupwindow.width !== w || popupwindow.height !== h){
                        popupwindow.setGeometry(p.x, p.y, Math.ceil(w), Math.ceil(h))
                        popupwindow.requestUpdate()
                    }

                }
                else{
                    textLineCount = 0
                    maxLineWidth = 0
                }
            }

            StylePrivate.PopupHandle{
                id: popHandle
//                blur: true
                titlebarVisible: false
                anchors.fill: parent
                posFollwMouse: true
                visible: popupwindow.visible
                radius: tooltip.radius
                windowStaysOnTopHint: true
            }

            Rectangle{
                id: rec
                visible: popupwindow.visible
                anchors.fill: parent
                radius: _opacity >= 1 ? tooltip.radius : 0
                color: {
                    var baccolor = getStartColor(isTransaprent ? tooltip.backTransparentColor : tooltip.backColor)
                    var alphaValue = popHandle.blur ? controlRoot._opacity : 1.0;
                    return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, alphaValue)
                }
                // border.color: getStartColor(isTransaprent ? tooltip.backBorderTransparentColor : tooltip.backBorderColor)
                // border.width: 1

            }
            Text {
                id: windowLabel
                anchors.fill: rec
                anchors.leftMargin: tooltip.padding
                anchors.rightMargin: tooltip.padding
                anchors.topMargin: tooltip.padding
                anchors.bottomMargin: tooltip.padding


                anchors.centerIn: rec
                visible: popupwindow.visible
                text: controlRoot.text
                wrapMode: controlRoot ? controlRoot.mode : Text.NoWrap
                font: controlRoot ? controlRoot.font : appP.font
                color: getStartColor(tooltip.textColor)
                textFormat: controlRoot ? controlRoot._textFormat : Qt.PlainText

                onTextChanged: {
                    if(controlRoot)
                    {
                       calculateTextLines()
                        if(controlRoot._maxWidth !== 0)
                            if(maxLineWidth > 0 )
                                popupwindow.width = maxLineWidth  + 2 * tooltip.padding
                            else if((Math.ceil(windowLabel.implicitWidth) +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                                popupwindow.width = Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding
                            else{
                                    popupwindow.width = controlRoot._maxWidth
                            }

                        if (typeof popHandle !== "undefined" && popHandle) {
                            popupwindow.height = Math.ceil(Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding)))
                            if(popHandle.getScreenSize().height < windowLabel.implicitHeight + 2 * tooltip.padding){
                                windowLabel.elide = Qt.ElideRight
                                windowLabel.maximumLineCount = 100
                            }
                        }
                        else
                        popupwindow.height =  Math.ceil(Math.max(1, windowLabel.implicitHeight + 2 * tooltip.padding))


                        controlRoot.implicitWidth = Math.ceil(Math.max(Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding, 1))
                    }

                }

                onContentWidthChanged: {
                    if(controlRoot)
                    {
                        if(controlRoot._maxWidth !== 0)
                            if(maxLineWidth > 0 )
                                popupwindow.width = maxLineWidth  + 2 * tooltip.padding
                            else if((windowLabel.implicitWidth +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                                popupwindow.width = Math.ceil(windowLabel.implicitWidth +  + 2 * tooltip.padding)
                            else{
                                    popupwindow.width = controlRoot._maxWidth
                            }

                        if (typeof popHandle !== "undefined" && popHandle) {
                            popupwindow.height = Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding))
                            if(popHandle.getScreenSize().height < windowLabel.implicitHeight + 2 * tooltip.padding){
                                windowLabel.elide = Qt.ElideRight
                                windowLabel.maximumLineCount = 100
                            }
                        }
                        else
                        popupwindow.height =  Math.ceil(Math.max(1, windowLabel.implicitHeight + 2 * tooltip.padding))

                        controlRoot.implicitWidth = Math.ceil(Math.max(windowLabel.implicitWidth + 2 * tooltip.padding, 1))
                    }

                }
                onContentHeightChanged: {
                    if(controlRoot)
                    {
                        if(controlRoot._maxWidth !== 0)
                            if(maxLineWidth > 0 )
                                popupwindow.width = maxLineWidth  + 2 * tooltip.padding
                            else if((Math.ceil(windowLabel.implicitWidth) +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                                popupwindow.width = Math.ceil(windowLabel.implicitWidth) +  + 2 * tooltip.padding
                            else{
                                    popupwindow.width = controlRoot._maxWidth
                            }

                        if (typeof popHandle !== "undefined" && popHandle) {
                            popupwindow.height = Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding))
                            if(popHandle.getScreenSize().height < windowLabel.implicitHeight + 2 * tooltip.padding){
                                windowLabel.elide = Qt.ElideRight
                                windowLabel.maximumLineCount = 100
                            }
                        }
                        else
                        popupwindow.height =  Math.max(1, windowLabel.implicitHeight + 2 * tooltip.padding)

                        controlRoot.implicitWidth = Math.max(Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding, 1)
                    }
                }
                Component.onCompleted: {
                    if(controlRoot)
                    {
                        if(controlRoot._maxWidth !== 0)
                            if((Math.ceil(windowLabel.implicitWidth) +  + 2 * tooltip.padding) < controlRoot._maxWidth)
                                popupwindow.width = Math.ceil(windowLabel.implicitWidth) +  + 2 * tooltip.padding
                            else{
                                if(maxLineWidth > 0 )
                                    popupwindow.width = maxLineWidth  + 2 * tooltip.padding
                                else
                                    popupwindow.width = controlRoot._maxWidth
                            }

                        if (typeof popHandle !== "undefined" && popHandle) {
                            popupwindow.height = Math.max(1, Math.min(popHandle.getScreenSize().height, windowLabel.implicitHeight + 2 * tooltip.padding))
                            if(popHandle.getScreenSize().height < windowLabel.implicitHeight + 2 * tooltip.padding){
                                windowLabel.elide = Qt.ElideRight
                                windowLabel.maximumLineCount = 100
                            }
                        }
                        else
                        popupwindow.height =  Math.ceil(Math.max(1, windowLabel.implicitHeight + 2 * tooltip.padding))

                        controlRoot.implicitWidth = Math.ceil(Math.max(Math.ceil(windowLabel.implicitWidth) + 2 * tooltip.padding, 1))
                    }
                }


            }
        }

    }

    // 用于测量的隐藏 Text 组件（不渲染到界面）
    Text {
        id: measureText
        visible: false  // 隐藏，仅用于测量
        font: windowLabel.font
        wrapMode: Text.NoWrap  // 不换行，用于测量单行宽度
        textFormat: windowLabel.textFormat
    }

    // 计算单个字符串的宽度（Qt Quick 原生方法）
    function getTextWidth(text) {
        measureText.text = text
        return measureText.implicitWidth
    }

    // 计算文本行数和每行最大宽度（纯 Quick 兼容版）
    function calculateTextLines() {
        if(_textFormat !== Qt.PlainText || _maxWidth === 0)
            return;
        if (!windowLabel || windowLabel.text === "") {
            controlRoot.textLineCount = 0
            controlRoot.maxLineWidth = 0
            return
        }

        // 获取文本显示的实际宽度（去除padding）
        const textDisplayWidth = Math.max(1, (controlRoot._maxWidth !== 0) ?
                                          controlRoot._maxWidth - 2 * tooltip.padding :
                                          getTextWidth(windowLabel.text))

        // 第一步：先按显式换行符 \n 拆分原始行
        const originalLines = windowLabel.text.split('\n')
        let totalLines = 0    // 最终总行数
        let maxWidth = 0      // 每行最大宽度

        // 第二步：遍历每一行，处理自动换行并测量宽度
        for (let i = 0; i < originalLines.length; i++) {
            const originalLine = originalLines[i]
            // 空行：行数+1，宽度0
            if (originalLine === "") {
                totalLines++
                continue
            }

            // 测量当前原始行的宽度
            const originalLineWidth = getTextWidth(originalLine)

            // 情况1：不换行 或 行宽度 <= 显示宽度 → 直接作为一行
            if (windowLabel.wrapMode === Text.NoWrap /*|| originalLineWidth <= (controlRoot._maxWidth - 2 * tooltip.padding)*/) {
                totalLines++
                maxWidth = Math.max(maxWidth, originalLineWidth)
                continue
            }

            // 情况2：需要自动换行 → 逐字符拆分（Qt Quick 原生方式）
            let currentLineText = ""
            let currentLineWidth = 0

            // 逐字符处理当前行
            for (let j = 0; j < originalLine.length; j++) {
                const charc = originalLine[j]
                const charWidth = getTextWidth(currentLineText + charc)

                // 如果添加当前字符后宽度超过限制 → 换行
                if (charWidth > (controlRoot._maxWidth - 2 * tooltip.padding)) {
                    // 记录当前行
                    totalLines++
                    maxWidth = Math.max(maxWidth, currentLineWidth)

                    // 重置当前行，开始新行
                    currentLineText = charc
                    currentLineWidth = getTextWidth(charc)
                } else {
                    // 继续添加字符到当前行
                    currentLineText += charc
                    currentLineWidth = charWidth
                }
            }

            // 处理最后一行剩余的字符
            if (currentLineText !== "") {
                totalLines++
                maxWidth = Math.max(maxWidth, currentLineWidth)
            }
        }

        // 更新最终结果
        controlRoot.textLineCount = totalLines
        controlRoot.maxLineWidth = Math.ceil(maxWidth)

    }
    // ========== 修正结束 ==========

}

