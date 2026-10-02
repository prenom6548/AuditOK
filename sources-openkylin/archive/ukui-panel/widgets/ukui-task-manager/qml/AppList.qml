/*
 *
 *  Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQml.Models 2.15
import QtQuick.Layouts 1.12
import org.ukui.quick.widgets 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager

Item {
    id: root
    property int iconIndexAboutToBeScaled: -1
    property alias model: visualModel.model
    property bool isHorizontal: orientation === ListView.Horizontal
    property alias spacing: normalView.spacing
    property int itemSize: 48
    property real itemMargin: 4 * ratio
    property real ratio: 1

    signal toNormalFinished()
    signal toDraggingFinished()
    signal requestThumbnailView(bool show, TaskItemData data, Item item)

    function indexAt(x,  y) {
        let idx = normalView.normalViewItemIdx(x, y, normalView.spacing/2, true);
        if (idx === -1) {
            idx = normalView.normalViewItemIdx(x, y, normalView.spacing/2, false);
        }
        return idx;
    }

    function updateIconGeometry() {
        for (var i = 0; i < visualModel.count; i++) {
            let item = repeater.itemAt(i);
            if (item) {
                Platform.WindowManager.setMinimizedGeometry(model[i], item);
            }
        }
    }

    DelegateModel {
        id: visualModel
        delegate: Item {
            width: isHorizontal ? visualDelegate.width : root.itemSize
            height: root.itemSize

            Timer {
                id: listDragTimer
                interval: 100
                onTriggered: {
                    if (listDropArea.containsDrag) {
                        root.updateIconGeometry();
                        if (modelData !== "") {
                            UkuiTaskManager.TaskManager.activateWindow(modelData)
                        }
                    }
                }
            }
            DropArea {
                id: listDropArea
                anchors.fill: parent
                enabled: UkuiTaskManager.TaskManager.enableDragToApp

                onEntered: (dragEvent) => {
                    if (taskItemListRoot.dragEventSource(dragEvent) !== "DropToAppIcon") {
                        dragEvent.accepted = false;
                        return;
                    }
                    listDragTimer.restart();
                }
                onExited: {
                    listDragTimer.stop();
                    tooltip.hideTooltip();
                }
            }

            UkuiItems.Tooltip {
                id: tooltip
                anchors.fill: parent
                mainText: taskItemData.Name
                posFollowCursor: false
                active: listDropArea.containsDrag
                // TODO: 优化
                // 先有entered信号，再触发containsDragChanged信号，之后才有tooltip.active变为true
                // 只有当tooltip.active为true时，调用tooltip.showTooltip()才可以生效
                onActiveChanged: {
                    if (active) {
                        tooltip.showTooltip();
                    }
                }
                location: {
                    switch(Widget.container.position) {
                        case UkuiItems.Types.Bottom:
                            return UkuiItems.Dialog.BottomEdge;
                        case UkuiItems.Types.Left:
                            return UkuiItems.Dialog.LeftEdge;
                        case UkuiItems.Types.Top:
                            return UkuiItems.Dialog.TopEdge;
                        case UkuiItems.Types.Right:
                            return UkuiItems.Dialog.RightEdge;
                    }
                }
                margin: 8
            }

            UkuiItems.DtThemeBackground {
                id: visualDelegate
                width: viewLayout.width
                height: root.itemSize - itemMargin*2
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                useStyleTransparency: false
                backgroundColor: listDropArea.containsDrag ? Platform.GlobalTheme.kComponentAlphaHover :
                                                              normalLayout.containsPress ? Platform.GlobalTheme.kComponentAlphaClick :
                                                              normalLayout.containsMouse ? Platform.GlobalTheme.kComponentAlphaHover :
                                                                                           Platform.GlobalTheme.kContainGeneralAlphaNormal
                radius: Platform.GlobalTheme.kRadiusNormal


                RowLayout {
                    id: viewLayout
                    height: parent.height
                    spacing: 0
                    onChildrenRectChanged: width = childrenRect.width
                    z: 1

                    UkuiItems.Icon {
                        id: themeicon
                        Layout.margins: itemMargin
                        Layout.preferredWidth: height
                        Layout.maximumWidth: height
                        Layout.fillHeight: true
                        source: taskItemData.WindowIconsName[modelData] === "" ? taskItemData.WindowIcons[modelData] : taskItemData.WindowIconsName[modelData]
                        addUnifiedIconBox: true

                        scale: (iconIndexAboutToBeScaled === index) ? 0.8 : 1.0
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    UkuiItems.DtThemeText {
                        id: text
                        Layout.preferredWidth: text.implicitWidth
                        Layout.maximumWidth: 192
                        Layout.fillHeight: true
                        Layout.rightMargin: 8
                        visible: isHorizontal

                        text: (taskItemData.WindowTitles[modelData] === undefined) ? "" : taskItemData.WindowTitles[modelData]
                        textColor: Platform.GlobalTheme.kFontPrimary
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                        fontSizeMode: Text.VerticalFit
                    }
                }

                // 仅用于获取鼠标悬浮事件，requestThumbnailView
                MouseArea {
                    id: normalLayout
                    Accessible.role: Accessible.Button
                    Accessible.description: "This is task manager button on list mode of ukui-panel"
                    Accessible.name: taskItemData.Id + index
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true

                    TaskItemData {
                        id: singleItemData
                        windowIdList: [modelData]
                        windowIcons:  taskItemData.WindowIcons
                        windowIconsName: taskItemData.WindowIconsName
                        windowTitles: taskItemData.WindowTitles
                    }

                    onContainsMouseChanged: {
                        mouseArea.aboutToShowThumbnailView(containsMouse, singleItemData, this);
                    }
                }

                Rectangle {
                    id: demandsAttention
                    anchors.fill: parent
                    color: "#FF9100"
                    radius: 8
                    visible: taskItemData.DemandsAttentionWinIdList.includes(modelData)

                    NumberAnimation on opacity {
                        running: demandsAttention.visible
                        loops: 4
                        from: 0.25
                        to: 0.6
                        duration: 450
                    }
                }
            }
            UkuiItems.DtThemeBackground {
                height: 4 * ratio
                width: 8 * ratio
                radius: 2 * ratio

                anchors.verticalCenter: visualDelegate.bottom
                anchors.left: visualDelegate.left
                anchors.leftMargin: 16 * ratio
                useStyleTransparency: false
                backgroundColor: (modelData === UkuiTaskManager.TaskManager.currentActiveWinId)
                    ? Platform.GlobalTheme.kBrandNormal : Platform.GlobalTheme.kGrayAlpha9
            }
        }
    }

    Grid {
        id: normalView
        width: childrenRect.width
        height: childrenRect.height
        rows: isHorizontal ? 1 : visualModel.count
        columns: isHorizontal ? visualModel.count : 1
        Repeater {
            id: repeater
            model: visualModel
        }
        function indexAtPosition(x, y) {
            for (let i = 0; i < visualModel.count; ++i) {
                let item = repeater.itemAt(i);
                let point = item.mapFromItem(root, Qt.point(x, y));
                if (item.contains(point)) {
                    return i;
                }
            }
            return -1;
        }
        function normalViewItemIdx(x, y, offset, forward) {
            if (isHorizontal) {
                return normalView.indexAtPosition(forward ? x - offset : x + offset, y);
            } else {
                return normalView.indexAtPosition(x, forward ? y - offset : y + offset);
            }
        }
    }
}
