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

import QtQuick 2.0
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Layouts 1.0

import org.ukui.quick.items 1.0 as UKUIItems
import org.ukui.quick.platform 1.0 as UKUIPlatform

import SearchResultProxyModel 1.0
import QWindow 1.0

Item {
    property string keyword: searchEdit.text
    property bool searchInSpace: false
    property FileItemView lastSearchView: (mainWindow.modelInited && swipeView.count > 0)? mainWindow.currentView: emptyView
    property int controlSpacing: 8
    property QQC1.Menu spaceMenu: spaceSelectionMenu
    property string currentSpace: (mainWindow.modelInited && swipeView.count > 0)? mainWindow.currentView.viewSpaceName: ""

    onSearchInSpaceChanged: {
        searchModel.setSearchInSpacePaths(lastSearchView.proxyModel.getSpaceFilePaths());
        searchInComputerButton.checked = swipeView.count > 0? !searchInSpace: true;
        searchInSpaceButton.checked = swipeView.count > 0? searchInSpace: false;
        searchModel.searchInSpace = searchInSpace;
    }

    onLastSearchViewChanged: {
        searchModel.setSearchInSpacePaths(lastSearchView.proxyModel.getSpaceFilePaths());
        searchModel.currentSpaceId = lastSearchView.spaceId;
    }

    onKeywordChanged: {
        searchModel.searchKeyword = keyword;
    }

    onVisibleChanged: {
        searchModel.clearCondition();
    }

    Item {
        id: searchHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: controlSpacing
        height: 48

        RowLayout {
            id: searchLocationButtons
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            QQC1.Button {
                id: searchInComputerButton
                height: 36
                text: qsTr("Computer")

                checkable: true
                checked: swipeView.count > 0? !searchInSpace: true

                onClicked: {
                    searchInSpace = false;
                }
            }

            QQC1.Button {
                id: searchInSpaceButton
                height: 36
                Layout.maximumWidth: 300
                text: currentSpace
                menu: spaceSelectionMenu
                visible: swipeView.count > 0

                checkable: true
                checked: swipeView.count > 0? searchInSpace: false

                tooltip: text

                QQC1.Menu {
                    id: spaceSelectionMenu
                }
            }
        }

        Row {
            id: optionButtons
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            QQC1.Button {
                height: 36
                text: qsTr("Save as Space")

                visible: !searchInSpace

                onClicked: {
                    modelController.saveSearchResultAsSpace();
                }
            }
        }

        LinkButton {
            anchors.right: optionButtons.left
            anchors.rightMargin: controlSpacing
            anchors.verticalCenter: optionButtons.verticalCenter
            text: qsTr("Edit Condition")

            visible: !searchInSpace

            onTriggered: {
                modelController.editSearch();
            }
        }
    }

    Item {
        id: searchConditionSubLine
        anchors.top: searchHeader.bottom
        anchors.topMargin: controlSpacing
        anchors.left: parent.left
        anchors.leftMargin: controlSpacing
        anchors.right: parent.right
        anchors.rightMargin: controlSpacing
        height: 30

        Text {
            id: searchSettingsLabel
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Search Condition Settings")
        }

        LinkButton {
            anchors.left: searchSettingsLabel.right
            anchors.leftMargin: controlSpacing
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Edit Condition")

            visible: !searchInSpace

            onTriggered: {
                modelController.editSearch();
            }
        }
    }

    Item {
        anchors.top: searchConditionSubLine.bottom
        anchors.topMargin: controlSpacing
        anchors.left: parent.left
        anchors.leftMargin: controlSpacing
        anchors.right: parent.right
        anchors.rightMargin: controlSpacing
        anchors.bottom: parent.bottom
        anchors.bottomMargin: controlSpacing

        QQC1.TreeView {
            id: searchResultView
            anchors.fill: parent
            backgroundVisible: false
            itemDelegate: Item {
                Item {
                    id: treeViewIconRect
                    visible: styleData.column === 0 && styleData.depth > 0
                    anchors.left: parent.left
                    anchors.leftMargin: controlSpacing
                    anchors.verticalCenter: parent.verticalCenter
                    width: 24
                    height: 24
                    UKUIItems.ThemeIcon {
                        source: model.decoration
                        anchors.fill: parent
                    }
                }

                Text {
                    anchors.left: treeViewIconRect.visible? treeViewIconRect.right: parent.left
                    anchors.leftMargin: treeViewIconRect.visible? controlSpacing: 0
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: styleData.textColor
                    elide: styleData.elideMode
                    text: styleData.value
                }
            }

            QQC1.TableViewColumn {
                title: qsTr("File Name")
                role: "display"
                width: sizeColumn.visible? searchResultView.contentItem.width - 236 * 2: searchResultView.contentItem.width
            }
            QQC1.TableViewColumn {
                id: sizeColumn
                title: qsTr("File Size")
                role: "displaySize"
                width: 236
                visible: !previewPageVisible || mainWindow.visibility === QWindow.Maximized
            }
            QQC1.TableViewColumn {
                title: qsTr("Time Info")
                role: "displayTime"
                width: 236
                visible: !previewPageVisible || mainWindow.visibility === QWindow.Maximized
            }

            model: searchModel

            onClicked: {
                var icon = model.data(currentIndex, Qt.DecorationRole);
                var path = model.data(currentIndex, Qt.UserRole + 1);
                console.log("tree view clicked", path);
                previewPage.startPreviewFile(path, icon, false);
            }

            onDoubleClicked: {
                var path = model.data(currentIndex, Qt.UserRole + 1);
                modelController.openFile(path);
            }

            Component.onCompleted: {
                searchModel.modelReset.connect(function (){
                    searchResultView.expand(model.index(0, 0));
                    searchResultView.expand(model.index(1, 0));
                });
            }
        }
    }

    Text {
        text: qsTr("Loading...")
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: false
    }
}
