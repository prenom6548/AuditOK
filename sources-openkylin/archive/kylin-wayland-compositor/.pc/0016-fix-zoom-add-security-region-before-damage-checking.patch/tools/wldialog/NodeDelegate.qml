// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

ColumnLayout {
    id: root

    property var widgetModel: null

    Layout.fillWidth: true
    spacing: 10

    ColumnLayout {
        id: contentColumn

        Layout.fillWidth: true
        spacing: 8

        Loader {
            id: mainWidgetLoader

            Layout.fillWidth: true
            Layout.minimumHeight: item ? item.implicitHeight : 0
            active: widgetModel !== undefined && widgetModel.type !== undefined
            sourceComponent: {
                switch (widgetModel.type) {
                case "CheckBox":
                    return checkBoxComp;
                case "Custom":
                    return customGroupComp;
                default:
                    return null;
                }
            }
        }
    }

    Component {
        id: checkBoxComp

        RowLayout {
            CheckBox {
                text: widgetModel.data.text
                checked: {
                    var name = widgetModel.data.name;
                    dynamicModel.updateCheckBoxState(name, widgetModel.data.default);
                    return widgetModel.data.default;
                }
                onCheckedChanged: {
                    var name = widgetModel.data.name;
                    if (name) {
                        dynamicModel.updateCheckBoxState(name, checked);
                    }
                }
            }
        }
    }

    Component {
        id: customGroupComp

        GroupBox {
            id: groupBox

            Layout.fillWidth: true

            ColumnLayout {
                id: customLayout

                anchors.fill: parent
                spacing: 8

                Repeater {
                    model: widgetModel.child

                    delegate: Loader {
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            setSource("NodeDelegate.qml", {
                                "widgetModel": modelData
                            });
                        }
                    }
                }
            }

            background: Rectangle {
                y: groupBox.topPadding - groupBox.bottomPadding
                width: parent.width
                height: parent.height - groupBox.topPadding + groupBox.bottomPadding
                color: gsettingsManager.themeColor
                border.width: 1
                border.color: "#d7d9db"
                radius: 8
            }

            label: Label {
                visible: widgetModel.data.title
                text: widgetModel.data.title ? widgetModel.data.title : ""
                wrapMode: Text.WordWrap
                x: groupBox.leftPadding
                width: groupBox.availableWidth
            }
        }
    }
}
