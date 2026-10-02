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

import QtQml 2.15
import QtQuick 2.12
import org.ukui.notification.ui 1.0

ListView {
    id: root
    property int radius: 24
    // 单独一个应用消息的最大窗口高度
    property int maxHeight: 0
    property bool disableUpdateHeight: true
    spacing: 10
    boundsBehavior: Flickable.StopAtBounds

    // 必须有一个初始的宽用于生成正常的delegate
    width: tabletPopupView.windowWidth
    onHeightChanged: {
        tabletPopupView.visible = (height > 1) && (count > 0);
    }

    Timer {
        id: resetHeightTimer
        interval: 10000; running: false
        onTriggered: {
            root.maxHeight = 0;
            root.updateViewHeight();
        }
    }

    function updateViewHeight() {
        if (!disableUpdateHeight) {
            let h = childrenRect.height;
            if (count === 1) {
                if (h <= maxHeight) {
                    return;
                }
                maxHeight = h;
                resetHeightTimer.restart();
            }

            tabletPopupView.updateHeight(h);
        }
    }

    onDisableUpdateHeightChanged: {
        updateViewHeight();
    }

    onChildrenRectChanged: {
        updateViewHeight();
    }

    onCountChanged: {
        if (count <= 1) {
            maxHeight = 0;
        }
    }

    Component.onCompleted: {
        model = groupModel;
        // update height
        disableUpdateHeight = false;
    }

    delegate: CollapsedList {
        width: ListView.view.width
        enableAnimation: tabletPopupView.enableAnimation

        ListView.onAdd: {
            ListView.view.disableUpdateHeight = false;
        }

        ListView.onRemove: {
            ListView.view.disableUpdateHeight = true;
        }
    }

    // add: Transition {
    //     id: addTransition
    //     enabled: tabletPopupView.enableAnimation
    //     SequentialAnimation {
    //         PropertyAction {
    //             property: "y"
    //             value: -addTransition.ViewTransition.item.height
    //         }
    //         NumberAnimation {
    //             property: "y"
    //             duration: 600
    //             easing.type: Easing.Bezier
    //             easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1]
    //         }
    //         ScriptAction {
    //             script: {
    //                 // tabletPopupView.enableWindowBlur(root.radius, true);
    //             }
    //         }
    //     }
    // }

    remove: Transition {
        id: removeTransition
        enabled: tabletPopupView.enableAnimation
        SequentialAnimation {
            ScriptAction {
                script: {
                    tabletPopupView.enableWindowBlur(root.radius, false);
                }
            }
            ParallelAnimation {
                NumberAnimation {
                    property: "y";
                    to: (removeTransition.ViewTransition.item.y - removeTransition.ViewTransition.item.height)
                    duration: 500
                    easing.type: Easing.Bezier; easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1]
                }
                NumberAnimation {
                    property: "opacity";
                    to: 0.1
                    duration: 500
                    easing.type: Easing.Bezier; easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1]
                }
            }
            PropertyAction { target: root; property: "disableUpdateHeight"; value: false }
        }
    }

    displaced: Transition {
        enabled: tabletPopupView.enableAnimation
        NumberAnimation { properties: "y"; duration: 500; easing.type: Easing.Bezier; easing.bezierCurve: [0.25, 0.1, 0, 1, 1, 1] }
    }

    moveDisplaced: Transition {
        enabled: tabletPopupView.enableAnimation
        NumberAnimation { properties: "y"; duration: 300 }
    }

    // move: Transition {
    //     enabled: tabletPopupView.enableAnimation
    //     NumberAnimation { properties: "y"; duration: 300 }
    //     NumberAnimation { properties: "y" }
    // }
}
