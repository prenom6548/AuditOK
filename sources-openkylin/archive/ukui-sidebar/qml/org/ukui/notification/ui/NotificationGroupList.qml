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
import QtQml.Models 2.12
import QtQuick.Window 2.15

ListView {
    id: root
    spacing: 8
    cacheBuffer: Math.max(childrenRect.height, Screen.height)
    boundsBehavior: Flickable.StopAtBounds
    property alias viewClearAnimation: clearAnimation
    property int dragIndex: -1
    focus: true
    flickDeceleration : 15000
    flickableDirection: Flickable.VerticalFlick

    function filckView(distance) {
        // 计算初始速度
        var velocity = Math.round(Math.sqrt(2 * Math.abs(distance) * root.flickDeceleration));
        // 确定速度方向
        var direction = (distance > 0) ? 1 : -1
        root.maximumFlickVelocity = velocity;
        root.flick(0, velocity * direction);
    }

    model: DelegateModel {
        model: groupModel
        rootIndex: modelIndex(-1)
        delegate: NotificationList {
            id: notificationList
            groupData: model
            groupIndex: model.groupIndex
            width: ListView.view.width
            z: index
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_PageUp) {
            filckView(root.height);
        } else if (event.key === Qt.Key_PageDown) {
            filckView(-root.height);
        }
    }

    onDragIndexChanged: {
        for (var i = 0; i < count; i ++) {
            if (i === dragIndex) {
                continue;
            }
            root.itemAtIndex(i).listState = "normal";
        }
    }

    SequentialAnimation {
        id: clearAnimation
        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "y"
                to: -root.height
                easing.type: Easing.Bezier
                easing.bezierCurve: [0.25,0.1,0,1,1,1]
                duration: 300
            }
            NumberAnimation {
                target: root
                property: "opacity"
                to: 0
                easing.type: Easing.Bezier
                easing.bezierCurve: [0.25,0.1,0,1,1,1]
                duration: 300
            }
        }
        ScriptAction {
            script: {
                sourceModel.clearAll();
                root.opacity = 1;
                root.y = 0;
            }
        }
    }
}
