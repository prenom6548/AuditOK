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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.15
import KywcCapture 1.0

Rectangle {
    id: root

    property alias captureType: captureItem.captureType
    property alias uuid: captureItem.uuid
    property alias outputUuid: captureItem.outputUuid
    property alias overlayCursor: captureItem.overlayCursor
    property alias withoutDecoration: captureItem.withoutDecoration
    property alias active: captureItem.active

    property int previewWidth: 400
    property int previewHeight: 300

    signal captureFailed()

    width: previewWidth
    height: previewHeight

    KywcCaptureThumbnailItem {
        id: captureItem
        anchors.fill: parent
        anchors.margins: 10
        active: root.active
        onCaptureFailed: {
            console.log('Capture failed!');
            root.captureFailed();
        }
    }

    Text {
        id: statusText
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        verticalAlignment: Text.AlignVCenter
        padding: 5
        text: {
            var typeName = "Unknown";
            if (captureType === 0) typeName = "Toplevel";
            else if (captureType === 1) typeName = "Output";
            else if (captureType === 2) typeName = "Workspace";

            if (typeName === "Unknown") {
                return "No target specified";
            }
            return typeName + " [" + uuid + "]";
        }
    }
}