import QtQuick 2.15
import QtQuick.Controls 1.4

Rectangle {
    id: root
    color: "#f2f4f7"

    property var selected: outputMonitor.selectedOutput

    Row {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        Column {
            id: leftPane
            width: Math.max(390, parent.width * 0.36)
            height: parent.height
            spacing: 14
            property real outputPaneHeight: height * 0.52
            property real minimumOutputPaneHeight: 150
            property real minimumLogPaneHeight: 150

            Rectangle {
                id: outputsPane
                width: parent.width
                height: leftPane.outputPaneHeight
                radius: 8
                color: "#ffffff"
                border.color: "#d9dee7"

                Text {
                    id: outputsTitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 14
                    text: "Outputs (" + outputMonitor.outputCount + ")"
                    color: "#20242a"
                    font.pixelSize: 18
                    font.bold: true
                }

                ListView {
                    id: outputList
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: outputsTitle.bottom
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                    anchors.topMargin: 8
                    clip: true
                    spacing: 6
                    model: outputMonitor.outputs

                    delegate: Rectangle {
                        property var outputData: modelData

                        width: outputList.width
                        height: 82
                        radius: 6
                        color: index === outputMonitor.selectedIndex ? "#e8f1ff" : "#f8f9fb"
                        border.color: index === outputMonitor.selectedIndex ? "#4a90e2" : "#e0e4ea"

                        Column {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 3

                            Text {
                                text: outputData.name.length > 0 ? outputData.name : "Unnamed output"
                                color: "#20242a"
                                font.pixelSize: 15
                                font.bold: true
                            }
                            Text {
                                text: "UUID: " + outputData.uuid
                                color: "#687180"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: "Position: " + outputData.position + "    "
                                      + (outputData.enabled ? "Enabled" : "Disabled")
                                color: outputData.enabled ? "#16794c" : "#a34747"
                                font.pixelSize: 12
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: outputMonitor.selectedIndex = index
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: outputMonitor.outputCount === 0
                        text: outputMonitor.available
                              ? "Waiting for outputs…"
                              : "Output manager is unavailable"
                        color: "#7b8491"
                        font.pixelSize: 15
                    }
                }
            }

            Rectangle {
                id: splitter
                width: parent.width
                height: 8
                radius: 4
                color: splitterMouseArea.pressed ? "#4a90e2" : "#c8d0dc"

                MouseArea {
                    id: splitterMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.SizeVerCursor
                    property real previousY: 0

                    onPressed: previousY = mouse.y
                    onPositionChanged: {
                        var delta = mouse.y - previousY
                        var maximumHeight = leftPane.height
                                - leftPane.minimumLogPaneHeight
                                - splitter.height
                                - leftPane.spacing * 2
                        leftPane.outputPaneHeight = Math.max(leftPane.minimumOutputPaneHeight,
                                                              Math.min(maximumHeight,
                                                                       leftPane.outputPaneHeight + delta))
                        previousY = mouse.y
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height - outputsPane.height - splitter.height - leftPane.spacing * 2
                radius: 8
                color: "#ffffff"
                border.color: "#d9dee7"

                Text {
                    id: logTitle
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    text: "Signal log — one block per update cycle"
                    color: "#20242a"
                    font.pixelSize: 15
                    font.bold: true
                }

                ListView {
                    id: logList
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: logTitle.bottom
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                    anchors.topMargin: 6
                    clip: true
                    spacing: 3
                    model: outputMonitor.logEntries
                    onCountChanged: positionViewAtEnd()

                    delegate: Text {
                        property var entry: modelData
                        width: logList.width
                        text: entry.text
                        color: entry.color
                        font.pixelSize: 13
                        font.bold: entry.header
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        Rectangle {
            width: parent.width - parent.children[0].width - parent.spacing
            height: parent.height
            radius: 8
            color: "#ffffff"
            border.color: "#d9dee7"

            Text {
                id: detailTitle
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 16
                text: outputMonitor.selectedIndex >= 0
                      ? "Selected output properties"
                      : "Selected output properties — no output selected"
                color: "#20242a"
                font.pixelSize: 20
                font.bold: true
            }

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: detailTitle.bottom
                anchors.margins: 16
                anchors.topMargin: 5
                text: "available: " + outputMonitor.available
                      + "    initialized: " + outputMonitor.initialized
                      + "    primary: " + outputMonitor.primaryOutputUuid
                color: "#687180"
                font.pixelSize: 14
            }

            Flickable {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: detailTitle.bottom
                anchors.bottom: parent.bottom
                anchors.margins: 16
                anchors.topMargin: 38
                clip: true
                contentWidth: width
                contentHeight: details.implicitHeight

                Column {
                    id: details
                    width: parent.width
                    spacing: 11
                    visible: outputMonitor.selectedIndex >= 0

                    Text { text: "UUID: " + root.selected.uuid; font.pixelSize: 16; font.bold: true }
                    Text { text: "Name: " + root.selected.name }
                    Text { text: "Make / model: " + root.selected.make + " / " + root.selected.model }
                    Text { text: "Serial number: " + root.selected.serialNumber }
                    Text { text: "Description: " + root.selected.description; width: parent.width; wrapMode: Text.Wrap }
                    Rectangle { width: parent.width; height: 1; color: "#e5e7eb" }
                    Text { text: "Physical size: " + root.selected.physicalSize + "    Position: " + root.selected.position }
                    Text { text: "Enabled: " + root.selected.enabled + "    Transform: " + root.selected.transformText + " (" + root.selected.transform + ")    Scale: " + Number(root.selected.scale).toFixed(2) }
                    Text { text: "Power: " + root.selected.power + "    Brightness: " + root.selected.brightness + "    Color temperature: " + root.selected.colorTemp }
                    Text { text: "Capabilities: " + root.selected.capabilities; width: parent.width; wrapMode: Text.Wrap }
                    Rectangle { width: parent.width; height: 1; color: "#e5e7eb" }
                    Text { text: "Current mode: " + root.selected.currentMode; width: parent.width; wrapMode: Text.Wrap }
                    Text { text: "All modes"; font.pixelSize: 16; font.bold: true }

                    Repeater {
                        model: root.selected.modeSummary || []
                        delegate: Text {
                            text: "• " + modelData
                            color: "#4b5563"
                            width: details.width
                            wrapMode: Text.Wrap
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    visible: outputMonitor.selectedIndex < 0
                    text: "Select an Output from the list to inspect its properties."
                    color: "#7b8491"
                    font.pixelSize: 16
                }
            }
        }
    }
}
