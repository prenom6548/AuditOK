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

import org.ukui.notification.ui 1.0

import org.ukui.sidebar.items 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    readonly property int animationDuration: 300
    focus: true

    Keys.onEscapePressed: {
        notificationWindow.activeNotificationCenter(false);
    }

    function updateContentY(posY) {
        if (contentAnimation.running) {
            return;
        }

        content.y = posY;
    }

    function startAnimation(isShow) {
        if (contentAnimation.running) {
            return;
        }

        contentAnimation.isShow = isShow;
        contentAnimation.from = content.y;
        contentAnimation.to = isShow ? 0 : -height;

        contentAnimation.start();
    }

    function isRunning() {
        return contentAnimation.running;
    }

    NumberAnimation {
        id: contentAnimation
        property bool isShow: true

        target: content
        property: "y"
        easing.type: Easing.InOutQuad
        duration: root.animationDuration;
        onStopped: {
            notificationWindow.contentVisible = isShow;

            if (isShow) {
                bottomIcon.state = "Close";

            } else {
                bottomIcon.state = "Open";
                notificationWindow.visible = false;
            }
        }
    }

    MouseArea {
        z: 1
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        enabled: !contentAnimation.running && notificationWindow.contentVisible

        property point pressedPoint
        property int startContentY: 0

        onPressed: function(event) {
            if (scrollableArea.interactive) {
                pressedPoint = Qt.point(event.x, event.y);
            } else {
                pressedPoint = scrollableArea.dragStartPoint;
            }

            startContentY = content.y;
        }

        onPositionChanged: function(event) {
            if (pressed) {
                var contentY = startContentY + (event.y - pressedPoint.y);
                if (contentY > 0) {
                    contentY = 0;
                } else if (contentY < -root.height) {
                    contentY = -root.height;
                }

                root.updateContentY(contentY);
            }
        }

        onReleased: function(event) {
            scrollableArea.interactive = true;
            if (content.y < (-root.height*0.25)) {
                root.startAnimation(false);
                return;
            }

            // click
            // TODO: 在按钮区域内小幅度移动会出发收起
            if (Math.abs(event.x - pressedPoint.x) < 3 && Math.abs(event.y - pressedPoint.y) < 3) {
                var point = mapToItem(iconMouseArea, event.x, event.y);
                if (iconMouseArea.contains(point)) {
                    root.startAnimation(false);
                    return;
                }
            }

            root.startAnimation(true);
        }
    }

    Item {
        id: content

        z: 10
        x: 0; y: 0
        width: parent.width
        height: parent.height

        DesktopBackground {
            anchors.fill: parent
            z: 0
            useDesktopBackground: false
        }

        ColumnLayout {
            id: notificationBody
            width: parent.width * 0.45
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            z: 10

            Flickable {
                id: scrollableArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: timeInfo.height + notificationCenterContent.childrenRect.height
                clip: true
                property point dragStartPoint

                NotificationCenterHeader {
                    id: timeInfo
                    width: parent.width
                    height: 330
                    haveMessage: notificationCenterContent.count > 0
                    onClearClick: {
                        notificationCenterContent.viewClearAnimation.start();
                    }
                }

                Item {
                    width: parent.width
                    height: childrenRect.height
                    anchors.top: timeInfo.bottom
                    clip: true
                    NotificationGroupList {
                        id: notificationCenterContent
                        width: parent.width
                        height: childrenRect.height
                        interactive: false
                    }
                }

                MouseArea {
                    width: notificationBody.width
                    height: notificationBody.height
                    z: 1;
                    onPressed: {
                        if (mouse.y > scrollableArea.contentHeight) {
                            // 通过改变interactive实现鼠标事件传递
                            scrollableArea.interactive = false
                            scrollableArea.dragStartPoint = Qt.point(mouse.x, mouse.y);
                        }
                        mouse.accepted = false
                    }
                }
            }

            // bottom Icon
            MouseArea {
                id: iconMouseArea
                Layout.maximumHeight: 48
                Layout.minimumHeight: 48
                Layout.maximumWidth: 48
                Layout.minimumWidth: 48
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                enabled: !contentAnimation.running && notificationWindow.contentVisible
                hoverEnabled: true

                onEntered: {
                    bottomIcon.opacity = 1.0;
                }
                onPressed: {
                    bottomIcon.opacity = 0.65;
                    mouse.accepted = false;
                }
                onReleased: {
                    bottomIcon.opacity = 0.45;
                }
                onExited: {
                    bottomIcon.opacity = 0.45;
                }
                onCanceled: {
                    bottomIcon.opacity = 0.45;
                }

                UkuiItems.Icon {
                    id: bottomIcon
                    anchors.centerIn: parent
                    height: parent.height
                    width: height
                    opacity: 0.45
                    mode: UkuiItems.Icon.AutoHighlight

                    state: "Open"
                    states: [
                        State {
                            name: "Open"
                            PropertyChanges {
                                target: bottomIcon
                                source: "ukui-sidebar-fold-symbolic"
                            }
                        },
                        State {
                            name: "Close"
                            PropertyChanges {
                                target: bottomIcon
                                source: "ukui-sidebar-open-symbolic"
                            }
                        }
                    ]
                }
            }
        }
    }
}
