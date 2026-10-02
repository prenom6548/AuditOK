import QtQuick 2.15
import QtGraphicalEffects 1.15

Item {
    id: root
    width: 96
    height: 96

    readonly property bool widgetManagerPreviewShell: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode

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
            radius: 12
            color: Qt.rgba(0.10, 0.16, 0.30, 0.92)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.55)
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(72, parent.width - 16)
            height: 22
            radius: 6
            color: Qt.rgba(1, 1, 1, 0.2)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.65)

            Text {
                anchors.centerIn: parent
                text: qsTr("Open")
                color: "white"
                font.pixelSize: 10
                font.bold: true
            }
        }
    }

    OpacityMask {
        visible: !widgetManagerPreviewShell
        anchors.fill: parent
        source: previewContent
        maskSource: cornerMask
        cached: true
    }
}
