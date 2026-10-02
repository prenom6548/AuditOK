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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

import QtQuick 2.0
import org.ukui.quick.platform 1.0
import org.ukui.quick.items 1.0
Column {
    spacing: 10
    SettingsValue {
        name: "liteAnimation"
        value: Settings.liteAnimation
    }
    SettingsValue {
        name: "liteFunction"
        value: Settings.liteFunction
    }
    SettingsValue {
        name: "tabletMode"
        value: Settings.tabletMode
    }
    ShadowedRectangle {
        width: 800
        height: 500
        pureColor: false
        startColor: "red"
        endColor: "yellow"
        color: "blue"
        angle: 2
        // color: "transparent"
        corners {
            topLeftRadius: 10
            topRightRadius: 50
            bottomLeftRadius: 80
            bottomRightRadius: 0
        }
        radius: 5
        shadow {
            size: 10
            color: Qt.rgba(0, 0, 0, 0.5)
            yOffset: 2
        }

        border {
            width: 1
            color: "black"
        }
    }

    Button {
        id: roundButton
        width: 100
        height: 100
        background.radius: 50

        ShadowedTexture {
            source: roundButton
            anchors.fill: parent
            radius: 50
            shadow {
                size: 20
                color: Qt.rgba(0, 0, 0, 1)
                yOffset: 2
            }

            border {
                width: 1
                color: "blue"
            }
        }
    }

}