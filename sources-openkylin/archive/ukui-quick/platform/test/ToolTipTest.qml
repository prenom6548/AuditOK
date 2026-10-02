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

import QtQuick 2.15
import org.ukui.quick.platform 1.0
import org.ukui.quick.items 1.0
Row {
    width: 900
    height: 900
    spacing: 4
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 1"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 2 \n"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
    StyleBackground {
        width: 80
        height: 80
        radius: 12
        paletteRole: Theme.BrightText
        useStyleTransparency: false
        alpha: 0.5
        Tooltip {
            width: parent.width
            height: parent.height
            mainText: "tooltip test 3 tooltip test 3 tooltip test 3 tooltip test 3"
            posFollowCursor: false
            location: Dialog.TopEdge
            margin: 9
        }
    }
}