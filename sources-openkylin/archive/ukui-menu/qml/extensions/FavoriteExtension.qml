/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.menu.extension 1.0
import AppControls2 1.0 as AppControls2
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import AppUI 1.0 as AppUI

UkuiMenuExtension {
    MouseArea {
        id: viewMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            if (mouse.button === Qt.LeftButton) {
                if (mainWindow.editMode) {
                    mainWindow.editMode = false;
                } else {
                    folderLoader.isFolderOpened = false;
                }
            }
        }

        onPressed: {
            if (mouse.button === Qt.RightButton) {
                normalUI.focus = false;
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
        UkuiItems.DtThemeBackground {
            anchors.top: parent.top
            width: parent.width; height: 1
            useStyleTransparency: false
            backgroundColor: Platform.GlobalTheme.kContainAlphaClick
            visible: favoriteView.contentY > 0
            z: 1
        }

        // 无收藏应用时 字体提示
        UkuiItems.DtThemeText {
            width: implicitWidth
            height: implicitHeight
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 3 //字体上下空白比例为1：2

            visible:  favoriteView.viewModel.count === 0
            textColor: Platform.GlobalTheme.kFontSecondary
            font.weight: Font.Normal

            text: qsTr("No apps in favorites")
        }

        FolderGridView {
            id: folderLoader
            anchors.fill: parent
            z: 10

            isFullScreen: false
            folderModel: extensionData.folderModel
            onActiveFocusChanged: {
                if (!activeFocus && !favoriteView.activeFocus) {
                    favoriteView.folderOpenByReturn = false;
                }
            }

            Component.onCompleted: {
                favoriteView.openFolderSignal.connect(initFolder);
                mainWindow.windowVisibleChanged.connect(resetFolder);
                mainWindow.fullScreenChanged.connect(resetFolder);
            }
            Component.onDestruction: {
                favoriteView.openFolderSignal.disconnect(initFolder);
                mainWindow.windowVisibleChanged.disconnect(resetFolder);
                mainWindow.fullScreenChanged.disconnect(resetFolder);
            }

            function resetFolder() {
                folderLoader.isFolderOpened = false;
            }
            Keys.onEscapePressed: {
                resetFolder();
            }
            onIsFolderOpenedChanged: {
                if (!isFolderOpened && folderLoader.activeFocus) {
                    favoriteView.forceActiveFocus();
                    favoriteView.itemShowBorder = favoriteView.folderOpenByReturn;
                    favoriteView.folderOpenByReturn = false;
                }
            }
        }

        Item {
            anchors.fill: parent
            anchors.bottomMargin: 8
            // 设计稿原左右边距为16，由于favoriteView为GridView，delegate宽度需要加上2的margin才能满足spacing为4，所以代码中leftMargin、rightMargin为16-2=14
            // 2026.02.07: 根据最新设计稿改动，收藏区左右边距调整为24，leftMargin、rightMargin为24-2=22
            // https://mastergo.com/file/174791660183880?fileOpenFrom=shared&page_id=326%3A09307&layer_id=590%3A54741%2F345%3A02735%2F345%3A02105
            anchors.leftMargin: 22
            anchors.rightMargin: 22
            anchors.topMargin: 8

            // 拖动到文件到空白区域 添加到收藏
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
                            id = extensionData.favoriteAppsModel.getDesktopFile(drag.getDataAsString("url"));
                        }
                        if (addedId === id || id === "") {
                            return;
                        }

                        addedId = id
                        addedIndex = favoriteView.count;
                        favoriteView.viewModel.items.insert(addedIndex, {"id":"", "icon":"", "name": ""});
                    } else if (drag.getDataAsString("favorite") === "true") {
                        drag.accepted = false;

                    } else {
                        // 收藏插件内拖拽
                        favoriteView.dragTypeIsMerge = false;
                    }
                }

                onPositionChanged: {
                    if ((drag.getDataAsString("folder") === "true" || drag.getDataAsString("favorite") === "false") && addedIndex > -1) {
                        var dragIndex = favoriteView.indexAt(drag.x + favoriteView.contentX, drag.y + favoriteView.contentY);

                        if (dragIndex < 0) {
                            return;
                        }

                        favoriteView.viewModel.items.move(addedIndex, dragIndex);
                        addedIndex = dragIndex;
                    }
                }

                onDropped: {
                    // folder需要在favorite之前判断，因为应用组内的应用favorite都为true
                    if (drop.getDataAsString("folder") === "true") {
                        reset();
                        extensionData.favoriteAppsModel.addAppFromFolder(drop.getDataAsString("url").slice(6), addedIndex);
                        folderLoader.isFolderOpened = false;

                    } else if (drop.getDataAsString("favorite") === "false") {
                        reset();
                        extensionData.favoriteAppsModel.addAppToFavorites(drop.getDataAsString("url").slice(6), addedIndex);
                    }
                }

                onExited: reset()
            }

            FavoriteGridView {
                id: favoriteView
                anchors.horizontalCenter: parent.horizontalCenter
                parentWidth: parent.width
                height: (contentHeight > parent.height) ? parent.height : contentHeight
                interactive: contentHeight > parent.height
                isContentShow: !folderLoader.isFolderOpened

                onActiveFocusChanged: {
                    if (activeFocus) {
                        favoriteView.itemShowBorder = true;
                    }
                }

                Component.onCompleted: {
                    favoriteView.viewModel.model = extensionData.favoriteAppsModel
                    folderLoader.turnPageFinished.connect(contentShowFinished)
                }
                Component.onDestruction: {
                    folderLoader.turnPageFinished.disconnect(contentShowFinished);
                }
                ScrollBar.vertical: scrollBar
            }
        }
        AppControls2.ScrollBar {
            id: scrollBar
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            visible: viewMouseArea.containsMouse && favoriteView.interactive && !folderLoader.isFolderOpened
            width: 14
            height: favoriteView.height
        }
    }
}
