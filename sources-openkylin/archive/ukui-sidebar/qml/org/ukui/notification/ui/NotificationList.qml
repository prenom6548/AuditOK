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
import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import QtGraphicalEffects 1.0

import org.ukui.positioner 1.0
import org.ukui.notification.core 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    height: childrenRect.height
    property int duration: 300
    property var groupData: null
    property int groupIndex: -1
    property alias listState: notificationListContent.state

    //限定折叠区域enable
    property bool itemClearButtonValid: false
    Component.onCompleted: {
        notificationModel.model = groupModel;
        notificationModel.rootIndex = notificationModel.modelIndex(root.groupIndex);
        notificationList.model = notificationModel;
    }

    DelegateModel {
        id: notificationModel
        property string name: groupData.groupName

        delegate: Item {
            id: delegateItem
            height: childrenRect.height
            width: preAppLayout.width
            property bool animationEnable: false
            property bool removeToClose: false
            property bool delayRemove: model.delayRemove
            property bool groupIsExpand: groupData.groupIsExpand
            onDelayRemoveChanged: {
                if (delayRemove)  {
                    // 不显示时，state的Transition会失效，需要手动调用toRemoveScript
                    if (visible) {
                        state = "remove";
                    } else {
                        delegateItem.toRemoveScript()
                    }
                }
            }
            state: "normal"
            states: [
                State {
                    name: "normal"
                    PropertyChanges {
                        target: delegateContent
                        x: 0
                    }
                },
                State {
                    name: "remove"
                    PropertyChanges {
                        target: delegateItem
                        height: groupIsExpand ? 0 : childrenRect.height
                        z: -1
                    }
                    PropertyChanges {
                        target: delegateContent
                        scale: 0.8; opacity: 0
                    }
                },
                State {
                    name: "swipeToRemove"
                    PropertyChanges {
                        target: delegateContent
                        x: -preAppLayout.width
                    }
                },
                State {
                    name: "prepareRemove"
                    PropertyChanges {
                        target: delegateContent
                        x: -(removeButton.width + 8)
                    }
                    PropertyChanges {
                        target: removeButtonItem
                        visible: true
                    }
                }
            ]
            transitions: [
                Transition {
                    to: "normal"
                    enabled: !isLiteMode
                    SequentialAnimation {
                        NumberAnimation { properties: "x"; easing.type: Easing.Bezier; easing.bezierCurve: [0.12,0,0.18,1,1,1]; duration: 300 }
                        ScriptAction { script: {
                                delegateItem.toNormalScript();
                            }
                        }
                    }
                },
                Transition {
                    to: "remove, swipeToRemove"
                    enabled: !isLiteMode
                    SequentialAnimation {
                        NumberAnimation { properties: "x, height, scale, opacity"; easing.type: Easing.Bezier; easing.bezierCurve: [0.25,0.1,0.25,1]; duration: 260}
                        ScriptAction { script: {
                                delegateItem.toRemoveScript();
                            }
                        }
                    }
                },
                Transition {
                    to: "prepareRemove"
                    NumberAnimation { properties: "x"; easing.type: Easing.Bezier; easing.bezierCurve: [0.12,0,0.18,1,1,1]; duration: 300}
                }
            ]


            onStateChanged: {
                if (isLiteMode) {
                    if (state === "remove") {
                        delegateItem.toRemoveScript();
                        return;
                    }
                    if (state === "normal") {
                        delegateItem.toNormalScript();
                        return;
                    }
                }
            }
            function toNormalScript() {
                removeButtonItem.visible = false;
            }

            function toRemoveScript() {
                if (delayRemove) {
                    sourceModel.setNotificationData(id, false, NotificationItem.DelayRemove);
                    return;
                }
                if (delegateItem.removeToClose) {
                    sourceModel.closeNotification(model.id)
                } else {
                    // 结合model的execAction方法，非常驻通知执行action后删除
                    sourceModel.execAction(model.id, delegateContent.action);
                }
            }

            // *********[drag]**********
            DragHandler {
                property real taregtX: target.x
                target: delegateContent
                enabled: !preAppLayout.isFoldMode || notificationList.count === 1
                yAxis.enabled: false

                onTaregtXChanged: {
                    if (isNotificationCenter) {
                        if (delegateContent.x < -48) {
                            removeButtonItem.width = 40;
                        } else if (delegateContent.x < 0 ) {
                            removeButtonItem.visible = true;
                            removeButtonItem.width = -delegateContent.x - 8;
                        } else {
                            removeButtonItem.visible = false;
                        }
                    }
                }

                onActiveChanged: {
                    delegateItem.state = "";
                    if (!active) {
                        if (isNotificationCenter) {
                            if (delegateContent.x < -preAppLayout.width * 0.5) {
                                delegateItem.removeToClose = true;
                                delegateItem.state = "swipeToRemove";
                            } else if (delegateContent.x < -preAppLayout.width * 0.25) {
                                delegateItem.state = "prepareRemove";
                            } else {
                                delegateItem.state = "normal";
                            }
                        } else {
                            if (delegateContent.x < -preAppLayout.width * 0.5) {
                                delegateItem.removeToClose = true;
                                delegateItem.state = "swipeToRemove";
                            } else {
                                delegateItem.state = "normal";
                            }
                        }
                    } else {
                        notificationList.dragIndex = index;
                    }
                }
            }

            Item {
                id: removeButtonItem
                height: delegateContent.height
                width: 40
                visible: false
                opacity: width / 40
                anchors.right: parent.right

                UkuiItems.DtThemeBackground {
                    id: removeButton
                    height: width
                    width: parent.width
                    radius: width /2
                    useStyleTransparency: false
                    backgroundColor: removeButtonArea.containsPress ? Platform.GlobalTheme.kComponentClick
                                                                    : removeButtonArea.containsMouse ? Platform.GlobalTheme.kComponentHover
                                                                                                     : Platform.GlobalTheme.kContainGeneralNormal
                    anchors.verticalCenter: parent.verticalCenter

                    // 深色主题下反色
                    UkuiItems.Icon {
                        height: parent.width * 0.4
                        width: height
                        anchors.centerIn: parent
                        mode: UkuiItems.Icon.ForceHighlight
                        source: "window-close-symbolic"
                    }
                    MouseArea {
                        id: removeButtonArea
                        anchors.fill: parent
                        hoverEnabled: true
                        visible: removeButton.visible
                        onClicked: {
                            delegateItem.removeToClose = true;
                            delegateItem.state = "swipeToRemove";
                        }
                    }
                }
            }

            PopupNotificationItem {
                id: delegateContent
                property string action: ""
                height: childrenRect.height
                width: preAppLayout.width
                background.radius: isNotificationCenter ? 24 : Platform.GlobalTheme.kRadiusMax

                pcTips.text: qsTr("%1 more notifications").arg(notificationList.count - 1)
                pcTips.visible: !isNotificationCenter && preAppLayout.isFoldMode && notificationList.count > 1 && index === 0
                tabletTips.text: qsTr("%1 more notifications").arg(notificationList.count - 1)
                tabletTips.visible: isNotificationCenter && preAppLayout.isFoldMode && notificationList.count > 1 && index === 0
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
                    //折叠状态下删除所有delegateItem
                    if(preAppLayout.isFoldMode){
                        notificationListContent.state = "remove";
                        return;
                    }
                    delegateItem.removeToClose = true;
                    delegateItem.state = "remove";
                }
                onClearButtonAreaEntered: {
                     root.itemClearButtonValid = isEnter;
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
            }
        }
    }
    
    // [folder mouseArea]
    MouseArea {
        width: preAppLayout.width
        height: preAppLayout.height
        enabled: preAppLayout.isFoldMode && (notificationList.count > 1) && (!root.itemClearButtonValid)
        z: 10
        onClicked: {
            if (notificationListContent.state === "normal") {
                notificationModel.model.setData(notificationModel.rootIndex, true, NotificationItem.GroupIsExpand);
            }
        }

        drag.target: notificationListContent
        drag.axis: Drag.XAxis
        drag.onActiveChanged: {
            notificationListContent.state = ""
            if (!drag.active) {
                if (isNotificationCenter) {
                    if (notificationListContent.x < -preAppLayout.width * 0.5) {
                        notificationListContent.state = "swipeToRemove";
                    } else if (notificationListContent.x < -preAppLayout.width * 0.25) {
                        notificationListContent.state = "prepareRemove";
                    } else {
                        notificationListContent.state = "normal";
                    }
                } else {
                    if (notificationListContent.x < -preAppLayout.width * 0.5) {
                        notificationListContent.state = "swipeToRemove";
                    } else {
                        notificationListContent.state = "normal";
                    }
                }
            } else {
                parent.ListView.view.dragIndex = root.groupIndex;
            }
        }
    }

    // [===notificationListContent===]
    ColumnLayout {
        id: notificationListContent
        width: parent.width
        state: "normal"
        spacing: 0
        layer.enabled: !groupData.groupIsExpand
        // [header: folderButtton and clearButton]
        Item {
            id: functionButton
            Layout.preferredWidth: parent.width

            state: (!preAppLayout.isFoldMode && (notificationList.count > 1)) ? "show" : "hide"
            states: [
                State {
                    name: "show"
                    PropertyChanges { target: functionButton; opacity: 1;
                        Layout.preferredHeight: functionButton.visible ? (isNotificationCenter ? 48+8 : 36+8) : 0 }
                },
                State {
                    name: "hide"
                    PropertyChanges { target: functionButton; opacity: 0; Layout.preferredHeight: 0 }
                }
            ]
            transitions: Transition {
                enabled: !isLiteMode
                NumberAnimation { easing.type: Easing.InOutCubic; properties: "opacity, Layout.preferredHeight"; duration: 200 }
            }

            RowLayout {
                id: functionLayout
                width: parent.width
                height: isNotificationCenter ? 48 : 36

                anchors.topMargin: 8
                spacing: 8
                anchors.left: parent.left

                UkuiItems.DtThemeBackground {
                    id: foldbutton

                    Accessible.role: Accessible.Button
                    Accessible.description: "This is a fold button on notification center"
                    Accessible.name: "notification_center_notification_fold_button"

                    Layout.maximumWidth: parent.width - parent.spacing - clearButton.width
                    Layout.preferredWidth: foldButtonArea.width
                    Layout.fillHeight: true

                    radius: Platform.GlobalTheme.kRadiusNormal
                    useStyleTransparency: false
                    backgroundColor: foldbuttonMouseArea.containsPress ? Platform.GlobalTheme.kContainClick
                                                                       : foldbuttonMouseArea.containsMouse ? Platform.GlobalTheme.kContainHover
                                                                                                           : Platform.GlobalTheme.kContainGeneralNormal
                    RowLayout {
                        id: foldButtonArea
                        height: parent.height
                        clip: true
                        spacing: 8

                        UkuiItems.Icon {
                            id: showLessIcon
                            Layout.alignment: Qt.AlignLeft
                            Layout.leftMargin: 16

                            Layout.preferredHeight: 20
                            Layout.preferredWidth: Layout.preferredHeight
                            source: "ukui-up-symbolic"
                            mode: UkuiItems.Icon.AutoHighlight
                        }

                        UkuiItems.DtThemeText {
                            id: showLessText

                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            Layout.rightMargin: 16
                            Layout.maximumWidth: 160
                            text: qsTr("Show less")
                            elide: Text.ElideRight
                            textColor: Platform.GlobalTheme.kFontPrimary
                        }
                    }

                    MouseArea {
                        id: foldbuttonMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            notificationModel.model.setData(notificationModel.rootIndex, false, NotificationItem.GroupIsExpand);
                        }
                    }
                }

                Item {
                    id: clearButton
                    Layout.fillHeight: true
                    Layout.preferredWidth: height
                    Layout.maximumWidth: height
                    Layout.minimumWidth: height
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    //平板模式下清空按钮
                    UkuiItems.DtThemeButton {
                        id: clear
                        anchors.centerIn: parent
                        height: parent.height
                        width: height

                        background.radius: height / 2
                        background.useStyleTransparency: false
                        background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainClick
                                                                  : containsMouse ? Platform.GlobalTheme.kContainHover
                                                                                  : Platform.GlobalTheme.kContainGeneralNormal


                        icon.source: "window-close-symbolic"
                        icon.mode: UkuiItems.Icon.AutoHighlight
                        visible: isNotificationCenter

                        onClicked: {
                            notificationListContent.state = "removeFromY"
                        }

                        UkuiItems.Tooltip {
                            anchors.fill: parent
                            posFollowCursor: true
                            mainText: qsTr("Close all")
                        }
                    }

                    //pc侧边栏清空按钮
                    UkuiItems.DtThemeButton {
                        Accessible.role: Accessible.Button
                        Accessible.description: "This is a clear all button on notification center"
                        Accessible.name: "notification_center_clear_all_button"

                        anchors.centerIn: parent
                        height: parent.height
                        width: height

                        background.radius: height / 2
                        background.useStyleTransparency: false
                        background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainClick
                                                                  : containsMouse ? Platform.GlobalTheme.kContainHover
                                                                                  : Platform.GlobalTheme.kContainGeneralNormal

                        icon.source: "window-close-symbolic"
                        icon.mode: UkuiItems.Icon.AutoHighlight
                        visible: !isNotificationCenter

                        onClicked: {
                            notificationListContent.state = "remove"
                        }

                        UkuiItems.Tooltip {
                            anchors.fill: parent
                            posFollowCursor: true
                            mainText: qsTr("Close all")
                        }
                    }
                }
            }
        }

        // [content: notification]
        UkuiColumn {
            id: preAppLayout
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: childrenRect.height - preAppLayout.heightOffset

            spacing: 8
            
            Repeater {
                id: notificationList
                property bool isExpand: groupData.groupIsExpand
                property int dragIndex: -1

                onDragIndexChanged: {
                    for (var i = 0; i < count; i ++) {
                        if (i === dragIndex) {
                            continue;
                        }
                        itemAt(i).state = "normal";
                    }
                }
                onIsExpandChanged: {
                    preAppLayout.isFoldMode = !isExpand;
                }
            }
            add: Transition {
                enabled: !isLiteMode
                NumberAnimation { easing.type: Easing.InOutQuad; properties: "x"; from: preAppLayout.width; to: 0; duration: 300 }
            }
            move: Transition {
                enabled: !isLiteMode
                NumberAnimation {
                    properties : "y"
                    easing.type: Easing.Bezier
                    easing.bezierCurve: [0.25,0.1,0.25,1]
                    duration: 220
                }
            }
        }

        states: [
            State {
                name: "normal"
                PropertyChanges {
                    target: notificationListContent
                    x: 0
                }
            },
            State {
                name: "prepareRemove"
                PropertyChanges {
                    target: notificationListContent
                    x: -(removeButtonGroups.width + 8)
                }
            },
            State {
                name: "swipeToRemove"
                PropertyChanges {
                    target: notificationListContent
                    x: -preAppLayout.width
                }
            },
            State {
                name: "remove"
                PropertyChanges {
                    target: notificationListContent
                    scale: 0.8; opacity: 0
                }
                PropertyChanges {
                    target: root
                    height: 0
                }
            }
        ]

        onStateChanged: {
            if (isLiteMode) {
                if (state === "remove" || state === "swipeToRemove") {
                    groupModel.clearGroup(notificationModel.rootIndex);
                }
            }
        }
        transitions: [
            Transition {
                to: "normal, prepareRemove"
                enabled: !isLiteMode
                NumberAnimation { properties: "x"; easing.type: Easing.Bezier; easing.bezierCurve: [0.12,0,0.18,1,1,1]; duration: 300 }
            },
            Transition {
                to: "remove, swipeToRemove"
                enabled: !isLiteMode
                SequentialAnimation {
                    NumberAnimation { properties: "x, y, scale, opacity, height"; easing.type: Easing.Bezier; easing.bezierCurve: [0.25,0.1,0.25,1]; duration: 260}
                    ScriptAction { script: groupModel.clearGroup(notificationModel.rootIndex); }
                }
            }
        ]
    }

    Item {
        id: removeButtonGroups
        height: notificationListContent.height
        width: Math.min((-notificationListContent.x - 8), 40)
        visible: (notificationListContent.x < 0) && isNotificationCenter
        opacity: width / 40
        anchors.right: parent.right
        z: 20

        UkuiItems.DtThemeBackground {
            height: width
            width: parent.width
            radius: width /2
            useStyleTransparency: false
            backgroundColor: buttonArea.containsPress ? Platform.GlobalTheme.kComponentClick
                                                      : buttonArea.containsMouse ? Platform.GlobalTheme.kComponentHover
                                                                                 : Platform.GlobalTheme.kContainGeneralNormal
            anchors.verticalCenter: parent.verticalCenter

            // 深色主题下反色
            UkuiItems.Icon {
                height: parent.width * 0.4
                width: height
                anchors.centerIn: parent
                mode: UkuiItems.Icon.ForceHighlight
                source: "window-close-symbolic"
            }
            MouseArea {
                id: buttonArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    notificationListContent.state = "remove";
                }
            }
        }
    }
}
