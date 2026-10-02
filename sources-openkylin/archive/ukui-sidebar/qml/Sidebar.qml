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
import QtGraphicalEffects 1.12
import org.ukui.notification.ui 1.0
import org.ukui.sidebar.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: root
    property int duration: 300
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Component.onCompleted: {
        sidebarWindow.requestShowContent.connect(showContent);
        sidebarWindow.requestHideContent.connect(hideContent);
        sidebarWindow.radius = Qt.binding(function() { return Platform.GlobalTheme.kRadiusWindow; });

        handGestureHelper.controlCenterCalled.connect(trackMouse)
        handGestureHelper.right2LeftReleased.connect(mouseReleased)
    }

    Keys.onEscapePressed: {
        if (sidebarWindow.visible) {
            sidebarWindow.activeWindow(false);
        }
    }

    function updateProperty(active) {
        if (active) {
            notificationArea.x = notificationAreaBase.width + sidebarWindow.windowPadding;
            windowBlurHelper.radius = 0;

        } else {
            notificationArea.x = 0;
        }

        windowBlurHelper.enable = sidebarWindow.isTabletMode;
    }

    function isRunning(active) {
        return notificationAnimation.running;
    }

    function showContent() {
        notificationAreaBase.startAnimation(
                    LayoutMirroring.enabled
                    ? -(notificationAreaBase.width + sidebarWindow.windowPadding)
                    : notificationAreaBase.width + sidebarWindow.windowPadding, 0, root.duration, true);
    }

    function hideContent() {
        notificationAreaBase.startAnimation(
                    0,
                    LayoutMirroring.enabled
                    ? -(notificationAreaBase.width + sidebarWindow.windowPadding)
                    : notificationAreaBase.width + sidebarWindow.windowPadding, root.duration, false);
    }

    function trackMouse(posX) {
        if (sidebarWindow.isTabletMode) {
            return;
        }

        if (sidebarWindow.contentVisible) {
            return;
        }

        if (notificationAnimation.running === true) {
            notificationAnimation.running = false;
        }

        if (!sidebarWindow.visible) {
            updateProperty(true);
            sidebarWindow.show();
        }

        // 鼠标位置与当前主屏幕左边界/右边界的距离
        let distanceX = LayoutMirroring.enabled
            ? posX
            : sidebarWindow.primaryScreenRight - posX;

        if (distanceX < 0) {
            if (sidebarWindow.visible) {
                sidebarWindow.hide();
            }
            return;
        }
        // 加上padding忽略窗口的边距
        let notificationWidth = 384 + sidebarWindow.windowPadding;
        if (distanceX > notificationWidth) {
            distanceX = notificationWidth;
        }

        notificationArea.x = LayoutMirroring.enabled
                ? distanceX - notificationWidth
                : notificationWidth - distanceX;
    }

    // 唤起侧边栏手势释放事件
    function mouseReleased(posX) {
        if (!sidebarWindow.visible || sidebarWindow.contentVisible) {
            return;
        }

        // 鼠标位置与当前主屏幕左边界/右边界的距离
        let distanceX = LayoutMirroring.enabled
            ? posX
            : sidebarWindow.primaryScreenRight - posX;
        let notificationWidth = 384 + sidebarWindow.windowPadding;
        let duration = 50;

        // 如果为负值，快速隐藏
        if (distanceX < 0) {
            notificationAreaBase.startAnimation(notificationArea.x,
                                                LayoutMirroring.enabled
                                                ? -notificationWidth
                                                : notificationWidth, duration, false);
            return;
        }

        // 屏幕宽度的5%
        if (distanceX <= sidebarWindow.minimumThreshold) {
            duration = Math.floor(root.duration * (distanceX / notificationWidth));
            notificationAreaBase.startAnimation(notificationArea.x,
                                                LayoutMirroring.enabled
                                                ? -notificationWidth
                                                : notificationWidth, duration, false);
            return;
        }

        duration = Math.floor(root.duration * ((notificationWidth - distanceX) / notificationWidth));
        notificationAreaBase.startAnimation(notificationArea.x, 0, duration, true);
        EventTrack.sendSlideEvent("open_sidebar", "sidebar", {"type": "slide"});
    }

    NumberAnimation {
        id: showGroundGlass
        target: windowBlurHelper
        property: "radius"
        duration: root.duration
        from: 0
        to: 1

        function startAnimation() {
            if (isLiteMode) {
                windowBlurHelper.radius = 1;
                return;
            }

            start();
        }
    }

    Item {
        id: windowAreaBase
        anchors.fill: parent
        anchors.rightMargin: sidebarWindow.windowExpandPadding
        clip: true

        Item {
            id: notificationAreaBase
            property bool isNotifyEmpty: sidebarWindow.isNotificationEmpty
            z: 100
            anchors.leftMargin: sidebarWindow.windowPadding + sidebarWindow.windowExpandPadding
            anchors.rightMargin: sidebarWindow.windowPadding
            anchors.bottomMargin: sidebarWindow.windowPadding + sidebarWindow.windowExpandPadding
            anchors.left: windowAreaBase.left
            anchors.right: windowAreaBase.right
            anchors.bottom: windowAreaBase.bottom

            height: sidebarWindow.minWindowSize

            function startAnimation(from, to, duration, isShow) {
                // console.log("=== start animation ===", from, to, duration, isShow)
                if (isLiteMode) {
                    notificationArea.x = to;
                    notificationAnimation.animationStopped(isShow);
                    return;
                }

                if (notificationAnimation.running) {
                    return;
                }
                notificationAnimation.from = from;
                notificationAnimation.to = to;
                notificationAnimation.duration = duration < 50 ? 50 : duration;
                notificationAnimation.isShow = isShow;

                notificationAnimation.start();
            }

            NumberAnimation {
                id: notificationAnimation
                property bool isShow: false

                target: notificationArea
                property: "x"
                easing.type: Easing.InOutQuad

                onStopped: { animationStopped(isShow); }

                function animationStopped(isShow) {
                    if (isShow) {
                        sidebarWindow.contentVisible = true;
                        if (!sidebarWindow.isTabletMode) {
                            windowBlurHelper.enable = true;
                            showGroundGlass.startAnimation();
                        }
                    } else {
                        sidebarWindow.contentVisible = false;
                        sidebarWindow.hide();
                    }
                }
            }

            NotificationArea {
                id: notificationArea;
                visible: true
                width: 384;
                height: parent.height;
                clip: true;
                radius: sidebarWindow.radius;

                onXChanged: {
                    if (sidebarWindow.isTabletMode) {
                        let shortAreaWidth = width + sidebarWindow.windowPadding;
                        let rightWidth = Math.abs(Math.floor(shortAreaWidth - x));
                        windowBlurHelper.radius = Math.floor(rightWidth / shortAreaWidth);
                        windowBlurHelper.enable = (rightWidth !== 0);
                    }
                }
            }

            // 阴影区域
            UkuiItems.DtDropShadow {
                visible: sidebarWindow.visible
                anchors.fill: notificationArea
                shadowData: sidebarWindow.isWayland() ? Platform.GlobalTheme.kShadowPrimaryActive
                                                      : {xOffset: 0, yOffset: 3, blurRadius: 8, spread: 0,
                                                         shadowColor: Qt.rgba(0, 0, 0, 0.13), isMultiShadow: false}
                shadowRadius: notificationArea.radius

                onShadowSizeChanged: {
                    sidebarWindow.windowExpandPadding = shadowSize;
                }
            }

            NumberAnimation {
                id: foldChangeAnimation
                target: notificationAreaBase
                property: "height"
                duration: 300
                easing.type: Easing.InOutQuad
                onFinished: {
                    if (sidebarWindow.isNotificationEmpty) {
                        //折叠
                        sidebarWindow.isWindowFold = true;
                    }
                }
            }

            onIsNotifyEmptyChanged: {
                if (foldChangeAnimation.running) {
                    foldChangeAnimation.running = false;
                }
                if (sidebarWindow.isNotificationEmpty) {
                    foldChangeAnimation.to = sidebarWindow.minWindowSize;
                    foldChangeAnimation.start();
                } else {
                    //展开
                    sidebarWindow.isWindowFold = false;
                    foldChangeAnimation.to = root.height - sidebarWindow.windowPadding*2 - sidebarWindow.windowExpandPadding*2;
                    foldChangeAnimation.start();
                }
            }

            function setWindowHeight() {
                if (!sidebarWindow.isWindowFold) {
                    height = root.height - sidebarWindow.windowPadding*2 - sidebarWindow.windowExpandPadding*2;
                }
            }

            onHeightChanged: {
                sidebarWindow.setBlurHeight(height);
            }
        }
    }

    onHeightChanged: {
        notificationAreaBase.setWindowHeight();
    }
}
