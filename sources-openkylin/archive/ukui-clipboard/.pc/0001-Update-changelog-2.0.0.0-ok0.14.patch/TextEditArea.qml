import QtQuick 2.15
import QtQuick.Controls 2.15
import ContentMenu 1.0

Item {
    id: root
    property int visibleWidth: 0
    property int visibleHeight: 0
    property int textFormat: Qt.RichText
    Rectangle {
        width: parent.visibleWidth
        height: parent.visibleHeight
        anchors.horizontalCenter: parent.horizontalCenter
        color: "transparent"

        Flickable {
            id: flick
            anchors.fill: parent
            contentWidth: textEdit.paintedWidth
            contentHeight: textEdit.paintedHeight
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

            TextEdit {
                id: textEdit
                focus: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                text: ""
                textFormat: root.textFormat
                onCursorRectangleChanged: {
                    flick.ensureVisible(cursorRectangle)
                }
                Connections {
                    target: backend
                    function onPlainTextChanged() {
                        textEdit.text = backend.plainText
                    }
                }
                Component.onCompleted: {
                    // OCR识别的文本不会特别长,所以直接调用后台接口获取文本内容即可
                    if (backend.originalType === ContentType.Image && backend.displayingType === ContentType.PlainText) {
                        text = backend.plainText
                    } else {
                        this.clear()
                        this.paste()
                    }
                    flick.contentX = 0
                    flick.contentY = 0
                }
            }

            function ensureVisible(r) {
                if (contentX >= r.x)
                    contentX = r.x;
                else if (contentX + width <= r.x + r.width)
                    contentX = r.x + r.width - width;
                if (contentY >= r.y)
                    contentY = r.y;
                else if (contentY + height <= r.y + r.height)
                    contentY = r.y + r.height - height;
            }
        }
    }

    function copyToClipboard() {
        textEdit.selectAll()
        textEdit.copy()
    }
}
