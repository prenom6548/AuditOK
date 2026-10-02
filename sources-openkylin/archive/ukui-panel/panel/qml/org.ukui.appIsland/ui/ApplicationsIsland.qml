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
    property bool fillWidth: false
    property double ratio: parent.ratio ? parent.ratio : 1
    Layout.preferredWidth: backGround.width
    Layout.maximumWidth: parent.Layout.maximumWidth

    Layout.onMaximumWidthChanged: {
        mainLayout.updateTaskManagerMaxLength();
    }
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

    DtThemeBackground {
        id: backGround
        property double ratio: containerItem.ratio
        Binding on width {
            value: {
                if (containerItem.fillWidth) {
                    return containerItem.width
                } else {
                    return Math.min(mainLayout.width + flickView.anchors.leftMargin + flickView.anchors.rightMargin,
                                    containerItem.Layout.maximumWidth)
                }
            }
            delayed: true;
        }
        height: parent.height
        borderColor: GlobalTheme.buttonTextActive
        borderAlpha: 0.25
        border.width: 1
        radius: ratio * GlobalTheme.kRadiusWindow

        MouseArea {
            Accessible.name: "ukui-panel_applications_Island"
            anchors.fill: parent
            enabled: flickView.contentWidth > flickView.width
            onWheel: (wheel) => {
                flickView.flick(wheel.angleDelta.y * 10, 0);
            }
        }
        Flickable {
            id: flickView
            z: 10
            anchors.fill: parent
            anchors {
                leftMargin: 12 * backGround.ratio
                topMargin: 0
                rightMargin: 12 * backGround.ratio
                bottomMargin: 0
            }

            contentWidth: mainLayout.width
            contentHeight: backGround.height

            boundsBehavior: Flickable.StopAtBounds
            boundsMovement: Flickable.StopAtBounds

            GridLayout {
                id: mainLayout
                height: backGround.height

                rows: containerItem.isHorizontal ? 1 : repeater.count
                columns: containerItem.isHorizontal ? repeater.count : 1
                rowSpacing: 4 * backGround.ratio
                columnSpacing: 4 * backGround.ratio
                property int taskManagerMaxLength: containerItem.Layout.maximumWidth
                property var otherPluginsWidth: {"": 0}

                function otherPluginsTotalWidth() {
                    let sum = 0;

                    for (var key in mainLayout.otherPluginsWidth) {
                        var width = mainLayout.otherPluginsWidth[key];
                        if (typeof width === 'number') {
                            sum += width;
                        }
                    }
                    return sum;
                }

                function updateTaskManagerMaxLength() {
                    let spacing = mainLayout.rowSpacing * repeater.count + flickView.anchors.leftMargin + flickView.anchors.rightMargin;
                    taskManagerMaxLength = Math.max(0, (containerItem.Layout.maximumWidth - spacing - otherPluginsTotalWidth()));
                }

                Repeater {
                    id: repeater
                    model: Panel.WidgetSortModel
                    {
                        // TODO: use config
                        widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                        sourceModel: containerItem.widgetItemModel
                    }
                    delegate: widgetLoaderComponent
                }
            }
        }
        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                Component.onDestruction: {
                    if (itemId !== "org.ukui.panel.taskManager" && widgetItem.Widget.instanceId > -1) {
                        let instanceId = widgetItem.Widget.instanceId;
                        let success = delete mainLayout.otherPluginsWidth[instanceId];
                        if (!success) {
                            mainLayout.otherPluginsWidth[instanceId] = 0;
                        }
                        mainLayout.updateTaskManagerMaxLength();
                    }
                }
                property int index: model.index
                property double ratio: backGround.ratio
                property Item widgetItem: model.widgetItem
                property string itemId: widgetItem.Widget.id
                property bool isIslandMode: true

                Layout.fillWidth: widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem.Layout.minimumWidth
                Layout.minimumHeight: widgetItem.Layout.minimumHeight

                Layout.maximumWidth: (itemId === "org.ukui.panel.taskManager") ?
                                    Math.max(widgetItem.Layout.minimumWidth, mainLayout.taskManagerMaxLength) : widgetItem.Layout.maximumWidth

                Layout.maximumHeight: widgetItem.Layout.maximumHeight

                Layout.preferredWidth: widgetItem.Layout.preferredWidth
                Layout.preferredHeight: widgetItem.Layout.preferredHeight

                onWidthChanged: {
                    if (itemId !== "org.ukui.panel.taskManager" && widgetItem.Widget.instanceId > -1) {
                        mainLayout.otherPluginsWidth[widgetItem.Widget.instanceId] = width;
                        mainLayout.updateTaskManagerMaxLength();
                    }
                }
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
