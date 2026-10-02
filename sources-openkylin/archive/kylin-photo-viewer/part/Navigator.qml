import QtQuick 2.12

Item {
    property url targetImageUrl
    property Image targetImage
    property bool imageNeedNavi: false
    property bool enableRefresh: true
    property real imgLeft: 0
    property real imgTop: 0
    property real imgRight: 0
    property real imgBottom: 0
    property double visibleWidth: 0
    property double visibleHeight: 0
    property double visibleLeftUpX: 0
    property double visibleLeftUpY: 0
    property point leftUpPoint: Qt.point(-1, -1)
    property double lastHightLightX: 0
    property double lastHightLightY: 0
    property double startPosX: 0
    property double startPosY: 0
    property double endPosX: 0
    property double endPosY: 0
    property color blurryColor: Qt.rgba(0, 0, 0, 0.3)
    width: 200
    height: 133
    visible: true

    Rectangle {
        id: imageRect
        anchors.fill: parent
        color: blurryColor

        Image {
            id: currentImage
            anchors.fill: parent
            asynchronous: true
            cache: false
            fillMode: Image.PreserveAspectFit
            source: parent.parent.targetImageUrl
            onStatusChanged: {
                if (Image.Ready === status) {
                    imgLeft = (currentImage.width - currentImage.paintedWidth) / 2
                    imgTop = (currentImage.height - currentImage.paintedHeight) / 2
                    imgRight = imgLeft + currentImage.paintedWidth
                    imgBottom = imgTop + currentImage.paintedHeight
                    refreshNaviHightLight()
                }
            }
        }
    }

    Rectangle{
        id: topRect
        anchors{
            top: parent.top
            left: parent.left
        }
        width: parent.width
        height: hightLight.y
        color: blurryColor
    }

    Rectangle{
        id: leftRect
        anchors{
            top: topRect.bottom
            left: parent.left
        }
        width: hightLight.x
        height: parent.height - topRect.height - bottomRect.height
        color: blurryColor
    }

    Rectangle{
        id: rightRect
        anchors{
            top: topRect.bottom
            right: parent.right
        }
        width: parent.width - leftRect.width - hightLight.width
        height: leftRect.height
        color: blurryColor
    }

    Rectangle{
        id: bottomRect
        anchors{
            bottom: parent.bottom
            left: parent.left
        }
        width: parent.width
        height: parent.height - topRect.height - hightLight.height
        color: blurryColor
    }

    Rectangle {
        id: hightLight
        opacity: 0.4
        color: Qt.rgba(255, 255, 255, 0)
        border.width: 1
        border.color: "white"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: false
        drag.target: hightLight
        drag.axis: Drag.XAndYAxis
        drag.minimumX: imgLeft
        drag.maximumX: imgRight - hightLight.width
        drag.minimumY: imgTop
        drag.maximumY: imgBottom - hightLight.height

        onPressed: {
            clickNavigatorUpdateImagePos(mouseX, mouseY)
            mouseArea.cursorShape = Qt.ClosedHandCursor
        }
        onReleased: {
            if ((mouseX >= startPosX && mouseX <= endPosX)
                    && (mouseY >= startPosY && mouseY <= endPosY)) {
                mouseArea.cursorShape = Qt.OpenHandCursor
            } else {
                mouseArea.cursorShape = Qt.ArrowCursor
            }
        }
        onPositionChanged: {
            if (mouseArea.pressed) {
                clickNavigatorUpdateImagePos(mouseX, mouseY)
            } else {
                if ((mouseX >= startPosX && mouseX <= endPosX)
                        && (mouseY >= startPosY && mouseY <= endPosY)) {
                    mouseArea.cursorShape = Qt.OpenHandCursor
                } else {
                    mouseArea.cursorShape = Qt.ArrowCursor
                }
            }
        }
    }

    Connections {
        target: variables
        onNeedShowNavitor: refreshNaviHightLight()
    }

    function refreshNaviHightLight()
    {
        // 图片实际绘制大小
        var paintedWidth = targetImage.paintedWidth * targetImage.scale
        var paintedHeight = targetImage.paintedHeight * targetImage.scale
        // 导航栏高亮区域
        hightLight.width = currentImage.paintedWidth * visibleWidth / paintedWidth
        hightLight.height = currentImage.paintedHeight * visibleHeight / paintedHeight
        if (hightLight.width > currentImage.paintedWidth) {
            hightLight.width = currentImage.paintedWidth
        }
        if (hightLight.height > currentImage.paintedHeight) {
            hightLight.height = currentImage.paintedHeight
        }
        hightLight.x = visibleLeftUpX / targetImage.scale * (currentImage.paintedWidth / targetImage.width) + imgLeft
        hightLight.y = visibleLeftUpY / targetImage.scale * (currentImage.paintedHeight / targetImage.height) + imgTop
        // 限制鼠标点击或移动的范围
//        hightLight.x = Math.max(imgLeft, Math.min(hightLight.x, imgRight - hightLight.width))
//        hightLight.y = Math.max(imgTop, Math.min(hightLight.y, imgBottom - hightLight.height))

        getHighLightRegion()

        lastHightLightX = hightLight.x - imgLeft
        lastHightLightY = hightLight.y - imgTop
    }

    function clickNavigatorUpdateImagePos(curClickPosX, curClickPosY)
    {
        hightLight.x = Math.max(curClickPosX - hightLight.width / 2, 0)
        hightLight.y = Math.max(curClickPosY - hightLight.height / 2, 0)
        // 限制鼠标点击或移动的范围
        hightLight.x = Math.max(imgLeft, Math.min(hightLight.x, imgRight - hightLight.width)) - imgLeft
        hightLight.y = Math.max(imgTop, Math.min(hightLight.y, imgBottom - hightLight.height)) - imgTop
        // 计算与上次高亮位置偏移量
        leftUpPoint.x = (hightLight.x - lastHightLightX) / (currentImage.paintedWidth / targetImage.width) * targetImage.scale
        leftUpPoint.y = (hightLight.y - lastHightLightY) / (currentImage.paintedHeight / targetImage.height) * targetImage.scale
        // 更新图片位置
        imageBase.operateNavigatorChangeImagePos(leftUpPoint,targetImage)

        getHighLightRegion()

        lastHightLightX = hightLight.x - imgLeft
        lastHightLightY = hightLight.y - imgTop
    }

    function getHighLightRegion()
    {
        startPosX = hightLight.x
        startPosY = hightLight.y
        endPosX = hightLight.x + hightLight.width
        endPosY = hightLight.y + hightLight.height
    }
}
