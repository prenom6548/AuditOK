/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import org.kylin.background 3.0 as KWin
import AppWindowFilterModel 1.0
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform


    Item {
        id : tabletViewMain
        property var isWlcom : multitaskViewManager.isWlcom()
        property rect screenRect : Qt.rect(x, y, width, height)
        // 设备的物理方向是否为横屏
        property bool isLandSpace: screenRect.width > screenRect.height
        // 整体缩放比
        property var tabletScaleFactor : isLandSpace ? screenRect.height / 1080 : screenRect.width / 1080
        // 预览窗口宽高比
        property real previewWinAspectRatio: screenRect.width / screenRect.height
        // 图标和缩略图的间距
        property int iconToThumbSpacing : 12 * tabletScaleFactor
        // 图标高度
        property int iconAndCloseButtonHeight : 32 * tabletScaleFactor
        // 缩略图高度
        property var preWinHeight : getPreWinHeight()
        // 缩略图宽度
        property var preWinWidth : getPreWinWidth()
        property int borderWidth : 4 * tabletScaleFactor
        // 预览窗口列间距
        property var preWinVecSpace : getPreWinVecSpace()
        // 预览窗口行间距
        property var preWinHorSpace : getPreWinHorSpace()
        // 滚动区域距离左侧和右侧的间距
        property var scrollViewLeftSpace : getScrollViewLeftSpace()
        // 滚动区域高度
        property var scrollViewHeight : getScrollViewHeight()
        // 滚动区域 y 轴起始高度
        property var scrollViewYStartPoint : getYStartPoint()
        property ListModel tabletViewRootModel : ListModel{}
        property var language : multitaskViewModel.getCurrentLanguage()
        property int screenIndex : 0
        // 水平布局方向
        property bool isLayoutLeftToRight: Qt.application.layoutDirection === Qt.LeftToRight
        // 是否拥有tab焦点
        property bool shouldHaveTabFocus: false

        focus : true
        property int loadedWindowNum : 0
        /*
            此处的focus不可以删除，因为后台在grabbedKeyboardEvent()函数中增加了setfocus()
            不增加focus:true前端无法接受键盘事件
            这是正向解决方法，其他方法非正向解决，不推荐！！！
        */

        signal blankAreaClickSignal()
        signal clearAllWindowButtonClickSignal()
        //由于执行小窗口关闭动效时，会关闭多任务视图多次，导致崩溃，故增加此属性
        property bool alreadyCloseView: false


        function getPreWinHeight() {
            return isLandSpace ?  Math.floor(324 * tabletScaleFactor) : Math.floor(498 * tabletScaleFactor)
        }

        function getPreWinWidth() {
            return preWinHeight * previewWinAspectRatio
        }

        function getPreWinVecSpace() {
            return isLandSpace ? Math.floor(112 * tabletScaleFactor) : Math.floor(96 * tabletScaleFactor)
        }

        function getPreWinHorSpace() {
            return isLandSpace ? Math.floor(56 * tabletScaleFactor) : Math.floor(68 * tabletScaleFactor)
        }

        function getScrollViewLeftSpace() {
            return isLandSpace ? Math.floor(112 * tabletScaleFactor) : Math.floor(96 * tabletScaleFactor)
        }

        function getScrollViewHeight() {
            return isLandSpace ? Math.floor(820 * tabletScaleFactor) : Math.floor(1196 * tabletScaleFactor)
        }

        function getYStartPoint() {
            return isLandSpace ? Math.floor(99 * tabletScaleFactor) : Math.floor(196 * tabletScaleFactor)
        }

        function sequentialLoadWindow() {
            var appWindowCount = appListModel.rowCount()
            if (loadedWindowNum < appWindowCount && loadedWindowNum < 6 ) {
                tabletViewRootModel.append(getAppWindowData(loadedWindowNum))
                loadedWindowNum = loadedWindowNum + 1
                // console.log("加载窗口中，目前已加载窗口数量为" + loadedWindowNum)

            } else {
                sequentialLoadWindowTimer.stop()
                // console.log("计时器停止，窗口数量为： " + tabletViewRootModel.count)
            }
            if (loadedWindowNum >= 6) {
                sequentialLoadWindowTimer.stop()
                console.log("计时器停止，接下来一次性加载所有窗口")
                while (loadedWindowNum < appWindowCount) {
                    tabletViewRootModel.append(getAppWindowData(loadedWindowNum))
                    loadedWindowNum = loadedWindowNum + 1
                }
            }
            if (loadedWindowNum >= appWindowCount) {
                sequentialLoadWindowTimer.stop()
                // console.log("所有窗口加载完毕，显示全部清除按钮")
                clearAllWindowButtonAppearAni.start()
            }
        }

        function getAppWindowData(ind) {
            var sourceIndex = appListModel.mapToSource(appListModel.index(ind, 0))
            if (!sourceIndex || sourceIndex.row < 0) {
                return null
            }
            var info = appWindowListModel.getItemData(sourceIndex.row)
            if (!info || !info.Geometry) {
                return null;
            }
            var data = {
                "x": info.Geometry.x,
                "y": info.Geometry.y,
                "width": info.Geometry.width,
                "height": info.Geometry.height,
                "id": info.Id,
                "minimized": info.Minimized,
                "caption": info.Caption,
                "uuid": String(info.Uuid),
                "icon": info.Icon
            }
            return data
        }

        function closeMultitaskView() {
            if (!alreadyCloseView) {
                alreadyCloseView = true
                console.log("closeMultitaskView() !")
                multitaskViewManager.close()
            }
        }

        AppWindowFilterModel {
            id: appListModel
            desktopId: desktopListModel.highlightDesktop
            screenId: tabletViewMain.screenIndex
            sourceModel: appWindowListModel
        }

        // 这里放置后台提供的桌面背景的 Item
        Loader {
            anchors.fill: parent
            //active: !tabletViewMain.isWlcom
            sourceComponent: KWin.DesktopBackground {
                anchors.fill : parent
                blurAlpha : 0.7
            }
        }

        // 控制启动动效中，小窗口之间的间隔
        Timer {
            id : sequentialLoadWindowTimer
            interval : 150
            repeat : true
            running : false
            triggeredOnStart : false
            onTriggered: {
                // console.log("计时器启动 间隔150")
                tabletViewMain.sequentialLoadWindow()
            }
        }

        UkuiItem.DtThemeBackground {
            id : scrollBackgroundArea
            anchors.fill: parent
            useStyleTransparency: true
            backgroundColor: UkuiPlatform.GlobalTheme.kModalMask
            // 无应用时的窗口显示
            UkuiItem.DtThemeText {
                id : noWindowText
                anchors.centerIn : parent
                font.bold : true
                font.pixelSize : Math.floor(25 * tabletScaleFactor)
                textColor : UkuiPlatform.GlobalTheme.kFontWhite
                text : qsTr("No recent tasks") // 无应用开启
                visible : tabletViewRootModel.count === 0
                opacity : 0

                SequentialAnimation{
                    id :  noWindowTipsAnimation
                    // 无窗口文字出现动画
                    NumberAnimation {
                        id : noWindowTipsAppearAni
                        target : noWindowText
                        property : "opacity"
                        duration : 1000
                        easing.type : Easing.InOutQuart
                        from : 0
                        to : 1
                    }
                    // 无窗口文字消失动画
                    NumberAnimation {
                        id : noWindowTipsDisappearAni
                        target : noWindowText
                        property : "opacity"
                        duration : 1000
                        easing.type : Easing.InOutQuart
                        from : 1
                        to : 0
                    }
                    onFinished: {
                        console.log("无窗口文字消失动画")
                        tabletViewMain.closeMultitaskView()
                    }
                }
            }
        }

        // 预览窗口显示区域
        ScrollView {
            id : previewWindowShownRegion
            anchors.fill : parent

            //更新滚动区域的大小，之所以这么做，是为了解决滚动区域增加鼠标事件后，滚动失效的问题
            function getContentWidth()
            {
                var maxColumn = Math.ceil(tabletViewRootModel.count / 2)
                var scrollViewContentWidth = parent.width
                if (tabletViewRootModel.count <= 4)
                {
                    scrollViewContentWidth = parent.width
                    return Math.floor(scrollViewContentWidth)
                }
                scrollViewContentWidth = maxColumn * preWinWidth + maxColumn * gridLayout.columnSpacing + 2 * gridLayout.leftPadding
                return Math.floor(scrollViewContentWidth)
            }
            contentWidth : Math.floor(previewWindowShownRegion.getContentWidth())
            contentHeight : Math.floor(this.height)
            ScrollBar.horizontal.policy : ScrollBar.AlwaysOff
            ScrollBar.vertical.policy : ScrollBar.AlwaysOff

            MouseArea {
                Accessible.name: "tablet view"
                anchors.fill : parent
                onClicked : {
                    if (multitaskViewModel.isTabletMode) {
                        multitaskViewManager.setShowingDesktop(true)
                    }
                    tabletViewMain.blankAreaClickSignal()
                }
            }

            Grid {
                id : gridLayout
                anchors.fill : parent
                anchors.centerIn : parent
                topPadding : Math.floor(scrollViewYStartPoint)
                columnSpacing : Math.floor(preWinVecSpace)
                rowSpacing : Math.floor(preWinHorSpace)
                leftPadding : Math.floor(scrollViewLeftSpace)
                rightPadding : Math.floor(scrollViewLeftSpace)
                rows : 2
                flow : Grid.TopToBottom
                transformOrigin : "TopLeft"

                //显示多任务视图时，每个Item在显示的时候执行的动效
                add : Transition {
                    id : displayPreviewWinAnimation
                    ParallelAnimation {
                        NumberAnimation {
                            property : "opacity"
                            duration : 900
                            easing.type : Easing.InOutQuart
                            from : 0
                            to : 1
                        }
                        NumberAnimation {
                            property : "x"
                            duration : 900
                            easing.type : Easing.InOutQuart
                            from : Math.floor(parent.width / 2 - 510/*窗口的宽度*/ / 2)
                        }
                        NumberAnimation {
                            property : "y"
                            duration : 900
                            easing.type : Easing.InOutQuart
                            from : Math.floor(parent.height)
                        }
                        NumberAnimation {
                            property : "scale"
                            from : 0
                            to : 1.0
                            duration : 450
                        }
                    }
                }

                //关闭小窗口时，W补齐方式的动画
                move : Transition {
                    NumberAnimation{
                        properties : "x,y";
                        easing.type : Easing.InOutQuart;
                        duration : 300
                    }
                }

                Repeater {
                    id : previewWindowRepeater
                    model : tabletViewRootModel

                    Item {
                        id : appWindowBack
                        height : Math.floor(preWinHeight + iconToThumbSpacing + iconAndCloseButtonHeight)
                        width : Math.floor(preWinWidth) + borderWidth * 2

                        TabletPreviewWindow {
                            id : appWindow
                            height : parent.height
                            width : parent.width
                            x_wid : model.id
                            wid : model.uuid
                            captionText : model.caption
                            preViewNoScaledWidth : model.width
                            preViewNoScaledHeight : model.height
                            preViewNoScaledXCoordinate : model.x
                            preViewNoScaledYCoordinate : model.y
                            icon: model.icon
                        }
                    }
                }
            }
        }

        // 清除所有窗口的按钮
        UkuiItem.DtThemeBackground {
            id : clearAllWindowButton
            property int bottomMargin : 38 * tabletScaleFactor
            height : Math.floor(48 * tabletScaleFactor)
            width : language === "Chinese" ? Math.floor(112 * tabletScaleFactor) : Math.floor(184 * tabletScaleFactor)
            backgroundColor: clearAllMousearea.containsPress ? UkuiPlatform.GlobalTheme.kComponentClick
                                 : clearAllMousearea.containsMouse ? UkuiPlatform.GlobalTheme.kComponentHover : UkuiPlatform.GlobalTheme.kComponentNormal
            radius : Math.floor(24 * tabletScaleFactor)
            anchors.verticalCenter : parent.verticalCenter
            anchors.verticalCenterOffset : Math.floor(tabletViewMain.height / 2) - bottomMargin - this.height
            anchors.horizontalCenter : parent.horizontalCenter
            visible : tabletViewRootModel.count > 0
            opacity : 0

            Accessible.role: Accessible.Button
            Accessible.name: "clear"
            Accessible.description: "a push button, click to clear all preview windows"

            Row {
                anchors.centerIn: parent
                spacing: Math.floor(8 * tabletScaleFactor)
                UkuiItem.Icon {
                    id : deleteIcon
                    anchors.verticalCenter : parent.verticalCenter
                    width : Math.floor(16 * tabletScaleFactor)
                    height : Math.floor(16 * tabletScaleFactor)
                    source: "window-close-symbolic"
                    mode: UkuiItem.Icon.AutoHighlight
                }

                UkuiItem.DtThemeText {
                    id : deletText
                    anchors.verticalCenter : parent.verticalCenter
                    text : qsTr("Clear") // 清除
                    elide : isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                    font.pixelSize : Math.floor(18 * tabletScaleFactor)
                    textColor : UkuiPlatform.GlobalTheme.kFontPrimary
                }
            }

            MouseArea {
                id: clearAllMousearea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    console.log("全部清除被点击！")
                    clearAllWindowButtonDisappearAni.start()
                    tabletViewMain.clearAllWindowButtonClickSignal()
                }
            }

            //全部清除按钮出现的动画
            NumberAnimation {
                id : clearAllWindowButtonAppearAni
                target : clearAllWindowButton
                properties : "opacity"
                from : 0
                to : 1
                duration : tabletViewRootModel.count >= 6 ? 5 * 100 + 300 : (tabletViewRootModel.count - 1) * 100 + 300
                easing.type : Easing.InOutQuart
            }

            //全部清除按钮消失的动画
            NumberAnimation {
                id : clearAllWindowButtonDisappearAni
                target : clearAllWindowButton
                properties : "opacity"
                from : 1
                to : 0
                duration : 300
                easing.type : Easing.InOutQuart
            }

            Connections {
                target : tabletViewMain
                function onBlankAreaClickSignal() {
                    clearAllWindowButtonDisappearAni.start()
                }
            }
        }

        Timer {
            id : closeViewTimer
            interval : 0
            repeat : false
            running : false
            triggeredOnStart : false
            onTriggered: {
                closeMultitaskView()
            }
        }

        Component.onCompleted: {
            if (isWlcom) {
                return
            }

            if (currentDesktopHasWindow()) {
                console.log("顺序加载窗口的启动计时器")
                sequentialLoadWindowTimer.start()
            } else {
                console.log("启动无窗口动画")
                noWindowTipsAnimation.start()
            }
        }

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                closeMultitaskView();
                return;
            }
            if (event.modifiers & Qt.MetaModifier) {
                if (event.key === Qt.Key_Tab) {
                    closeMultitaskView();
                    return;
                }
            }
        }

        Connections {
            target: tabletViewMain.isWlcom ? multitaskViewManager : null
            function onWinTabVisibleChanged() {
                if (!multitaskViewManager.winTabVisible) {
                    // 清空数据
                    loadedWindowNum = 0
                    tabletViewRootModel.clear()
                    return
                }
                // 播放显示动画
                if (currentDesktopHasWindow()) {
                    sequentialLoadWindowTimer.start()
                } else {
                    noWindowTipsAnimation.start()
                }
            }
        }

        Connections {
            target: appWindowListModel
            function onAppWindowRemoved(windowId) {
                // remove window from tabletViewRootModel
                for(var i = 0; i < tabletViewRootModel.count; i++){
                    var info = tabletViewRootModel.get(i);
                    if(info.uuid === windowId) {
                        tabletViewRootModel.remove(i);
                        break;
                    }
                }
                if (!currentDesktopHasWindow()) {
                    closeViewTimer.start()
                }
            }
        }

        function currentDesktopHasWindow() {
            return appListModel.rowCount() > 0
        }
    }
