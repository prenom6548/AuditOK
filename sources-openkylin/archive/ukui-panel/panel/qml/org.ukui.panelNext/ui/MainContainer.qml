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

WidgetContainerItem {
    id: containerItem
    property int orientation: WidgetContainer.orientation
    property bool isHorizontal: orientation === Types.Horizontal
    onParentChanged: {
        anchors.fill = parent
    }

    Item {
        id: backGround
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height - anchors.topMargin - anchors.bottomMargin

        anchors {
            leftMargin: containerItem.WidgetContainer.margin.left
            topMargin: containerItem.WidgetContainer.margin.top
            rightMargin: containerItem.WidgetContainer.margin.right
            bottomMargin: containerItem.WidgetContainer.margin.bottom
        }
        Binding on width {
            value: {
                if(mainLayout.fillWidth) {
                    return parent.width - containerItem.WidgetContainer.margin.left - containerItem.WidgetContainer.margin.right
                } else {
                    return Math.max(1, (mainLayout.dataIslandWidth + mainLayout.settingsIslandWidth + mainLayout.appIslandWidth + mainLayout.columnSpacing * (repeater.count - 1)))
                }
            }
            delayed: true;
        }

        MouseArea {
            Accessible.name: "ukui-panel_Next_widget_container"
            x: mainLayout.x; y: mainLayout.y
            width: mainLayout.width; height: mainLayout.height
            hoverEnabled: panelNext.enableCustomSize
            enabled: !panelNext.lockPanel
            property bool resizing: false

            onResizingChanged: {
                if (!resizing) {
                    panelNext.endResizeCustomPanelSize();
                }
            }

            onPositionChanged: (event) => {
                if (!pressed) {
                    // 悬浮状态，切换鼠标样式
                    if (containsMouse) {
                        let cursor = (event.y < 2) ? "SizeVerCursor" : "";
                        panelNext.changeCursor(cursor);
                    }
                }
            }

            onReleased: {
                panelNext.changeCursor("");
                resizing = false;
            }

            onExited: {
                if (!pressed) {
                    panelNext.changeCursor("");
                }
                resizing = false;
            }

            onPressed: (event) => {
                let cursor = (event.y < 2) ? "SizeVerCursor" : "";
                if ((cursor !== "") && panelNext.enableCustomSize) {
                    resizing = true;
                    panelNext.startResizeCustomPanelSize(Qt.TopEdge);
                }
            }
        }
        GridLayout {
            id: mainLayout
            property int dataIslandWidth: 0
            property int settingsIslandWidth: 0
            property int appIslandWidth: 0
            property bool fillWidth: false
            property double ratio: 1

            Binding {
                target: mainLayout
                property: "ratio"
                value: Math.max(Math.min(backGround.height / WidgetContainer.config.panelDefaultSize, 1.43), 0.85)
                delayed: true
            }

            Component.onCompleted: {
                fillWidth = Qt.binding(function () {
                    for (var i = 0; i < repeater.count; i++) {
                        if (repeater.itemAt(i).Layout.fillWidth) {
                            return true;
                        }
                    }
                    return false
                });
            }
            z: 10
            anchors.fill: parent
            anchors {
                leftMargin: 0
                topMargin: 4 * ratio
                rightMargin: 0
                bottomMargin: 4 * ratio
            }
            rows: containerItem.isHorizontal ? 1 : repeater.count
            columns: containerItem.isHorizontal ? repeater.count : 1
            rowSpacing: 8 * ratio
            columnSpacing: 8 * ratio

            Repeater {
                id: repeater
                model: Panel.WidgetSortModel
                {
                    widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                    sourceModel: containerItem.widgetItemModel
                }
                delegate: widgetLoaderComponent
            }
            onWidthChanged: {
                blurRegionHelper.requestUpdate()
                panelNext.setPanelLength(width)
            }
        }
        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                property int index: model.index
                property Item widgetItem: model.widgetItem
                property double ratio: mainLayout.ratio
                property string containerType: "panel"
                property string type: ""

                Layout.fillWidth: widgetItem.fillWidth
                Layout.fillHeight: true

                Layout.minimumHeight: backGround.height;
                Layout.maximumHeight: backGround.height;
                Layout.minimumWidth : widgetItem.WidgetContainer.config.minimumWidth * ratio
                Layout.maximumWidth : (widgetItem.WidgetContainer.config.type === "app") ?
                                    (containerItem.width - mainLayout.settingsIslandWidth - mainLayout.dataIslandWidth - mainLayout.columnSpacing * (repeater.count -1))
                                    : widgetItem.WidgetContainer.config.maximumWidth * ratio

                Layout.preferredHeight: mainLayout.height
                Layout.preferredWidth: {
                    const type = widgetItem.WidgetContainer.config.type
                    if(type === "data") {
                        return mainLayout.dataIslandWidth;
                    } else if(type === "settings") {
                        return mainLayout.settingsIslandWidth
                    } else {
                        return mainLayout.appIslandWidth;
                    }
                }

                onWidgetItemChanged: {
                    type = widgetItem.WidgetContainer.config.type
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = Qt.binding(function () {return widgetParent})
                        if(type === "data") {
                            mainLayout.dataIslandWidth = Qt.binding(function () {return widgetItem.widgetsWidth;});
                        }
                        if(type === "settings") {
                            mainLayout.settingsIslandWidth = Qt.binding(function () {return widgetItem.widgetsWidth;})
                        }
                        if(type === "app") {
                            mainLayout.appIslandWidth = Qt.binding(function ()
                                {
                                    return widgetItem.Layout.preferredWidth;
                                })
                        }
                    }
                }

                Component.onDestruction: {
                    if(type === "data") {
                        mainLayout.dataIslandWidth = 0;
                    }
                    if(type === "settings") {
                        mainLayout.settingsIslandWidth = 0;
                    }
                }

                opacity: {
                    if (IslandDragHelper.draggingIsland === widgetItem.WidgetContainer.id
                        && IslandDragHelper.dragging
                        && !IslandDragHelper.islandPendingAdd2Panel) {
                        blurRegionHelper.removeItem(widgetParent);
                        return 0;
                    } else {
                        blurRegionHelper.addItem(widgetParent, GlobalTheme.kRadiusWindow * ratio);
                        return 1;
                    }
                }

                Component.onCompleted: {
                    blurRegionHelper.addItem(widgetParent, GlobalTheme.kRadiusWindow * ratio);
                    widgetParent.onRatioChanged.connect(function () {blurRegionHelper.addItem(widgetParent, GlobalTheme.kRadiusWindow * ratio);})
                }
                onXChanged: {
                    blurRegionHelper.requestUpdate(widgetParent)
                }
                onYChanged: {
                    blurRegionHelper.requestUpdate(widgetParent)
                }
                onWidthChanged: {
                    blurRegionHelper.requestUpdate(widgetParent)
                }
                onHeightChanged: {
                    blurRegionHelper.requestUpdate(widgetParent)
                }
            }
        }
    }
}
