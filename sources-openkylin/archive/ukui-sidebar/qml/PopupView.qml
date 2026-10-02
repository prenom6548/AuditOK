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

import QtQuick 2.12
import org.ukui.notification.ui 1.0
import org.ukui.notification.core 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import org.ukui.positioner 1.0

Flickable {
    id: baseView
    property int itemWidth: 374
    property int itemRadius: Platform.GlobalTheme.kRadiusMenu
    property int maxHeight: popupNotificationWindow.maximumHeight
    property int duration: 300
    property real preContentY: 0
    property int viewMargin: 8
    property int shadowMargin: 0

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    boundsBehavior: isLiteMode ? Flickable.StopAtBounds : Flickable.DragAndOvershootBounds

    width: contentWidth
    height: contentHeight > maxHeight ? maxHeight : contentHeight
    interactive: contentHeight > maxHeight

    contentHeight: baseLayout.height + viewMargin + shadowMargin
    contentWidth: baseLayout.width + viewMargin + shadowMargin

    Column {
        id: baseLayout
        width: itemWidth
        height: childrenRect.height
        spacing: 8
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: baseView.viewMargin
        anchors.topMargin: baseView.viewMargin

        move: Transition {
            enabled: !isLiteMode
            NumberAnimation {
                properties: "y"; easing.type: Easing.InOutQuad; duration: duration
            }
        }

        Repeater {
            id: baseRepeater
            model: baseModel
        }
    }

    // ******************[baseModel]****************
    DelegateModel {
        id: baseModel
        model: groupModel
        rootIndex: modelIndex(-1)

        delegate: Item {
            id: componentPerApp
            property int groupIndex: model.groupIndex
            property int itemCount: repeaterPerApp.count

            width: childrenRect.width
            height: childrenRect.height

            // ******************[perApp]****************

            Component.onCompleted: {
                modelPreApp.model =  groupModel;
                modelPreApp.rootIndex = modelPreApp.modelIndex(groupIndex);
                repeaterPerApp.model = modelPreApp;
            }

            DelegateModel {
                id: modelPreApp
                delegate: Item {
                    id: delegateItem
                    height: childrenRect.height
                    width: itemWidth
                    property bool removeToClose: false
                    property bool toBeStored: model.toBeStored

                    onToBeStoredChanged: {
                        if (toBeStored)  state = "store";
                    }
                    state: "normal"
                    states: [
                        State {
                            name: "normal"
                            PropertyChanges {
                                target: delegateContent
                                x: 0; opacity: 1
                            }
                        },
                        State {
                            name: "remove"
                            PropertyChanges {
                                target: delegateContent
                                scale: 0.8; opacity: 0
                            }
                        },
                        State {
                            name: "store"
                            PropertyChanges {
                                target: delegateContent
                                x: LayoutMirroring.enabled ? -itemWidth : itemWidth; opacity: 0
                            }
                        }
                    ]
                    transitions: [
                        Transition {
                            to: "remove"
                            SequentialAnimation {
                                NumberAnimation { properties: "opacity, scale"; easing.type: Easing.Bezier; easing.bezierCurve: [0.25,0.1,0.25,1]; duration: 260}
                                ScriptAction {
                                    script: {
                                        delegateItem.toRemoveScript();
                                    }
                                }
                            }
                        },
                        Transition {
                            to: "store"
                            SequentialAnimation {
                                NumberAnimation { properties: "x, opacity"; easing.type: Easing.Bezier; easing.bezierCurve: [0.25,0.1,0.25,1]; duration: 260}
                                ScriptAction {
                                    script: {
                                        delegateItem.toStoreScript();
                                    }
                                }
                            }
                        }
                    ]

                    function toRemoveScript() {
                        if (delegateItem.removeToClose) {
                            sourceModel.closeNotification(model.id);
                        } else {
                            sourceModel.execAction(model.id, delegateContent.action);
                        }
                    }
                    function toStoreScript() {
                        if (toBeStored) {
                            sourceModel.setNotificationData(id, true, NotificationItem.IsStored);
                        }
                    }
                    PopupNotificationItem {
                        id: delegateContent
                        property string action: ""
                        height: childrenRect.height
                        width: itemWidth
                        screenLock: popupNotificationWindow.screenLockState
                        background.radius: itemRadius
                        pcTips.text: qsTr("%1 more notifications").arg(repeaterPerApp.count - 1)
                        pcTips.visible: repeaterPerApp.count > 1
                        isTablet: false
                        hideBorderShadow: (index > 3) && preAppLayout.isFoldMode

                        onActionExecuted: function (action) {
                            delegateContent.action = action;
                            if (model.resident) {
                                sourceModel.execAction(model.id, action);
                            } else {
                                delegateItem.removeToClose = false;
                                delegateItem.state = "remove";
                            }
                        }

                        onNotificationClosed: {
                            delegateItem.removeToClose = true;
                            delegateItem.state = "remove";
                        }

                        onClicked: {
                            if (model.hasDefaultAction) {
                                if (model.resident) {
                                    sourceModel.execAction(model.id, "default");

                                } else {
                                    delegateContent.action = "default";
                                    delegateItem.removeToClose = false;
                                    delegateItem.state = "remove";
                                }
                            }
                        }

                        Component.onCompleted: {
                            delegateItem.state = "normal";
                            baseView.shadowMargin = delegateContent.shadowSize;
                        }
                    }
                }
            }

            UkuiColumn {
                id: preAppLayout
                width: itemWidth
                height: childrenRect.height - heightOffset

                Repeater {
                    id: repeaterPerApp
                }

                move: Transition {
                    NumberAnimation {
                        properties : "x,y"
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [0.25,0.1,0.25,1]
                        duration: 220
                    }
                }
                add: Transition {
                    NumberAnimation {
                        properties : "x"
                        from: itemWidth
                        to: 0
                        easing.type : Easing.InOutQuad;
                        duration: 300
                    }
                }
            }
        }
    }
}
