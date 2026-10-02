/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

import QtQuick 2.1
import QtQuick.Controls 2.5
import QtQuick.Window 2.15

Rectangle {
    id: root;

    property string outputName;
    property string modeName;

    color: Qt.rgba(0, 0, 0, 0.6);
    radius: 12;

    width: childrenRect.width + 2 * childrenRect.x
    height: childrenRect.height + 2 * childrenRect.y
    enabled: false
    Window.onWindowChanged: {
        if (Window.window) {
            Window.window.flags |= Qt.WindowDoesNotAcceptFocus;
        }
    }

    Label {
        id: displayName
        x: 20 * 2
        y: 20
        font.pointSize: 10 * 2
        text: root.outputName;
        color: "white";
        wrapMode: Text.WordWrap;
        horizontalAlignment: Text.AlignHCenter;
    }

    Label {
        id: modeLabel;
        anchors {
            horizontalCenter: displayName.horizontalCenter
            top: displayName.bottom
        }
        text: root.modeName;
        color: "white";
        horizontalAlignment: Text.AlignHCenter;
    }
}
