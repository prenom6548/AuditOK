import QtQuick 2.15
import QtGraphicalEffects 1.15
import "."
import org.ukui.desktopclock 1.0

Item {
    id: root
    width: 209
    height: 209

    readonly property bool widgetManagerPreviewShell: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode

    // 与表盘秒针一致，需每秒刷新 clockNow
    readonly property int previewTickMs: 1000
    readonly property string fontAnalogDigits: "OPPOSans"
    // 与 Canvas 8px 边距对齐的可视边长；略大于原 0.15+8，让数字离刻度更靠内
    readonly property int largePreviewDigitInset: Math.round((Math.min(width, height) - 16) * 0.15) + 12

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

        AnalogClockFace {
            id: dial
            anchors.fill: parent
            anchors.margins: 8
            sizeMode: "large"
            hour: clockNow.getHours()
            minute: clockNow.getMinutes()
            second: clockNow.getSeconds()
        }

        Text {
            text: "12"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: root.largePreviewDigitInset
            color: "#2F3440"
            font.family: root.fontAnalogDigits
            font.pixelSize: 28
            font.bold: true
        }
        Text {
            text: "3"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: root.largePreviewDigitInset
            color: "#2F3440"
            font.family: root.fontAnalogDigits
            font.pixelSize: 28
            font.bold: true
        }
        Text {
            text: "6"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: root.largePreviewDigitInset
            color: "#2F3440"
            font.family: root.fontAnalogDigits
            font.pixelSize: 28
            font.bold: true
        }
        Text {
            text: "9"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: root.largePreviewDigitInset
            color: "#2F3440"
            font.family: root.fontAnalogDigits
            font.pixelSize: 28
            font.bold: true
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
