/*
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
 * Authors: zhouyong <zhouyong01@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.widgets 1.0
import org.ukui.shortcut.audio 1.0
import org.ukui.audio.common 1.0

WidgetItem {
    Layout.fillWidth: true
    Layout.fillHeight: true
    clip: true

    Loader {
        id: mainPage
        anchors.fill: parent

        onStatusChanged: {
            if (status === Loader.Ready) {
                console.log("main.qml Loader onStatusChanged loaded suc")
            } else if (status === Loader.Error) {
                console.log("main.qml Loader onStatusChanged error loading")
            }
        }
    }

    Component.onCompleted: {
        console.log("main.qml onCompleted")
        UKUIAudioControl.initMainThreadEvent()
        UKUIAudioControl.startTask(Common.TaskID_Init)
    }

    Component.onDestruction: {
        console.log("main.qml onDestruction")
        UKUIAudioControl.releaseSlots();
        UKUIAudioControl.releaseData();
    }

    Connections {
        target: UKUIAudioControl

        function onTaskCompleted(taskID, retValue) {
            if (taskID === Common.TaskID_Init) {
                console.log("main.qml onTaskCompleted TaskID_Init")
                if (mainPage.status !== Loader.Ready) {
                    mainPage.source = "AudioArea.qml"
                }
            }
        }

        function onTaskError(taskID) {
            if (taskID === Common.TaskID_Init) {
                console.log("main.qml onTaskError TaskID_Init")
            }
        }
    }
}
