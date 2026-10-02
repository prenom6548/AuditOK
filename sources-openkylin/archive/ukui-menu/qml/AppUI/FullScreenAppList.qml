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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Window 2.15
import QtQuick.Layouts 1.12

import org.ukui.menu.core 1.0
import org.ukui.menu.extension 1.0
import "../extensions" as Extension
import AppControls2 1.0 as AppControls2

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

ListView {
    id: root
    // 应用单元的宽高值：A = 160
    property int appBaseSize: 160
    // 应用区拟定宽度：c =（4+ 2560/ScreenWidth）* ScreenWidth/8
    property int proposedAppAreaWidth: (4 + 2560 / Screen.width) * Screen.width / 8
    // 应用区列数：n = round(c /(94 * 2)),*5<=*n<=9(后续：改为无最小列数限制）
    property int columnCount: Math.min(Math.max(1, Math.ceil(proposedAppAreaWidth / (94 * 2))), 9)
    // 应用单元水平间距：Gh = (c - n * A) / (n - 1)
    property int hSpacing: Math.floor((proposedAppAreaWidth - columnCount * appBaseSize) / (columnCount - 1))
    // 应用单元垂直间距：Gv = ScreenHeight / 27
    property int vSpacing: Math.floor(Screen.height / 27)
    // 应用区实际宽度：w = n * A + (n - 1) * Gh
    property int actualAppAreaWidth: columnCount * appBaseSize + (columnCount - 1) * hSpacing

    property int itemSpacing: 5
    signal openFolderSignal(string folderId, string folderName, int x, int y)
    signal contentShowFinished()
    property bool isContentShow
    property bool isSearching

    property int itemHeight: 40
    property alias sourceModel: appGroupModel.sourceModel
    property int cellWidth: appBaseSize + hSpacing
    property int cellHeight: appBaseSize + vSpacing
    property int column: Math.floor(width / cellWidth)

    property var currentGridView: null
    property int currentNum: -2
    property string currentId: ""
    property int currentLabelIndex: -1

    property bool isTouch: false

    spacing: 5
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    currentIndex: -1
    highlightMoveDuration: 0

    //除header外其他鼠标区域，用于关闭编辑模式或隐藏全屏开始菜单，当鼠标进入appItem和编辑按钮时失效
    MouseArea {
        id: appArea
        anchors.fill: headerItem.parent
        onClicked: {
            if (mainWindow.editMode) {
                mainWindow.editMode = false;
            } else {
                mainWindow.activeMenuWindow(false);
            }
        }
    }

    onFocusChanged: {
        if (focus) {
            if (root.isSearching) {
                currentIndex = 0;
                itemAtIndex(0).forceActiveFocus();
            } else {
                if (currentLabelIndex >= 0 && currentLabelIndex < root.count) {
                    currentIndex = currentLabelIndex;
                    itemAtIndex(currentLabelIndex).forceActiveFocus();
                } else {
                    if (headerItem.itemCount === 0) {
                        currentIndex = 0;
                        itemAtIndex(0).forceActiveFocus();
                    } else {
                        headerItem.forceActiveFocus();
                        headerItem.itemShowBorder = true;
                    }
                }
            }
        } else {
            currentIndex = -1;
            currentGridView.focus = false
            currentGridView = null;
            currentNum = -2;
        }
    }
    function resetHeaderItemShowBorder() {
        if (currentGridView === headerItem && !headerItem.itemShowBorder) {
            headerItem.itemShowBorder = true;
            headerItem.itemCurrentIndex = 0;
            return true;
        }
        return false;
    }

    //应用项通过Loader分批异步创建，避免打开时一次性实例化所有Item导致卡顿
    property var appLoadQueue: []
    property var currentLoadingAppItems: []
    property int maxConcurrentAppLoads: 5
    property bool appLoadQueueScheduled: false

    function compareAppLoadOrder(a, b) {
        var posA = a.mapToItem(root.contentItem, 0, 0);
        var posB = b.mapToItem(root.contentItem, 0, 0);

        if (posA.y !== posB.y) {
            return posA.y - posB.y;
        }

        return posA.x - posB.x;
    }

    //把应用项加入等待加载队列
    function enqueueAppLoadItem(item) {
        if (!item || item.contentLoaded || item.loadQueued || item.loadAllowed || item.appIndex < 0) {
            return;
        }

        item.loadQueued = true;
        appLoadQueue.push(item);
        scheduleAppLoadQueue();
    }

    //在应用项销毁或失效时，把它从等待队列和正在加载列表中移除
    function removeAppLoadItem(item) {
        if (!item) {
            return;
        }

        var itemIndex = appLoadQueue.indexOf(item);
        if (itemIndex >= 0) {
            appLoadQueue.splice(itemIndex, 1);
        }

        var loadingIndex = currentLoadingAppItems.indexOf(item);
        if (loadingIndex >= 0) {
            currentLoadingAppItems.splice(loadingIndex, 1);
            scheduleAppLoadQueue();
        }
    }

    //在应用项加载完成或加载失败后，释放它占用的加载名额
    function finishAppLoadItem(item) {
        if (item) {
            item.loadAllowed = false;
        }

        var loadingIndex = currentLoadingAppItems.indexOf(item);
        if (loadingIndex >= 0) {
            currentLoadingAppItems.splice(loadingIndex, 1);
        }

        scheduleAppLoadQueue();
    }

    //合并多次调度请求，通过Qt.callLater延后执行一次队列处理
    function scheduleAppLoadQueue() {
        if (appLoadQueueScheduled) {
            return;
        }

        appLoadQueueScheduled = true;
        Qt.callLater(root.processAppLoadQueue);
    }

    function processAppLoadQueue() {
        appLoadQueueScheduled = false;

        //将加载完成的项移出队列
        var loadingItems = [];
        for (var loadingIndex = 0; loadingIndex < currentLoadingAppItems.length; ++loadingIndex) {
            var loadingItem = currentLoadingAppItems[loadingIndex];
            if (loadingItem && loadingItem.isAlive && !loadingItem.contentLoaded && loadingItem.loadAllowed) {
                loadingItems.push(loadingItem);
            }
        }
        currentLoadingAppItems = loadingItems;

        var availableLoads = Math.max(1, maxConcurrentAppLoads) - currentLoadingAppItems.length;
        if (availableLoads <= 0) {
            return;
        }

        //将等待加载队列里不符合要求的项移出
        var validItems = [];
        for (var i = 0; i < appLoadQueue.length; ++i) {
            var item = appLoadQueue[i];
            if (item && !item.contentLoaded && item.appIndex >= 0) {
                validItems.push(item);
            } else if (item) {
                item.loadQueued = false;
            }
        }

        appLoadQueue = validItems;
        if (appLoadQueue.length === 0) {
            return;
        }

        //按当前布局中的行列顺序加载，优先显示列表前面的应用项
        appLoadQueue.sort(root.compareAppLoadOrder);

        while (availableLoads > 0 && appLoadQueue.length > 0) {
            var nextItem = appLoadQueue.shift();
            if (nextItem) {
                nextItem.loadQueued = false;
            }

            if (!nextItem || !nextItem.isAlive || nextItem.contentLoaded
                    || nextItem.loadAllowed || nextItem.appIndex < 0) {
                continue;
            }

            currentLoadingAppItems.push(nextItem);
            nextItem.loadAllowed = true;
            --availableLoads;
        }
    }

    Keys.onPressed: {
        if (!currentGridView || currentGridView.itemCurrentIndex === -1) return;

        var newIndex = currentGridView.itemCurrentIndex;
        var row = Math.floor(newIndex / root.column);
        var col = newIndex % root.column;

        switch (event.key) {
        case Qt.Key_Up:
            if (resetHeaderItemShowBorder()) return;
            row = row - 1;
            break;
        case Qt.Key_Down:
            if (resetHeaderItemShowBorder()) return;
            row = row + 1;
            break;
        case Qt.Key_Left:
            if (resetHeaderItemShowBorder()) return;
            col = Math.max(0, col - 1);
            break;
        case Qt.Key_Right:
            if (resetHeaderItemShowBorder()) return;
            col = Math.min(root.column - 1, col + 1);
            break;
        case Qt.Key_Enter:
        case Qt.Key_Return:
            if (root.currentGridView === headerItem) {
                if (!headerItem.itemShowBorder) return;
                headerItem.openFavoriteItem();
            } else {
                if (currentId) {
                    appManager.launchApp(currentId);
                }
            }
            event.accepted = true;
            return;
        default:
            return;
        }

        newIndex = row * root.column + col;
        if (row < 0 || newIndex >= currentGridView.itemCount) {
            if (currentNum === 0 && newIndex < 0) {
                if (!root.isSearching) headerItem.forceActiveFocus();
                event.accepted = false;
            } else if (currentNum === -1 && newIndex >= currentGridView.itemCount) {
                currentIndex = 0;
                itemAtIndex(0).forceActiveFocus();
                event.accepted = true;
            }
        } else {
            currentGridView.itemCurrentIndex = newIndex;
            event.accepted = true;
        }
    }

    model: AppGroupModel {
        id: appGroupModel
    }

    delegate: Item {
        id: groupItem
        property int itemCount: appGridView.count
        property int itemCurrentIndex: -1
        width: ListView.view.width
        height: childrenRect.height

        onFocusChanged: {
            if (focus) {
                root.currentGridView = this;
                itemCurrentIndex = 0;
                currentNum = index;
            } else {
                itemCurrentIndex = -1;
            }
        }

        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 0

            AppControls2.LabelItem {
                width: parent.width
                height: root.itemHeight
                displayName: (model.name === undefined || model.name === "") ? qsTr("All Applications") : model.name
                visible: displayName !== ""
                onEntered: appArea.enabled = false
                onExited: appArea.enabled = true
            }

            GridView {
                id: appGridView
                property bool isFavorite: false
                anchors.horizontalCenter: parent.horizontalCenter
                width: root.cellWidth * root.column
                height: contentHeight
                cellWidth: root.cellWidth
                cellHeight: root.cellHeight
                interactive: false

                currentIndex: groupItem.itemCurrentIndex
                onCurrentItemChanged: {
                    if (!currentItem) return;
                    var itemYInListView = currentItem.y + root.y + groupItem.y;
                    var visibleTop = root.contentY;
                    var visibleBottom = visibleTop + root.height;

                    if (itemYInListView < visibleTop) {
                        var targetY = itemYInListView;
                        if (currentItem.y === 0) targetY -= root.itemHeight;
                        root.contentY = targetY;
                    } else if (itemYInListView + currentItem.height > visibleBottom) {
                        targetY = itemYInListView + currentItem.height - root.height;
                        root.contentY = targetY;
                    }
                }

                model: DelegateModel {
                    id: appDelegateModel
                    property bool rootIndexReady: false
                    model: appGroupModel

                    Component.onCompleted: {
                        // Warning: rootIndex只需要设置一次，多次设置会导致对应关系错误，
                        // delegateModel使用persistedIndex保存rootIndex，会自动进行修正
                        rootIndex = modelIndex(index);
                        rootIndexReady = true;
                    }

                    delegate: Item {
                        id: appBaseItem
                        property int appIndex: index >= 0 ? index : -1
                        property string appName: model.name === undefined ? "" : model.name
                        property string appId: model.id === undefined ? "" : model.id
                        property bool isAlive: true
                        property bool contentLoaded: false
                        property bool loadQueued: false
                        property bool loadAllowed: false
                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight

                        Accessible.role: Accessible.Button
                        Accessible.name: appBaseItem.appName

                        focus: GridView.isCurrentItem
                        onFocusChanged: {
                            if (focus) {
                                root.currentId = appId;
                            }
                        }

                        function registerLoadItem() {
                            if (!isAlive || appIndex < 0) {
                                return;
                            }

                            //rootIndex设置完成后再入队，避免DelegateModel尚未建立映射时触发异步加载。
                            if (typeof appDelegateModel === "undefined"
                                    || appDelegateModel === null
                                    || !appDelegateModel.rootIndexReady) {
                                return;
                            }

                            if (root) {
                                root.enqueueAppLoadItem(appBaseItem);
                            }
                        }

                        Component.onCompleted: Qt.callLater(registerLoadItem)
                        Component.onDestruction: {
                            isAlive = false;
                            root.removeAppLoadItem(appBaseItem);
                        }
                        onAppIndexChanged: Qt.callLater(registerLoadItem)

                        Connections {
                            target: appDelegateModel
                            function onRootIndexReadyChanged() {
                                Qt.callLater(appBaseItem.registerLoadItem);
                            }
                        }

                        Loader {
                            id: appContentLoader
                            anchors.fill: parent
                            active: appBaseItem.contentLoaded || appBaseItem.loadAllowed
                            asynchronous: true
                            sourceComponent: appContentComponent

                            onLoaded: {
                                appBaseItem.contentLoaded = true;
                                root.finishAppLoadItem(appBaseItem);
                            }

                            onStatusChanged: {
                                if (status === Loader.Error) {
                                    appBaseItem.contentLoaded = true;
                                    root.finishAppLoadItem(appBaseItem);
                                }
                            }
                        }

                        Component {
                            id: appContentComponent

                            Item {
                                anchors.fill: parent

                                FullScreenAppItem {
                                    id: appItem
                                    anchors.fill: parent
                                    // 原设计稿view中有spacing间距，应用到Gridview中就是margin
                                    anchors.topMargin: vSpacing /2
                                    anchors.bottomMargin: vSpacing /2
                                    anchors.leftMargin: hSpacing /2
                                    anchors.rightMargin: hSpacing /2

                                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                                    text.text: appBaseItem.appName
                                    icon.source: model.icon === undefined ? "" : model.icon
                                    icon.addUnifiedIconBox: true
                                    isRecentInstalled: model.recentInstall === undefined ? false : model.recentInstall

                                    hasFocus: (appBaseItem.GridView.isCurrentItem && root.currentIndex !== -1) ? true : false

                                    onClicked: (event) => {
                                        console.log("ICON_CLICK!", "DESKTOPFILE:", appBaseItem.appId);
                                        if (event.button === Qt.LeftButton) {
                                            // openApplication
                                            if (appBaseItem.appId !== "") {
                                                appManager.launchApp(appBaseItem.appId);
                                            }
                                        }
                                    }
                                    onEntered: appArea.enabled = false
                                    onExited: appArea.enabled = true

                                    drag.target: root.isTouch ? null : appItem
                                    drag.onActiveChanged: {
                                        Drag.active = drag.active;
                                    }

                                    Drag.supportedActions: Qt.CopyAction
                                    Drag.dragType: Drag.Automatic
                                    Drag.hotSpot.x: icon.width / 2
                                    Drag.hotSpot.y: icon.height / 2
                                    Drag.mimeData: {
                                        "source": "ukui-menu",
                                        "url": appBaseItem.appId === "" ? "" : "app://" + appBaseItem.appId,
                                        "favorite": model.favorite === undefined ? false : model.favorite > 0
                                    }
                                    Drag.onDragFinished: function (dropAction) {
                                        appItem.Drag.active = false;
                                    }

                                    onPressed: {
                                        if (mouse.button === Qt.RightButton) {
                                            if (root.isTouch) {
                                                root.isTouch = false;
                                                Drag.active = false;
                                                Drag.cancel();
                                            }
                                            if (appBaseItem.appId !== "") {
                                                Qt.callLater(() => {
                                                    menuManager.showMenu(appBaseItem.appId, MenuInfo.FullScreen);
                                                });
                                            }
                                            return;
                                        }
                                        fullScreenUI.focus = false;
                                        icon.grabToImage(function(result) {
                                            Drag.imageSource = result.url
                                        })
                                        if (mouse.source === Qt.MouseEventSynthesizedBySystem ||
                                                mouse.source === Qt.MouseEventSynthesizedByQt) {
                                            root.isTouch = true;
                                        } else {
                                            root.isTouch = false;
                                        }
                                    }

                                    onPressAndHold: {
                                        // 触摸长按时启动拖拽
                                        if (root.isTouch) {
                                            Drag.active =  true;
                                        }
                                    }
                                }

                                // 收藏按钮
                                Extension.EditModeFlag {
                                    isFavorited: model.favorite === undefined ? false : model.favorite > 0
                                    anchors.top: parent.top
                                    anchors.topMargin: 10
                                    anchors.right: parent.right
                                    anchors.rightMargin: 28
                                    onClicked: {
                                        if (appBaseItem.appId !== "") {
                                            appManager.changeFavoriteState(appBaseItem.appId, !isFavorited);
                                        }
                                    }
                                    onIsEnteredChanged: {
                                        appArea.enabled = !isEntered;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    header: MouseArea {
        id: favoriteItem
        property bool show: (visualModel.count > 0) && (!root.isSearching)
        property int itemCount: favoriteView.count
        property int itemCurrentIndex: -1
        property bool itemShowBorder: true
        property bool openFolderByEnter: false
        width: show ? root.width : 0
        height: show ? childrenRect.height : 0
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        visible: show

        function openFavoriteItem() {
            if (favoriteItem.itemCurrentIndex >= 0 && favoriteItem.itemCurrentIndex < favoriteView.count) {
                favoriteView.itemAtIndex(favoriteItem.itemCurrentIndex).itemClicked();
            }
        }
        property var widgets: []
        property var widgetInfos: []
        property int widgetCount: 1

        Component.onCompleted: {
            widgetInfos.push({label: "favorite", display: "non-starred-symbolic", type: WidgetMetadata.Widget});
            widgets.push("favorite");
        }

        onFocusChanged: {
            if (focus) {
                root.currentGridView = this;
                if (itemCurrentIndex === -1) itemCurrentIndex = 0;
                currentNum = -1;
            } else {
                if (!folderLoader.activeFocus) itemCurrentIndex = -1;
            }
        }

        onClicked: {
            if (mouse.button === Qt.LeftButton) {
                if (mainWindow.editMode) {
                    mainWindow.editMode = false;
                } else {
                    mainWindow.activeMenuWindow(false);
                }
            }
        }

        onPressed: {
            if (mouse.button === Qt.RightButton) {
                menu.open();
                return;
            }
        }

        UkuiItems.Menu {
            id: menu
            content: UkuiItems.MenuItem {
                text: qsTr("Editing mode")
                icon: "edit-symbolic"
                onClicked: mainWindow.editMode = true;
            }
        }

        Column {
            width: parent.width
            height: childrenRect.height + spacing
            spacing: root.spacing

            AppControls2.LabelItem {
                width: parent.width
                height: root.itemHeight
                displayName: qsTr("Favorite")
            }

            Item {
                width: parent.width
                height: favoriteView.height

                DropArea {
                    z: -10
                    anchors.fill: parent
                    property int addedIndex: -1
                    property string addedId: ""

                    function reset() {
                        addedId = "";
                        var nullIndex = favoriteView.indexAtNullItem();
                        if (nullIndex > -1 && nullIndex < favoriteView.count) {
                            favoriteView.viewModel.items.remove(nullIndex);
                        }
                    }

                    onEntered: {
                        if (drag.getDataAsString("folder") === "true" || drag.getDataAsString("favorite") === "false") {
                            // 从应用组内部拖拽 || 从左侧列表添加到收藏
                            // 已经触发从左侧拖动到收藏区域的事件，不需要再次添加空白item
                            let id = "";
                            if (drag.getDataAsString("url").startsWith("app://")) {
                                id = drag.getDataAsString("url").slice(6);
                            }
                            if (addedId === id || id === "") {
                                return;
                            }

                            addedId = id;
                            addedIndex = favoriteView.count;
                            visualModel.items.insert(addedIndex, {"id":"", "icon":"", "name": ""});

                        } else if (drag.getDataAsString("favorite") === "true") {
                            drag.accepted = false;
                        } else {
                            // 收藏插件内拖拽
                            favoriteView.dragTypeIsMerge = false;
                        }
                    }

                    onPositionChanged: {
                        if ((drag.getDataAsString("folder") === "true" || drag.getDataAsString("favorite") === "false") && addedIndex > -1) {
                            var dragIndex = favoriteView.indexAt(drag.x, drag.y);

                            if (dragIndex < 0) {
                                return;
                            }

                            visualModel.items.move(addedIndex, dragIndex);
                            addedIndex = dragIndex;
                        }
                    }

                    onDropped: {
                        // folder需要在favorite之前判断，因为应用组内的应用favorite都为true
                        if (drop.getDataAsString("folder") === "true") {
                            reset();
                            favoriteModel.addAppFromFolder(drop.getDataAsString("url").slice(6), addedIndex);
                            folderLoader.isFolderOpened = false;

                        } else if (drop.getDataAsString("favorite") === "false") {
                            reset();
                            favoriteModel.addAppToFavorites(drop.getDataAsString("url").slice(6), addedIndex);
                        }
                    }
                    onExited: reset()
                }

                GridView {
                    id: favoriteView
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.cellWidth * root.column
                    height: contentHeight
                    property string mergeToAppId: ""
                    property bool isMergeToFolder: false
                    property bool dragTypeIsMerge: false
                    property int exchangedStartIndex: 0
                    property alias viewModel: visualModel

                    property bool isFavorite: true

                    cellWidth: root.cellWidth
                    cellHeight: root.cellHeight
                    interactive: false

                    currentIndex: favoriteItem.itemCurrentIndex

                    function indexAtNullItem() {
                        var nullItemIndex;
                        for (var i = 0; i < favoriteView.count; i ++) {
                            if (favoriteView.itemAtIndex(i).id === "") {
                                nullItemIndex = i;
                                return nullItemIndex;
                            }
                        }
                        return -1;
                    }

                    onCurrentItemChanged: {
                        if (!currentItem) return;
                        var itemYInListView = currentItem.y + root.y + favoriteItem.y;
                        var visibleTop = root.contentY;
                        var visibleBottom = visibleTop + root.height;

                        if (itemYInListView < visibleTop) {
                            var targetY = itemYInListView;
                            if (currentItem.y === 0) targetY -= root.itemHeight;
                            root.contentY = targetY;
                        } else if (itemYInListView + currentItem.height > visibleBottom) {
                            targetY = itemYInListView + currentItem.height - root.height;
                            root.contentY = targetY;
                        }
                    }

                    model: DelegateModel {
                        id: visualModel
                        model: favoriteModel
                        delegate: Item {
                            id: container
                            width: favoriteView.cellWidth
                            height: favoriteView.cellHeight
                            property var id: model.id
                            focus: GridView.isCurrentItem
                            function itemClicked() {
                                favoriteDelegate.keyPressed();
                                favoriteItem.openFolderByEnter = (type === DataType.Folder);
                            }
                            onFocusChanged: {
                                if (focus) {
                                    root.currentId = id;
                                }
                            }
                            Extension.FavoriteDelegate {
                                id: favoriteDelegate
                                anchors.fill: parent
                                // 原设计稿view中有spacing间距，应用到Gridview中就是margin
                                anchors.topMargin: vSpacing /2
                                anchors.bottomMargin: vSpacing /2
                                anchors.leftMargin: hSpacing /2
                                anchors.rightMargin: hSpacing /2
                                isFullScreen: true

                                borderColor: Platform.GlobalTheme.kBrandFocus
                                border.width: (container.GridView.isCurrentItem && root.currentNum === -1 && itemShowBorder) ? 2 : 0

                                visualIndex: container.DelegateModel.itemsIndex
                                delegateLayout.anchors.topMargin: 16
                                delegateLayout.anchors.bottomMargin: 16
                                delegateLayout.spacing: 8
                                mergePrompt.anchors.topMargin: 10
                                mergePrompt.width: width*0.675
    //                            mergePrompt.width: 108
                                mergePrompt.radius: 32

                                Component.onCompleted: contentShowFinished.connect(resetOpacity)
                                Component.onDestruction: contentShowFinished.disconnect(resetOpacity)
                            }

                            //编辑模式标志
                            Extension.EditModeFlag {
                                id: tag
                                isFavorited: true
                                anchors.top: parent.top
                                anchors.topMargin: 10
                                anchors.right: parent.right
                                anchors.rightMargin: 28
                                onClicked: {
                                    visualModel.model.removeAppFromFavorites(id);
                                }
                            }
                        }
                    }

                    displaced: Transition {
                        NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 200 }
                    }
                    remove: Transition {
                        NumberAnimation { properties: "opacity"; to: 0; easing.type: Easing.InOutQuad; duration: 200 }
                    }
                }
            }
        }
    }
}
