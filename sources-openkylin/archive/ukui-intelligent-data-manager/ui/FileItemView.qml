/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4 as QQC1

import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import FileItemModel 1.0
import FileItemProxySortFilterModel 1.0
import SpacesSettings 1.0
import QWindow 1.0

FocusScope {
    id: fileItemView
    property int spaceId: -1
    property FileItemProxySortFilterModel proxyModel: FileItemProxySortFilterModel{}
    property var tabButton: TabButton{}
    property int contentWidth: width
    property color bgColor: "transparent"
    property bool acceptDrop: true
    property bool collapse: mainWindow.visibility == QWindow.Maximized? false: previewPage.visible
    property string viewSpaceName: proxyModel.spaceName
    property int sortType: spacesSettings.useGlobalSorting? spacesSettings.sortType: selfSortType
    property int selfSortType: proxyModel.sortType
    property int sortOrder: spacesSettings.useGlobalSorting? spacesSettings.sortOrder: selfSortOrder
    property int selfSortOrder: proxyModel.sortOrder
    property bool userAddedFirst: spacesSettings.useGlobalSorting? spacesSettings.userAddedFirst: selfUserAddedFirst
    property bool selfUserAddedFirst: proxyModel.userAddedFirst
    signal previewRequest(string path, var icon, bool userAdded)
    signal previewSpaceRequest(int spaceId)
    signal openRequest(string path)
    signal menuRequest(FileItemProxySortFilterModel model, int index, var globalPos)
    signal dropRequest(var urls)
    signal sortRequest(int sortType, int sortOrder, bool userAddedFirst)

    focus: true

    Rectangle {
        anchors.fill: parent
        color: bgColor
    }

    GridView {
        z: 5
        id: gridView
        anchors.fill: parent
        topMargin: 15
        leftMargin: 15
        contentWidth: parent.contentWidth
        visible: true
        focus: visible
        clip: true
        cellWidth: 134;
        cellHeight: 141;

        delegate: ItemDelegate {
            z: 10
            highlighted: GridView.isCurrentItem
            width: 104
            height: 133
            focus: visible

            Rectangle {
                id: gridViewIconRect
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                width: 64
                height: 64
                color:  "transparent"
                ThemeIcon {
                    source: model.decoration
                    anchors.fill: parent
                }
                ThemeIcon {
                    source: "emblem-personal-symbolic"
                    width: 20
                    height: 20
                    anchors.horizontalCenter: parent.right
                    anchors.verticalCenter: parent.bottom
                    visible: model.isUserAdded
                }
            }

            Canvas {
                id: gridLabelColors
                property var colors: model.labelColors
                anchors.top: gridViewIconRect.bottom
                anchors.left: gridViewIconRect.left
                anchors.right: gridViewIconRect.right
                height: controlSpacing
                visible: colors.length > 0

                onPaint: {
                    var offset = 0;
                    var ctx = getContext("2d");
                    ctx.reset(); // 清除之前的绘制内容
                    for (var i = 0; i < colors.length; i++) {
                        console.log(model.path, colors[i]);
                        ctx.beginPath();
                        ctx.arc(offset + height/2, height/2, height/2, 0, Math.PI * 2);
                        ctx.closePath();

                        ctx.fillStyle = colors[i];
                        ctx.fill();

                        offset += height/2;
                    }
                }
            }

            Text {
                anchors.top: gridViewIconRect.bottom
                anchors.topMargin: 8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 4
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 8
                text: model.display
                wrapMode: Text.WrapAnywhere
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
            }

            Connections {
                target: model
                onLabelColorsChanged: {
                    gridLabelColors.requestPaint();
                }
            }

            onClicked: {
                gridView.currentIndex = index;
                console.log("on clicked", model.path, model.decoration, model.isUserAdded);
                previewRequest(model.path, model.decoration, model.isUserAdded);
                locationText.text = model.parentPath;
                this.forceActiveFocus();
            }

            onDoubleClicked: {
                gridView.currentIndex = index;
                console.log("on double clicked", model.path);
                openRequest(model.path);
            }

            MouseArea {
                id: gridViewDelegateMouseArea
                z: 8
                anchors.fill: parent
                acceptedButtons: Qt.RightButton

                onClicked: {
                    mouse.accepted = true;
                    var pos = gridViewDelegateMouseArea.mapToItem(gridView, mouse.x, mouse.y);
                    var index = gridView.indexAt(pos.x, pos.y + gridView.contentY);
                    gridView.currentIndex = index;
                    tableView.currentIndex = index;
                    if (mouse.button == Qt.RightButton) {
                        console.log(gridView.model, index, gridViewDelegateMouseArea.mapToGlobal(mouse.x, mouse.y));
                        modelController.requestShowMenu(gridView.model, index, gridViewDelegateMouseArea.mapToGlobal(mouse.x, mouse.y));
                    }
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            minimumSize: 0.3
        }
    }

    ListView {
        z: 5
        id: tableView
        contentWidth: parent.contentWidth
        leftMargin: 24
        rightMargin: 24
        focus: visible

        // 不允许横向滚动
        //contentWidth: parent.width
        //anchors.fill: parent
        //model: sqlitemodel
        anchors.fill: parent
        clip: true

        delegate: ItemDelegate {
            z: 10
            width: tableView.contentWidth - tableView.leftMargin - tableView.rightMargin
            height: 36
            highlighted: ListView.isCurrentItem
            focus: visible
            Rectangle {
                anchors.fill: parent
                color: highlighted? Theme.alternateBase(Theme.Active): "transparent"
                radius: 6
            }

            Canvas {
                id: listLabelColors
                property var colors: model.labelColors
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: controlSpacing
                height: controlSpacing
                width: controlSpacing/2 + colors.length * controlSpacing/2
                visible: colors.length > 0

                onPaint: {
                    var offset = 0;
                    var ctx = getContext("2d");
                    ctx.reset(); // 清除之前的绘制内容
                    for (var i = 0; i < colors.length; i++) {
                        console.log(model.path, colors[i]);
                        ctx.beginPath();
                        ctx.arc(offset + height/2, height/2, height/2, 0, Math.PI * 2);
                        ctx.closePath();

                        ctx.fillStyle = colors[i];
                        ctx.fill();

                        offset += height/2;
                    }
                }
            }

            Item {
                id: listViewIconRect
                anchors.left: listLabelColors.right
                anchors.verticalCenter: parent.verticalCenter
                width: 24
                height: 24
                ThemeIcon {
                    source: model.decoration
                    anchors.fill: parent
                }
                ThemeIcon {
                    source: "emblem-personal-symbolic"
                    width: 16
                    height: 16
                    anchors.horizontalCenter: parent.right
                    anchors.verticalCenter: parent.bottom
                    visible: model.isUserAdded
                }
            }

            Text {
                anchors.left: listViewIconRect.right
                anchors.leftMargin: 4
                anchors.right: collapse? openButton.left: tableViewSizeText.left
                anchors.rightMargin: controlSpacing
                height: parent.height
                text: model.display
                elide: Text.ElideMiddle
                verticalAlignment: Qt.AlignVCenter
            }

            Button {
                id: openButton
                visible: collapse
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    anchors.fill: parent
                    id: openButtonText
                    text: qsTr("Open")
                }
                width: 24
                height: 24
                onClicked: {
                    modelController.openFile(model.path);
                }
            }

            Text {
                id: tableViewPathText
                visible: !collapse
                anchors.right: parent.right
                anchors.rightMargin: 8
                width: collapse? 0: 160
                height: parent.height
                text: model.parentPath
                elide: Text.ElideMiddle
                color: "#7C7C7C"
                verticalAlignment: Qt.AlignVCenter
            }

            Text {
                id: tableViewDateText
                visible: spaceId < 0? !collapse: mainWindow.visibility == QWindow.Maximized
                anchors.right: tableViewPathText.left
                anchors.rightMargin: controlSpacing
                width: implicitWidth
                height: parent.height
                text: model.displayTime
                color: "#7C7C7C"
                verticalAlignment: Qt.AlignVCenter
            }

            Text {
                id: tableViewSizeText
                visible: !collapse
                anchors.right: tableViewDateText.visible? tableViewDateText.left: tableViewPathText.left
                anchors.rightMargin: controlSpacing
                width: implicitWidth
                height: parent.height
                text: model.displaySize
                color: "#7C7C7C"
                verticalAlignment: Qt.AlignVCenter
            }

            Connections {
                target: model
                onLabelColorsChanged: {
                    listLabelColors.requestPaint();
                }
            }

            onClicked: {
                tableView.currentIndex = index;
                console.log("on clicked", model.path, model.decoration, model.isUserAdded);
                previewRequest(model.path, model.decoration, model.isUserAdded);
                locationText.text = model.parentPath;
                this.forceActiveFocus();
            }

            onDoubleClicked: {
                tableView.currentIndex = index;
                console.log("on double clicked", model.path);
                openRequest(model.path);
            }

            MouseArea {
                id: tableViewDelegateMouseArea
                z: 8
                anchors.fill: parent
                acceptedButtons: Qt.RightButton

                onClicked: {
                    mouse.accepted = true;
                    var pos = tableViewDelegateMouseArea.mapToItem(tableView, mouse.x, mouse.y);
                    var index = tableView.indexAt(pos.x, pos.y + tableView.contentY);
                    console.log(pos, index);
                    gridView.currentIndex = index;
                    tableView.currentIndex = index;
                    if (mouse.button == Qt.RightButton) {
                        console.log(tableView.model, index, tableViewDelegateMouseArea.mapToGlobal(mouse.x, mouse.y));
                        modelController.requestShowMenu(tableView.model, index, tableViewDelegateMouseArea.mapToGlobal(mouse.x, mouse.y));
                    }
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            minimumSize: 0.3
        }

        visible: false
    }

    EmptyView {
        z: 10
        anchors.fill: parent;
        visible: !spacesSettings.isSpacesCollapsed && (spaceId >= 0? (gridView.count == 0? true: false): false)
    }

    DisabledText {
        id: statusText
        z: 999
        anchors.bottom: parent.bottom
        anchors.bottomMargin: controlSpacing
        anchors.right: parent.right
        anchors.rightMargin: controlSpacing * 2
        visible: !locationText.visible

        onVisibleChanged: {
            console.log("on status text visible changed");
            text = proxyModel.getSpaceStatusDescription();
        }
    }

    DisabledText {
        id: locationText
        z: 999
        anchors.bottom: parent.bottom
        anchors.bottomMargin: controlSpacing
        anchors.right: parent.right
        anchors.rightMargin: controlSpacing * 2
        width: parent.width*2/3
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideLeft
        visible: (gridView.currentIndex >= 0 || tableView.currentIndex >= 0) && spaceId >= 0
    }

    MouseArea {
        anchors.fill: parent
        id: itemViewBlankMouseArea
        z: 2
        acceptedButtons: Qt.RightButton
        onClicked: {
            mouse.accepted = true;
            gridView.currentIndex = -1;
            tableView.currentIndex = -1;
            if (gridView.visible) {
                gridView.forceActiveFocus();
            } else {
                tableView.forceActiveFocus();
            }

            if (spaceId < 0)
                return;

            if (mouse.button == Qt.LeftButton) {
                previewPage.startPreviewSpace(spaceId);
            } else {
                console.log(proxyModel, mouse.x, mouse.y);
                modelController.requestShowMenu(proxyModel, -1, itemViewBlankMouseArea.mapToGlobal(mouse.x, mouse.y));
            }
        }
    }

    DropArea {
        anchors.fill: parent
        visible: spaceId > 0
        onEntered: {
            if (!acceptDrop) {
                drag.accepted = false;
            }
        }

        onDropped: {
            if (drop.hasUrls) {
                dropRequest(drop.urls);
            }
        }
    }

    function setModel(model) {
        console.log("set model");
        proxyModel = model;
        spaceId = model.getSpaceId();
        gridView.model = model;
        tableView.model = model;

        model.spacesStatusDescriptionChanged.connect(function(){
            statusText.text = model.getSpaceStatusDescription();
        });
    }

    function switchView(mode) {
        if (mode === 1) {
            console.log("switch to icon view");
            gridView.visible = true;
            tableView.visible = false;
            return;
        } else if (mode === 2) {
            console.log("switch to list view");
            gridView.visible = false;
            tableView.visible = true;
            return;
        }

        console.log("switch view");
        gridView.visible = !gridView.visible;
        tableView.visible = !tableView.visible;
    }

    function getSpaceName() {
        return proxyModel.getSpaceName();
    }
}
