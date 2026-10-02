// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.2

Rectangle {
    id: root

    property bool isRTL: Qt.application.layoutDirection === Qt.RightToLeft
    property bool windowIsFinalSize: false

    width: 360
    height: 200
    anchors.fill: parent
    color: gsettingsManager.themeColor
    clip: true
    Component.onCompleted: {
        Qt.callLater(function () {
            controller.setFinalSize(mainLayout.implicitWidth + 64, mainLayout.implicitHeight + 48);
            windowIsFinalSize = true;
        });
    }

    LayoutMirroring.enabled: isRTL
    LayoutMirroring.childrenInherit: true

    Flickable {
        id: flickable
        property int updateContentWidth: 200
        property int updateContentHeight: 100

        Connections {
            target: controller
            function onContentWidthOverflow(width) {
                flickable.updateContentWidth = width;
            }
            function onContentHeightOverflow(height) {
                flickable.updateContentHeight = height;
            }
        }

        anchors.fill: parent
        contentWidth: Math.max(width, updateContentWidth)
        contentHeight: Math.max(height, updateContentHeight)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Item {
            id: rootItem
            anchors.fill: parent

            ColumnLayout {
                id: mainLayout

                anchors.fill: parent
                anchors.margins: 24
                spacing: 8

                ColumnLayout {
                    id: mainItem

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    RowLayout {
                        id: header
                        visible: iconImage.visible | title.visible

                        Layout.fillWidth: true
                        spacing: 8

                        Image {
                            id: iconImage

                            visible: controller.iconSource
                            source: controller.iconSource
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            fillMode: Image.PreserveAspectFit
                        }

                        ColumnLayout {
                            id: title
                            visible: mainTitle.visible | subTitle.visible

                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                id: mainTitle

                                visible: controller.mainTitle
                                text: controller.mainTitle
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignTop
                                font.family: subTitle.font.family
                                font.pointSize: subTitle.font.pointSize
                                font.bold: controller.titleFontBold
                                Layout.minimumWidth: 300
                            }

                            Label {
                                id: subTitle

                                visible: controller.subTitle
                                text: controller.subTitle
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                Layout.maximumWidth: mainTitle.visible ? Math.max(mainTitle.width, 300) : iconImage.visible ? Math.max((header.width - 56), 300) : Math.max(mainItem.width, 300)
                                opacity: controller.subTitleFontLight ? 0.8 : 1
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignTop
                            }
                        }
                    }

                    Label {
                        id: message

                        visible: controller.messageText
                        text: controller.messageText
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.maximumWidth: header.width ? Math.max(header.width, 300) : Math.max(parent.width, 300)
                    }

                    Item {
                        id: detailItem

                        visible: controller.detailsText

                        Layout.fillWidth: true
                        height: Math.min(400, Math.max(100, detailsTextArea.height + 8))
                        Rectangle {
                            anchors.fill: parent

                            color: gsettingsManager.backgroundColor
                            radius: 12
                            opacity: 0.8
                            border.width: 1
                            border.color: Qt.rgba(gsettingsManager.borderColor.r, gsettingsManager.borderColor.g, gsettingsManager.borderColor.b, 0.5)
                        }

                        Flickable {
                            id: detailsFlickable

                            anchors.fill: parent
                            anchors.margins: 4

                            clip: true
                            boundsBehavior: Flickable.StopAtBounds

                            contentWidth: detailsTextArea.width
                            contentHeight: detailsTextArea.height

                            TextArea.flickable: TextArea {
                                id: detailsTextArea

                                text: controller.detailsText
                                wrapMode: Text.WordWrap
                                readOnly: true
                                background: Rectangle {
                                    color: "transparent"
                                }
                                leftPadding: 16
                                rightPadding: 16
                                topPadding: 16
                                bottomPadding: 16
                            }

                            ScrollBar.vertical: ScrollBar {
                                visible: detailsFlickable.contentHeight > detailsFlickable.height

                                contentItem: Rectangle {
                                    implicitWidth: 8
                                    color: "#bdbebf"
                                    opacity: 0.6
                                    radius: 4
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        id: dynamicLayout

                        visible: dynamicModel.rowCount() > 0
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            id: topRepeater

                            model: dynamicModel

                            delegate: NodeDelegate {
                                widgetModel: model.widgetData
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    RowLayout {
                        id: buttonBox

                        visible: controller.buttonsVisible
                        Layout.topMargin: 32
                        Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                        spacing: 8

                        Button {
                            id: denyButton

                            visible: controller.denyButtonVisible
                            text: controller.denyText
                            highlighted: true
                            Layout.minimumHeight: 36
                            Layout.maximumHeight: 36
                            onClicked: {
                                controller.deny();
                            }
                        }

                        Button {
                            id: allowButton

                            visible: controller.allowButtonVisible
                            text: controller.allowText
                            Layout.minimumHeight: 36
                            Layout.maximumHeight: 36
                            onClicked: {
                                controller.allow();
                            }
                        }
                    }
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: verticalBar

            visible: flickable.contentHeight > root.height

            contentItem: Rectangle {
                implicitWidth: 8
                color: "#555659"
                opacity: 0.8
                radius: 4
            }
        }

        ScrollBar.horizontal: ScrollBar {
            id: horizontalBar

            visible: flickable.contentWidth > root.width

            contentItem: Rectangle {
                implicitHeight: 8
                color: "#555659"
                opacity: 0.8
                radius: 4
            }
        }
    }

    Connections {
        target: mainLayout

        function onImplicitHeightChanged() {
            if (windowIsFinalSize) {
                controller.updateWindowSize(mainLayout.implicitWidth + 48, mainLayout.implicitHeight + 48);
            }
        }
        function onImplicitWidthChanged() {
            if (windowIsFinalSize) {
                controller.updateWindowSize(mainLayout.implicitWidth + 48, mainLayout.implicitHeight + 48);
            }
        }
    }
}
