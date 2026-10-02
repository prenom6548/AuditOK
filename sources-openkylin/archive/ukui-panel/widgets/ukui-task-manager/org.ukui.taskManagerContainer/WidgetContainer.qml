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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0

WidgetContainerItem {
    id: containerItem
    anchors.fill: parent

    GridLayout {
        id: mainLayout
        z: 10

        height: parent.height
        width: childrenRect.width
        rows: 1
        columns: repeater.count

        Repeater {
            id: repeater
            model: containerItem.widgetItemModel
            delegate: widgetLoaderComponent
        }
    }
    Component {
        id: widgetLoaderComponent

        Item {
            id: widgetParent
            clip: true

            property Item widgetItem: model.widgetItem
            Layout.fillWidth: widgetItem.Layout.fillWidth
            Layout.fillHeight: widgetItem.Layout.fillHeight
            Layout.minimumHeight: widgetItem.Layout.minimumHeight;
            Layout.maximumHeight: mainLayout.height;
            Layout.minimumWidth : widgetItem.Layout.minimumWidth
            Layout.maximumWidth : widgetItem.Layout.maximumWidth

            Layout.preferredHeight: mainLayout.height
            Layout.preferredWidth: widgetItem.Layout.preferredWidth

            onWidgetItemChanged: {
                if (widgetItem) {
                    widgetItem.parent = widgetParent
                    widgetItem.anchors.fill = Qt.binding(function () {return widgetParent})
                }
            }
        }
    }
}
