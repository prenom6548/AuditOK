/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.windowThumbnail.test 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: testType === "workspace" ? "WorkspaceThumbnailItem Test - " + testOutputId : "WindowThumbnailItem Test - " + testUuid

    Component {
        id: windowThumbnail
        WindowThumbnailItem {
            anchors.fill: parent
            anchors.margins: 10
            uuid: testUuid
        }
    }

    Component {
        id: workspaceThumbnail
        WorkspaceThumbnailItem {
            anchors.fill: parent
            anchors.margins: 10
            outputId: testOutputId
            uuid: testUuid
        }
    }

    Loader {
        id: thumbnailLoader
        anchors.fill: parent
        sourceComponent: testType === "workspace" ? workspaceThumbnail : windowThumbnail
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: thumbnailLoader.item ? thumbnailLoader.item.paintedWidth + "x" + thumbnailLoader.item.paintedHeight : ""
    }
}