/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

import QtQuick 2.0
import QtGraphicalEffects 1.12

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

// 阴影
Item {
    id: root

    property int radius: 8
    property int shadowWidth: 8

    layer.enabled: true;
    layer.effect: OpacityMask {
        maskSource: Rectangle {
            width: root.width
            height: root.height
            radius: root.shadowWidth + root.radius
            color: "transparent"
            border.width: root.shadowWidth
            border.color: "white"
        }
    }

    Rectangle {
        x: parent.shadowWidth
        y: parent.shadowWidth
        width: parent.width - parent.shadowWidth * 2
        height: parent.height - parent.shadowWidth * 2
        radius: parent.radius

        layer.enabled: true
        layer.effect: DropShadow {
            radius: 12
            samples: 25
            color: Platform.Theme.color(Platform.Theme.Shadow, Platform.Theme.Active, 0.16)
        }
    }
}
