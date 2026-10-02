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
import org.ukui.quick.platform 1.0 as Platform

Rectangle {
    height: 1
    color: Platform.Theme.separator()

    Timer {
        id: timer
        interval: 100
        onTriggered: {
            console.log("update color");
            color = Platform.Theme.separator();
        }
    }

    Platform.Theme.onThemeColorChanged: {
        console.log("theme color changed");
        timer.restart();
    }

    Platform.Theme.onThemeNameChanged: {
        console.log("theme name changed");
        timer.restart();
    }

    Platform.Theme.onPaletteChanged: {
        console.log("palette changed");
        timer.restart();
    }
}
