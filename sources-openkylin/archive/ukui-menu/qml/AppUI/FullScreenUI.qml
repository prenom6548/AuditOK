import QtQuick 2.15
import QtQml.Models 2.12
import QtQuick.Window 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import org.ukui.menu.core 1.0
import "../extensions" as Extension
import AppControls2 1.0 as AppControls2

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

FocusScope {
    id: fullScreenUI

    Component.onCompleted: {
        resetToDefaultState();
        menuStateChanged();
        mainWindow.windowVisibleChanged.connect(resetToDefaultState);
        mainWindow.fullScreenChanged.connect(menuStateChanged);
    }
    Component.onDestruction: {
        mainWindow.windowVisibleChanged.disconnect(resetToDefaultState);
        mainWindow.fullScreenChanged.disconnect(menuStateChanged);
    }

    function resetToDefaultState() {
        if (mainWindow.visible) {
            searchInputBar.text = "";
            searchInputBase.forceActiveFocus();
        }
        // 退出编辑模式
        mainWindow.editMode = false;
        folderLoader.isFolderOpened = false;
    }

    function keyPressed(event) {
        if ((0x2f < event.key && event.key < 0x3a)||(0x40 < event.key && event.key < 0x5b)) {
            focus = true;
            searchInputBase.forceActiveFocus();
        }
    }

    function menuStateChanged () {
        if (mainWindow.isFullScreen) {
            if (root.focusOnFoldButton) {
                root.focusOnFoldButton = false;
                fullScreenButton.forceActiveFocus();
            } else {
                resetToDefaultState();
            }
        }
    }

    Item {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (mainWindow.editMode) {
                    mainWindow.editMode = false;
                } else {
                    if (mainContainer.visible) {
                        //执行全屏退出操作
                        mainWindow.activeMenuWindow(false);
                    } else {
                        folderLoader.isFolderOpened = false;
                    }
                }
            }
        }

        Extension.FolderGridView {
            id: folderLoader
            anchors.fill: parent

            ratio: Screen.height / 1080
            isFullScreen: true
            folderModel: FolderModel
            Component.onCompleted: fullScreenAppList.openFolderSignal.connect(initFolder)
            Component.onDestruction: fullScreenAppList.openFolderSignal.disconnect(initFolder)

            Keys.onEscapePressed: {
                isFolderOpened = false;
            }
            onIsFolderOpenedChanged: {
                if (!isFolderOpened && folderLoader.activeFocus) {
                    fullScreenAppList.forceActiveFocus();
                    fullScreenAppList.headerItem.itemShowBorder = fullScreenAppList.headerItem.openFolderByEnter;
                    fullScreenAppList.headerItem.openFolderByEnter = false;
                }
            }
        }

        Item {
            id: mainContainer
            anchors.fill: parent
            z: 10

            state: !fullScreenAppList.isContentShow ? "contentShow" : "contentHidden"
            states: [
                State {
                    name: "contentHidden"
                    PropertyChanges { target: mainContainer; opacity: 0; scale: 0.95 }
                },
                State {
                    name: "contentShow"
                    PropertyChanges { target: mainContainer; opacity: 1; scale: 1 }
                }
            ]

            transitions: [
                Transition {
                    to:"contentHidden"
                    SequentialAnimation {
                        PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
                        ScriptAction { script: mainContainer.visible = false }
                    }
                },
                Transition {
                    to: "contentShow"
                    SequentialAnimation {
                        ScriptAction { script: mainContainer.visible = true }
                        PropertyAnimation { properties: "opacity, scale"; duration: 300; easing.type: Easing.InOutCubic }
                    }
                }
            ]

            // 两行三列
            GridLayout {
                // 搜索上下间距： b=round(48*(H/1080))
                property int searchMargin: Math.floor(48 * (Screen.height / 1080))

                anchors.fill: parent
                anchors.leftMargin: 48 * (Screen.width / 1920)
                anchors.topMargin: searchMargin
                anchors.rightMargin: 0
                anchors.bottomMargin: 4

                rowSpacing: searchMargin
                columnSpacing: 5
                rows: 2
                columns: 3

                // 搜索框: [row: 0, column: 1]
                Item {
                    id: searchInputBase
                    property bool showSearchState: searchInputBar.text !== ""
                    Layout.row: 0
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    activeFocusOnTab: true
                    KeyNavigation.tab: showSearchState ? fullScreenAppList : (appLabelPage.count > 0) ? appLabelBase : fullScreenAppList
                    KeyNavigation.backtab: sidebar.userButton
                    onFocusChanged: {
                        if (focus) searchInputBar.textInput.forceActiveFocus();
                    }

                    Accessible.role: Accessible.EditableText
                    Accessible.name: qsTr("search box")

                    UkuiItems.SearchLineEdit {
                        id: searchInputBar
                        width: 372; height: 36
                        anchors.centerIn: parent
                        enableContextMenu: true

                        Keys.onDownPressed: changeFocusToListView()
                        Keys.onReturnPressed: changeFocusToListView()

                        onTextChanged: {
                            if (text === "") {
                                AppPageBackend.group = PluginGroup.Display;
                            } else {
                                AppPageBackend.group = PluginGroup.Search;
                                AppPageBackend.startSearch(text);
                            }
                        }

                        function changeFocusToListView() {
                            if (mainWindow.isFullScreen) {
                                fullScreenUI.focus = true;
                                fullScreenAppList.focus = true;
                            }
                        }

                        function menuStateChanged () {
                            text = root.currentSearchText;
                        }

                        Component.onCompleted: {
                            menuStateChanged();
                            mainWindow.fullScreenChanged.connect(menuStateChanged);
                        }

                        Component.onDestruction: {
                            mainWindow.fullScreenChanged.disconnect(menuStateChanged);
                        }

                    }
                }

                // 全屏切换按钮: [row: 0, column: 2]
                UkuiItems.DtThemeButton {
                    id: fullScreenButton
                    visible: !isLiteMode && functionControl.showFoldButton
                    background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                        : containsMouse ? Platform.GlobalTheme.kContainAlphaHover
                            : Platform.GlobalTheme.kContainGeneralAlphaNormal
                    background.borderColor: Platform.GlobalTheme.kBrandFocus

                    Layout.row: 0
                    Layout.column: 2
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignRight
                    Layout.rightMargin: 40 * (Screen.width / 1920)

                    KeyNavigation.tab: sidebar
                    KeyNavigation.backtab: fullScreenAppList

                    Accessible.role: Accessible.Button
                    Accessible.name: mainWindow.isFullScreen ? qsTr("Contract") : qsTr("Expand")

                    UkuiItems.Tooltip {
                        anchors.fill: parent
                        mainText: mainWindow.isFullScreen ? qsTr("Contract") : qsTr("Expand")
                        posFollowCursor: true
                        margin: 6
                    }

                    onClicked: {
                        if (mainWindow.isFullScreen) {
                            root.currentSearchText = searchInputBar.text;
                        } else {
                            root.currentSearchText = appPage.search.searchInputBar.text;
                        }
                        mainWindow.isFullScreen = !mainWindow.isFullScreen;
                    }
                    Keys.onPressed: {
                        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return
                            || event.key === Qt.Key_Space) {
                            root.focusOnFoldButton = true;
                            if (mainWindow.isFullScreen) {
                                root.currentSearchText = searchInputBar.text;
                            } else {
                                root.currentSearchText = appPage.search.searchInputBar.text;
                            }
                            mainWindow.isFullScreen = !mainWindow.isFullScreen;
                        }
                    }

                    background.radius: Platform.GlobalTheme.kRadiusMin
                    icon.mode: UkuiItems.Icon.AutoHighlight
                    icon.source: mainWindow.isFullScreen ? "view-restore-symbolic" : "view-fullscreen-symbolic"
                }

                // 应用列表模式选择按钮 + 右侧按钮区域: [row: 1, column: 2] [rowspan: 2, columnSpan: 1]
                FullScreenSidebar {
                    id: sidebar
                    Layout.row: 1
                    Layout.column: 2
                    Layout.preferredWidth: 112
                    Layout.fillHeight: true
                    Layout.rightMargin: 32 * (Screen.width / 1920)

                    activeFocusOnTab: true
                    focusToAppPage: searchInputBase

                    onFocusChanged: {
                        if (focus) sidebarAreaButton.forceActiveFocus();
                    }
                }

                // 左侧标签列表: [row: 0, column: 0, rowspan: 2]
                Item {
                    id: appLabelBase
                    Layout.row: 0
                    Layout.column: 0
                    Layout.rowSpan: 2
                    Layout.preferredWidth: labelsItem.width
                    Layout.fillHeight: true
                    activeFocusOnTab: true
                    KeyNavigation.tab: fullScreenAppList
                    KeyNavigation.backtab: searchInputBase
                    onFocusChanged: {
                        if (focus && appLabelPage.count > 0) appLabelPage.forceActiveFocus();
                    }

                    Item {
                        id: labelsItem
                        anchors.centerIn: parent
                        width: 120
                        height: parent.height
                        clip: true

                        AppLabelPage {
                            id: appLabelPage
                            anchors.centerIn: parent

                            height: (contentHeight > parent.height) ? parent.height : contentHeight
                            width: parent.width
                            labelBottle: AppPageBackend.appModel.labelBottle
                            labelColum: 1
                            cellHeight: 32

                            // TODO: 潜在的优化点，尝试组合widget的model和应用model
                            model: {
                                if ((labelBottle !== null) && (labelBottle.visible)) {
                                    let labelItems = [], i = 0, item = null;
                                    let widgetInfos = fullScreenAppList.headerItem.widgetInfos;
                                    for (i = 0; i < widgetInfos.length; ++i) {
                                        item = widgetInfos[i];
                                        labelItems.push({label: item.label, type: item.type, display: item.display});
                                    }

                                    let labels = labelBottle.labels;
                                    for (i = 0; i < labels.length; ++i) {
                                        item = labels[i];
                                        labelItems.push({label: item.label, type: item.type, display: item.display});
                                    }

                                    return labelItems;
                                }

                                return [];
                            }

                            boundsBehavior: Flickable.StopAtBounds
                            interactive: height >= parent.height
                            highlightMoveDuration: 300
                            highlight: UkuiItems.DtThemeBackground {
                                width: appLabelPage.cellWidth
                                height: appLabelPage.cellHeight
                                useStyleTransparency: false
                                radius: Platform.GlobalTheme.kRadiusMin
                                backgroundColor: Platform.GlobalTheme.kContainAlphaClick
                            }

                            onLabelSelected: (label) => {
                                                 fullScreenAppList.positionLabel(label);
                                             }

                            onCurrentIndexChanged: {
                                if (model.length === 0) return;
                                let label = model[currentIndex].label;
                                if (fullScreenAppList.headerItem.widgets.includes(label)) {
                                    fullScreenAppList.currentLabelIndex = -1;

                                } else {
                                    let index = fullScreenAppList.model.findLabelIndex(label);
                                    fullScreenAppList.currentLabelIndex = index;
                                }
                            }
                        }
                    }
                }

                // 应用列表: [row: 1, column: 1]
                MouseArea {
                    Layout.row: 1
                    Layout.column: 1
                    Layout.preferredWidth: fullScreenAppList.actualAppAreaWidth + fullScreenAppList.hSpacing
                    Layout.maximumWidth: fullScreenAppList.actualAppAreaWidth + fullScreenAppList.hSpacing
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter

                    activeFocusOnTab: false
                    hoverEnabled: true

                    FullScreenAppList {
                        id: fullScreenAppList
                        anchors.fill: parent
                        ScrollBar.vertical: viewScrollBar

                        activeFocusOnTab: true
                        KeyNavigation.tab: fullScreenButton
                        KeyNavigation.backtab: searchInputBase.showSearchState ? searchInputBase
                                                                               : appLabelPage.count > 0 ? appLabelBase : searchInputBase

                        sourceModel: AppPageBackend.appModel
                        isContentShow: folderLoader.isFolderOpened
                        isSearching: searchInputBar.text !== ""
                        onContentHeightChanged: {
                            cacheBuffer = contentHeight >= 0 ? contentHeight : 0;
                        }

                        AppControls2.ScrollBar {
                            id: viewScrollBar
                            Accessible.role: Accessible.ScrollBar
                            Accessible.name: "ukui-menu_appList_scrollbar"
                            width: 14
                            height: fullScreenAppList.height
                            visible: parent.parent.containsMouse && visual
                        }

                        function positionLabel(label) {
                            // 如果是Widget那么直接滚动到最顶上
                            // console.log("=positionLabel=", label)
                            if (headerItem.widgets.includes(label)) {
                                positionViewAtBeginning();

                            } else {
                                let index = model.findLabelIndex(label);
                                if (index >= 0) {
                                    positionViewAtIndex(index, ListView.Beginning)
                                }
                            }
                        }

                        onContentYChanged: {
                            // 向下偏移200, 计算偏移后在那个item里
                            let index = indexAt(10, contentY + 200);
                            if (contentY < 0) {
                                appLabelPage.currentIndex = 0;
                            } else {
                                if (index >= 0) {
                                    appLabelPage.currentIndex = index + headerItem.widgetCount;
                                } else {
                                    index = indexAt(10, contentY + 205);
                                    if (index >= 0) {
                                        appLabelPage.currentIndex = index + headerItem.widgetCount;
                                    }
                                }
                            }
                        }

                        Component.onCompleted: {
                            Qt.callLater(()=>positionViewAtBeginning());
                            folderLoader.turnPageFinished.connect(contentShowFinished)
                        }
                        Component.onDestruction: folderLoader.turnPageFinished.disconnect(contentShowFinished)
                    }
                }
            }
        }
    }
}
