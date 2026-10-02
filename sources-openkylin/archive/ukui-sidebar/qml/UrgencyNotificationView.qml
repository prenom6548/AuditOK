/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    anchors.centerIn: parent

    Component {
        id: page
        UkuiItems.StyleBackground {
            clip: false
            width: 374
            height: 60 + pageSummary.height
                    + pageBody.contentHeight + pageBottom.height
            radius: 12
            anchors.centerIn: parent
            useStyleTransparency: false
            paletteRole: Platform.Theme.Base
            opacity: index == repeater.count - 1 ? 1 : 0

            Component.onCompleted: {
                pageSummary.height = pageSummary.visible === true ? pageSummary.contentHeight : 0;
            }

            Column {
                anchors.fill: parent
                spacing: 4
                topPadding: 10; bottomPadding: 16
                leftPadding: 16; rightPadding: 16

                RowLayout {
                    spacing: 8
                    width: 342

                    UkuiItems.ThemeIcon {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        radius: 4
                        source: model.appIconName
                        mode: UkuiItems.Icon.AutoHighlight
                    }
                    UkuiItems.StyleText {
                        Layout.preferredHeight: 24
                        Layout.fillWidth: true
                        text: model.appName
                    }
                    //关闭按钮
                    UkuiItems.StyleBackground {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        radius: 16
                        Layout.alignment: Qt.AlignRight
                        useStyleTransparency: false
                        paletteRole: Platform.Theme.Base
                        alpha: closeBtnArea.containsPress ? 0.15 : closeBtnArea.containsMouse ? 0.08 : 0

                        UkuiItems.Icon {
                            id: closeIcon
                            width: Math.floor(Math.min(parent.width, parent.height) / 2);
                            height: width;
                            anchors.centerIn: parent;
                            mode: UkuiItems.Icon.ForceHighlight
                            source: "window-close-symbolic";
                        }

                        MouseArea {
                            id: closeBtnArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                urgencyNotificationModel.closeNotification(id);
                            }
                        }
                    }
                }

                Column {
                    leftPadding: 32

                    UkuiItems.StyleText {
                        id: pageSummary
                        width: 310
                        font.bold: true
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        text: model.summary
                        visible: model.summary !== ""
                    }

                    UkuiItems.StyleText {
                        id: pageBody
                        width: 310
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        text: model.body
                    }

                    ListView {
                        id: pageBottom
                        width: 310; height: 40
                        spacing: 8
                        orientation: Qt.Horizontal
                        layoutDirection: Qt.RightToLeft
                        interactive : false
                        model: actions

                        delegate: Item {
                            id: name
                            width: 98; height: 40

                            //按钮
                            UkuiItems.StyleBackground {
                                width: 98; height: 36; radius: 6
                                anchors.bottom: parent.bottom

                                useStyleTransparency: false
                                paletteRole: Platform.Theme.Base
                                alpha: closeBtnArea.containsPress ? 0.15 : closeBtnArea.containsMouse ? 0.08 : 0

                                UkuiItems.StyleText {
                                    anchors.centerIn: parent
                                    text: modelData.name
                                }
                                MouseArea {
                                    id: btnArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        urgencyNotificationModel.execAction(id, modelData.index);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            states: [
                State {
                    name: "hideButtons"
                    when: hasDefaultAction() === true
                    PropertyChanges { target: pageBottom; visible: false; height: 0 }
                },
                State {
                    name: "showButtons"
                    when: hasDefaultAction() === false
                    PropertyChanges { target: pageBottom; visible: true; height: 40 }
                }
            ]

            function hasDefaultAction() {
                for (var i = 0; i < actions.length; i++) {
                    if (actions[i].isDefault === true) {
                        return true;
                    }
                }
            }

        }
    }

    Repeater {
        id: repeater
        anchors.fill: parent
        model: urgencyNotificationModel
        delegate: page
    }
}
