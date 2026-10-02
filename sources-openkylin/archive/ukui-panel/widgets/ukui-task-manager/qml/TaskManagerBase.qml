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
import QtQuick 2.15
import QtQml.Models 2.15

import org.ukui.quick.widgets 1.0
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager
import org.ukui.quick.items 1.0 as Items
import org.ukui.quick.platform 1.0 as Platform

DropArea {
    id: taskItemListRoot

    property real ratio: 1
    property var dragedObj: null
    property alias view: taskItemView
    property alias viewModel: taskItemProxyModel
    property bool taskManagerActived: false
    property bool isTaskMenuShow: false
    property bool isDragging: false
    property int resetItemsIndex: -1

    // 请求显示预览图
    signal requestThumbnailView(bool show, TaskItemData data, Item mouseArea)

    UkuiTaskManager.TaskManagerFilterModel {
        id: taskItemProxyModel
        screen: Widget.container.screen
    }

    Component {
        id: taskIconComponent
        Items.Icon {}
    }

    DelegateModel {
        id: taskItemDelegateModel
        property var dragedObject: null

        /**
         * 删除临时Item
         */
        function removeTempItems() {
            for (let i = tempItems.count - 1; i >= 0; --i) {
                const obj = tempItems.get(i);
                obj.groups = [tempItems.name];
                tempItems.remove(i);
            }
        }

        /**
         * 将对应itemsIndex的项，存在persistedItems组中，并从界面移除
         * @param itemsIndex
         */
        function removeVisualItem(itemsIndex) {
            if (itemsIndex < 0 || itemsIndex >= taskItemView.count) {
                return;
            }
            // 将被拖拽的项添加到持久化组中, 如果在items组中, 那么从可见列表移除
            items.get(itemsIndex).groups = [persistedItems.name];
        }

        /**
         * 将存起来的Item全部归位，并从persistedItems组中移除
         */
        function resetPersistedItems() {
            if (persistedItems.count > 0) {
                let i = persistedItems.count - 1;
                for (; i >= 0; --i) {
                    const persistedItemObj = persistedItems.get(i);
                    if (!persistedItemObj.inItems) {
                        // TODO: listview中的数据往前挪可以同步缓存位置，往后挪则位置不会变，需要把临时Item最后出现的位置记下来
                        let tmpItemsIndex = tempItems.get(0).itemsIndex;
                        persistedItems.addGroups(i, 1, [items.name]);
                        items.move(persistedItemObj.itemsIndex, tmpItemsIndex, 1);
                    }
                    persistedItems.remove(i, 1);
                }
            }

            dragedObject = null;
            removeTempItems();
        }

        // 临时生成的用于定位拖拽落点的Items组
        groups: [
            DelegateModelGroup { id: tempItems; name: "tempItems" }
        ]

        model: taskItemProxyModel
        delegate: Item {
            id: taskItemRoot

            property int dragStartIndex: -1
            property var icon: model.Icon
            property bool isDragStatus: false
            property bool useSingleItem: taskItemView.isMergeStatus || (CurrentWinIdList ? CurrentWinIdList.length < 1 : true)
            //非合并状态下激活预览图时 对应的winId
            property var singleWinId: []
            property bool iconNeedsUpdate: true
            // 三岛状态下，是否已切换为小插件样式
            property bool quickWidgetLoaded: false
            property bool isItemActived: model.DemandsAttentionWinIdList ? (model.DemandsAttentionWinIdList.length !== 0) : false

            width: taskItemLoader.width
            height: taskItemLoader.height
            visible: !DelegateModel.inPersistedItems

            onIsItemActivedChanged: {
                taskItemView.updateActivedItem(isItemActived);
            }

            onIconChanged: {
                iconNeedsUpdate = true;
            }
            function generateDragImage() {
                let icon = taskIconComponent.createObject(this, {width: taskItemView.taskItemSize, height: taskItemView.taskItemSize});
                if (icon !== null) {
                    icon.addUnifiedIconBox = true
                    icon.source = model.Icon
                    icon.grabToImage(function (result) {
                        taskItemRoot.Drag.imageSource = result.url;
                        icon.destroy();
                    });
                    icon.visible = false
                }
            }

            // 拖拽设置, 手动开启拖拽
            Drag.dragType: Drag.None
            Drag.hotSpot: Qt.point(taskItemView.taskItemSize/2, taskItemView.taskItemSize/2)
            // 处理拖拽完成信号
            Drag.onDragFinished: function (dropAction) {
                taskItemListRoot.isDragging = false;
                // 拖拽被取消，仅处理内部拖拽
                if (taskItemDelegateModel.persistedItems.count > 0) {
                    let i = taskItemDelegateModel.persistedItems.count - 1;
                    for (; i >= 0; --i) {
                        const persistedItemObj = taskItemDelegateModel.persistedItems.get(i);
                        if (!persistedItemObj.inItems) {
                            // TODO: listview中的数据往前挪可以同步缓存位置，往后挪则位置不会变，需要把临时Item最后出现的位置记下来
                            persistedItemObj.groups = [taskItemDelegateModel.items.name];
                        }
                    }
                }

                taskItemRoot.isDragStatus = false;
            }

            function startDrag() {
                // 存起来的被拖拽项目和临时生成的项目都不允许被拖动
                if (!DelegateModel.inItems || DelegateModel.inPersistedItems || DelegateModel.inTempItems) {
                    return;
                }

                const delegateModel = DelegateModel.model;
                const itemsIndex = DelegateModel.itemsIndex;
                dragStartIndex = itemsIndex;
                // 当前拖拽对象
                delegateModel.dragedObject = delegateModel.items.get(itemsIndex);
                taskItemDelegateModel.removeVisualItem(itemsIndex);

                taskItemRoot.isDragStatus = true;
                const modelIndex = delegateModel.dragedObject.model.index;
                Drag.mimeData = {
                    "text/plain": '{"dragStartIndex":' + dragStartIndex + ',"modelIndex":' +  modelIndex + '}'
                };
                Drag.active = true;
                // 不同的Action鼠标光标不同
                Drag.startDrag(Qt.MoveAction);
            }

            MouseArea {
                id: mouseArea
                x: 0; y: 0
                Accessible.name: "ukui-panel_taskManager_widget"
                width: parent.width; height: parent.height
                hoverEnabled: !UkuiTaskManager.TaskManager.disableMouseHover
                acceptedButtons: {
                    let button = Qt.NoButton;
                    if (!UkuiTaskManager.TaskManager.disableMouseLeftButton) {
                        button |= Qt.LeftButton
                    }
                    if (!UkuiTaskManager.TaskManager.disableMouseRightButton) {
                        button |= Qt.RightButton
                    }
                    return button;
                }

                property int lastIndex: -1
                property string currentWid: ""

                function activeWinId(winId) {
                    if (UkuiTaskManager.TaskManager.windowIsActivated(winId)) {
                        UkuiTaskManager.TaskManager.execSpecifiedAction(UkuiTaskManager.Action.Minimize, winId);
                    } else {
                        UkuiTaskManager.TaskManager.activateWindow(winId);
                    }
                }

                function aboutToShowThumbnailView(isShow, data, item) {
                    // 设置预览图窗口信息
                    // 在非合并状态下，由loader里的item触发request信号和参数
                    data.currentTaskItem = item;
                    taskItemListRoot.requestThumbnailView(isShow, data, item);
                }

                // 为拖拽截图
                onPressed: {
                    if (iconNeedsUpdate) {
                        parent.generateDragImage();
                        iconNeedsUpdate = false;
                    }
                }
                drag.target: mouseArea
                drag.onActiveChanged: {
                    taskItemListRoot.requestThumbnailView(false, null, mouseArea);
                    if (drag.active) {
                        taskItemListRoot.isDragging = true;
                        taskItemRoot.startDrag();
                    } else {
                        mouseArea.x = 0;
                        mouseArea.y = 0;
                    }
                }

                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        taskItemProxyModel.openMenu(false, taskItemProxyModel.menuActions(model.index), taskItemView);
                        taskManagerActived = true;
                        isTaskMenuShow = true;

                    } else {
                        if (CurrentWinIdList.length === 0) {
                            taskItemDelegateModel.model.launch(taskItemDelegateModel.model.index(model.index, 0), taskItemRoot);
                            return;
                        }

                        let index = -1;
                        if (taskItemView.isMergeStatus) {
                            // 轮转
                            if (currentWid !== "") {
                                index = CurrentWinIdList.indexOf(currentWid);
                                if (index < (CurrentWinIdList.length - 1)) {
                                    ++index;
                                } else {
                                    index = 0;
                                }
                            }
                        } else {
                            // 由taskList实现
                            index = taskItemLoader.item.indexAt(mouseX, mouseY);
                        }

                        if (index < 0) {
                            index = 0;
                        } else if (index >= CurrentWinIdList.length) {
                            index = CurrentWinIdList.length - 1;
                        }

                        currentWid = CurrentWinIdList[index];
                        /**
                         * 鼠标点击，更新窗口最小化和激活动画。
                         */
                        updateIconGeometry(currentWid);
                        activeWinId(currentWid);
                    }
                    taskManager.hidethumbnail();
                }

                onEntered: {
                    if (CurrentWinIdList !== undefined && CurrentWinIdList.length > 0 && !taskItemRoot.quickWidgetLoaded) {
                        if (taskItemView.isMergeStatus) {
                            mouseArea.aboutToShowThumbnailView(true, thumbnailData, mouseArea);
                        }
                    }
                }

                onExited: {
                    taskItemListRoot.requestThumbnailView(false, thumbnailData, mouseArea);
                    pluginButtonLoader.active = false;
                }

                TaskItemData {
                    id: thumbnailData
                    windowIdList: model.CurrentWinIdList
                    windowIcons: model.WindowIcons
                    windowIconsName: model.WindowIconsName
                    windowTitles: model.WindowTitles
                }

                Loader {
                    id: taskItemLoader
                    property var taskItemData: model
                    property var thumbnailData: thumbnailData

                    sourceComponent: {
                        if (taskItemRoot.DelegateModel.inTempItems) {
                            return tempTaskitem;
                        }

                        if (taskItemRoot.useSingleItem) {
                            if (taskItemRoot.quickWidgetLoaded) {
                                return quickWidgetItem;
                            } else {
                                return singleTaskItem;
                            }
                        } else {
                            return listTaskItem;
                        }
                    }
                }

                Component {
                    id: tempTaskitem
                    Item {
                        width: taskItemView.taskItemSize
                        height: taskItemView.taskItemSize
                    }
                }

                // 非合并状态 图标+名称
                Component {
                    id: listTaskItem
                    AppList {
                        height: childrenRect.height
                        width: childrenRect.width

                        itemSize: taskItemView.taskItemSize
                        isHorizontal: taskItemView.orientation === ListView.Horizontal
                        spacing: taskItemView.spacing
                        model: taskItemData.CurrentWinIdList
                        ratio: taskItemListRoot.ratio
                        iconIndexAboutToBeScaled: {
                            if (mouseArea.containsPress) {
                                return indexAt(mouseArea.mouseX, mouseArea.mouseY);
                            }
                            return -1;
                        }
                    }
                }

                Component {
                    id: singleTaskItem
                    AppIcon {
                        // 在任务栏横向的时候，单个图标的三态区域上下边距为4，左右边距为8。两方不一致，需要特殊处理。
                        // 并且在设计稿最新改动中（2025.9.19），各个插件的间距也为8，所以作为首项item不能留有左右的两边的间距。
                        // 任务栏竖向的时候，同理。
                        width: isHorizontal ? (taskItemView.taskItemSize - inset * 2) : taskItemView.taskItemSize
                        height: isHorizontal ? taskItemView.taskItemSize : (taskItemView.taskItemSize - inset * 2)

                        appIcon.source: taskItemData.IconName === "" ? taskItemData.Icon : taskItemData.IconName
                        appIcon.addUnifiedIconBox: true
                        appIcon.scale: mouseArea.containsPress ? 0.8 : 1.0
                        ratio: taskItemListRoot.ratio
                    }
                }
                Component {
                    id: quickWidgetItem
                    Item {
                        height: taskItemView.taskItemSize
                        width: quickWidget.preferredWidth + margins * 2
                        property int margins: 4 * taskItemListRoot.ratio
                        UkuiTaskManager.WidgetDelegate {
                            id: quickWidget
                            anchors.fill: parent
                            anchors.margins: parent.margins

                            quickWidgetId: taskItemData.QuickWidgetId
                        }
                        function updateIconGeometry(winIdList) {
                            Platform.WindowManager.setMinimizedGeometry(winIdList, this);
                        }
                    }
                }

                /**
                 * 三岛任务栏小插件切换按钮,相关属性：taskItemRoot.quickWidgetLoaded
                 */
                property bool hasQuickWidgetId: model.QuickWidgetId !== "" && model.QuickWidgetId !== undefined && root.isIslandMode

                onHasQuickWidgetIdChanged: {
                    if (hasQuickWidgetId) {
                        quickWidgetTimer.createObject(mouseArea);
                    }
                }
                Component {
                    id: quickWidgetTimer
                    Timer {
                        interval: 1000
                        running: mouseArea.containsMouse
                        onTriggered: {
                            pluginButtonLoader.active = true;
                        }
                    }
                }
                Loader {
                    id: pluginButtonLoader
                    anchors.top: mouseArea.top
                    anchors.topMargin: 1
                    anchors.right: mouseArea.right
                    anchors.rightMargin: 1
                    active: false
                    sourceComponent: pluginButton
                }

                Component {
                    id: pluginButton
                    Items.DtThemeBackground {
                        width: 20; height: width
                        radius: width / 2
                        useStyleTransparency: false
                        backgroundColor: mousearea.containsPress ? Platform.GlobalTheme.kGray7 :
                                                                   mousearea.containsMouse ? Platform.GlobalTheme.kGray6 :
                                                                                             Platform.GlobalTheme.kContainSecondaryNormal
                        MouseArea {
                            id:mousearea
                            Accessible.name: "ukui-panel_taskManager_widget_pluginButton"
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                taskItemRoot.quickWidgetLoaded = !taskItemRoot.quickWidgetLoaded;
                                taskManager.hidethumbnail();
                            }
                        }
                        Items.Icon {
                            anchors.fill: parent
                            anchors.margins: 2
                            source: taskItemRoot.quickWidgetLoaded ?
                                "://org.ukui.panel.taskManager/qml/icons/back.svg" : "://org.ukui.panel.taskManager/qml/icons/to.svg"
                        }
                    }
                }
            }

            /**
             * iconGeometry功能
             */
            property var currentWinIdList: model.CurrentWinIdList
            property var hasActiveWindow: model.HasActiveWindow
            property int itemIndex: DelegateModel.itemsIndex
            property bool invisibleOfFront: (x < taskItemView.contentX) || (y < taskItemView.contentY)
            property bool invisibleOfBack: (x > taskItemView.width + taskItemView.contentX) || (y > taskItemView.height + taskItemView.contentY)

            // 调用setMinimizedGeometry接口，默认参数为CurrentScreenWinIdList
            function updateIconGeometry(winIdList = CurrentScreenWinIdList) {
                if (taskItemLoader.item && !taskItemRoot.DelegateModel.inTempItems) {
                    if (invisibleOfFront) {
                        Platform.WindowManager.setMinimizedGeometry(winIdList, virtualFrontItem);
                    } else if (invisibleOfBack) {
                        Platform.WindowManager.setMinimizedGeometry(winIdList, virtualBackItem);
                    } else {
                        taskItemLoader.item.updateIconGeometry(winIdList);
                    }
                }
            }

            function addIndexToGeometryList() {
                if (model.CurrentScreenWinIdList) {
                    // 在 *所在屏幕* 的位置，添加最小化动画
                    // Note: 图标Click后，会在点击所对应的位置，进行最小化动画
                    if (model.CurrentScreenWinIdList.length > 0 && !taskItemView.pendingUpdatesList.includes(itemIndex)) {
                        taskItemView.pendingUpdatesList.push(itemIndex);
                        updateGeometryList.restart();
                    }
                }
            }
            Component.onCompleted: {
                //初始化、拖拽后更新model、切换任务栏方向，都会触发
                addIndexToGeometryList();
            }
            Component.onDestruction: {
                if (isItemActived) {
                    taskItemView.updateActivedItem(false);
                }
            }
            onCurrentWinIdListChanged: {
                addIndexToGeometryList();
            }
            // 激活窗口后，在窗口 *对应屏幕* 上进行最小化动画
            onHasActiveWindowChanged: {
                if (HasActiveWindow && model.CurrentScreenWinIdList.length > 0) {
                    updateIconGeometry();
                }
            }
            Behavior on width {
                // 主要提供三岛小插件切换的动画
                enabled: root.isIslandMode
                NumberAnimation {
                    duration: 150
                    easing.type: Easing.OutQuad
                }
            }
        }
    }

    ListView {
        id: taskItemView
        Accessible.name: "ukui-panel_taskManager_widget_taskItemView"

        // view的宽高 在main的view.width  view.height中定义
        property bool windowVisible: false
        property bool isMergeStatus: {
            var status = UkuiTaskManager.TaskManager.mergeIcons;
            return (status === undefined) || (status === 0) ;
        }
        property bool isHorizontal:  Widget.orientation === Items.Types.Horizontal
        property real taskItemSize: 48
        property int activedItem: 0
        interactive: false

        // flickDeceleration： filck减速速率，默认为1500。
        // 调整减速速率，达到在翻页时，以较快速度进行滑动。
        // boundsMovement: Flickable.StopAtBounds 设置此属性会偶现切换方向后，contentX和ContentY出现异常导致内部不显示的问题
        flickDeceleration: 10000

        spacing: 0
        model: taskItemDelegateModel
        cacheBuffer: isHorizontal ? Widget.container.screen.size.width : Widget.container.screen.size.height
        // 使用delayed属性，避免方向切换时，taskItemSize立即更新，导致过程中数值错误
        Binding {
            target: taskItemView
            property: "taskItemSize"
            value: isHorizontal ? taskItemListRoot.parent.height : taskItemListRoot.parent.width
            delayed: true
        }
        onTaskItemSizeChanged: {
            if (contentX !== 0 || contentY !== 0) {
                positionViewAtBeginning();
            }
            pushAllIndex();
        }
        onIsMergeStatusChanged: {
            pushAllIndex();
        }
        onWindowVisibleChanged: {
            if(Platform.WindowManager.isWaylandSession) {
                pushAllIndex();
            }
        }
        onContentXChanged: {
            pushAllIndex();
        }
        onContentYChanged: {
            pushAllIndex();
        }
        onIsHorizontalChanged: {
            // 重置滚动位置
            positionViewAtBeginning();
            orientation = isHorizontal ? ListView.Horizontal : ListView.Vertical;
        }
        function updateActivedItem(isActived) {
            taskItemView.activedItem = isActived ? taskItemView.activedItem + 1 : Math.max(taskItemView.activedItem - 1, 0);
            taskManagerActived = taskItemView.activedItem > 0;
        }
        function updateIconState() {
            for (let i = 0; i < taskItemDelegateModel.count; ++i) {
                taskItemView.itemAtIndex(i).iconNeedsUpdate = true;
            }
        }

        Component.onCompleted: {
            orientation = isHorizontal ? ListView.Horizontal : ListView.Vertical;
            Platform.Theme.iconThemeChanged.connect(updateIconState);
            // 仅在移除图标时，才进行remove动画
            taskItemProxyModel.rowsAboutToBeRemoved.connect(function () {
                removeTransition.enabled = true;
            })
        }

        /**
         * iconGeometry
         */
        property var pendingUpdatesList: []

        Connections {
            target: Widget.container
            function onPositionChanged() {
                taskItemView.pushAllIndex();
            }
        }

        function pushAllIndex() {
            pendingUpdatesList.length = 0;
            for (var i = 0; i < taskItemView.count; ++i) {
                pendingUpdatesList.push(i);
            }
            if (pendingUpdatesList.length > 0) {
                updateGeometryList.restart();
            }
        }

        Timer {
            id: updateGeometryList
            interval: 500

            onTriggered: {
                for (let i = 0; i < taskItemView.pendingUpdatesList.length; ++i) {
                    let index = taskItemView.pendingUpdatesList[i];
                    let item = taskItemView.itemAtIndex(index);
                    if (item) {
                        item.updateIconGeometry();
                    }
                }
                taskItemView.pendingUpdatesList.length = 0;
            }
        }

        Item {
            id: virtualFrontItem
            x: 0; y: 0
            width: 10
            height: 10
        }
        Item {
            id: virtualBackItem
            x: taskItemView.width - width; y: taskItemView.height - height
            width: 10
            height: 10
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                easing.type: Easing.OutQuad
                duration: 300
            }
        }

        move: Transition {
            NumberAnimation { properties: "x,y"; duration: 300 }
        }

        add: Transition {
            NumberAnimation {
                id: addAnimation
                property: taskItemView.isHorizontal ? "y" : "x"
                from: (Widget.container.position === Items.Types.Left) || (Widget.container.position === Items.Types.Top) ? -taskItemView.taskItemSize : taskItemView.taskItemSize
                to: 0
                easing.type: Easing.OutCubic
                duration: 300
            }
        }

        remove: Transition {
            id: removeTransition
            enabled: false
            SequentialAnimation {
                NumberAnimation {
                    id: removeAnimation
                    property: addAnimation.property
                    easing.type: Easing.OutCubic
                    from: 0; to: addAnimation.from
                    duration: 300
                }
                ScriptAction {
                    script: {
                        removeTransition.enabled = false;
                    }
                }
            }
        }
    }

    // 获取ListView的坐标对应的ContentItem内部坐标
    // listview与dropArea的大小需要重合
    function getContentPoint(localX, localY) {
        return Qt.point(localX + taskItemView.contentX, localY + taskItemView.contentY);
    }

    /**
     * 事件来源判断
     * 内部事件: 任务栏内部拖拽换位--"InternalEvent"
     * 外部事件: 从开始菜单或桌面，固定到任务栏--"PinToTaskbar"
     *          从桌面、文件管理器，拖到任务栏可打开应用图标上 --"DropToAppIcon" (enableDragToApp需为true)
     * 无效事件: "InvalidEvent"
     */

    property string panelDragEvent: "InvalidEvent"
    function dragEventSource(event) {
        if (taskItemDelegateModel.dragedObject !== null) {
            return "InternalEvent";
        }

        let source = event.getDataAsString("source");
        if (source === "ukui-menu") {
            let url = event.getDataAsString("url");
            return url.startsWith("app://") ? "PinToTaskbar" : "InvalidEvent";
        }
        if (UkuiTaskManager.TaskManager.enableDragToApp) {
            if (event.urls && event.urls.length > 0) {
                return "DropToAppIcon";
            }
        }

        return "InvalidEvent";
    }

    // 获取一个Item的中点坐标
    function getItemMidPoint(item) {
        return Qt.point(item.x + item.width/2, item.y + item.height/2);
    }

    // point 是否在item的中点前
    function beforeMidpoint(item, point) {
        let midPoint = getItemMidPoint(item);
        if (taskItemView.orientation === ListView.Horizontal) {
            return point.x < midPoint.x;
        } else {
            return point.y < midPoint.y
        }
    }

    // 1.拖拽进入事件
    // 拖拽进入，需要生成临时Item
    onEntered: (dragEvent) => {
        // 1.处理进入拖拽区域时的边缘点情况，避免丢失拖拽事件
        let enterX = dragEvent.x === width ? dragEvent.x - 1 : dragEvent.x;
        let enterY = dragEvent.y === height ? dragEvent.y - 1 : dragEvent.y;
        const contentPoint = getContentPoint(enterX, enterY);

        // 2.删除上次未删除的临时Item,保证只有一个临时Item
        if (tempItems.count > 0) {
            taskItemDelegateModel.removeTempItems();
        }

        // 3.获取进入点下方的Item, 如果为null，则说明拖拽到了空白位置
        let insertIndex = -1;
        let pointItem = taskItemView.itemAt(contentPoint.x, contentPoint.y);
        if (pointItem !== null) {
            // 4.通过进入点的对象的itemsIndex获取各种数据。
            insertIndex = pointItem.DelegateModel.itemsIndex;

            // 判断中点位置，确定在插入在前还是后，insertIndex or insertIndex+1
            const maxIdx = taskItemDelegateModel.items.count - 1;
            if ((insertIndex > 0) && (insertIndex < maxIdx) && beforeMidpoint(pointItem, contentPoint)) {
                ++insertIndex;
            }
        } else {
            // 5.拖拽进入点是空白区域
            insertIndex = taskItemDelegateModel.items.count;
        }

        /**
         * 4.判断事件来源
         * 区分拖拽事件的来源，一般有两个来源：
         * 1.内部拖拽事件，拖拽listview的Item进行位置的调整。
         * 2.外部事件，将桌面图标拖拽到任务栏上。
         */
        const eventType = dragEventSource(dragEvent);
        panelDragEvent = eventType;
        if (eventType === "InvalidEvent") {
            dragEvent.accepted = false;
            return;

        } else if (eventType === "PinToTaskbar") {
            let id = dragEvent.getDataAsString("url").slice(6);
            let itemsIndex = -1;

            // 判断任务栏是否已经有同id图标
            for (var i = 0; i < taskItemView.count; ++i) {
                if (taskItemDelegateModel.items.get(i).model.Id === id) {
                    itemsIndex = taskItemDelegateModel.items.get(i).itemsIndex;
                    resetItemsIndex = itemsIndex;
                }
            }

            taskItemDelegateModel.removeVisualItem(itemsIndex);

        } else if (eventType === "DropToAppIcon" && UkuiTaskManager.TaskManager.enableDragToApp) {
            dragEvent.accepted = false;
            return;
        }

        // 5.创建临时Item，并将他放到tempItems组中，后续可通过该组删除它
        if (insertIndex < 0) {
            insertIndex = 0;
        } else if (insertIndex > taskItemDelegateModel.items.count) {
            insertIndex = taskItemDelegateModel.items.count;
        }

        taskItemDelegateModel.items.insert(
            insertIndex,
            {
                Name: ""
            },
            [taskItemDelegateModel.items.name, "tempItems"]
        );
    }

    // 2.拖拽移出事件
    onExited: {
        taskItemDelegateModel.removeTempItems();

        if (panelDragEvent === "PinToTaskbar") {
            // 外部拖拽退出后，任务栏已有图标返回原来位置
            let i = taskItemDelegateModel.persistedItems.count - 1;
            for (; i >= 0; --i) {
                const persistedItemObj = taskItemDelegateModel.persistedItems.get(i);
                if (!persistedItemObj.inItems) {
                    persistedItemObj.groups = [taskItemDelegateModel.items.name];
                    if (resetItemsIndex !== -1) {
                        taskItemDelegateModel.items.move(persistedItemObj.itemsIndex, resetItemsIndex, 1);
                    }
                }
            }
            resetItemsIndex = -1;
        }
    }

    // 3.拖拽移动事件
    onPositionChanged: (dragEvent) => {
        if (tempItems.count === 1) {
            // 落点
            let currentPoint = getContentPoint(dragEvent.x, dragEvent.y);
            let targetItem = taskItemView.itemAt(currentPoint.x, currentPoint.y);
            if (targetItem !== null && !targetItem.DelegateModel.inTempItems) {
                let sourceIndex = tempItems.get(0).itemsIndex;
                let targetIndex = targetItem.DelegateModel.itemsIndex;

                const bmp = beforeMidpoint(targetItem, currentPoint);
                if ((sourceIndex < targetIndex) && bmp) {
                    --targetIndex;
                } else if ((sourceIndex > targetIndex) && !bmp) {
                    ++targetIndex;
                }

                let moveCount = Math.abs(targetIndex - sourceIndex);
                if (moveCount === 0) {
                    return;
                }

                if (sourceIndex < targetIndex) {
                    taskItemDelegateModel.items.move(sourceIndex + 1, sourceIndex, moveCount);
                } else {
                    taskItemDelegateModel.items.move(targetIndex, targetIndex + 1, moveCount);
                }
            }
        }
    }

    // 4.拖拽放下事件
    onDropped: (dragEvent) => {
        // 1.获取丢下点的Index
        let droppedIndex = -1;
        if (tempItems.count > 0) {
            // 默认情况下临时Item对应的位置就是丢下的位置
            let tempObj = tempItems.get(0);
            droppedIndex = tempObj.inItems ? tempObj.itemsIndex : taskItemDelegateModel.items.count;
        } else {
            // 修正边缘落点情况
            let enterX = dragEvent.x === width ? dragEvent.x - 1 : dragEvent.x;
            let enterY = dragEvent.y === height ? dragEvent.y - 1 : dragEvent.y;
            let contentPoint = getContentPoint(enterX, enterY);

            let dropTargetItem = taskItemView.itemAt(contentPoint.x, contentPoint.y);
            droppedIndex = (dropTargetItem === null) ? taskItemDelegateModel.items.count : dropTargetItem.DelegateModel.itemsIndex;
        }

        // 2.将存起来的Item全部归位，并清空临时对象
        let eventType = dragEventSource(dragEvent);
        taskItemDelegateModel.resetPersistedItems();

        // 3.事件类型
        if (eventType === "InternalEvent") {
            const obj = JSON.parse(dragEvent.text);
            if (obj !== null) {
                taskItemProxyModel.setOrder(taskItemDelegateModel.model.index(obj.modelIndex, 0), droppedIndex);
            }
        } else if (eventType === "PinToTaskbar") {
            // 外部事件，执行外部操作
            let id = dragEvent.getDataAsString("url").slice(6);
            taskItemProxyModel.addQuickLauncher(id, droppedIndex);
        }
        taskItemListRoot.isDragging = false;
        panelDragEvent = "InvalidEvent";
    }
}
