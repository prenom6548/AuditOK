/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.12
import QtQuick.Window 2.0
import QtQuick.Controls 2.12
import "./part"
import "./mousebase"
import "./signcanvas"

Rectangle {
    objectName: "mainRect"
    id: mainWindow
    visible: true
    property string dropImagePath: ""
    signal sendDropImagePath(var path)
    signal sendDragPath(var path)
    property int mouseMoveDistance: 0
    property point startPressPoint: Qt.point(-1,-1)
    property point releasePoint: Qt.point(-1,-1)
    signal changeImage(var nextOrBack)
    property bool mouseIsDoubleClick: false
    signal doubleSignal()
    signal changeImageToNext()
    signal changeImageToBack()
    Variables { id: variables }
    ImageBaseFunction {id: imageBase}
    onWidthChanged: {
        variables.curWidW = width
    }
    onHeightChanged: {
        variables.curWidH = height
    }

    //鼠标事件响应区域
    MouseAreaEvent { id: mouseshortcuts }
    //外边框---最外层的，需要用到别的组件相对于它的位置
    Image {
        id: bgimage
        anchors.fill: parent
        MouseArea {
            id: mousemouse
            enabled: !variables.signMouseStart
            anchors.fill: parent
            hoverEnabled: true
            onReleased: {
                //导航栏存在的话直接return
                if (variables.navigatorState) {
                    return
                }
                if (mouseIsDoubleClick) {
                    mouseIsDoubleClick = false
                    return
                }
                releasePoint = Qt.point(mouseX, mouseY)
                mouseMoveDistance = releasePoint.x - startPressPoint.x

                if (mouseMoveDistance > 100) {
                    variables.operateImageWay = 18
                    //切换图图片
                    changeImage(true)
                }
                if (mouseMoveDistance < -100) {
                    variables.operateImageWay = 17
                    //切换图图片
                    changeImage(false)
                }
            }

            onPressed: {
                startPressPoint = Qt.point(mouseX, mouseY)
            }

            onPositionChanged: {
                variables.mousePos = Qt.point(mouseX, mouseY)
            }
            onMouseXChanged: {
                variables.mouseCurPos = Qt.point(mouseX, mouseY)
            }
            onMouseYChanged: {
                variables.mouseCurPos = Qt.point(mouseX, mouseY)
            }
            onDoubleClicked: {
                mouseIsDoubleClick = true
                doubleSignal()
            }
        }
    }
    //主要的图片区域
    MainImageArea {
        objectName: "imageItem"
        id: imageArea
        x: 0
        y: 0
        width: mainWindow.width
        height: mainWindow.height
    }
    //拖拽文件打开图片
    DropArea{
        anchors.fill: parent
        onEntered: {
            if (drag.hasUrls) {
                sendDragPath(drag.urls[0])
            }
        }
        onDropped: {
            if (variables.operateImageWay == 9 || variables.operateImageWay == 10) {
                return
            }
            if (drop.hasUrls) {
                if (drop.urls.length > 0) {
                    dropImagePath = drop.urls[0]
                    sendDropImagePath(drop.urls[0])
                }
            }
        }
    }

    function setOperateWay(way) {
        variables.operateImageWay = way
    }
    function setFormat(format) {
        variables.format = format
    }
    function setNavigatorState(state) {
        variables.navigatorState = state
    }
    function setchangImageUseAnimation(isUse) {
        variables.changImageUseAnimation = isUse
        if (variables.changImageUseAnimation) {
            variables.imageAniDuration = 300
        } else {
            variables.imageAniDuration = 20
        }
    }

    //sign
    function setpainterType(type) {
        variables.painterType = type
    }
    function setpainterThickness(thickness) {
        variables.painterThickness = thickness
    }
    function setpainterColor(color) {
        variables.painterColor = color
    }
    function setBold(isBold) {
        variables.isBold = isBold
    }
    function setDeleteLine(isDeleteLine) {
        variables.isDeleteLine = isDeleteLine
    }
    function setUnderLine(isUnderLine) {
        variables.isUnderLine = isUnderLine
    }
    function setItalics(isItalics) {
        variables.isItalics = isItalics
    }
    function setFontType(fontType) {
        variables.fontType = fontType
    }
    function setFontSize(fontSize) {
        variables.fontSize = fontSize
    }
    function getOperateTime() {
        if (variables.objList.length > 0) {
            if ( variables.objList[variables.objList.length - 1].objectName == "textEditPage") {
                variables.objList[variables.objList.length - 1].sendTextOperate()
            }
        }
        return variables.operateTime
    }
    function exitOperate() {
        variables.needExitPaint = !variables.needExitPaint
    }
}

