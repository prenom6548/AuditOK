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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

WidgetItem {
    id: root
    property bool isHorizontal: Widget.orientation == Types.Horizontal
    property real inset: parent.ratio ?  4 * parent.ratio : 4  // 分割线的左右间距（任务栏处于上下方向时）

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? childrenRect.width : width
    Layout.preferredHeight: isHorizontal ? height : childrenRect.height

    Item {
        width: isHorizontal ? separator.width + inset * 2 : parent.width
        height: isHorizontal ? parent.height : separator.height + inset * 2
        StyleBackground {
            id: separator
            width: root.isHorizontal ? 1 : parent.width * 0.5
            height: root.isHorizontal ? parent.height * 0.5 : 1
            anchors.centerIn: parent

            useStyleTransparency: false
            alpha: 0.3
            paletteRole: Theme.ButtonText
        }
    }
}
