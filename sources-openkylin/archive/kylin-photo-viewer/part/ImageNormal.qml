/**************************************************************************
 **                                                                      **
 ** Copyright (C) 2011-2021 Lukas Spies                                  **
 ** Contact: http://photoqt.org                                          **
 **                                                                      **
 ** This file is part of PhotoQt.                                        **
 **                                                                      **
 ** PhotoQt is free software: you can redistribute it and/or modify      **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation, either version 2 of the License, or    **
 ** (at your option) any later version.                                  **
 **                                                                      **
 ** PhotoQt is distributed in the hope that it will be useful,           **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with PhotoQt. If not, see <http://www.gnu.org/licenses/>.      **
 **                                                                      **
 **************************************************************************/

import QtQuick 2.12
//import "../signcanvas"
Item {
    objectName: "imageItemLower"
    id: imageNormal
    x:  parent.width/2 - width/2
    y:  parent.height/2 - height/2
    width: container.width
    height: container.height
    property real defaultScale: 1.0
    signal sendScale(var scale)
    property bool needWheelReact: true //是否需要滑轮响应
    property bool newImageLoaded: false //新图片加载
    signal sendPos(var xPos,var yPos)
    property double imageVisibleLeftUpX: 0 //可视区域左上角的点在原图上的位置
    property double imageVisibleLeftUpY: 0
    property double imageLeftUpX: 0  //缩放后的图片的左上角相对于窗口的位置
    property double imageLeftUpY: 0
    property double imageRealLeftX:  0 == imageVisibleLeftUpX ? imageLeftUpX : -imageVisibleLeftUpX
    property double imageRealLeftY:  0 == imageVisibleLeftUpY ? imageLeftUpY : -imageVisibleLeftUpY
    property bool imageNeedCenterZoom: true
    property bool mouseareaEnabeled: false
    property int mouseDragType: -1
    property bool startAni: true
    signal sendRectInfo(var rect)
    signal sendCircleInfo(var leftX, var leftY, var painterW, var painterH)
    signal sendLineInfo(var startPos, var endPos)
    signal sendArrowInfo(var startPos, var endPos, var arrowStartPos, var arrowEndPos)
    signal sendPencilInfo(var pointList)
    signal sendMarkInfo(var startPos, var endPos,var alpha)
    signal sendTextInfo(var textContent, var startPos,var type)
    signal sendBlurInfot(var blurRect)
    property double curMouseX: 0
    property double curMouseY: 0
    property double backScale
    property double backX
    property double backY
    property double realWidth : 0
    property double realHeight : 0
    property rect paintblurRect
    signal changeOperateWay()

    Image {
        objectName: "image"
        id: theImage
        property real curX: imageNormal.width/2 - width/2
        property real curY: imageNormal.height/2 - height/2
        x: curX
        y: curY
        focus: true
        //原始图像不做处理
        fillMode: Image.Pad
        //设置clip:true后当元素的子项超出父项范围后会自动裁剪
        clip: true
        cache: false
        asynchronous: true
        //图像是否应水平镜像，默认为false，可以做镜像来用
        mirror: false
        //图像在缩放或变换时是否平滑过滤。平滑过滤可以提供更好的视觉质量
        smooth: true
        //保存图像在缩放或变换时是否使用mipmap过滤。与平滑相比，缩小比例时，mipmap过滤提供了更好的视觉质量，但它可能会以性能为代价（无论是在初始化图像时还是在渲染期间）。
        //龙芯机器集显，设置mipmap属性会导致蓝色和红色翻转，进行关闭
        //mipmap: (scale < defaultScale || (scale < 0.8 && defaultScale < 0.8)) && (width > variables.curWidW || height > variables.curWidH)
        //source: "qrc:/res/res/kyview_logo.png"

        //旋转--参数：real，角度。默认0 degree
        rotation: 0
        property real rotateTo: 0
        onRotateToChanged: {
            rotation = rotateTo
        }
        onRotationChanged: {
            if(!rotani.running) {
                rotateTo = rotation
            }
        }
        property real curScale: 1
        scale: curScale

        Behavior on x {
            PropertyAnimation {
                id: xani
                duration: startAni ? variables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }

        Behavior on y {
            PropertyAnimation {
                id: yani
                duration: startAni ? variables.imageAniDuration : 0
                easing.type: Easing.OutCubic
            }
        }
        Behavior on rotation {
            PropertyAnimation {
                id: rotani
                duration: startAni ? variables.imageAniDuration : 0
                onRunningChanged: {
                    if (!rotani.running) {
                        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }
        Behavior on scale {
            PropertyAnimation {
                id: scaleani
                duration: startAni ? variables.imageAniDuration : 0
                onRunningChanged: {
                    if (!scaleani.running) {
                        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                        imageBase.makeSureNavigatorNeedShow(theImage)
                    }
                }
                easing.type: Easing.OutCubic
            }
        }
        onXChanged: {
            setDrogLimit()
        }
        onYChanged: {
            setDrogLimit()
        }
        function changeScale(scaleValue) {
            imageBase.setImageScaleWithImageUpdate(theImage,scaleValue)
        }

        function changeScaleWithFocus(scaleValue, focusX, focusY) {
            // 计算缩放倍率因子
            var factor = scaleValue / theImage.curScale
            // 暂时关闭动画以保证实时跟随
            startAni = false

            // 严格遵循中心规则：小图居中缩放，大图焦点缩放
            if (!imageNeedCenterZoom) {
                // 调用底层的缩放核心函数，传入手势焦点坐标
                imageBase.performZoom(theImage, theImage.mapFromItem(bgimage, focusX, focusY),
                                      undefined, false, false, true, factor)
            } else {
                // 强制中心缩放并重置位置
                imageBase.performZoom(theImage, theImage.mapFromItem(bgimage, mainWindow.width/2, mainWindow.height/2),
                                      undefined, false, false, true, factor)
                theImage.imagePostionReset()
            }
            startAni = true
        }

        function sendScaleFirstTime() {
            return imageBase.getImageScale(theImage)
        }

        //重设位置
        function imagePostionReset() {
            imageBase.resetImagePostion(theImage)
        }

        function getOperateImageWay() {
            return variables.operateImageWay
        }

        onStatusChanged: {
            if (theImage.status == Image.Ready) {
                if (newImageLoaded) {
                    theImage.imagePostionReset()
                }
                if (variables.changImageUseAnimation) {
                    startAni = false
                } else {
                    startAni = true
                }
            }
        }
        function changeImage(imageUrl,imageWidth,imageHeight) {
//            sourceSize = Qt.size(imageWidth  ,imageHeight )
            realWidth = imageWidth
            realHeight = imageHeight
            if (variables.operateImageWay === 12) {
                theImage.source = ""
                startAni = false
                theImage.x = variables.curWidW / 2 - imageWidth / 2
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                theImage.source = imageUrl
                startAni = true
            }
            if (variables.operateImageWay === 17) {
                theImage.source = ""
                startAni = false
                theImage.x = variables.curWidW / 2 - imageWidth / 2 + imageWidth + 10
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                theImage.source = imageUrl
                startAni = true
                theImage.x = variables.curWidW / 2 - imageWidth / 2
            }
            if (variables.operateImageWay === 18) {
                theImage.source = ""
                startAni = false
                theImage.x = - imageWidth  - 10
                theImage.y = variables.curWidH / 2 - imageHeight / 2
                theImage.source = imageUrl
                startAni = true
                theImage.x = variables.curWidW / 2 - imageWidth / 2
            }
        }

        function showOrigImage(imageUrl,imageWidth,imageHeight) {
//            sourceSize = Qt.size(imageWidth  ,imageHeight )
            theImage.source = ""
            startAni = false
            theImage.x = variables.curWidW / 2 - imageWidth / 2
            theImage.y = variables.curWidH / 2 - imageHeight / 2
            theImage.source = imageUrl
        }

        Connections {
            target: variables
            onPainterTypeChanged: {
                if (variables.painterType == -1) {
                    return;
                }
                theImage.paintCanvas()
            }
            onPainterReleaseChanged:{
                theImage.paintCanvas()
            }
            onCoreNeedPaintChanged: {
                theImage.paintCanvas()
            }
            onNeedExitPaintChanged: {
                exitSign()
                variables.signMouseStart = false
            }

            onOperateImageWayChanged: {
                if (variables.operateImageWay == 0) {
                    //放开拖拽和缩放
                    pincharea.enabled = true
                    mousearea.enabled = true
                    needWheelReact = true
                }
                if (variables.operateImageWay == 15 || variables.operateImageWay == 9 || variables.operateImageWay == 10) {
                    if (variables.operateImageWay == 15) {
                        variables.signMouseStart = true
                    }
                    mousearea.enabled = true
                } /*else {
                    area.enabled = false
                }*/
            }
        }
        function createTextObj(mx,my) {
            if (variables.painterType === 6) {
                var component61 = Qt.createComponent("qrc:/signcanvas/TextEditPage.qml");
                if (component61.status === Component.Ready) {
                    var rect61 = component61.createObject(imageNormal,{imageText:theImage});
                    variables.objList[variables.operateTime++] = rect61
                    variables.objList[variables.objList.length - 1].setProptyReact(mx,my)
                }
            }
        }

        Component.onCompleted: {
            variables.painterRectSignal.connect(sendRectInfo)
            variables.painterCircleSignal.connect(sendCircleInfo)
            variables.painterLinePaint.connect(sendLineInfo)
            variables.painterArrowPaint.connect(sendArrowInfo)
            variables.painterPencilPaint.connect(sendPencilInfo)
            variables.painterMarkPaint.connect(sendMarkInfo)
            variables.painterTextPaint.connect(sendTextInfo)
            variables.painterBlurPaint.connect(sendBlurInfot)
            variables.painterTextChanged.connect(createTextObj)
            backScale = theImage.scale
            //            backX = theImage.realCurX
            //            backY = theImage.realCurY
            backX = imageRealLeftX
            backY = imageRealLeftY
        }

        function paintCanvas() {
            if (variables.painterType == -1) {
                mousearea.enabled = true
                return
            } else {
                mousearea.enabled = false
            }

            if (variables.objList.length > 0) {
                if (variables.objList[variables.objList.length - 1].hasPaint === false) {
                    variables.objList[variables.objList.length - 1].clearMask()
                    if (variables.operateTime !== 0) {
                        --variables.operateTime
                    }
                }
            }
            //避免最后一个text没有将绘制的信号发出
            if (variables.painterType !== 6) {
                if (variables.objList.length > 0) {
                    if (variables.objList[variables.objList.length - 1].objectName === "textEditPage") {
                        variables.objList[variables.objList.length - 1].sendTextOperate()
                    }
                }
            }

            if (variables.painterType === 0) {
                var component0 = Qt.createComponent("qrc:/signcanvas/RectanglePage.qml");
                if (component0.status === Component.Ready) {
                    var rect0 = component0.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect0
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 1) {
                var component1 = Qt.createComponent("qrc:/signcanvas/CirclePage.qml");
                if (component1.status === Component.Ready) {
                    var rect1 = component1.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect1
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 2) {
                var component2 = Qt.createComponent("qrc:/signcanvas/StraightLinePage.qml");
                if (component2.status === Component.Ready) {
                    var rect2 = component2.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect2
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 3) {
                var component3 = Qt.createComponent("qrc:/signcanvas/LineArrowPage.qml");
                if (component3.status === Component.Ready) {
                    var rect3 = component3.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect3
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 4) {
                var component4 = Qt.createComponent("qrc:/signcanvas/PainterPenPage.qml");
                if (component4.status === Component.Ready) {
                    var rect4 = component4.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect4
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 5) {
                var component5 = Qt.createComponent("qrc:/signcanvas/LinePage.qml");
                if (component5.status === Component.Ready) {
                    var rect5 = component5.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect5
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                }
            }
            if (variables.painterType === 6) {
                var component6 = Qt.createComponent("qrc:/signcanvas/TextEditPage.qml");
                if (component6.status === Component.Ready) {
                    var rect6 = component6.createObject(imageNormal,{imageText:theImage});
                    variables.objList[variables.operateTime++] = rect6
                }
            }
            if (variables.painterType === 7) {
                var component7 = Qt.createComponent("qrc:/signcanvas/FuzzyPage.qml");
                if (component7.status === Component.Ready) {
                    var rect7 = component7.createObject(imageNormal,{imageTarget:theImage});
                    variables.objList[variables.operateTime++] = rect7
                    variables.objList[variables.objList.length - 1].forceActiveFocus()
                    //                    variables.objList[variables.objList.length - 1].setProptyReact(curMouseX,curMouseY)
                }
            }
        }
    }

    PinchArea {
        id: pincharea
        anchors.fill: theImage
        scale: theImage.scale
        rotation: theImage.rotation
        enabled: true

        property real initialScale
        onPinchStarted: {
            initialScale = theImage.curScale
        }
        onPinchUpdated: {
            // 关闭动画
            startAni = false
            if (!imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(pincharea,pinch.center.x, pinch.center.y), undefined, false, false, true, (initialScale*pinch.scale)/theImage.curScale)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(pincharea, mainWindow.width/2, mainWindow.height/2), undefined, false, false, true,(initialScale*pinch.scale)/theImage.curScale)
                theImage.imagePostionReset()
            }
            // 开启动画
            startAni = true
        }

        MouseArea {
            id: mousearea
            enabled: mouseareaEnabeled
            anchors.fill: parent
            drag.target: theImage
            hoverEnabled: false // 必须设置为false，否则，鼠标的坐标抓取的就不是全局的
            propagateComposedEvents: true
            onPressAndHold: {
                variables.mousePos = mousearea.mapToItem(bgimage, Qt.point(mouse.x, mouse.y))
            }
            onPressed: {
                if (variables.navigatorState) {
                    mousearea.cursorShape = Qt.ClosedHandCursor
                }
            }
            onReleased: {

                theImage.curX = theImage.x
                theImage.curY = theImage.y
                theImage.x = Qt.binding(function() { return theImage.curX })
                theImage.y = Qt.binding(function() { return theImage.curY })
                imageBase.controlImageInWid(theImage)
                imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
                imageBase.makeSureNavigatorNeedShow(theImage)
                if (variables.navigatorState) {
                    mousearea.cursorShape = Qt.OpenHandCursor
                }
            }
            onPositionChanged: {
                setDrogLimit()
                imageBase.imageActuralLeftUpPos(theImage,theImage.x ,theImage.y)
                imageBase.makeSureNavigatorNeedShow(theImage)
            }
            onDoubleClicked: {
                mouse.accepted = false
            }

            Connections {
                target: variables
                onMousePosChanged: {
                    if (variables.navigatorState) {
                        mousearea.cursorShape = Qt.OpenHandCursor
                    } else {
                        mousearea.cursorShape = Qt.ArrowCursor
                    }
                }
                onVisibleItemChanged: {
                    if (variables.visibleItem !== "") {
                        mousearea.cursorShape = Qt.ArrowCursor
                    } else {
                        mousearea.cursorShape = Qt.ArrowCursor
                    }
                }
            }
        }
    }

    Connections {
        target: container
        onZoomIn: {
            if (isMoiveRunning()) {
                return
            }
            //某些情况需要静止进行缩放
            if (needWheelReact == false) {
                return
            }
            startAni = false
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, variables.mousePos.x, variables.mousePos.y), undefined, true, false, false)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, mainWindow.width/2, mainWindow.height/2), undefined, true, false, false)
            }
            startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
            imageBase.makeSureNavigatorNeedShow(theImage)
            variables.operateImageWay = 1
            changeOperateWay()
            if (variables.navigatorState) {
                mousearea.cursorShape = Qt.OpenHandCursor
            } else {
                mousearea.cursorShape = Qt.ArrowCursor
            }
            //触摸板缩放之后重置缩放速度
            variables.imageZoomSpeed = 20
        }

        onZoomOut: {
            if (isMoiveRunning()) {
                return
            }
            //某些情况需要静止进行缩放
            if (needWheelReact == false) {
                return
            }
            startAni = false
            if (wheelDelta != undefined && !imageNeedCenterZoom) {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, variables.mousePos.x, variables.mousePos.y), undefined, false, false, false)
            } else {
                imageBase.performZoom(theImage,theImage.mapFromItem(bgimage, mainWindow.width/2, mainWindow.height/2), undefined, false, false, false)
            }
            startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
            imageBase.makeSureNavigatorNeedShow(theImage)
            variables.operateImageWay = 2
            changeOperateWay()
            if (variables.navigatorState) {
                mousearea.cursorShape = Qt.OpenHandCursor
            } else {
                mousearea.cursorShape = Qt.ArrowCursor
            }
            //触摸板缩放之后重置缩放速度
            variables.imageZoomSpeed = 20
        }
        onRotate: {
            if (isMoiveRunning()) {
                return
            }
            theImage.rotateTo += deg
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }

        onMirrorH: {
            if (isMoiveRunning()) {
                return
            }
            var old = theImage.mirror
            theImage.mirror = !old
            startAni = false
            if (theImage.rotateTo / variables.angle90 % 2 != 0) {
                theImage.rotateTo += variables.angle180
            }
            startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }
        onMirrorV: {
            if (isMoiveRunning()) {
                return
            }
            var old = theImage.mirror
            theImage.mirror = !old
            startAni = false
            if (theImage.rotateTo / variables.angle90 % 2 == 0) {
                theImage.rotateTo += variables.angle180
            }
            startAni = true
            imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        }
    }
    Connections {
        target: variables
        onOperateImageWayChanged: {
            if (variables.operateImageWay === 3) {
                //逆时针旋转
                startAni = true
                container.rotate(-variables.angle90)
                theImage.imagePostionReset()
                startAni = false
            } else if (variables.operateImageWay === 4) {
                //顺时针旋转
                startAni = true
                container.rotate(variables.angle90)
                theImage.imagePostionReset()
                startAni = false
            } else if (variables.operateImageWay === 5) {
                //水平
                container.mirrorH()
                theImage.imagePostionReset()
            } else if (variables.operateImageWay === 6) {
                //垂直
                container.mirrorV()
                theImage.imagePostionReset()
            }
        }
    }

    function operateImage(operateWay) {

        if (container.imageType === 1) {
            return
        }
        variables.operateWay = operateWay
        if (operateWay === 1) {
            //放大
            container.zoomIn(undefined)
        } else if (operateWay === 2) {
            //缩小
            container.zoomOut(undefined)
        }/* else if (operateWay === 3) {
            //逆时针旋转
            container.rotate(-variables.angle90)
            theImage.imagePostionReset()
        } else if (operateWay === 4) {
            //顺时针旋转
            container.rotate(variables.angle90)
            theImage.imagePostionReset()
        } else if (operateWay === 5) {
            //水平
            container.mirrorH()
            theImage.imagePostionReset()
        } else if (operateWay === 6) {
            //垂直
            container.mirrorV()
            theImage.imagePostionReset()
        }*/
        //ocr和裁剪
        if (operateWay === 9 || operateWay === 10) {
            //禁止拖拽和缩放
            pincharea.enabled = false
            mousearea.enabled = false
            needWheelReact = false
        } else {
            //放开拖拽和缩放
            pincharea.enabled = true
            mousearea.enabled = true
            needWheelReact = true
        }
        //切图
        if (operateWay === 12 || operateWay === 17 || operateWay === 18) {
            if (variables.changImageUseAnimation && (operateWay === 17 || operateWay === 18)) {
                startAni = true
            } else {
                startAni = false
            }
            theImage.rotateTo = 0
            theImage.mirror = false
            newImageLoaded = true
            theImage.imagePostionReset()
        } else {
            newImageLoaded = false
        }
        //显示原图大小
        if (operateWay === 8) {
            theImage.curScale = 1
            theImage.imagePostionReset()
        }
    }
    //判断当前的图片加载模式，防止影响其他信号的接收
    function isMoiveRunning() {

        if (container.imageType === 1) {
            return true
        } else {
            return false
        }
    }
    //返回所需要相对于窗口的X和Y
    function getLeftUpPosAccordingWidX() {

        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        return imageBase.getLeftUpPosAccordingWidX(theImage)

    }
    function getLeftUpPosAccordingWidY() {

        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        return imageBase.getLeftUpPosAccordingWidY(theImage)
    }
    //返回所需要相对于窗口的在当前缩放比下的可视区域的高度和宽度
    function getVisibleSizeAccordingWidW() {

        return imageBase.getVisibleSizeAccordingWidW(theImage)

    }
    function getVisibleSizeAccordingWidH() {

        return imageBase.getVisibleSizeAccordingWidH(theImage)
    }
    //返回相对于原图的位置
    function getLeftUpPosAccordingImageX() {
        return imageVisibleLeftUpX / theImage.curScale
    }
    function getLeftUpPosAccordingImageY() {
        return imageVisibleLeftUpY / theImage.curScale
    }

    function operateNavigatorChangeImagePos(leftUpPoint) {
        imageBase.operateNavigatorChangeImagePos(leftUpPoint,theImage)
    }

    function setDrogLimit() {
        if(Math.abs(theImage.rotateTo % variables.angle360) == variables.angle90 || Math.abs(theImage.rotateTo % variables.angle360) == variables.angle270) {
            if (theImage.width * theImage.scale >= variables.curWidH) {
                if (theImage.height * theImage.scale >= variables.curWidW) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.YAxis
                    mouseDragType = 1
                }
            } else if (theImage.height * theImage.scale >= variables.curWidW) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.XAxis
                mouseDragType = 0
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        } else {
            if (theImage.width * theImage.scale >= variables.curWidW) {
                if (theImage.height * theImage.scale >= variables.curWidH) {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAndYAxis
                    mouseDragType = 2
                } else {
                    mousearea.enabled = true
                    mousearea.drag.axis= Drag.XAxis
                    mouseDragType = 0
                }
            } else if (theImage.height * theImage.scale >= variables.curWidH) {
                mousearea.enabled = true
                mousearea.drag.axis= Drag.YAxis
                mouseDragType = 1
            } else {
                mousearea.enabled = false
                mouseDragType = -1
            }
        }
        if(!variables.navigatorState) {
            mousearea.enabled = false
        } else {
            mousearea.enabled = true
        }
    }
    function adjustImagePostion() {
        setDrogLimit()
        imageBase.controlImageInWid(theImage)
    }
    function updatePosAccordingToWidSizeChange() {
        imageBase.imageActuralLeftUpPos(theImage,theImage.curX,theImage.curY)
        imageBase.makeSureNavigatorNeedShow(theImage)
        theImage.imagePostionReset()
    }

    function undoSignOperate() {
        if (variables.objList.length == 0) {
            variables.coreNeedPaint = !variables.coreNeedPaint
            return
        }
        for (var i = variables.objList.length - 1; i >= 0; i--) {
            if (variables.objList[i].hasPaint == false) {
                variables.objList[i].clearMask()
                variables.objList.pop()
                variables.operateTime -= 1
            }  else {
                break
            }
        }
        if (variables.objList.length == 0) {
            variables.coreNeedPaint = !variables.coreNeedPaint
            return
        }
        variables.objList[variables.objList.length - 1].clearMask()
        variables.objList.pop()
        variables.operateTime -= 1
        variables.coreNeedPaint = !variables.coreNeedPaint

    }
    function exitSign() {
        if (variables.objList.length == 0) {
            return
        }
        
        for (var i = variables.objList.length - 1; i >= 0; i--) {
            variables.objList[i].clearMask()
            variables.objList.pop()
            variables.operateTime -= 1
        }
    }

    Navigator {
        id: navi
        visible: variables.navigatorState
        width: 200
        height: 133
        targetImage: theImage
        targetImageUrl: theImage.source
        visibleWidth: getVisibleSizeAccordingWidW()
        visibleHeight: getVisibleSizeAccordingWidH()
        visibleLeftUpX: imageVisibleLeftUpX
        visibleLeftUpY: imageVisibleLeftUpY
        anchors {
            bottom: parent.bottom
            bottomMargin: 52
            right: parent.right
            rightMargin: 7
        }
    }
}
