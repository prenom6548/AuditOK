/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

import QtQuick 2.0
import QtQuick.Controls 2.0

ToolBar {
    background: Item{
        z: -1
        MouseArea {
            property bool pressed: false
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            acceptedButtons: Qt.LeftButton

            onPressed: {
                mouse.accepted = true;
                pressed = true;
            }

            onReleased: {
                mouse.accepted = true;
                console.log("mouse released");
                pressed = false;
            }

            onPositionChanged: {
                //console.log("mouse position changed", mouse.x, mouse.y);
                mouse.accepted = true;
                if (pressed) {
                    var pos = this.mapToGlobal(mouse.x, mouse.y);
                    mainWindow.systemMoveRequest(pos);
                    //mainWindow.startSystemMove();
                    pressed = false;
                }
            }
        }
    }
}
