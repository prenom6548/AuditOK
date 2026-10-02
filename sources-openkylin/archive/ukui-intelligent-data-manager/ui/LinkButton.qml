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
import org.ukui.quick.platform 1.0 as Platform

Text {
    signal triggered()

    color: Platform.Theme.link()
    Timer {
        id: timer
        interval: 100
        onTriggered: {
            color = Platform.Theme.link();
        }
    }

    Platform.Theme.onThemeColorChanged: {
        timer.restart();
    }

    Platform.Theme.onThemeNameChanged: {
        timer.restart();
    }

    Platform.Theme.onPaletteChanged: {
        timer.restart();
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("link button clicked");
            triggered();
        }
    }
}
