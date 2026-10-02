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
 */

import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems

GridView {
    id: folderGridView
    property bool needHide: false
    property int column: 4
    property int row: Math.ceil(count/column)

    // 应用组展开收起 相关
    property bool mouseEnable: false
    property real labelMagrins
    property real labelSpacing
    property bool isFullScreen: false
    property int maxItemNumPerPage: 16
    property int maxItemNumPerRow: 4
    property int maxRows: 4

    property real ratio: 1.0
    // 导航栏 相关
    property int pageCount: Math.ceil(count / maxItemNumPerPage)
    property int currentPageIndex: 0

    property bool isDragging: false
    property bool itemShowBorder: true

    Behavior on y {
        enabled: !baseMouseArea.isDragging
        NumberAnimation {
            duration: 220
            easing.type: Easing.Bezier
            easing.bezierCurve: [0.25, 0.1, 0.25, 1.0]
            onRunningChanged: {
                if (running && needHide) {
                    reset();
                }
            }
        }
    }
    onPageCountChanged: setCurrentPageIndex(0)
    function setCurrentPageIndex(index) {
        if (index < 0 || index > pageCount -1) {
            return;
        }
        currentPageIndex = index;
        y = -(cellHeight * maxRows * currentPageIndex); // 直接翻页效果
    }
    function normalScreenIconSignal() {
        turnPageFinished();
    }

    // 获取GridView的坐标对应的ContentItem内部坐标
    // GridView与dropArea的大小需要重合
    function getContentPoint(localX, localY) {
        return Qt.point(localX - x, localY - y);
    }

    function hideFolder() {
        needHide = true;
        // TODO：在非第一页时，先执行翻页动画，再隐藏
        if (y !== 0) {
            setCurrentPageIndex(0);
        } else {
            reset();
        }
    }

    function reset() {
        turnPageFinished();
        if (!isDragging) {
            root.active = false;
        }
        needHide = false;
    }

    function resetItemShowBorder() {
        if (!itemShowBorder) {
            itemShowBorder = true;
            return true;
        }
        return false;
    }

    cellHeight: isFullScreen ? cellWidth : (cellWidth + 12)
    cellWidth: width / maxItemNumPerRow
    interactive: false
    onCountChanged: {
        currentIndex = 0;
        if (count === 0) {
            reset();
            folderLoader.isFolderOpened = false;
        }
    }
    focus: true
    onActiveFocusChanged: currentIndex = 0
    Keys.onRightPressed: {
        if (resetItemShowBorder()) return;
        if(currentIndex === count - 1) {
            return;
        }
        currentIndex++;
    }
    Keys.onLeftPressed: {
        if (resetItemShowBorder()) return;
        if(currentIndex === 0) {
            return;
        }
        currentIndex--;
    }
    Keys.onDownPressed: {
        if (resetItemShowBorder()) return;
        if (currentIndex + column >= count) {
            return;
        }
        currentIndex = currentIndex + column;
    }
    Keys.onUpPressed: {
        if (resetItemShowBorder()) return;
        if (currentIndex - column < 0) {
            return;
        }
        currentIndex = currentIndex - column;
    }
    onCurrentIndexChanged: {
        let page = Math.floor(currentIndex / maxItemNumPerPage);
        if (page !== currentPageIndex) {
            setCurrentPageIndex(page);
        }
    }
    model: DelegateModel {
        model: folderBase.viewModel

        delegate: Item {
            id: delegateBase
            width: folderGridView.cellWidth
            height: folderGridView.cellHeight

            Accessible.role: Accessible.Button
            Accessible.name: model.name

            focus: true
            states: State {
                when: activeFocus && folderGridView.itemShowBorder
                PropertyChanges {
                    target: background
                    borderColor: Platform.GlobalTheme.kBrandFocus
                    border.width: 2
                }
            }
            Keys.onReturnPressed: {
                appManager.launchApp(model.id);
            }
            MouseArea {
                id: control
                hoverEnabled: true
                enabled: mouseEnable
                anchors.fill: parent
                anchors.margins: isFullScreen ? labelMagrins : 0
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                UkuiItems.Tooltip {
                    anchors.fill: parent
                    posFollowCursor: true
                    visible: nameText.truncated
                    mainText: nameText.text
                }

                UkuiItems.DtThemeBackground {
                    id: background
                    anchors.fill: parent
                    useStyleTransparency: false
                    backgroundColor: isDragging ? Platform.GlobalTheme.kContainGeneralAlphaNormal
                        : parent.containsPress ? Platform.GlobalTheme.kContainAlphaClick
                            : parent.containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                                : Platform.GlobalTheme.kContainGeneralAlphaNormal
                    radius: Platform.GlobalTheme.kRadiusMax
                    clip: false

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: labelMagrins
                        spacing: labelSpacing
                        Item {
                            id: iconBase
                            Layout.minimumHeight: 16 * ratio
                            Layout.minimumWidth: 16 * ratio
                            Layout.maximumWidth: 96 * ratio
                            Layout.maximumHeight: 96 * ratio
                            Layout.topMargin: 6

                            // !isFullScreen => parent.width=80, icon.width需要为48
                            Layout.preferredWidth: parent.width - (isFullScreen ? 48 * ratio : 32 * ratio)
                            Layout.preferredHeight: width
                            Layout.alignment: Qt.AlignHCenter

                            UkuiItems.Icon {
                                id: iconImage
                                width: iconBase.width
                                height: iconBase.width
                                source: icon
                                addUnifiedIconBox: true
                                anchors.centerIn: parent

                                // Drag
                                property int itemsIndex: delegateBase.DelegateModel.itemsIndex
                                Drag.dragType: Drag.None
                                Drag.proposedAction: Qt.CopyAction
                                Drag.hotSpot.x: width / 2
                                Drag.hotSpot.y: height / 2
                                // mimeData在脱拖拽开始时确定，并不再随属性变化更新
                                Drag.mimeData: {
                                    "source": "ukui-menu",
                                    "folder": "true",
                                    "url": "app://" + model.id,
                                    "text/plain": '{"dragStartIndex":' + delegateBase.DelegateModel.itemsIndex + '}'
                                }
                                Drag.onActiveChanged: {
                                    if (Drag.active) {
                                        iconImage.opacity = 0;
                                    } else {
                                        isDragging = false;
                                        iconImage.opacity = 1;
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            UkuiItems.DtThemeText {
                                id: nameText
                                width: parent.width
                                height: contentHeight
                                anchors.top: parent.top

                                text: name
                                elide: Text.ElideRight
                                textColor: Platform.GlobalTheme.textActive
                                horizontalAlignment: Text.AlignHCenter
                                opacity: !iconImage.Drag.active

                                Behavior on opacity {
                                    NumberAnimation { duration: 150 }
                                }
                            }
                        }
                    }
                }
                onClicked: {
                    console.log("ICON_CLICK!", "DESKTOPFILE:", id);
                    if (mouse.button === Qt.LeftButton) {
                        appManager.launchApp(id);
                        return;
                    }
                }

                // Drag
                pressAndHoldInterval: 300
                property bool isTouch: false
                drag.target: iconImage
                drag.onActiveChanged: {
                    if (control.isTouch) return;
                    if (drag.active) {
                        beginDrag();
                    } else {
                        isDragging = false;
                    }
                }

                onPressAndHold: {
                    if (!control.isTouch) return;
                    if (mouse.button === Qt.LeftButton) {
                        beginDrag();
                    }
                }
                onPressed: {
                    if (mouse.button === Qt.RightButton) {
                        menuManager.showMenu(id, MenuInfo.Folder);
                        return;
                    }

                    iconImage.grabToImage(function (result) {
                        iconImage.Drag.imageSource = result.url;
                    })
                    if (mouse.source === Qt.MouseEventSynthesizedBySystem ||
                        mouse.source === Qt.MouseEventSynthesizedByQt) {
                        control.isTouch = true;
                    } else {
                        control.isTouch = false;
                    }
                }

                function beginDrag() {
                    if (isFullScreen) {
                        fullScreenUI.focus = false;
                    } else {
                        normalUI.focus = false;
                    }

                    // 开始拖拽
                    isDragging = true;
                    iconImage.Drag.active = true;
                    iconImage.Drag.startDrag(Qt.CopyAction);
                }
            }

            EditModeFlag {
                isFavorited: true
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.rightMargin: 10
                onClicked: {
                    extensionData.favoriteAppsModel.removeAppFromFavorites(id);
                }
            }

        }
    }

    displaced: Transition {
        NumberAnimation {
            properties: "x,y"; duration: 300
        }
    }
}
