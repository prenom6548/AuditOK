import QtQuick 2.15
import QtGraphicalEffects 1.15
import "."
import org.ukui.desktopclock 1.0

Item {
    id: root
    width: 209
    height: 96

    readonly property bool widgetManagerPreviewShell: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode

    FontLoader {
        id: dmSerifDisplayFont
        source: "qrc:/org/ukui/desktopclock/fonts/DMSerifDisplay-Regular.ttf"
    }

    readonly property int previewTickMs: 1000
    readonly property int mediumTimeFontPx: Math.round(Math.min(34, Math.max(22, height * 0.30)))
    readonly property int mediumDateFontPx: 9
    readonly property string fontMediumTime: (dmSerifDisplayFont.status === FontLoader.Ready)
        ? dmSerifDisplayFont.name
        : "DM Serif Display"

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

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 14
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            spacing: Math.max(6, width * 0.03)

            Column {
                width: parent.width - parent.spacing - mediumDial.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: Math.max(3, parent.height * 0.03)

                Text {
                    text: Qt.locale().name.indexOf("zh") === 0 ? qsTr("Beijing Time") : qsTr("Local Time")
                    color: "#A2A2A2"
                    font.pixelSize: Math.max(8, Math.round(parent.width * 0.065))
                    font.bold: true
                }

                Text {
                    text: Qt.formatTime(clockNow, "HH:mm")
                    font.family: root.fontMediumTime
                    font.pixelSize: mediumTimeFontPx
                    font.bold: false
                    color: "#3F3F3F"
                }
                Row {
                    spacing: Math.max(6, width * 0.05)
                    Text {
                        text: Qt.formatDate(clockNow, "dddd")
                        font.pixelSize: mediumDateFontPx
                        font.bold: true
                        color: "#3F3F3F"
                    }
                    Text {
                        text: Qt.formatDate(clockNow, "M月d日")
                        font.pixelSize: mediumDateFontPx
                        color: "#4B4B4B"
                    }
                }
            }

            Item {
                id: mediumDial
                width: Math.min(parent.height - 2, parent.width * 0.44)
                height: width
                anchors.verticalCenter: parent.verticalCenter

                AnalogClockFace {
                    id: dial
                    anchors.fill: parent
                    sizeMode: "medium"
                    hour: clockNow.getHours()
                    minute: clockNow.getMinutes()
                    second: clockNow.getSeconds()
                }

                Text { text: "12"; anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; anchors.topMargin: Math.round(parent.height * 0.14); color: "#22252D"; font.pixelSize: Math.max(8, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "3"; anchors.verticalCenter: parent.verticalCenter; anchors.right: parent.right; anchors.rightMargin: Math.round(parent.width * 0.14); color: "#22252D"; font.pixelSize: Math.max(8, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "6"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: Math.round(parent.height * 0.14); color: "#22252D"; font.pixelSize: Math.max(8, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "9"; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: Math.round(parent.width * 0.14); color: "#22252D"; font.pixelSize: Math.max(8, Math.round(parent.width * 0.14)); font.bold: true }
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
