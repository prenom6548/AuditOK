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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */
import QtQml 2.15
import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Layouts 1.12

import org.ukui.quick.widgets 1.0
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager
import org.ukui.quick.items 1.0 as Items
import org.ukui.quick.platform 1.0 as Platform

// taskManager全部界面显示（包括预览图）
TaskManagerBase {
    id: taskManager
    // 任务栏是否激活： 预览图是否显示、是否有高亮显示、是否有拖拽事件、是否打开右键菜单
    Widget.active: taskManagerActived || thumbnailView.visible || isDragging
    property var widgetScreenGeo: Widget.container.screen.availableGeometry

    function menuAboutToHide() {
        viewContent.menuVisible = false;
        taskManagerActived = false;
        isTaskMenuShow = false;
    }

    function hidethumbnail() {
        UkuiTaskManager.TaskManager.activateWindowView([]);
        thumbnailView.hide();
        viewContent.clearThumbnailModel();
    }

    function showthumbnail() {
        if (!viewContent.currentWinIdList) {
            return;
        }
        if (viewContent.currentWinIdList.length !== 0) {
            thumbnailView.show();
        }
    }
    Component.onCompleted: {
        taskManager.viewModel.menuAboutToHide.connect(menuAboutToHide)
    }
    Component.onDestruction: {
        taskManager.viewModel.menuAboutToHide.disconnect(menuAboutToHide)
    }

    state: ""
    states: [
        State {
            name: "aboutToShowThumbnail"
        },
        State {
            name: "aboutToHideThumbanil"
        }
    ]

    transitions: [
        Transition {
            to: "aboutToShowThumbnail"
            SequentialAnimation {
                PauseAnimation { duration: 400 }
                ScriptAction {
                    script: {
                        if (!thumbnailView.visible && !isTaskMenuShow) {
                            viewContent.updateThumbnailView();
                            thumbnailView.updatePosition();
                            thumbnailView.updateThumbnailOffset();
                            showthumbnail();
                        }
                    }
                }
            }
        },
        Transition {
            to: "aboutToHideThumbanil"
            SequentialAnimation {
                PauseAnimation { duration: 400 }
                ScriptAction {
                    script: {
                        if (thumbnailView.visible && (!taskItemData.mouse.containsMouse || isTaskMenuShow)) {
                            hidethumbnail();
                        }
                    }
                }
            }
        }
    ]

    Timer {
        id: changedThumbnailData
        interval: 400
        onTriggered: {
            if (taskItemData.mouse.containsMouse) {
                viewContent.thumbnailData = taskItemData.dataToBeChanged;
            }
        }
    }
    onRequestThumbnailView: (show, data, mouse) => {
        taskItemData.mouse = mouse;
        if (show) {
            state = "aboutToShowThumbnail";
            if (thumbnailView.visible) {
                // timer
                taskItemData.dataToBeChanged = data;
                changedThumbnailData.restart();
            } else {
                viewContent.thumbnailData = data;
            }

        } else {
            if (taskItemData.mouse) {
                state = "aboutToHideThumbanil";
            } else {
                hidethumbnail();
            }
        }
    }

    QtObject {
        id: taskItemData
        // 当前预览图对应的taskItem，通过containsMouse判断是否数据更新 / 解决X下预览图尺寸变化会触发隐藏的bug
        property Item mouse
        property var dataToBeChanged
    }

    Items.Dialog {
        id: thumbnailView
        mainItem: viewContent
        enableWindowBlur: true
        type: Platform.Settings.platformName === "wayland" ? Platform.WindowType.Switcher : Platform.WindowType.PopupMenu
        flags: Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint

        /**
         *  [注]: 以Items.Types.Bottom（任务栏在下）为例，关于horizontalCenterOffset的计算：
         *  由于Dialog窗口需要加上阴影大小，所以窗口大小比内部预览图要大，默认情况下，预览图是居中显示的（horizontalCenterOffset = 0）
         *  在满屏时，此时Dialog已经比Screen.width要大，再居中就不对了，所以需要调整horizontalCenterOffset
         *  在缩放情况下有时不会完美铺满屏幕，所以预览图需要在屏幕上居中显示(horizontalCenterOffset = thumbnailView.width - Screen.width)/2 * -1
         *  并且设计稿标注，预览图与窗口间距为8(horizontalCenterOffset = (viewContent.cachedOffset - viewContent.screenMargin) * -1)
         */
        onWidthChanged: updateThumbnailOffset();
        onHeightChanged: updateThumbnailOffset();

        function updateListBlurRegion() {
            thumbnailView.windowBlurRegion.clear();
            thumbnailView.windowBlurRegion.appendRect(windowRegion);
        }

        function updateThumbnailOffset() {
            var actualHorizontalOffset = 0;
            var actualVerticalOffset = 0;
            if (viewContent.isHorizontal) {
                actualHorizontalOffset = (thumbnailView.width > taskManager.widgetScreenGeo.width) ?
                    Math.max((viewContent.cachedOffset - viewContent.screenMargin) * -1, (thumbnailView.width - taskManager.widgetScreenGeo.width)/2 * -1) : 0;
                    viewContent.anchors.horizontalCenterOffset = actualHorizontalOffset;
            } else {
                actualVerticalOffset = (thumbnailView.height > taskManager.widgetScreenGeo.height) ?
                    Math.max((viewContent.cachedOffset - viewContent.screenMargin) * -1, (thumbnailView.height - taskManager.widgetScreenGeo.height)/2 * -1) : 0;
                    viewContent.anchors.verticalCenterOffset = actualVerticalOffset;
            }
        }
        function updatePosition() {
            switch (Widget.container.position) {
                case Items.Types.Left:
                    thumbnailView.location = Items.Types.LeftEdge;
                    viewContent.anchors.horizontalCenterOffset = -(viewContent.offset-viewContent.viewMargin) / 2;
                    viewContent.anchors.verticalCenterOffset = 0;
                    break;

                case Items.Types.Top:
                    thumbnailView.location = Items.Types.TopEdge;
                    viewContent.anchors.horizontalCenterOffset = 0;
                    viewContent.anchors.verticalCenterOffset = -(viewContent.offset-viewContent.viewMargin)/2;
                    break;

                case Items.Types.Right:
                    thumbnailView.location = Items.Types.RightEdge;
                    viewContent.anchors.horizontalCenterOffset = (viewContent.offset-viewContent.viewMargin) / 2;
                    viewContent.anchors.verticalCenterOffset = 0;
                    break;

                case Items.Types.Bottom:
                    thumbnailView.location = Items.Types.BottomEdge;
                    viewContent.anchors.horizontalCenterOffset = 0;
                    viewContent.anchors.verticalCenterOffset = (viewContent.offset-viewContent.viewMargin) / 2;
                    break;

                default:
                    break;
            }
            thumbnailView.margin = 0;
        }

        Item {
            id: viewBase
            height: viewContent.isHorizontal ? (viewContent.height + viewContent.offset + viewContent.viewMargin) : (viewContent.height + viewContent.offset*2)
            width: viewContent.isHorizontal ? (viewContent.width + viewContent.offset*2) : (viewContent.width + viewContent.offset + viewContent.viewMargin)

            // 整个窗口的毛玻璃区域（为list模式时）
            Items.RectRegion {
                id: windowRegion
                x: viewContent.x
                y: viewContent.y
                width: viewContent.width
                height: viewContent.height
                radius: Platform.GlobalTheme.kRadiusWindow
            }
            /**
             * DropArea实现功能： 拖拽文件到多窗口下，预览图有悬浮显示，并600ms后激活对应窗口
             * 悬浮三态 ---- (ThumbnailWindow.qml处实现)mouseAreaContainsMouse与(viewDropArea.targetThumbnailIndex === index)绑定
             * 激活对应窗口 ---- 通过indexAt取到对应index并开始计时
             */
            DropArea {
                id: viewDropArea
                property int targetThumbnailIndex: -1

                enabled: UkuiTaskManager.TaskManager.enableDragToApp
                anchors.fill: viewContent
                anchors.centerIn: viewContent

                onPositionChanged: (drag) => {
                    if (taskManager.dragEventSource(drag) !== "DropToAppIcon") {
                        return;
                    }
                   var targetIndex = viewContent.thumbnailLayout.indexAt(drag.x, drag.y);
                    if (viewDropArea.targetThumbnailIndex !== targetIndex && targetIndex !== -1) {
                        viewDropArea.targetThumbnailIndex = targetIndex;
                        viewDragTimer.restart();
                    }
                }
                onExited: {
                    viewDropArea.targetThumbnailIndex = -1;
                    viewDragTimer.stop();
                }

                Timer {
                    id: viewDragTimer
                    interval: 600
                    onTriggered: {
                        if (viewDropArea.targetThumbnailIndex > -1 && viewDropArea.targetThumbnailIndex < thumbnailModel.count) {
                            UkuiTaskManager.TaskManager.activateWindow(thumbnailModel.get(viewDropArea.targetThumbnailIndex).winId);
                            taskManager.hidethumbnail();
                            viewDropArea.targetThumbnailIndex = -1;
                        }
                    }
                }
            }

            ThumbnailWindow {
                id: viewContent
                Accessible.role: Accessible.Button
                Accessible.description: "This is a thumbnail window of ukui-panel"
                Accessible.name: "ukui-panel thumbnail window"

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                property UkuiTaskManager.TaskItemData thumbnailData: null
                property var currentWinIdList: thumbnailData ? thumbnailData.windowIdList : []
                property var windowTitles: thumbnailData ? thumbnailData.windowTitles : []
                property var windowIcons: thumbnailData ? thumbnailData.windowIcons : []
                property var windowIconsName: thumbnailData ? thumbnailData.windowIconsName : []
                property bool isHorizontal: taskManager.view.orientation === ListView.Horizontal
                property bool hasFocus: viewContent.containsMouse || viewContent.menuVisible || viewDropArea.containsDrag
                property bool windowVisible: thumbnailView.visible
                property var widgetScreenGeo: taskManager.widgetScreenGeo

                onHasFocusChanged: {
                    if (hasFocus) {
                        taskManager.state = "aboutToShowThumbnail";
                    } else {
                        taskManager.state = "aboutToHideThumbanil";
                    }
                }

                onCurrentWinIdListChanged: updateThumbnailView()

                // 更新预览图model数据 && 更新毛玻璃区域
                function updateThumbnailView() {
                    updateThumbnailModel();
                    viewContent.determineListViewSize();
                    if (viewContent.isList) {
                        thumbnailView.updateListBlurRegion();
                    }

                    // 先设置mainItem的宽高 再切换visualParent,解决预览图闪烁问题
                    if (viewContent.thumbnailData) {
                        if (viewContent.thumbnailData.currentTaskItem !== null) {
                            thumbnailView.visualParent = viewContent.thumbnailData.currentTaskItem;
                        }
                    } else {
                        thumbnailView.visualParent = null;
                    }
                }

                property var modelList: []
                ListModel {
                    id: thumbnailModel
                }

                function clearThumbnailModel() {
                    modelList = [];
                    thumbnailModel.clear();
                }

                function updateThumbnailModel() {
                    // 这里不能直接用=（js中表示引用），需要浅拷贝
                    if (!currentWinIdList || currentWinIdList.length == 0) {
                        hidethumbnail();
                        return;
                    }
                    var listFromModel = modelList.slice(0);
                    var listFromCurrent = currentWinIdList.slice(0);
                    var indexOfRemoveList = [];

                    for (var i = 0; i < listFromModel.length; i++) {
                        if (listFromCurrent.includes(listFromModel[i])) {
                            listFromCurrent.splice(listFromCurrent.indexOf(listFromModel[i]), 1);
                        } else {
                            indexOfRemoveList.push(i);
                        }
                    }
                    for (var i = indexOfRemoveList.length - 1; i > -1; i--) {
                        listFromModel.splice(indexOfRemoveList[i], 1);
                        thumbnailModel.remove(indexOfRemoveList[i]);
                    }

                    for (var i = 0; i < listFromCurrent.length; i++) {
                        listFromModel.push(listFromCurrent[i]);
                        thumbnailModel.append({"winId": listFromCurrent[i]});
                    }

                    modelList = listFromModel;
                }
            }
        }
    }
}
