// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ItemDelegate {
    id: root

    property var view: null
    property int selectionMode: 0
    property bool isSingleItem: false
    signal checkedChanged(bool checked)

    property alias title: titleLabel.text
    property alias subTitle: subTitleLabel.text
    property alias bodyText: bodyLabel.text
    property alias iconSource: iconItem.source
    property bool isGridMode: false

    function checkElide() {
        return titleLabel.implicitWidth > titleLabel.width || subTitleLabel.implicitWidth > subTitleLabel.width || bodyLabel.implicitWidth > bodyLabel.width;
    }

    Item {
        anchors.fill: parent
        anchors.margins: isGridMode ? 6 : 0

        Rectangle {
            anchors.fill: parent
            property bool isActive: (root.selectionMode === 1) ? root.checked : root.highlighted

            color: isGridMode ? "transparent" : (isActive ? gsettingsManager.highLight : (mouseArea.isHoveredInViewport ? "#bdbebf" : "transparent"))
            radius: 8
            border.width: isGridMode ? (isActive ? 2 : (mouseArea.isHoveredInViewport ? 2 : 1)) : 0
            border.color: isGridMode ? (isActive ? gsettingsManager.highLight : "#bdbebf") : "transparent"
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 4
                Layout.leftMargin: 8
                Layout.rightMargin: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Image {
                        id: iconItem

                        visible: source != ""
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            id: titleLabel

                            visible: text !== ""
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            horizontalAlignment: (!iconItem.visible && isSingleItem) ? Text.AlignHCenter : Text.AlignLeft
                            verticalAlignment: Text.AlignTop
                            font.bold: true
                        }

                        Label {
                            id: subTitleLabel

                            visible: text !== ""
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            horizontalAlignment: (!iconItem.visible && isSingleItem) ? Text.AlignHCenter : Text.AlignLeft
                            verticalAlignment: Text.AlignTop
                            opacity: 0.9
                        }
                    }
                }

                Label {
                    id: bodyLabel

                    visible: text !== ""
                    Layout.fillWidth: true
                    horizontalAlignment: (!iconItem.visible && isSingleItem) ? Text.AlignHCenter : Text.AlignLeft
                    wrapMode: Text.WordWrap
                }
            }
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true

            property bool isHoveredInViewport: false

            onPositionChanged: {
                if (root.view) {
                    var posInView = root.view.mapFromItem(mouseArea, mouse.x, mouse.y);
                    var isInsideView = (posInView.x >= 0 && posInView.x <= root.view.width && posInView.y >= 0 && posInView.y <= root.view.height);
                    mouseArea.isHoveredInViewport = isInsideView && mouseArea.containsMouse;
                } else {
                    mouseArea.isHoveredInViewport = mouseArea.containsMouse;
                }
            }

            onExited: {
                mouseArea.isHoveredInViewport = false;
            }

            onClicked: {
                if (root.selectionMode === 1) {
                    root.checked = !root.checked;
                    root.checkedChanged(root.checked);
                } else {
                    if (root.view) {
                        root.view.currentIndex = index;
                    }
                }
            }
        }
    }

    background: Rectangle {
        color: "transparent"
    }

    ToolTip {
        visible: root.checkElide() && mouseArea.isHoveredInViewport
        text: [titleLabel.text, subTitleLabel.text, bodyLabel.text].filter(t => t !== "").join("\n")
    }
}
