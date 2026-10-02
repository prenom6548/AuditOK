/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQml.Models 2.15
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Flow {
    id: shortcutFlow

    property alias buttonModel: itemModel
    property var focusGroups: []
    property int itemWidth: 80
    property int itemHeight: 72

    property int rows
    property int rowSpacing: 12
    property int columns: 4
    property int columnSpacing: 12

    property int modelType: PluginState.Add
    property bool isExit: false

    property Component iconComponent: IconButton{}
    property Component menuComponent: MenuButton{}
    property Component noneComponent: Item { anchors.fill: parent;}

    signal changePosition()

    function refreshFocusItems() {
        var groups = [];
        for (var i = 0; i < itemView.count; ++i) {
            var delegate = itemView.itemAt(i);
            if (delegate && delegate.focusTargets && delegate.focusTargets.length > 0) {
                groups.push(delegate.focusTargets);
            }
        }
        focusGroups = groups;
    }

    width: columns * (itemWidth + columnSpacing) - columnSpacing
    height: Math.max((rows * (itemHeight + rowSpacing) - rowSpacing),
                     (3 * (itemHeight + rowSpacing) - rowSpacing))
    spacing: 12

    move: Transition {
        NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 200 }
    }

    Repeater {
        id: itemView
        property int sourceIndex

        model: DelegateModel {
            id: itemModel
            model: editModel.getModel(modelType)

            delegate: Item {
                id: controlBase

                property int visualIndex: DelegateModel.itemsIndex
                property int columnSpan: model.itemType === ShortcutPlugin.Icon ? 1 : 2
                property var focusTargets: itemLoader.item && itemLoader.item.focusTargets
                                          ? itemLoader.item.focusTargets : []

                width: isExit && control.drag.active ? 0 : model.itemType === ShortcutPlugin.Icon ? itemWidth : itemWidth * 2 + columnSpacing
                height: itemHeight

                Binding { target: control; property: "selectIndex"; value: visualIndex}

                Component.onCompleted: {
                    changeState(visualIndex);
                }

                onVisualIndexChanged: {
                    changeState(visualIndex);
                }

                function toChangeState(index) {
                    if (index < visualIndex && visualIndex <= itemModel.count - 1) {
                        changeState(visualIndex);
                    }
                }

                Connections {
                    target: itemView.parent
                    function onChangePosition() {
                        toChangeState();
                    }
                }

                function changeState() {
                    itemModel.model.updateItemRect(itemView.parent, modelType)
                }

                Loader {
                    id: itemLoader
                    anchors.fill: parent
                    anchors.centerIn: parent

                    // properties
                    property int index: model.index === undefined ? 0 : model.index
                    property int buttonColor: model.color === undefined ? 0 : model.color
                    property int buttonValue: model.value === undefined ? 0 : model.value
                    property bool disabled: model.disabled === undefined ? false : model.disabled
                    property bool dragMask: false
                    property string icon: model.icon === undefined ? "" : model.icon
                    property string name: model.name === undefined ? "" : model.name
                    property string statusName: model.statusName === undefined ? "" : model.statusName
                    property string widgetId: model.widgetId === undefined ? "" : model.widgetId
                    property string tooltip: model.tooltip === undefined ? "" : model.tooltip

                    sourceComponent: getComponent()

                    function getComponent() {
                        if (model.name === undefined) {
                            return noneComponent;
                        }
                        return model.itemType === ShortcutPlugin.Icon
                                ? iconComponent : menuComponent;
                    }

                    function activeButton(act) {
                        console.log(tooltip, "active", " action:", act)
                        itemModel.model.active(index, act)
                    }
                    function setValue(val) {
                        console.log(tooltip, "setValue", " value:", val)
                        itemModel.model.setValue(index, val)
                    }
                    function setDisableValueUpdate(disable) {
                        itemModel.model.disableValueUpdate(disable)
                    }

                    onLoaded: {
                        if (model.name === undefined) return;
                        item.active.connect(activeButton);
                        item.setValue.connect(setValue);
                        item.disableValueUpdate.connect(setDisableValueUpdate);
                        Qt.callLater(itemView.parent.refreshFocusItems);
                    }

                    Component.onDestruction: {
                        if (model.name === undefined) return;
                        item.active.disconnect(activeButton);
                        item.setValue.disconnect(setValue);
                        item.disableValueUpdate.disconnect(setDisableValueUpdate);
                        Qt.callLater(itemView.parent.refreshFocusItems);
                    }
                }

                Connections {
                    target: itemLoader.item
                    ignoreUnknownSignals: true
                    function onFocusTargetsChanged() {
                        Qt.callLater(itemView.parent.refreshFocusItems);
                    }
                }

                MouseArea {
                    id: control
                    property int selectIndex: 0
                    property var rootId: dropArea
                    property int type: modelType
                    property int buttonType: model.itemType
                    property string buttonId: model.pluginId === undefined ? "" : model.pluginId
                    anchors.fill: parent
                    enabled: dropArea.isEdit
                    hoverEnabled: dropArea.isEdit

                    Drag.active: false
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2
                    Drag.dragType: Drag.Automatic
                    Drag.mimeData: {
                        "source": "ukui-sidebar"
                    }
                    drag.target: control
                    drag.onActiveChanged: {
                        if (drag.active) {
                            toStartDrag();
                            itemLoader.opacity = 0;
                        }
                        Drag.active = drag.active;
                    }

                    onPressed: {
                        itemLoader.dragMask = true;
                        itemLoader.grabToImage(function(result) {
                            control.Drag.imageSource = result.url;
                        })
                    }

                    onReleased: {
                        itemLoader.dragMask = false;
                    }

                    Drag.onDragFinished: {
                        isExit = false;
                        dropArea.dragging = false;
                        itemLoader.dragMask = false;
                        itemLoader.opacity = 1;
                        if (!dropArea.inserted) {
                            itemModel.model.moveItem(itemView.sourceIndex, controlBase.visualIndex);
                        }
                    }

                    function toStartDrag() {
                        dropArea.dragging = true;
                        itemView.sourceIndex = selectIndex;
                    }
                }

                EditModeFlag {
                    isEdit: dropArea.isEdit && !dropArea.dragging
                    buttonType: model.itemType
                    addState: modelType
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: -8
                    anchors.rightMargin: -8

                    onClicked: {
                        itemModel.model.changeState(control.buttonId, modelType, (buttonType === ShortcutPlugin.Icon));
                    }
                }
            }
        }
        onCountChanged: {
            itemModel.model.updateItemRect(itemView.parent, modelType);
            Qt.callLater(parent.refreshFocusItems);
            if (!mainWindow.visible) {
                parent.forceLayout();
            }
        }
    }

    Component.onCompleted: Qt.callLater(refreshFocusItems)
}
