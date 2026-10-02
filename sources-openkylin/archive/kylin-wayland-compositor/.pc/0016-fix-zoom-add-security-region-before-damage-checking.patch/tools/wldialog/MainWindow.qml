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

    width: 360
    height: 200
    anchors.fill: parent
    color: gsettingsManager.themeColor
    clip: true
    Component.onCompleted: {
        Qt.callLater(function () {
            controller.setFinalSize(rootItem.width, rootItem.height);
        });
    }

    LayoutMirroring.enabled: isRTL
    LayoutMirroring.childrenInherit: true

    Flickable {
        id: flickable

        anchors.fill: parent
        contentWidth: rootItem.implicitWidth
        contentHeight: rootItem.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Item {
            id: rootItem

            implicitWidth: mainLayout.implicitWidth + 48
            implicitHeight: mainLayout.implicitHeight + 48
            width: Math.max(implicitWidth, root.width)
            height: Math.max(implicitHeight, root.height)

            ColumnLayout {
                id: mainLayout

                anchors.fill: parent
                anchors.margins: 24
                spacing: 8

                ColumnLayout {
                    height: root.height - 48
                    width: root.width - 48

                    RowLayout {
                        id: header

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

                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                id: mainTitle

                                visible: controller.mainTitle
                                text: controller.mainTitle
                                Layout.fillWidth: true
                                Layout.minimumWidth: 300
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignTop
                                font.family: subTitle.font.family
                                font.pointSize: subTitle.font.pointSize
                                font.bold: controller.titleFontBold
                            }

                            Label {
                                id: subTitle

                                visible: controller.subTitle
                                text: controller.subTitle
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                Layout.maximumWidth: title.width
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
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: Math.min(300, Math.max(100, detailsScrollView.contentHeight))

                        visible: controller.detailsText

                        Rectangle {
                            anchors.fill: parent

                            color: gsettingsManager.backgroundColor
                            radius: 12
                            opacity: 0.8
                            border.width: 1
                            border.color: Qt.rgba(gsettingsManager.borderColor.r, gsettingsManager.borderColor.g, gsettingsManager.borderColor.b, 0.5)
                        }

                        ScrollView {
                            id: detailsScrollView

                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                            anchors.fill: parent
                            anchors.margins: 12
                            contentWidth: width

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    id: detailsLabel

                                    Layout.fillWidth: true
                                    text: controller.detailsText
                                    wrapMode: Text.WordWrap
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

        function onImplicitWidthChanged() {
            controller.updateWindowSize(rootItem.width, rootItem.height);
        }

        function onImplicitHeightChanged() {
            controller.updateWindowSize(rootItem.width, rootItem.height);
        }
    }
}
