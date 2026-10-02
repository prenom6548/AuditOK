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
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import DesktopListModel 1.0
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform

FocusScope {
    id : root
    //虚拟桌面的各个参数
    property bool desktopHoverStatus : false
    //虚拟桌面标题栏高度
    property int desktopCaptionHeight : Math.floor(32 * scaleFactor)
    property int spacing : Math.floor(24 * scaleFactor)
    property int desktopRectWidth : desktopThumbnailWidth + 2 * workspaceBorderWidth
    property int desktopRectHeight : desktopRect.height
    property int desktopMoveDistance : desktopRectWidth / 2

    UkuiItem.DtThemeBackground {
        id : desktopRect
        anchors.fill : parent
        useStyleTransparency : true
        backgroundColor : UkuiPlatform.GlobalTheme.kContainGeneralNormal

        MouseArea {
            id : desktopMouseArea
            Accessible.name: "desktop area"
            anchors.fill : parent
            hoverEnabled : true
            onEntered: {
                desktopHoverStatus =  true
            }
            onExited: {
                desktopHoverStatus = false
            }
            onClicked: {
                focus = true
            }

            onWheel: {
                if (!hScrollBar.visible) {
                    return;
                }
                // 鼠标滚轮操作滚动条
                if (wheel.angleDelta.y > 0) {
                    hScrollBar.decrease();
                } else if (wheel.angleDelta.y < 0) {
                    hScrollBar.increase();
                }
            }

            ListView {
                id : desktopListView
                readonly property string newDesktop: "newDesktop"
                readonly property bool isOverflow: contentWidth > width
                Accessible.name: "desktop listview"
                anchors.fill : parent
                property bool initMultitask : true
                orientation : ListView.Horizontal
                layoutDirection: isLayoutLeftToRight ? Qt.LeftToRight : Qt.RightToLeft
                property int contentXoffset : isLayoutLeftToRight ? 0 : contentWidth
                model : desktopListModel
                spacing : root.spacing
                activeFocusOnTab: false
                clip : true
                keyNavigationEnabled: true
                interactive: true
                highlightMoveDuration: 200

                UkuiItem.DtThemeScrollBar.horizontal: UkuiItem.DtThemeScrollBar {
                    id: hScrollBar
                    Accessible.role: Accessible.ScrollBar
                    Accessible.name: "desktoparea-horizontal-scrollbar"
                    Accessible.description: "a scroll bar, which allows the user to scroll the visible area"
                    policy: UkuiItem.DtThemeScrollBar.AsNeeded
                    visual: desktopListView.isOverflow
                }

                onActiveFocusChanged: {
                    if (desktopListView.activeFocus) {
                        currentIndex = 0
                    }

                    // interactive == false,透传鼠标事件,使desktopMouseArea clicked事件得以触发,退出工作区名称编辑状态
                    // interactive == true,截获鼠标事件,ListView交互生效
                    interactive = !activeFocus
                }

                Keys.onPressed: {
                    if (rootWindow.isWlcom) {
                        multitaskViewManager.keyNavigationDataCollect(event.key)
                    }
                    if (event.key === Qt.Key_PageUp || event.key === Qt.Key_Home) {
                        if (currentIndex === 0) {
                            return
                        }
                        currentIndex = 0
                        if (isLayoutLeftToRight && desktopListView.contentX > 0) {
                            contentMoveAnimation.to = 0
                            contentMoveAnimation.start()
                        } else if (!isLayoutLeftToRight && desktopListView.contentX + desktopListView.width < 0) {
                            contentMoveAnimation.to = - desktopListView.width
                            contentMoveAnimation.start()
                        }
                    } else if (event.key === Qt.Key_PageDown || event.key === Qt.Key_End) {
                        if (currentIndex === desktopListModel.rowCount() - 2) {
                            return
                        }
                        currentIndex = desktopListModel.rowCount() - 2
                        if (isLayoutLeftToRight && desktopListView.contentX  < desktopListView.contentWidth - desktopListView.width) {
                            contentMoveAnimation.to = desktopListView.contentWidth - desktopListView.width
                            contentMoveAnimation.start()
                        } else if (!isLayoutLeftToRight && desktopListView.contentX + desktopListView.contentWidth > 0) {
                            contentMoveAnimation.to = - desktopListView.contentWidth
                            contentMoveAnimation.start()
                        }
                    } else if (event.key === Qt.Key_Left && currentIndex === 0) {
                        // 当焦点在第一个工作区，不响应左键；在其他工作区则正常响应左键
                        return
                    }

                    if (event.key === Qt.Key_Tab || event.key === Qt.Key_Left ||
                            event.key === Qt.Key_Right || event.key === Qt.Key_Home ||
                            event.key === Qt.Key_End || event.key === Qt.Key_PageUp ||
                            event.key === Qt.Key_PageDown) {
                        multitaskViewManager.setShouldShowTabBorder(true);
                        event.accepted = false;
                    }
                }

                //这里实现了虚拟桌面listview的居中
                NumberAnimation {
                    id : desktopListViewLeftMarginAni
                    target : desktopListView
                    property : isLayoutLeftToRight ? "leftMargin" : "rightMargin"
                    duration : 300
                    easing.type : Easing.InOutQuad
                }

                populate: Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 0
                        to : 1
                        duration : 300
                        easing.type : Easing.InOutQuad
                    }
                }

                add : Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 0
                        to : 1
                        duration : 400
                        easing.type : Easing.InOutQuad
                    }
                }
                remove : Transition {
                    NumberAnimation {
                        properties : "scale,opacity"
                        from : 1
                        to : 0
                        duration : 400
                        easing.type : Easing.InOutQuad
                    }
                }
                removeDisplaced : Transition {
                    NumberAnimation {
                        properties : "x,y"
                        duration : 400
                    }
                }

                move : Transition {
                    NumberAnimation {
                        property : "x"
                        duration : 200
                        easing.type : Easing.InOutQuad
                    }
                }
                moveDisplaced : Transition {
                    NumberAnimation {
                        property : "x"
                        duration : 200
                        easing.type : Easing.InOutQuad
                    }
                }

                onCountChanged : {
                    if (initMultitask || count <= 0) {
                        if (count === desktopListModel.rowCount()) {
                            initMultitask = false;
                        }
                        return;
                    }
                    desktopListView.playCenterAlignAnimation()
                }

                Connections {
                    target: rootWindow.isWlcom ? multitaskViewManager : null
                    function onTaskViewShowed() {
                        if (desktopListView.isOverflow) {
                            desktopListView.positionViewAtBeginning()
                        }
                        desktopListView.playCenterAlignAnimation()
                        desktopListView.playPopulateAnimation()
                    }
                }

                //the quickview rootObject would not adjust width & height when hidden, have to center the ListView.
                Connections {
                    target: rootWindow
                    function onWidthChanged() {
                        desktopListView.playCenterAlignAnimation()
                    }
                }

                function playPopulateAnimation() {
                    if (!model || count <= 0) {
                        return
                    }
                    var loader
                    var delegate
                    for (var i = 0; i < count; i++) {
                        loader = desktopListView.itemAtIndex(i)
                        delegate = loader && loader.item ? loader.item : null
                        if (delegate && typeof delegate.playEnterAnimation === "function") {
                            delegate.playEnterAnimation()
                        }
                    }
                }

                function playCenterAlignAnimation() {
                    var newWidth = desktopRectWidth * desktopListView.count + desktopListView.spacing * (desktopListView.count - 1)
                    desktopListViewLeftMarginAni.to = (rootWindow.width - newWidth) / 2 < 0 ? 0  : (rootWindow.width - newWidth) / 2
                    desktopListViewLeftMarginAni.restart()
                }

                function getWorkspaceItemOrNewWorkspaceItem(type) {
                    if(type === newDesktop) {
                        return "NewDesktopButton.qml"
                    }
                    return "VirtualDesktopWindow.qml"
                }

                NumberAnimation {
                    id : moveToScrollEndAnimation
                    target : desktopListView
                    property : "contentX"
                    from : desktopListView.contentX
                    to : isLayoutLeftToRight ? desktopListView.contentWidth + desktopRectWidth + root.spacing - desktopListView.width :
                                               - desktopListView.contentWidth - desktopRectWidth - root.spacing
                    duration : 300
                    easing.type : Easing.InOutQuart
                }

                function moveToScrollEnd() {
                    if (desktopListView.contentWidth + desktopRectWidth + root.spacing > desktopListView.width
                            && desktopListView.count < 16) {
                        moveToScrollEndAnimation.restart()
                    }
                }

                Connections {
                    target: desktopListModel
                    function onDesktopAppended(desktop) {
                        desktopListView.moveToScrollEnd()
                    }
                }

                delegate: Loader {
                    source : desktopListView.getWorkspaceItemOrNewWorkspaceItem(model.ItemType)
                    onLoaded: {
                        if ("customName" in item) {
                            item.customName = model.CustomName
                        }
                    }
                    Binding {
                        target: item
                        property: "wid"
                        value: model.Uuid
                        when: "wid" in item
                    }
                    Connections {
                        target: item
                        function onCustomNameChanged() {
                            if (model.CustomName !== item.customName) {
                                desktopListModel.setData(desktopListModel.index(index, 0),
                                                         item.customName,
                                                         DesktopListModel.CustomName)
                            }
                        }
                    }
                }
            } // desktopListView

            NumberAnimation {
                id : contentMoveAnimation
                target : desktopListView
                property : "contentX"
                from : desktopListView.contentX
                duration : 300
                easing.type : Easing.InOutQuart
            }

            ArrowButton {
                id : leftButton
                visible : desktopHoverStatus && desktopListView.contentX + desktopListView.contentXoffset > 0
                x : Math.floor(48 * scaleFactor)
                y : (parent.height-leftButton.height)/2
                direction: "left"
            }

            ArrowButton {
                id : rightButton
                visible : desktopHoverStatus && desktopListView.contentX + desktopListView.contentXoffset < desktopListView.contentWidth-desktopListView.width
                x : parent.width - Math.floor(48 * scaleFactor) - rightButton.width
                y : (parent.height-rightButton.height)/2
                direction: "right"
            }
        }
    }
}
