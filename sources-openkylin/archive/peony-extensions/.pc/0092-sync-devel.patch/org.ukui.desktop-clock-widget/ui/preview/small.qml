import QtQuick 2.15
import QtGraphicalEffects 1.15
import "."
import org.ukui.desktopclock 1.0

Item {
    id: root
    width: 96
    height: 96

    // 桌面小插件管理器已对预览包一层圆角 OpacityMask；此处再套一层会在四角产生深色描边。
    readonly property bool widgetManagerPreviewShell: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode

    // 与表盘秒针一致，需每秒刷新 clockNow
    readonly property int previewTickMs: 1000
    readonly property string fontAnalogDigits: "OPPOSans"

    property date clockNow: new Date()

    Rectangle {
        id: cornerMask
        anchors.fill: parent
        radius: 12
        color: "white"
        visible: false
    }

    Item {
        id: previewContent
        anchors.fill: parent
        visible: widgetManagerPreviewShell

        Rectangle {
            anchors.fill: parent
            color: "#F4F5F7"
        }

        Item {
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height) - 8
            height: width

            AnalogClockFace {
                id: dial
                anchors.fill: parent
                sizeMode: "small"
                hour: clockNow.getHours()
                minute: clockNow.getMinutes()
                second: clockNow.getSeconds()
            }

            Text {
                text: "12"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: Math.round(parent.height * 0.12)
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                text: "3"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Math.round(parent.width * 0.12)
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                text: "6"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Math.round(parent.height * 0.12)
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                text: "9"
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: Math.round(parent.width * 0.12)
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: 12
                font.bold: true
            }
        }
    }

    // 仅在非「桌面小插件管理器」预览时才实例化 OpacityMask。
    // OpacityMask 内部 SourceProxy 会强制把 source（previewContent）启用为 layer，
    // 配合 cached: true 会冻结纹理，使每秒变化的秒针绑定无法触发 QQuickWindow 重绘，
    // 表现为秒针在管理器预览中"卡住"。用 Loader.active 控制实例化时机可避免该副作用。
    Loader {
        anchors.fill: parent
        active: !widgetManagerPreviewShell
        sourceComponent: OpacityMask {
            anchors.fill: parent
            source: previewContent
            maskSource: cornerMask
            cached: true
        }
    }

    Timer {
        interval: previewTickMs
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: clockNow = new Date()
    }
}
