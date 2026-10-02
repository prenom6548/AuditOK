import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.peony.desktop 1.0
Item {
    id: resizeHandler
    // target必须为绑定了delegate的widget item，否则拖拽改变大小的功能不会生效
    property Item targetItem

    // 边框宽度和触发区域大小
    property int borderWidth: 10

    // 镜像布局
    property bool mirrored: true

    // 启用/禁用调整大小功能
    property bool resizeEnabled: true

    // 父项的最小尺寸限制
    property int minWidth:DesktopUKUIpluginnManager.minSize(targetItem)
    property int minHeight: DesktopUKUIpluginnManager.minSize(targetItem)

    // 父项的最大尺寸限制，确保不覆盖桌面图标区域
    // 使用安全访问，避免container未初始化时获取不到值
    property int maxWidth: 0//DesktopUKUIpluginnManager.maxWidth(targetItem, screenid)
    property int maxHeight: 0//DesktopUKUIpluginnManager.maxHeight(targetItem, screenid)
    // 在相交时禁用拖拽
    property bool intersected: false

    // 更新计时器
    property Timer delayer: Timer {
        interval: 1000
        onTriggered: {
            requestUpdateConfig()
        }
    }

    property bool pressed: topleftMouseArea.containsPress || topRightMouseArea.containsPress || bottomLeftMouseArea.containsPress || bottomRightMouseArea.containsPress || topMouseArea.containsPress || leftMouseArea.containsPress || bottomMouseArea.containsPress || rightMouseArea.containsPress
    property bool resizing: resizeHandler.presse || topleftMouseArea.drag.active || topRightMouseArea.drag.active || bottomLeftMouseArea.drag.active || bottomRightMouseArea.drag.active || topMouseArea.drag.active || leftMouseArea.drag.active || bottomMouseArea.drag.active || rightMouseArea.drag.active

    signal requestUpdateConfig()

    anchors.fill: parent ? parent : undefined

    // 边框矩形 - 提供可视化修饰
    Rectangle {
        id: borderRect
        anchors.centerIn: parent
        width: borderWidth * 2 + parent.width
        height: borderWidth * 2 + parent.height
        color: "transparent"
        border.color: "transparent"
        border.width: borderWidth

        // 四个角部调整区域
        Rectangle {
            id: topLeftCorner
            anchors.top: parent.top
            anchors.left: parent.left
            width: borderWidth
            height: borderWidth
            color: "transparent"

            MouseArea {
                id: topleftMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                drag{ target: parent; axis: Drag.XAndYAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - borderWidth/2
                        var deltaY = mouseY - borderWidth/2

                        if (mirrored) {
                            deltaX = -deltaX
                        }

                        var newX = Math.min(resizeHandler.parent.x + deltaX,
                                            resizeHandler.parent.x + resizeHandler.width - minWidth)
                        var newY = Math.min(resizeHandler.parent.y + deltaY,
                                            resizeHandler.parent.y + resizeHandler.height - minHeight)

                        var newWidth = mirrored? resizeHandler.parent.width + deltaX :resizeHandler.parent.width - (newX - resizeHandler.parent.x)
                        var newHeight = resizeHandler.parent.height - (newY - resizeHandler.parent.y)

                        if (mirrored) {
                            deltaX = -deltaX
                        }
                        if (newWidth >= minWidth) {
                            if(deltaX <0 && newWidth > maxWidth - 110){
                                return;
                            }
                            if (deltaX >= 0 || !intersected) {
                                if (!mirrored) {
                                    resizeHandler.parent.x = newX
                                    resizeHandler.targetItem.width = newWidth
                                } else {
                                    resizeHandler.targetItem.width = newWidth
                                }
                            }
                        }

                        if (newHeight >= minHeight) {
                            if(deltaX <0 && newHeight > maxHeight - 10){
                                return;
                            }
                            if (deltaY >= 0 || !intersected) {
                                resizeHandler.parent.y = newY
                                resizeHandler.targetItem.height = newHeight
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: topRightCorner
            anchors.top: parent.top
            anchors.right: parent.right
            width: borderWidth
            height: borderWidth
            color: "transparent"

            MouseArea {
                id: topRightMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                drag{ target: parent; axis: Drag.XAndYAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - borderWidth/2
                        var deltaY = mouseY - borderWidth/2
                        var newWidth = resizeHandler.parent.width + deltaX
                        if (mirrored) {
                            deltaX = -deltaX
                        }

                        var newY = Math.min(resizeHandler.parent.y + deltaY,
                                            resizeHandler.parent.y + resizeHandler.parent.height - minHeight)
                        var newHeight = resizeHandler.parent.height - (newY - resizeHandler.parent.y)

                        if (newWidth >= minWidth) {
                            if(deltaX <0 && newWidth > maxWidth - 110){
                                return;
                            }
                            if (!mirrored) {
                                resizeHandler.targetItem.width = newWidth
                            } else if (deltaX >= 0 || !intersected) {
                                var newX = Math.min(resizeHandler.parent.x + deltaX,
                                                    resizeHandler.parent.x + resizeHandler.width - minWidth)
                                resizeHandler.parent.x = newX
                                resizeHandler.targetItem.width = newWidth
                            }
                        }

                        if (newHeight >= minHeight) {
                            if(deltaX <0 && newHeight > maxHeight - 10){
                                return;
                            }
                            if (deltaY >= 0 || !intersected) {
                                resizeHandler.parent.y = newY
                                resizeHandler.targetItem.height = newHeight
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: bottomLeftCorner
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: borderWidth
            height: borderWidth
            color: "transparent"

            MouseArea {
                id: bottomLeftMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                drag{ target: parent; axis: Drag.XAndYAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - width/2
                        var deltaY = mouseY - height/2

                        if (mirrored) {
                            deltaX = -deltaX
                        }

                        var newX = Math.min(resizeHandler.parent.x + deltaX,
                                            resizeHandler.parent.x + resizeHandler.parent.width - minWidth)
                        var newHeight = resizeHandler.parent.height + deltaY

                        var newWidth = mirrored? resizeHandler.parent.width + deltaX :resizeHandler.parent.width - (newX - resizeHandler.parent.x)

                        if (mirrored) {
                            deltaX = -deltaX
                        }
                        if (newWidth >= minWidth) {
                            if(newWidth > maxWidth - 110){
                                return;
                            }
                            if (deltaX >= 0 || !intersected) {
                                if (!mirrored) {
                                    resizeHandler.parent.x = newX
                                    resizeHandler.targetItem.width = newWidth
                                } else {
                                    resizeHandler.targetItem.width = newWidth
                                }
                            }
                        }

                        if (newHeight >= minHeight) {
                            if(newHeight > maxHeight - 10){
                                return;
                            }
                            if (deltaY <= 0 || !intersected) {
                                resizeHandler.targetItem.height = newHeight
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: bottomRightCorner
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: borderWidth
            height: borderWidth
            color: "transparent"

            MouseArea {
                id: bottomRightMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                drag{ target: parent; axis: Drag.XAndYAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - width/2
                        var deltaY = mouseY - height/2
                        var newWidth = resizeHandler.parent.width + deltaX
                        var newHeight = resizeHandler.parent.height + deltaY

                        if (newWidth >= minWidth) {
                            if(/*deltaX <0 &&*/ newWidth > maxWidth - 110){
                                return;
                            }
                            if (deltaX <= 0 || !intersected) {
                                if (!mirrored) {
                                    resizeHandler.targetItem.width = newWidth
                                } else {
                                    var newX = Math.min(resizeHandler.parent.x - deltaX,
                                                        resizeHandler.parent.x + resizeHandler.width - minWidth)
                                    resizeHandler.parent.x = newX
                                    resizeHandler.targetItem.width = newWidth
                                }
                            }
                        }

                        if (newHeight >= minHeight) {
                            if(/*deltaY <0 &&*/ newHeight > maxHeight - 10){
                                return;
                            }
                            if (deltaY <= 0 || !intersected) {
                                resizeHandler.targetItem.height = newHeight
                            }
                        }
                    }
                }
            }
        }

        // 四边调整区域
        Rectangle {
            id: topEdge
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 2*borderWidth
            height: borderWidth
            color: "transparent"

            property int maxTop: 200

            MouseArea {
                id: topMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                drag{ target: parent; axis: Drag.YAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaY = mouseY - height/2

                        var newY = Math.min(resizeHandler.parent.y + deltaY,
                                            resizeHandler.parent.y + resizeHandler.parent.height - minHeight)
                        var newHeight = resizeHandler.parent.height - (newY - resizeHandler.parent.y)

                        if (newHeight >= minHeight) {
                            if (deltaY < 0 && intersected) {
                                return
                            }

                            resizeHandler.parent.y = newY
                            resizeHandler.targetItem.height = newHeight
                        }
                    }
                }
            }
        }

        Rectangle {
            id: bottomEdge
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 2*borderWidth
            height: borderWidth
            color: "transparent"

            MouseArea {
                id: bottomMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                drag{ target: parent; axis: Drag.YAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaY = mouseY - height/2
                        var newHeight = resizeHandler.parent.height + deltaY
                        var newY = Math.min(resizeHandler.parent.y + deltaY,
                                            resizeHandler.parent.y + resizeHandler.parent.height - minHeight)
                        if (newHeight >= minHeight) {
                            if (deltaY > 0  && (resizeHandler.intersected || newY + newHeight > maxHeight - 10)) {
                                return
                            }

                            resizeHandler.targetItem.height = newHeight
                        }
                    }
                }
            }
        }

        Rectangle {
            id: leftEdge
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: borderWidth
                height: parent.height - 2*borderWidth
            color: "transparent"

            MouseArea {
                id: leftMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                drag{ target: parent; axis: Drag.XAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - width/2
                        var newX
                        var newWidth
                        if (!mirrored) {
                            newX = Math.min(resizeHandler.parent.x + deltaX,
                                                resizeHandler.parent.x + resizeHandler.parent.width - minWidth)
                            newWidth = resizeHandler.parent.width - (newX - resizeHandler.parent.x)

                            if (newWidth >= minWidth) {
                                if (deltaX < 0 && intersected) {
                                    return
                                }

                                resizeHandler.parent.x = newX
                                resizeHandler.targetItem.width = newWidth
                            }
                        } else {
                            newWidth = resizeHandler.parent.width - deltaX
                            if (newWidth >= minWidth) {
                                if (deltaX < 0 && (intersected || newWidth > maxWidth -110)) {
                                    return
                                }

                                //resizeHandler.parent.x = newX
                                resizeHandler.targetItem.width = newWidth
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: rightEdge
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: borderWidth
            height: parent.height - 2*borderWidth
            color: "transparent"

            MouseArea {
                id: rightMouseArea
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                drag{ target: parent; axis: Drag.XAxis }
                onPositionChanged: {
                    if (pressed && resizeEnabled) {
                        var deltaX = mouseX - width/2
                        var newX
                        var newWidth

                        if (!mirrored) {
                            newWidth = resizeHandler.parent.width + deltaX

                            if (newWidth >= minWidth) {
                                if (deltaX > 0 && intersected) {
                                    return
                                }
                                resizeHandler.targetItem.width = newWidth
                            }
                        } else {
                            deltaX = -deltaX
                            newX = Math.min(resizeHandler.parent.x + deltaX,
                                                resizeHandler.parent.x + resizeHandler.parent.width - minWidth)
                            newWidth = resizeHandler.parent.width - (newX - resizeHandler.parent.x)
                            if (newWidth >= minWidth) {
                                if (deltaX < 0 && (intersected || newWidth > maxWidth - 110)) {
                                    return
                                }

                                resizeHandler.parent.x = newX
                                resizeHandler.targetItem.width = newWidth
                            }
                        }
                    }
                }
            }
        }
    }

    function delayUpdateWidgetSizeConfig() {

    }

    onWidthChanged: {
        delayer.start()
    }
    onHeightChanged: {
        delayer.start()
    }
}
