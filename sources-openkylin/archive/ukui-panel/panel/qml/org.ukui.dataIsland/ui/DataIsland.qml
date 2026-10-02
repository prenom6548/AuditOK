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
import org.ukui.quick.platform 1.0
import org.ukui.panel.impl 1.0 as Panel
import org.ukui.panel.shell 1.0
import QtGraphicalEffects 1.0
import "../../"
WidgetContainerItem {
    id: containerItem
    property bool isHorizontal: WidgetContainer.orientation === Types.Horizontal
    property int widgetsWidth : controlRod.width + mainLayout.childrenRect.width + mainLayout.anchors.leftMargin + mainLayout.anchors.rightMargin
    property int widgetsHeight: mainLayout.height
    property double ratio: parent.ratio ? parent.ratio : 1
    property bool isMiniLayout: isHorizontal && (height <= 36)
    property bool fillWidth: false
    property string containerType: parent.containerType
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
        property double ratio: containerItem.ratio

        Binding on width {
            value: parent.width
            delayed: true;
        }
        height: parent.height
        borderColor: GlobalTheme.buttonTextActive
        borderAlpha: 0.25
        border.width: containerItem.containerType === "panel"? 1 : 0
        radius: containerItem.containerType === "panel" ? GlobalTheme.kRadiusWindow * ratio : 0
        layer.enabled: true;
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: backGround.width
                height: backGround.height;
                radius: backGround.radius;
            }
        }

        IslandDragControlRod {
            Accessible.name: "ukui-panel_Island_drag_control_rod"
            id: controlRod
            height: parent.height
            anchors.left: parent.left
            anchors.top: parent.top
            targetIsland: containerItem
            container: containerItem.containerType
        }

        GridLayout {
            id: mainLayout
            z: 10
            height: parent.height
            anchors.left: controlRod.right
            anchors {
                leftMargin: 0
                topMargin: 0
                rightMargin: 16 * backGround.ratio
                bottomMargin: 0
            }
            rows: containerItem.isHorizontal ? 1 : repeater.count
            columns: containerItem.isHorizontal ? repeater.count : 1
            rowSpacing: isMiniLayout ? 8 * backGround.ratio : 16 * backGround.ratio
            columnSpacing: isMiniLayout ? 8 * backGround.ratio : 16 * backGround.ratio
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

                property double ratio: backGround.ratio
                property int index: model.index
                property Item widgetItem: model.widgetItem

                Layout.fillWidth: widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem.Layout.minimumWidth
                Layout.minimumHeight: widgetItem.Layout.minimumHeight

                Layout.maximumWidth: widgetItem.Layout.maximumWidth
                Layout.maximumHeight: widgetItem.Layout.maximumHeight

                Layout.preferredWidth: widgetItem.Layout.preferredWidth
                Layout.preferredHeight: mainLayout.height
                Layout.alignment: Qt.AlignLeft

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = widgetParent
                    }
                }
            }
        }
    }
}
