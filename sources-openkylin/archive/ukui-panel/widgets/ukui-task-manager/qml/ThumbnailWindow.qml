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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.ukui.quick.items 1.0
import org.ukui.windowThumbnail 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager

MouseArea {
    id: baseMouseArea

    property bool isList
    property var thumbnailLayout: viewLoader.item.viewRepeater
    hoverEnabled: true

    property bool menuVisible: false
    property int thumbnailImplicitHeight: 192
    // 设置初始高度，规避初次显示时出现鼠标悬浮退出事件，影响预览图显示
    width: 272
    height: 192

    // 判断相对于默认宽高来说，是否进行缩放(为true时，预览图在不达到最小宽高时进行等比缩放)
    property real thumbnailScale: 1.0
    // 阴影的宽度(采用GlobalTheme.kShadowMenu)
    property int cachedOffset: 14
    property int offset:  {
        if (isList) {
            cachedOffset = Math.max(baseShadow.shadowSize, 14);
            return cachedOffset;

        } else {
            if (thumbnailLayout.count > 0) {
                cachedOffset = Math.max(thumbnailLayout.itemAt(0).shadowSize, 14);
                return cachedOffset;
            }
            return cachedOffset;
        }
    }

    // 预览图窗口布局和*默认*大小相关参数
    property int viewMargin: 8  // 预览图显示内容与任务栏的间距
    property int screenMargin: 8  // 预览图满屏时，与屏幕边缘的间距
    property int viewSpacing: 8  // 预览图模式下窗口间距
    property int layoutSpacing: 8 // 内部预览图组件与窗口的左右margin
    property var maximumSize : Qt.size(344, 344)
    property var minimumSize : Qt.size(120, 120)
    property var initSize: Qt.size(192, 192)
    onXChanged: {
        if (!baseMouseArea.isList) {
            viewLoader.item.updateAllBlurPosition();
        }
    }
    onYChanged: {
        if (!baseMouseArea.isList) {
            viewLoader.item.updateAllBlurPosition();
        }
    }
    function determineListViewSize() {
        if (!currentWinIdList) {
            return;
        }
        if (isHorizontal) {
            var listPreHeight = listViewBase.determineHeight(currentWinIdList).height;
            if (listPreHeight < 124) {
                baseMouseArea.isList = true;
                return;
            } else {
                viewLoader.preChildrenSize = listPreHeight;
                baseMouseArea.isList = false;
            }
        } else {
            var listPreWidth = listViewBase.determineWidth(currentWinIdList).width;
            if (listPreWidth < 124) {
                baseMouseArea.isList = true;
                return;
            } else {
                viewLoader.preChildrenSize = listPreWidth;
                baseMouseArea.isList = false;
            }
        }
    }

    DtThemeBackground {
        id: listViewBase
        backgroundColor: Platform.GlobalTheme.baseActive
        //预览图列表模式跟随主题透明度
        alpha: 0
        useStyleTransparency: isList

        width: viewLoader.width
        height: viewLoader.height
        borderColor: Platform.GlobalTheme.kLineNormal
        border.width: isList ? 1 : 0
        radius: Platform.GlobalTheme.kRadiusWindow

        DtDropShadow {
            id: baseShadow
            anchors.fill: listViewBase
            shadowData: isList ? Platform.GlobalTheme.kShadowMenu : undefined
            shadowRadius: listViewBase.radius
        }
        // 窗口尺寸不能为0
        onWidthChanged: {
            baseMouseArea.width = Math.max(Math.ceil(listViewBase.width), 1);
            baseMouseArea.height = Math.max(Math.ceil(listViewBase.height), 1);
        }
        onHeightChanged: {
            baseMouseArea.width = Math.max(Math.ceil(listViewBase.width), 1);
            baseMouseArea.height = Math.max(Math.ceil(listViewBase.height), 1);
        }

        function determineHeight(winIdList) {
            var totalWidth = 0;
            var thumbnailHeight = 144; // 这是预览图内部组件默认高度（192-24-layoutSpacing*3）
            if (!winIdList) {
                return;
            }

            for (var i = 0; i < winIdList.length; i++) {
                var itemGeometry = Platform.WindowManager.geometry(winIdList[i]);
                var thumbWidth = itemGeometry.width * (thumbnailHeight / itemGeometry.height);
                // 预览图在计算宽高时，也要考虑大小限制
                totalWidth = totalWidth + Math.min(Math.max(thumbWidth, minimumSize.width - layoutSpacing*2), maximumSize.width - layoutSpacing*2);
            }

            var otherAreaSize;
            // 其余区域的大小，包括间距和预览图的margin
            otherAreaSize = (layoutSpacing * 2 * winIdList.length) + (viewSpacing * (winIdList.length - 1));
            if ((baseMouseArea.widgetScreenGeo.width - (screenMargin * 2) - otherAreaSize) < totalWidth) {
                thumbnailScale = (baseMouseArea.widgetScreenGeo.width - screenMargin * 2 - otherAreaSize) / totalWidth;
                thumbnailHeight = thumbnailHeight * thumbnailScale;
            } else {
                thumbnailScale = 1.0;
            }

            var listPreHeight = thumbnailHeight + 48;
            var listPreWidth = otherAreaSize + totalWidth;
            return Qt.rect(0, 0, listPreWidth, listPreHeight);
        }

        function determineWidth(winIdList) {
            var totalHeight = 0;
            var thumbnailWidth = 176; // 这是预览图内部组件默认宽度（192-layoutSpacing*2）
            var thumbnailOtherHeight = 48; //这是单个预览图内部，除预览图外的其他高度（48）
            if (!winIdList) {
                return;
            }

            for (var i = 0; i < winIdList.length; i++) {
                var itemGeometry = Platform.WindowManager.geometry(winIdList[i]);
                var thumbHeight = (itemGeometry.height * (thumbnailWidth / itemGeometry.width));
                // 预览图在计算宽高试，也要考虑大小限制
                totalHeight = totalHeight + Math.min(Math.max(thumbHeight, minimumSize.height - thumbnailOtherHeight), maximumSize.height - thumbnailOtherHeight);
            }

            var otherAreaSize;
            // 其余区域的大小，包括间距和预览图的margin,以及纵向布局时的title高度（24）
            otherAreaSize = (layoutSpacing * 3 + 24) * winIdList.length + viewSpacing * (winIdList.length - 1);

            if ((baseMouseArea.widgetScreenGeo.height - screenMargin * 2 - otherAreaSize) < totalHeight) {
                thumbnailScale = (baseMouseArea.widgetScreenGeo.height - screenMargin * 2 - otherAreaSize) / totalHeight;
                thumbnailWidth = thumbnailWidth * thumbnailScale;
            } else {
                thumbnailScale = 1.0;
            }

            var listPreWidth = thumbnailWidth + layoutSpacing*2;
            var listPreHeight = totalHeight + otherAreaSize;
            return Qt.rect(0, 0, listPreWidth, listPreHeight);
        }

        Loader {
            id: viewLoader
            property real preChildrenSize
            sourceComponent: baseMouseArea.isList ? listMode : thumbnailMode
        }

        /**
         * 预览图显示View： 列表模式、预览图模式
         */
        Component {
            id: thumbnailMode
            Item {
                width: childrenRect.width
                height: childrenRect.height
                property var viewRepeater: repeater

                function updateAllBlurPosition() {
                    thumbnailView.windowBlurRegion.clear();
                    let rects = [];

                    for (var i = 0; i < repeater.count; i++) {
                        let item = repeater.itemAt(i);
                        if (item) {
                            repeater.itemAt(i).updateBlurPosition();
                            rects.push(item.itemRegion);
                        }
                    }
                    thumbnailView.windowBlurRegion.appendRectList(rects);
                }

                Column {
                    id: columnLayout
                    spacing: viewSpacing
                    onPositioningComplete: {
                        updateAllBlurPosition();
                    }
                }
                Row {
                    id: rowLayout
                    spacing: viewSpacing
                    onPositioningComplete: {
                        updateAllBlurPosition();
                    }
                }
                Repeater {
                    id: repeater
                    parent: isHorizontal ? rowLayout : columnLayout
                    model: thumbnailModel
                    delegate: thumbnailDelegate
                    function indexAt(x, y) {
                        for (let i = 0; i < thumbnailModel.count; ++i) {
                            let item = repeater.itemAt(i);
                            let point = item.mapFromItem(baseMouseArea, Qt.point(x, y));
                            if (item.contains(point)) {
                                return i;
                            }
                        }
                        return -1;
                    }
                }
            }
        }
    }
    Component {
        id: listMode
        RowLayout {
            property bool isIntervactive: listModelView.contentItem.childrenRect.height > listModelView.height
            property var viewRepeater: listModelView

            width: 240
            height: listModelView.height + listModelView.Layout.margins * 2
            spacing: 0

            ListView {
                id: listModelView
                Accessible.name: "ukui-panel_taskManager_widget_thumbnail_listModelView"
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(contentItem.childrenRect.height, 360)
                Layout.margins: 4
                Layout.rightMargin: 0

                clip: true
                spacing: 0
                interactive: parent.isIntervactive
                orientation: ListView.Vertical
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: scrollBarControl

                model: thumbnailModel
                delegate: thumbnailDelegate
            }

            ScrollBar {
                id: scrollBarControl
                Layout.fillHeight: true
                Layout.preferredWidth: 12
                Layout.margins: 4
                Layout.leftMargin: 0
                visible: (size < 1 && size > 0)

                padding: 4
                contentItem: DtThemeBackground {
                    radius: width / 2
                    useStyleTransparency: false
                    backgroundColor: Platform.GlobalTheme.kGrayAlpha3
                }
            }
        }
    }

    /**
     * 预览图delegate
     */
    Component {
        id: thumbnailDelegate
        MouseArea {
            id: itemMouseArea
            width: thumbnailLoader.width
            height: thumbnailLoader.height
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property alias itemRegion: blurRegion
            property alias shadowSize: itemShadow.shadowSize

            function activateWindowView() {
                var winIds = [model.winId];
                if (baseMouseArea.menuVisible || containsMouse) {
                    activeTimer.interval = 500;
                    activeTimer.activeWinIds = winIds;
                    activeTimer.start();
                } else {
                    activeTimer.stop();
                    UkuiTaskManager.TaskManager.activateWindowView([]);
                }
            }

            // itemMouseArea.mapToItem(viewBase, 0, 0).x的属性绑定设置，在itemMouseArea的位置变化时，不能直接更新绑定，只能用显示函数绑定的方式
            function updateBlurPosition(){
                blurRegion.x = Qt.binding(function() {
                    return itemMouseArea.mapToItem(viewBase, 0, 0).x
                });
                blurRegion.y = Qt.binding(function() {
                    return itemMouseArea.mapToItem(viewBase, 0, 0).y
                });
                blurRegion.width = Qt.binding(function() {
                    return itemMouseArea.width
                });
                blurRegion.height = Qt.binding(function() {
                    return itemMouseArea.height
                });
            }

            Component.onDestruction: {
                thumbnailView.windowBlurRegion.removeRect(blurRegion);
            }

            RectRegion {
                id: blurRegion
                radius: thumbnailLoader.item.radius
            }

            DtDropShadow {
                id: itemShadow
                anchors.fill: thumbnailLoader
                shadowData: isList ? undefined : Platform.GlobalTheme.kShadowMenu
                shadowRadius: thumbnailLoader.item.radius
            }
            Timer {
                id: activeTimer
                property var activeWinIds: []
                onTriggered: {
                    UkuiTaskManager.TaskManager.activateWindowView(activeTimer.activeWinIds)
                }
            }

            onContainsMouseChanged: activateWindowView()

            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    var actions = UkuiTaskManager.TaskManager.windowActions(model.winId);
                    baseMouseArea.menuVisible = taskManager.viewModel.openMenu(true, actions, viewLoader);
                } else {
                    // 需要先激活窗口再隐藏预览图
                    // 如果先隐藏预览图再激活窗口，会因为先取消highlight窗口，导致屏幕闪一下其他窗口的问题
                    UkuiTaskManager.TaskManager.activateWindow(model.winId);
                    taskManager.hidethumbnail();
                }
            }
            Loader {
                id: thumbnailLoader
                WindowThumbnailMprisModel {
                    id: mprisModel
                    winID: model.winId
                    pid: Platform.WindowManager.pid(model.winId)
                }
                property string modelData: model.winId
                property var title: windowTitles ? windowTitles[model.winId] : ""
                property var icon: windowIcons ? windowIcons[model.winId] : ""
                property var iconName: windowIconsName ? windowIconsName[model.winId] : ""
                property var windowThumbnailMprisModel: mprisModel
                property bool mouseAreaContainsMouse: itemMouseArea.containsMouse || viewDropArea.targetThumbnailIndex === index
                property bool mouseAreaContainsPress: itemMouseArea.containsPress

                // 预览图限制宽高(最大最小宽高限制，要随满屏*等比例缩放* ）
                property real maximumHeight: isHorizontal ? maximumSize.height : maximumSize.height * baseMouseArea.thumbnailScale
                property real minimumHeight: isHorizontal ? minimumSize.height : minimumSize.height * baseMouseArea.thumbnailScale
                property real maximumWidth: isHorizontal ? maximumSize.width * baseMouseArea.thumbnailScale : maximumSize.width
                property real minimumWidth: isHorizontal ? minimumSize.width * baseMouseArea.thumbnailScale : minimumSize.width

                // mprisModel.count > 0为mpris音视频预览图
                sourceComponent: {
                    if (mprisModel.count > 0) {
                        if (mprisModel.isCurrentMediaAudio) {
                            return musicThumbanilComponent;
                        } else {
                            return videoThumbnailComponent;
                        }
                    } else {
                        return thumbnailComponent;
                    }
                }
            }
        }
    }
    /**
     * 预览图组件显示: 普通预览图（预览图界面、列表界面）、mpris音频预览图、mpris视频预览图
     */
    Component {
        id: thumbnailComponent
        ThumbnailContainer {
            width: baseMouseArea.isList ? 220 : (isHorizontal ? contentWidth : viewLoader.preChildrenSize)
            height: baseMouseArea.isList ? 36 : (isHorizontal ? viewLoader.preChildrenSize : contentHeight)

            property real contentHeight: childrenRect.height
            property real contentWidth: childrenRect.width

            isList: baseMouseArea.isList
            spacing: layoutSpacing
            containerIcon: (iconName === undefined || iconName === "") ? icon : iconName
            containerTitle: title === undefined ? "" : title
            containsMouse: mouseAreaContainsMouse
            containsPress: mouseAreaContainsPress
            viewIsHorizontal: isHorizontal

            onCloseButtonClicked: {
                UkuiTaskManager.TaskManager.activateWindowView([]);
                UkuiTaskManager.TaskManager.execSpecifiedAction(UkuiTaskManager.Action.Close, modelData);
            }
        }
    }
    Component {
        id: musicThumbanilComponent
        DtThemeBackground {
            backgroundColor: Platform.GlobalTheme.baseActive
            radius: Platform.GlobalTheme.kRadiusWindow
            useStyleTransparency: true
            width: childrenRect.width
            height: childrenRect.height
            borderColor: Platform.GlobalTheme.kLineNormal
            border.width: 1


            AudioPalyerThumbnail {
                playerData: windowThumbnailMprisModel
                containsMouse: mouseAreaContainsMouse
                windowRadius: parent.radius
                onCloseButtonClicked: {
                    UkuiTaskManager.TaskManager.activateWindowView([]);
                    UkuiTaskManager.TaskManager.execSpecifiedAction(UkuiTaskManager.Action.Close, modelData);
                }
            }
        }
    }
    Component {
        id: videoThumbnailComponent
        DtThemeBackground {
            backgroundColor: Platform.GlobalTheme.baseActive
            radius: Platform.GlobalTheme.kRadiusWindow
            useStyleTransparency: true
            width: childrenRect.width
            height: childrenRect.height
            property int thumbnailMargin: 8
            borderColor: Platform.GlobalTheme.kLineNormal
            border.width: 1

            Repeater {
                id: view
                model: windowThumbnailMprisModel
                ColumnLayout {
                    id: baseLayout
                    spacing: 0
                    height: {
                        if (isHorizontal) {
                            return viewLoader.preChildrenSize;
                        } else {
                            var preHeight = windowThumbnail.height + layoutSpacing * 3 + 24;
                            return Math.max(preHeight, minimumHeight);
                        }
                    }
                    width: {
                        if (isHorizontal) {
                            var preWidth = windowThumbnail.width + layoutSpacing * 2;
                            return Math.max(preWidth, minimumWidth);
                        } else {
                            return viewLoader.preChildrenSize;
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24
                        Layout.maximumHeight: 24
                        Layout.margins: thumbnailMargin
                        Layout.bottomMargin: 0

                        Icon {
                            Layout.preferredHeight: 24
                            Layout.preferredWidth: 24
                            Layout.rightMargin: thumbnailMargin
                            source: (iconName === undefined || iconName === "") ? icon : iconName
                        }
                        DtThemeText {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            text: title === undefined ? "" : title
                            textColor: Platform.GlobalTheme.kFontPrimary
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter

                            Tooltip {
                                anchors.fill: parent
                                mainText: parent.text
                                posFollowCursor: true
                                visible: parent.truncated
                            }
                        }
                        MouseArea {
                            id: closeButtonArea
                            Layout.preferredHeight: 24
                            Layout.preferredWidth: 24
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            hoverEnabled: true
                            visible: mouseAreaContainsMouse

                            DtThemeBackground {
                                Accessible.role: Accessible.Button
                                Accessible.description: "This is a close button of ukui-panel thumbnail window"
                                Accessible.name: "ukui-panel_thumbnail_window_close_button"
                                backgroundColor: closeButtonArea.containsPress ? Platform.GlobalTheme.kErrorClick :
                                                                                 Platform.GlobalTheme.kErrorNormal
                                radius: Platform.GlobalTheme.kRadiusMin
                                anchors.fill: parent
                                visible: parent.containsMouse

                                Tooltip {
                                    anchors.fill: parent
                                    mainText: qsTr("Close")
                                    posFollowCursor: true
                                }
                            }
                            Icon {
                                anchors.centerIn: parent
                                width: 16
                                height: width
                                mode: parent.containsMouse ? Icon.Highlight : Icon.AutoHighlight
                                source: "window-close-symbolic"
                            }
                            onClicked: {
                                UkuiTaskManager.TaskManager.activateWindowView([]);
                                UkuiTaskManager.TaskManager.execSpecifiedAction(UkuiTaskManager.Action.Close, modelData);
                            }
                        }
                    }
                    VideoPlayerThumbnail {
                        id: windowThumbnail
                        Layout.alignment: Qt.AlignHCenter
                        Layout.margins: thumbnailMargin
                        Layout.preferredWidth: isHorizontal ? itemWidth : viewLoader.preChildrenSize - layoutSpacing*2
                        Layout.preferredHeight: isHorizontal ? viewLoader.preChildrenSize - 48 : itemHeight
                        Layout.maximumHeight: maximumHeight - layoutSpacing * 3 - 24
                        Layout.maximumWidth: maximumWidth - layoutSpacing * 2

                        property real itemHeight
                        property real itemWidth
                        winId: windowVisible ? modelData : ""
                        playerData: windowThumbnailMprisModel
                        viewModel: view.model
                        // 视频预览图需要保证按钮显示完全
                        clip: false
                        onWinIdChanged: {
                            if (winId === "") {
                                return;
                            }
                            var itemGeometry = Platform.WindowManager.geometry(modelData);
                            if (isHorizontal) {
                                itemWidth = itemGeometry.width * ((viewLoader.preChildrenSize - 48) / itemGeometry.height);
                            } else {
                                itemHeight = itemGeometry.height * ((viewLoader.preChildrenSize - layoutSpacing*2) / itemGeometry.width);
                            }
                        }
                    }
                }
            }
        }
    }
}
