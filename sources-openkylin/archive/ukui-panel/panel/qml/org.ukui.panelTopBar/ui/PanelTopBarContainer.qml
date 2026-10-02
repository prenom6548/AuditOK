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

import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import org.ukui.panel.impl 1.0 as Panel

WidgetContainerItem {
    id: containerItem
    property int orientation: WidgetContainer.orientation
    property bool isHorizontal: orientation === Types.Horizontal

    onParentChanged: {
        anchors.fill = parent
    }
    Item {
        id: backGround
        anchors.fill: parent
        anchors {
            leftMargin: containerItem.WidgetContainer.margin.left
            topMargin: containerItem.WidgetContainer.margin.top
            rightMargin: containerItem.WidgetContainer.margin.right
            bottomMargin: containerItem.WidgetContainer.margin.bottom
        }

        RowLayout {
            id: mainLayout
            z: 10
            anchors.fill: parent
            property int settingsIslandWidth: (repeater.count === 1) ? mainLayout.width : mainLayout.width * 0.6
            property int dataIslandWidth: (repeater.count === 1) ? mainLayout.width : mainLayout.width * 0.4
            spacing: 0
            Repeater {
                id: repeater
                model: Panel.WidgetSortModel
                {
                    widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                    sourceModel: containerItem.widgetItemModel
                }
                delegate: widgetLoaderComponent
            }
        }
        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                property int index: model.index
                property Item widgetItem: model.widgetItem
                property double ratio: 1
                property string containerType: "topBar"

                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.minimumHeight: mainLayout.height
                Layout.maximumHeight: mainLayout.height

                Layout.minimumWidth: {
                    const type = widgetItem.WidgetContainer.id
                    if(type === "org.ukui.dataIsland") {
                        return mainLayout.dataIslandWidth;
                    } else if (type === "org.ukui.settingsIsland") {
                        return mainLayout.settingsIslandWidth;
                    }
                }
                Layout.maximumWidth: {
                    const type = widgetItem.WidgetContainer.id
                    if(type === "org.ukui.dataIsland") {
                        return mainLayout.dataIslandWidth;
                    } else if (type === "org.ukui.settingsIsland") {
                        return mainLayout.settingsIslandWidth;
                    }
                }

                Layout.preferredHeight: mainLayout.height
                Layout.preferredWidth: {
                    const type = widgetItem.WidgetContainer.id
                    if(type === "org.ukui.dataIsland") {
                        return mainLayout.dataIslandWidth;
                    } else if (type === "org.ukui.settingsIsland"){
                        return mainLayout.settingsIslandWidth;
                    }
                }

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = Qt.binding(function () {return widgetParent})
                    }
                }
            }
        }
    }
}
