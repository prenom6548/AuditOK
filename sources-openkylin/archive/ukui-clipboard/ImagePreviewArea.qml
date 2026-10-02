import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ContentMenu 1.0

Item {
    id: root
    property int visibleWidth: 0
    property int visibleHeight: 0
    Rectangle {
        width: parent.visibleWidth
        height: parent.visibleHeight
        anchors.horizontalCenter: parent.horizontalCenter
        color: "transparent"

        Flickable {
            id: flick
            anchors.fill: parent
            contentWidth: screenshot.paintedWidth
            contentHeight: screenshot.paintedHeight
            boundsMovement: Flickable.StopAtBounds
            clip: true
            ScrollBar.vertical: ScrollBar {
                active: true
                visible: flick.contentHeight > flick.height
            }
            ScrollBar.horizontal: ScrollBar {
                active: true
                visible: flick.contentWidth > flick.width
            }

            // 鼠标事件处理区域
            MouseArea {
                anchors.fill: parent
                onWheel: {
                    if (wheel.modifiers & Qt.ShiftModifier) {
                        flick.contentX -= wheel.angleDelta.y
                    } else {
                        flick.contentY -= wheel.angleDelta.y
                    }
                }
            }

            Image {
                id: screenshot
                x: root.width > paintedWidth ? Math.round((root.width - paintedWidth) / 2) : 0
                y: root.height > paintedHeight ? Math.round((root.height - paintedHeight) / 2) : 0
                cache: false
                fillMode: Image.PreserveAspectFit
                source: "image://inMemoryImages/Image0"
            }
        }
    }
}
