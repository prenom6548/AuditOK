/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import AppControls2 1.0 as AppControls2
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.menu.core 1.0

MouseArea {
    id: root

    readonly property alias view: listView
    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias listFocus: listView.focus
    property int itemHeight: 40
    property string boldGroupName: ""
    property string currentGroupName: ""
    property bool appItemShowBorder: false
    property bool showBorder: false
    property int viewIndex: -1
    hoverEnabled: true
    clip: true

    function resetNavigationArgs() {
        var currentContentY = listView.contentY;
        if (listFocus) {
            viewIndex = 0;
            listView.currentIndex = 0;
            listView.positionViewAtBeginning();
        }
        //根据是否包含焦点设置相关变量值
        currentGroupName = listView.focus ? listView.currentItem.group : "";
        boldGroupName = currentGroupName;
        appItemShowBorder = boldGroupName === "";
        if (root.containsMouse) {
            listView.contentY = currentContentY;
        }
    }

    function setAppItemBorderBold() {
        appItemShowBorder = true;
        listView.currentIndex = viewIndex;
        boldGroupName = "";
        currentGroupName = listView.currentItem.group;
    }

    function setLabelItemBorderBold(event) {
        if (event.key === Qt.Key_Up) {
            boldGroupName = currentGroupName;
            currentGroupName = listView.currentItem.group;
        } else if (event.key === Qt.Key_Down) {
            currentGroupName = listView.currentItem.group;
            boldGroupName = currentGroupName;
        }
    }

    UkuiItems.DtThemeBackground {
        anchors.top: parent.top
        width: parent.width
        height: 1
        useStyleTransparency: false
        backgroundColor: Platform.GlobalTheme.kLineNormal
        visible: listView.contentY > 0
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        anchors.leftMargin: 4

        ListView {
            id: listView
            cacheBuffer: count * root.itemHeight
            spacing: 4
            Layout.fillHeight: true
            Layout.fillWidth: true
            highlightMoveDuration: 0
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: listViewScrollBar
            keyNavigationWraps: true

            // 焦点切换后，listView按键导航重新开始
            onCountChanged: resetCurrentIndex()
            function resetCurrentIndex() { currentIndex = 0 }
            Component.onCompleted: mainWindow.windowVisibleChanged.connect(resetCurrentIndex)
            Component.onDestruction: mainWindow.windowVisibleChanged.disconnect(resetCurrentIndex)

            function moveHiddenItemToTop() {
                if (listView.contentY > listView.itemAtIndex(currentIndex).y ||
                    listView.contentY + listView.height < listView.itemAtIndex(currentIndex).y) {
                    listView.positionViewAtIndex(listView.currentIndex, ListView.Beginning);
                }
            }

            Keys.onDownPressed: {
                if (root.containsMouse) return;
                if (!showBorder) {
                    showBorder = true;
                    //焦点所在元素未在可视区域内,将当前元素置顶
                    moveHiddenItemToTop();
                    return;
                }

                //未包含分组的焦点逻辑
                if (currentGroupName === "") {
                    viewIndex = viewIndex === listView.count - 1 ? 0 : viewIndex + 1;
                    currentIndex = viewIndex;
                } else {
                    //当边框在分组名上的情况
                    if (!appItemShowBorder) {
                        //边框通过向上按键移动到分组名的情况
                        if (boldGroupName !== currentGroupName) {
                            viewIndex += 1;
                        }
                        setAppItemBorderBold();
                        return;
                    }
                    //焦点移动逻辑
                    viewIndex = viewIndex === listView.count - 1 ? 0 : viewIndex + 1;
                    currentIndex = viewIndex;
                    //判断分组名是否发生变化
                    if (currentGroupName !== listView.currentItem.group) {
                        appItemShowBorder = false;
                        setLabelItemBorderBold(event);
                    }
                }
            }

            Keys.onUpPressed: {
                if (root.containsMouse) return;
                if (!showBorder) {
                    showBorder = true;
                    //焦点所在元素未在可视区域内,将当前元素置顶
                    moveHiddenItemToTop();
                    return;
                }
                //未包含分组焦点逻辑
                if (currentGroupName === "") {
                    viewIndex = viewIndex === 0 ? listView.count - 1 : viewIndex - 1;
                    currentIndex = viewIndex;
                } else {
                    if (!appItemShowBorder) {
                        //边框通过向下按键移动到分组名的情况
                        if (currentGroupName === boldGroupName) {
                            viewIndex = viewIndex === 0 ? listView.count - 1 : viewIndex - 1;
                        }
                        setAppItemBorderBold();
                        return;
                    }
                    //焦点移动逻辑
                    viewIndex -= 1;
                    if (viewIndex === -1) {
                        //当焦点所在元素索引为0时，为保证边框在组名上正常显示,将viewIndex设置为0
                        viewIndex = 0;
                        appItemShowBorder = false;
                        boldGroupName = currentGroupName;
                        return;
                    }
                    //判断分组名是否加粗
                    currentIndex = viewIndex;
                    if (currentGroupName !== listView.currentItem.group) {
                        appItemShowBorder = false;
                        setLabelItemBorderBold(event);
                    }
                }
            }

            onFocusChanged: {
                resetNavigationArgs();
                showBorder = !root.containsMouse;
            }

            Connections {
                //分组改变时，重新定位索引
                target: appListHeader.header
                function onTitleChanged() {
                    resetNavigationArgs();
                    showBorder = false;
                }
            }
        }

        AppControls2.ScrollBar {
            id: listViewScrollBar
            Accessible.role: Accessible.ScrollBar
            Accessible.name: "ukui-menu_appList_scrollbar"
            Layout.fillHeight: true
            Layout.preferredWidth: 14
            visual: root.containsMouse
        }
    }
}
