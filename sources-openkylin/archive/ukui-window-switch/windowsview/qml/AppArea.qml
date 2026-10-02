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
import QtQuick.Layouts 1.3
import "Controller.js" as Controller
import org.ukui.positioner 1.0
import org.ukui.quick.items 1.0 as UkuiItem

FocusScope {
    id : root
    property var appAreaController : new Controller.AppController(appWindowListModel, root, multitaskViewManager, ["none"])
    property int currentTabIndex: 0

    Flickable {
        id : scrollArea
        anchors.fill : parent
        // 预览图布局区域 y 轴起始高度
        property int scrollViewYStartPoint : 32 * scaleFactor
        // 预览图布局区域距离左侧和右侧的间距
        property int scrollViewLeftSpace : isVerticalLayout ? 32 : 0
        anchors.leftMargin: scrollViewLeftSpace //注意：间距为0则无法显示滚动条
        anchors.rightMargin: scrollViewLeftSpace
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        property ListModel windowPosInfoModel: ListModel{}
        property var rowSpacing : isVerticalLayout ? Math.floor(16 * scaleFactor) : Math.floor(10 * scaleFactor)
        property var columnSpacing : isVerticalLayout ? Math.floor(16 * scaleFactor) : Math.floor(10 * scaleFactor)
        height: parent.height
        width: parent.width
        contentWidth : parent.width - scrollViewLeftSpace * 2
        contentHeight : Math.max(flow.height, height)
        clip : true

        UkuiItem.DtThemeScrollBar.vertical: UkuiItem.DtThemeScrollBar {
            id: vScrollBar
            Accessible.role: Accessible.ScrollBar
            Accessible.name: "apparea-vertical-scrollbar"
            Accessible.description: "a scroll bar, which allows the user to scroll the visible area"
            parent: scrollArea.parent
            anchors.top: scrollArea.top
            anchors.left: scrollArea.right
            anchors.bottom: scrollArea.bottom
            visual : flow.height > scrollArea.height
        }

        MouseArea{
            anchors.fill : parent
            Accessible.name: "app area"
            onClicked : {
                appAreaController.closeMultitaskView()
            }
        }


        NumberAnimation {
            id : scrollMoveAnimation
            target : vScrollBar
            property : "position"
            from : vScrollBar.position
            duration : 300
            easing.type : Easing.InOutQuart
        }

        UkuiFlow {
            id : flow
            property bool needFlowYAnimation : false

            spacing : scrollArea.rowSpacing
            topPadding: scrollArea.scrollViewYStartPoint
            width : parent.width
            flow : Flow.LeftToRight
            layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft
            lineHorizontalCenterAlignment: true

            move: Transition { //子项增加时或者减少时的过度，也就是子项的visible变化的过度
                enabled: flow.needFlowYAnimation // 避免首次打开时播放移动动画
                NumberAnimation {
                    properties : "x,y";
                    easing.type : Easing.InOutQuad;
                    duration : isVerticalLayout ? 500 : 300
                }
            }

            onHeightChanged: {
                y = Math.max((scrollArea.height - flow.height) / 2, 0)
            }

            // flow 垂直居中动画
            Behavior on y {
                enabled: flow.needFlowYAnimation
                NumberAnimation {
                    duration : isVerticalLayout ? 500 : 300
                    easing.type : Easing.InOutQuart
                }
            }

            onPositioningComplete: {
                updateWindowPosInfo()
            }

            function updateWindowPosInfo() {
                var rowWindowNum = 0  //窗口区域每行的窗口数
                var leftX = 0
                var rightX = 0
                var rowY = flow.topPadding
                var itemCount = 0
                var winModel = []
                scrollArea.windowPosInfoModel.clear()

                for (let i = 0; i < flow.children.length; ++i) {
                    if (flow.children[i].width === 0) {
                        continue
                    }

                    // 根据预览窗 Y 坐标判断是否到下一行
                    if (flow.children[i].y !== rowY) {
                        if (winModel.length > 0) {
                            scrollArea.windowPosInfoModel.append({model : winModel, rowWindowCount : rowWindowNum})
                            winModel = []
                            rowWindowNum = 0
                        }
                        rowY += flow.children[i].height + flow.spacing
                    }

                    // 更新预览窗在 Flow 布局中的 X 坐标并存入局部 model
                    itemCount++
                    leftX = flow.children[i].x
                    rightX = leftX + flow.children[i].width
                    winModel.push({index: itemCount - 1, leftX: leftX, rightX: rightX})
                    rowWindowNum++
                }
                if (winModel.length > 0) {
                    scrollArea.windowPosInfoModel.append({model : winModel, rowWindowCount : rowWindowNum})
                }
            }

            Repeater {
                id : previewWindowRepeater
                model : appAreaModel

                // item调用forceActiveFocus()获取焦点，这会强制Qt引擎建立从根节点到item的完整焦点通路，
                // 自动将路径上所有中间节点的 focus 设为 true
                // focus 只是一个内部状态标记，对于判断用户交互始终使用 activeFocus

                onCountChanged: {
                    if (count > 0) {
                        // shouldShowTabBorder == true说明已有组件显示活动焦点，不能强制转移活跃焦点
                        // 无组件显示活动焦点，则恢复默认焦点设置
                        if (!multitaskViewManager.shouldShowTabBorder) {
                            previewWindowRepeater.forceActiveFocus()
                        }
                        activeFocusOnTab = true
                    } else {
                        // 1.清除活跃焦点：强制转移活跃焦点到父容器scrollArea
                        if (!multitaskViewManager.shouldShowTabBorder) {
                            scrollArea.forceActiveFocus()
                        }
                        // 2.再修改属性（有活动焦点时，无法设置activeFocusOnTab属性为false）
                        activeFocusOnTab = false
                    }
                }

                delegate: Rectangle {
                    id : thumbnailitem
                    width: appWindow.width
                    height: appWindow.height
                    focus: true
                    color : "transparent"
                    property var thumbWidth : getThumbnailSize()[0]
                    property var thumbHeight : getThumbnailSize()[1]
                    property string uuid: model.Uuid

                    states: [
                        State {
                            name : "tabActive"
                            when: previewWindowRepeater.activeFocus && index === currentTabIndex && (multitaskViewManager.shouldShowTabBorder) && shouldHaveTabFocus
                            PropertyChanges {
                                target: appWindow.borderRectangle
                                borderColor : tabFocusBorderColor
                            }
                        }
                    ]

                    function getThumbnailSize()
                    {
                        var thumbHeight = thumbRectangleHeight
                        var max_width = parent.width / 3
                        var clientWidth = model.Geometry.width
                        var clientHeight = model.Geometry.height
                        var thumbWidth = Math.floor(thumbHeight * clientWidth / clientHeight)
                        if (thumbWidth > max_width) {
                            thumbWidth = max_width
                            thumbHeight = Math.floor(thumbWidth * clientHeight / clientWidth)
                        }
                        return [thumbWidth, thumbHeight]
                    }

                    AppPreviewWindow {
                        id : appWindow
                        Accessible.name: "app preview window"
                        // Warning: 因为 appWindow 的 parent 会改变，所以不要使用 parent 获取宽高
                        minWidth: Math.floor(root.width / 10)

                        thumbHeight : thumbnailitem.thumbHeight
                        thumbWidth : thumbnailitem.thumbWidth
                        windowType : "app"
                        x_wid : model.Id
                        wid : model.Uuid
                        icon : model.Icon
                        caption : model.Caption
                        parent: thumbnailitem

                        Drag.onActiveChanged : {
                            if (!Drag.active) {
                                appAreaController.onSourceDropped(appWindow, rootWindow.highlightDesktop, rootWindow.highlightDesktop)
                            } else {
                                zoomOutAnimation()
                            }
                        }
                    }
                }//Rectangle

                onActiveFocusChanged: {
                    if (previewWindowRepeater.activeFocus) {
                        currentTabIndex = 0
                        if (vScrollBar.position > 0) {
                            scrollMoveAnimation.to = 0
                            scrollMoveAnimation.start()
                        }
                        multitaskViewManager.activeAreaChangeDataCollect("Windows Preview Area")
                    }
                }

                onItemAdded : {
                    if (flow.needFlowYAnimation !== false) {
                        flow.needFlowYAnimation = false
                    }
                }

                onItemRemoved : {
                    if (flow.needFlowYAnimation !== true) {
                        flow.needFlowYAnimation = true
                    }
                }

                Keys.onPressed: {
                    if (rootWindow.isWlcom) {
                        multitaskViewManager.keyNavigationDataCollect(event.key)
                    }
                    if (event.key === Qt.Key_Left) {
                        currentTabIndex = flow.moveCurrentIndexLeft(currentTabIndex);
                        event.accepted = true

                    } else if (event.key === Qt.Key_Right ) {
                        currentTabIndex = flow.moveCurrentIndexRight(currentTabIndex);
                        event.accepted = true

                    } else if (event.key === Qt.Key_Up) {
                        currentTabIndex = flow.moveCurrentIndexUp(currentTabIndex);
                        event.accepted = true

                    } else if (event.key === Qt.Key_Down) {
                        currentTabIndex = flow.moveCurrentIndexDown(currentTabIndex);
                        event.accepted = true

                    } else if (event.key === Qt.Key_Tab) {
                        if (event.modifiers & Qt.MetaModifier) {
                            appAreaController.closeMultitaskView()
                            return
                        }

                        currentTabIndex = 0
                        event.accepted = false

                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                        appAreaController.onWindowClicked(previewWindowRepeater.itemAt(currentTabIndex).uuid);
                        event.accepted = true

                    } else if (event.key === Qt.Key_End) {
                        currentTabIndex = previewWindowRepeater.count - 1
                        event.accepted = true

                    } else if (event.key === Qt.Key_Home) {
                        currentTabIndex = 0
                        event.accepted = true

                    } else if (event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
                        var isKeyPageUp = event.key === Qt.Key_PageUp
                        currentTabIndex = appAreaController.keyPageUpDownSelectWindow(isKeyPageUp, scrollArea.windowPosInfoModel, currentTabIndex)
                        event.accepted = true

                    } else if (event.key === Qt.Key_Escape) {
                        appAreaController.closeMultitaskView()
                    }

                    if (event.key === Qt.Key_Tab || event.key === Qt.Key_Left ||
                            event.key === Qt.Key_Right || event.key === Qt.Key_Up ||
                            event.key === Qt.Key_Down || event.key === Qt.Key_Home ||
                            event.key === Qt.Key_End || event.key === Qt.Key_PageUp ||
                            event.key === Qt.Key_PageDown) {
                        multitaskViewManager.setShouldShowTabBorder(true)
                    }

                    if (event.accepted) {
                        scrollToTabItem()
                    }
                }

                Keys.onBacktabPressed : {
                    if (multitaskViewModel.logicalScreenCount > 1) {
                        multitaskViewManager.activatePreviousTaskView(rootWindow.screenIndex)
                        shouldHaveTabFocus = false
                    } else {
                        event.accepted = false
                    }
                }

                function scrollToTabItem() {
                    var currentNextY = itemAt(currentTabIndex).mapToItem(scrollArea, 0, 0).y
                    var previewWindowHeight = itemAt(currentTabIndex).height
                    if (currentNextY < 0) {
                        scrollMoveAnimation.to = vScrollBar.position - (-currentNextY) / scrollArea.contentHeight
                        scrollMoveAnimation.start()
                    } else if (currentNextY + previewWindowHeight > scrollArea.height) {
                        scrollMoveAnimation.to = vScrollBar.position + (currentNextY + previewWindowHeight - scrollArea.height) / scrollArea.contentHeight
                        scrollMoveAnimation.start()
                    }
                }
            }//Repeater

            Component.onCompleted: {
                if (appAreaModel.rowCount() !== 0) {
                    previewWindowRepeater.forceActiveFocus()
                }
                // 无应用窗口时不获取tab焦点
            }

            Connections {
                target: rootWindow.isWlcom ? multitaskViewManager : null
                enabled: multitaskViewManager.winTabVisible
                function onWinTabVisibleChanged() {
                    if (appAreaModel.rowCount() !== 0
                            // 扩展屏模式，避免首次按下Tab键，非激活窗口预览窗显示焦点边框
                            && shouldHaveTabFocus) {
                        previewWindowRepeater.forceActiveFocus()
                    }
                }
            }

            Connections {
                target: previewWindowRepeater
                enabled: !multitaskViewManager.winTabVisible
                function onCountChanged() {
                    flow.forceLayout();
                }
            }
        }//Flow
    }//ScrollView
}
