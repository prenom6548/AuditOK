import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls

Item {
    id: root

    function parsePosition(value, fallback) {
        var parsed = parseInt(value, 10)
        return isNaN(parsed) ? fallback : parsed
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Row {
            width: parent.width
            height: 32
            spacing: 12

            Text {
                width: 160
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "Available: " + WorkspaceDemo.available
                color: WorkspaceDemo.available ? "#237804" : "#a8071a"
                font.bold: true
            }

            Text {
                width: 180
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "Initialized: " + WorkspaceDemo.initialized
                color: WorkspaceDemo.initialized ? "#237804" : "#a8071a"
                font.bold: true
            }

            Text {
                width: 220
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: "Count: " + WorkspaceDemo.workspaces.length
            }

            Controls.Button {
                width: 120
                height: parent.height
                text: "Refresh"
                onClicked: WorkspaceDemo.refresh()
            }
        }

        Row {
            width: parent.width
            height: Math.max(0, parent.height - 42)
            spacing: 12

            Rectangle {
                width: parent.width * 0.56
                height: parent.height
                radius: 4
                border.width: 1
                border.color: "#d9d9d9"
                color: "#ffffff"

                ListView {
                    id: workspaceList
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    model: WorkspaceDemo.workspaces
                    spacing: 1

                    delegate: Rectangle {
                        width: workspaceList.width
                        height: 58
                        color: WorkspaceDemo.selectedId === modelData.id ? "#e6f7ff" : mouseArea.containsMouse ? "#f5f5f5" : "#ffffff"
                        border.width: WorkspaceDemo.selectedId === modelData.id ? 1 : 0
                        border.color: "#1890ff"

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: WorkspaceDemo.selectWorkspace(modelData.id)
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 10

                            Text {
                                width: 64
                                height: parent.height
                                verticalAlignment: Text.AlignVCenter
                                text: "#" + modelData.position
                                font.bold: modelData.active
                                color: modelData.active ? "#237804" : "#595959"
                            }

                            Column {
                                width: parent.width - 180
                                height: parent.height
                                spacing: 2

                                Text {
                                    width: parent.width
                                    height: 30
                                    verticalAlignment: Text.AlignBottom
                                    text: modelData.name.length > 0 ? modelData.name : "(empty name)"
                                    elide: Text.ElideRight
                                    font.bold: modelData.active
                                }

                                Text {
                                    width: parent.width
                                    height: 20
                                    verticalAlignment: Text.AlignTop
                                    text: modelData.id
                                    color: "#8c8c8c"
                                    elide: Text.ElideRight
                                    font.pixelSize: 12
                                }
                            }

                            Text {
                                width: 86
                                height: parent.height
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignRight
                                text: modelData.active ? "Active" : ""
                                color: "#237804"
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: workspaceList.count === 0
                        text: WorkspaceDemo.initialized ? "No workspaces" : "Waiting for workspace manager"
                        color: "#8c8c8c"
                    }
                }
            }

            Column {
                width: parent.width * 0.44 - 12
                height: parent.height
                spacing: 10

                Rectangle {
                    width: parent.width
                    height: 216
                    radius: 4
                    border.width: 1
                    border.color: "#d9d9d9"
                    color: "#ffffff"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            width: parent.width
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                            text: "Selected: " + (WorkspaceDemo.hasSelection ? WorkspaceDemo.selectedName : "none")
                            elide: Text.ElideRight
                            font.bold: true
                        }

                        Text {
                            width: parent.width
                            height: 22
                            verticalAlignment: Text.AlignVCenter
                            text: WorkspaceDemo.hasSelection ? WorkspaceDemo.selectedId : ""
                            color: "#8c8c8c"
                            elide: Text.ElideRight
                            font.pixelSize: 12
                        }

                        Row {
                            width: parent.width
                            height: 34
                            spacing: 8

                            Controls.Button {
                                width: 120
                                height: parent.height
                                text: "Activate"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.activateSelectedWorkspace()
                            }

                            Controls.Button {
                                width: 120
                                height: parent.height
                                text: "Delete"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.removeSelectedWorkspace()
                            }
                        }

                        Row {
                            width: parent.width
                            height: 34
                            spacing: 8

                            Controls.TextField {
                                id: renameInput
                                width: parent.width - 128
                                height: parent.height
                                placeholderText: "New name"
                                text: WorkspaceDemo.selectedName
                                selectByMouse: true
                            }

                            Controls.Button {
                                width: 120
                                height: parent.height
                                text: "Rename"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.renameSelectedWorkspace(renameInput.text)
                            }
                        }

                        Row {
                            width: parent.width
                            height: 34
                            spacing: 8

                            Controls.Button {
                                width: 86
                                height: parent.height
                                text: "Move -1"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.moveSelectedWorkspaceBy(-1)
                            }

                            Controls.Button {
                                width: 86
                                height: parent.height
                                text: "Move +1"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.moveSelectedWorkspaceBy(1)
                            }

                            Controls.TextField {
                                id: moveInput
                                width: 70
                                height: parent.height
                                text: WorkspaceDemo.selectedPosition >= 0 ? WorkspaceDemo.selectedPosition : ""
                                selectByMouse: true
                            }

                            Controls.Button {
                                width: 92
                                height: parent.height
                                text: "Move To"
                                enabled: WorkspaceDemo.hasSelection
                                onClicked: WorkspaceDemo.moveSelectedWorkspace(root.parsePosition(moveInput.text, WorkspaceDemo.selectedPosition))
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 104
                    radius: 4
                    border.width: 1
                    border.color: "#d9d9d9"
                    color: "#ffffff"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Row {
                            width: parent.width
                            height: 34
                            spacing: 8

                            Controls.TextField {
                                id: createNameInput
                                width: parent.width - 198
                                height: parent.height
                                placeholderText: "Workspace name"
                                text: "Demo Workspace"
                                selectByMouse: true
                            }

                            Controls.TextField {
                                id: createPositionInput
                                width: 62
                                height: parent.height
                                placeholderText: "pos"
                                text: "-1"
                                selectByMouse: true
                            }

                            Controls.Button {
                                width: 120
                                height: parent.height
                                text: "Create"
                                enabled: WorkspaceDemo.available
                                onClicked: WorkspaceDemo.createWorkspace(createNameInput.text, root.parsePosition(createPositionInput.text, -1))
                            }
                        }

                        Text {
                            width: parent.width
                            height: 28
                            verticalAlignment: Text.AlignVCenter
                            text: "position -1 appends at the end"
                            color: "#8c8c8c"
                            font.pixelSize: 12
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: Math.max(80, parent.height - 340)
                    radius: 4
                    border.width: 1
                    border.color: "#d9d9d9"
                    color: "#ffffff"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 6

                        Row {
                            width: parent.width
                            height: 28
                            spacing: 8

                            Text {
                                width: parent.width - 108
                                height: parent.height
                                verticalAlignment: Text.AlignVCenter
                                text: "Log"
                                font.bold: true
                            }

                            Controls.Button {
                                width: 100
                                height: parent.height
                                text: "Clear"
                                onClicked: WorkspaceDemo.clearLogs()
                            }
                        }

                        ListView {
                            id: logList
                            width: parent.width
                            height: parent.height - 34
                            clip: true
                            model: WorkspaceDemo.logs

                            delegate: Text {
                                width: logList.width
                                text: modelData
                                color: "#595959"
                                wrapMode: Text.WrapAnywhere
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }
        }
    }
}
