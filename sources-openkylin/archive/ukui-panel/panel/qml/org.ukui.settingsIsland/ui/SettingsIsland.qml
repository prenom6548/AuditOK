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
import org.ukui.panel.shell 1.0
import QtGraphicalEffects 1.0
import "../../"

WidgetContainerItem {
    id: containerItem
    property int orientation: WidgetContainer.orientation
    property bool isHorizontal: orientation === Types.Horizontal
    property bool fillWidth: false
    property double ratio: parent.ratio ? parent.ratio : 1
    property bool isMiniLayout: isHorizontal && (height <= 36)
    property int widgetsWidth : controlRod.width + mainLayout.childrenRect.width + mainLayout.anchors.leftMargin + mainLayout.anchors.rightMargin
    property int widgetsHeight: mainLayout.height
    property string containerType: parent.containerType
    property int trayAreaMaxLength: containerItem.WidgetContainer.screen === null ? 0 : containerItem.WidgetContainer.screen.size.width * 0.27
    property int systemTrayLength: Math.max(0, trayAreaMaxLength - calendarLength)
    property int calendarLength: 0

    Component.onCompleted: {
        fillWidth = Qt.binding(function (){
            for (var i = 0; i < repeater.count; i++) {
                if(repeater.itemAt(i).Layout.fillWidth) {
                    return true;
                }
            }
            return false
        });
    }
    WidgetContainer.active: IslandDragHelper.dragging;

    DtThemeBackground {
        id: backGround
        Binding on width {
            value: parent.width
            delayed: true;
        }
        height: parent.height
        borderColor: GlobalTheme.buttonTextActive
        borderAlpha: 0.25
        border.width: containerItem.containerType === "panel"? 1 : 0
        radius: containerItem.containerType === "panel" ? GlobalTheme.kRadiusWindow * ratio: 0
        layer.enabled: true;
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: backGround.width
                height: backGround.height;
                radius: backGround.radius;
            }
        }
        IslandDragControlRod {
            id: controlRod
            Accessible.name: "ukui-panel_Island_drag_control_rod"
            height: parent.height
            anchors.right: parent.right
            anchors.top: parent.top
            targetIsland: containerItem
            container: containerItem.containerType
        }

        GridLayout {
            id: mainLayout
            z: 10
            // anchors.fill: parent
            height: parent.height
            anchors {
                leftMargin: 8 * ratio
                topMargin: 0
                rightMargin: 0
                bottomMargin: 0
            }
            anchors.right: controlRod.left
            rows: containerItem.isHorizontal ? 1 : repeater.count
            columns: containerItem.isHorizontal ? repeater.count : 1
            rowSpacing: isMiniLayout ? 6 * ratio : 4 * ratio
            columnSpacing: isMiniLayout ? 6 * ratio : 4 * ratio
            opacity: controlRod.Drag.active ? 0 :
                    IslandDragHelper.dragging && IslandDragHelper.draggingIsland === WidgetContainer.id ? 0 : 1

            Repeater {
                id: repeater
                model: Panel.WidgetSortModel {
                    // TODO: use config
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

                property double ratio: containerItem.ratio
                property int index: model.index
                property Item widgetItem: model.widgetItem
                property string itemId: widgetItem.Widget.id

                Layout.fillWidth: widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem.Layout.minimumWidth
                Layout.minimumHeight: widgetItem.Layout.minimumHeight

                Layout.preferredWidth: widgetItem.Layout.preferredWidth
                Layout.preferredHeight: widgetItem.Layout.preferredHeight

                Layout.maximumWidth: itemId === "org.ukui.systemTray" ? containerItem.systemTrayLength : widgetItem.Layout.maximumWidth
                Layout.maximumHeight: widgetItem.Layout.maximumHeight;

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = widgetParent
                    }

                    if (itemId === "org.ukui.panel.calendar") {
                        containerItem.calendarLength = Qt.binding(function(){
                            return width;
                        })
                    }
                }
            }
        }
    }
}
