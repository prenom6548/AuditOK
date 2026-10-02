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
import QtQuick.Window 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import SqliteModels 1.0
import ModelController 1.0
import FileItemModel 1.0
import FileItemProxySortFilterModel 1.0
import SearchResultProxyModel 1.0
import QWindow 1.0
import SpacesSettings 1.0

ApplicationWindow {
    id: mainWindow
    property bool windowMode: false
    property bool previewPageVisible: spacesSettings.isPreviewMode
    property color windowBg: "transparent"
    property FileItemView currentView: modelInited && swipeView.count > 0? swipeView.currentItem: emptyView
    property int expandedMode: 3
    property int expanderHeight: 48
    property int expandDuration: 200
    property int controlSpacing: 8
    property bool modelInited: false
    signal expandedChanged()
    signal viewTypeChanged()
    signal sortTypeChanged()
    signal systemMoveRequest(point pos)
    signal optionMenuRequest(point pos)
    signal windowModeTriggered()
    signal windowClosed()
    visible: false
    width: 1096
    height: 688
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr("Intelligent Data Manager")

    flags: Qt.WA_TranslucentBackground|Qt.WA_NoSystemBackground
    color: windowBg

    FileItemView {
        id: emptyView
    }

    onClosing: {
        if (windowMode) {
            windowClosed();
        }
    }

    onWindowModeChanged: {
        if (windowMode) {
            windowModeTriggered()
        }
    }

    onExpandedChanged: {
        var i = 0;
        if (fileItemViewExpander.expanded) {
            i = i + 1;
        }
        if (recentExpander.expanded) {
            i = i + 2;
        }

        mainWindow.expandedMode = i;
        console.log("expanded change", mainWindow.expandedMode, fileItemViewExpander.expanded, recentExpander.expanded);
        switch (mainWindow.expandedMode) {
        case 0: {
            mainContentView.state = "allCollapsed";
            break;
        }
        case 1: {
            mainContentView.state = "spaceViewExpanded";
            break;
        }
        case 2: {
            mainContentView.state = "recentViewExpanded";
            break;
        }
        case 3: {
            mainContentView.state = "allExpanded";
            break;
        }
        default:
            mainContentView.state = "allExpanded";
        }
    }

    onCurrentViewChanged: {
        console.log(searchPage.currentSpace);
        previewPage.startPreviewSpace(currentView.spaceId);
    }

    ThemeBackground {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        ThemeBackground {
            id: headerBarContainer
            isTransparent: false
            Layout.fillWidth: true
            height: 56

            MouseArea {
                property bool pressed: false
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                acceptedButtons: Qt.LeftButton

                onPressed: {
                    mouse.accepted = true;
                    pressed = true;
                }

                onReleased: {
                    mouse.accepted = true;
                    console.log("mouse released");
                    pressed = false;
                }

                onPositionChanged: {
                    //console.log("mouse position changed", mouse.x, mouse.y);
                    mouse.accepted = true;
                    if (pressed) {
                        var pos = headerBarContainer.mapToGlobal(mouse.x, mouse.y);
                        systemMoveRequest(pos);
                        //mainWindow.startSystemMove();
                        pressed = false;
                    }
                }

                onDoubleClicked: {
                    pressed = false;
                    if (mainWindow.visibility != QWindow.Maximized) {
                        mainWindow.showMaximized();
                    } else {
                        mainWindow.showNormal();
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: controlSpacing

                Item {
                    visible: !windowMode
                    width: 36
                    height: 36
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    StyledButton {
                        id: dragButton
                        focusPolicy: Qt.NoFocus
                        anchors.fill: parent
                        state: "windowButton"
                        flat: true
                        iconName: "/usr/share/icons/hicolor/scalable/actions/ukui-idm-window-drag.svg"
                    }

                    MouseArea {
                        property bool pressed: false
                        property int dragStartY: 0
                        property int dragStartX: 0
                        hoverEnabled: true
                        preventStealing: true
                        anchors.fill: parent

                        onHoveredChanged: {

                        }

                        onPressed: {
                            pressed = true;
                            dragStartX = mouse.x;
                            dragStartY = mouse.y;
                        }

                        onMouseXChanged: {
                            if (pressed) {
                                if (Math.abs(dragStartY - mouse.y) > 5 || Math.abs(dragStartX - mouse.x) > 5) {
                                    mainWindow.windowMode = true;
                                    pressed = false;
                                    dragStartY = 0;
                                    dragStartX = 0;
                                    var pos = dragButton.mapToGlobal(mouse.x, mouse.y);
                                    systemMoveRequest(pos);
                                }
                            }
                        }

                        onMouseYChanged: {
                            if (pressed) {
                                if (Math.abs(dragStartY - mouse.y) > 5 || Math.abs(dragStartX - mouse.x) > 5) {
                                    mainWindow.windowMode = true;
                                    pressed = false;
                                    dragStartY = 0;
                                    dragStartX = 0;
                                    var pos = dragButton.mapToGlobal(mouse.x, mouse.y);
                                    systemMoveRequest(pos);
                                }
                            }
                        }

                        onReleased: {
                            pressed = false;
                            dragStartY = 0;
                        }
                    }
                }

                Item {
                    id: headerBody
                    visible: windowMode
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: windowMode? headerButtons.width: 200
                    Layout.fillWidth: false;

                    Image {
                        id: headerIcon
                        width: 20
                        height: 20
                        anchors.left: parent.left
                        anchors.leftMargin: controlSpacing
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: Qt.AlignVCenter
                        antialiasing: true
                        source: "qrc:/data/ukui-idm.png"
                    }

                    Text {
                        id: headerTitle
                        anchors.left: headerIcon.right
                        anchors.right: headerBody.right
                        anchors.leftMargin: controlSpacing
                        anchors.rightMargin: controlSpacing
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                        verticalAlignment: Qt.AlignVCenter
                        text: mainWindow.title
                    }
                }

                SearchEdit {
                    id: searchEdit
                    Layout.alignment: Qt.AlignHCenter
                    Layout.maximumWidth: 480
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.maximumHeight: 36

                    Component.onCompleted: {
                        searchModel.setSearchKeywordReqeust.connect(function (keyword){
                            searchEdit.setSearchKeyword(keyword);
                        });
                    }
                }

                StyledToolBar {
                    id: headerButtons
                    z: 999
                    Layout.fillHeight: true
                    Layout.rightMargin: controlSpacing
                    Layout.alignment: Qt.AlignRight

                    RowLayout {
                        height: parent.height
                        StyledButton {
                            Layout.preferredWidth: windowMode? 30: 36
                            Layout.preferredHeight: windowMode? 30: 36
                            width: windowMode? 30: 36
                            height: windowMode? 30: 36
                            flat: true
                            state: "WindowButton"
                            iconName: "ukui-preview-file-symbolic"

                            visible: mainWindow.currentView.spaceId > 0

                            onClicked: {
                                spacesSettings.isPreviewMode = !spacesSettings.isPreviewMode;
                            }
                        }

                        ToolSeparator {
                            visible: mainWindow.currentView.spaceId > 0 && windowMode
                        }

                        StyledButton {
                            visible: windowMode
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            width: 30
                            height: 30
                            flat: true
                            state: "WindowButton"
                            iconName: "open-menu-symbolic"

                            onClicked: {
                                optionMenuRequest(mapToGlobal(0, height));
                            }
                        }

                        StyledButton {
                            visible: windowMode
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            width: 30
                            height: 30
                            flat: true
                            state: "WindowButton"
                            iconName: "window-minimize-symbolic"

                            onClicked: {
                                mainWindow.showMinimized();
                            }
                        }

                        StyledButton {
                            visible: windowMode
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            width: 30
                            height: 30
                            flat: true
                            state: "WindowButton"
                            iconName: mainWindow.visibility === QWindow.Maximized? "window-restore-symbolic": "window-maximize-symbolic"

                            onClicked: {
                                if (mainWindow.visibility === QWindow.Maximized) {
                                    mainWindow.showNormal();
                                } else {
                                    mainWindow.showMaximized();
                                }
                            }
                        }

                        StyledButton {
                            visible: windowMode
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            width: 30
                            height: 30
                            iconName: "window-close-symbolic"
                            flat: true
                            state: "CloseButton"

                            onClicked: mainWindow.close();
                        }
                    }
                }
            }
        }

        SplitView {
            id: hSplitView
            Layout.fillWidth: true
            Layout.fillHeight: true
            //anchors.fill: parent

            state: spacesSettings.isPreviewMode? "previewOpened": "previewClosed";

            states: [
                State {
                    name: "previewClosed"
                    PropertyChanges {
                        target: previewPageContainer
                        width: 0
                    }
                },
                State {
                    name: "previewOpened"
                    PropertyChanges {
                        target: previewPageContainer
                        width: parent.width * 1/3
                    }
                }
            ]

            transitions: [
                Transition {
                    from: "previewClosed"
                    to: "previewOpened"
                    NumberAnimation {
                        target: previewPageContainer
                        property: "width"
                        duration: expandDuration
                        easing.type: Easing.InOutQuad
                    }
                },
                Transition {
                    from: "previewOpened"
                    to: "previewClosed"
                    NumberAnimation {
                        target: previewPageContainer
                        property: "width"
                        duration: expandDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            ]

            Item {
                Layout.fillWidth: true
                width: parent.width * 2/3

                SearchPage {
                    id: searchPage
                    visible: !(state == "hidden" && y == -parent.height)
                    width: parent.width
                    height: parent.height
                    anchors.left: parent.left
                    state: searchEdit.text.length? "shown": "hidden"

                    states: [
                        State {
                            name: "hidden"
                            PropertyChanges {
                                target: searchPage
                                y: -parent.height
                                opacity: 0
                            }
                        },
                        State {
                            name: "shown"
                            PropertyChanges {
                                target: searchPage
                                y: 0
                                opacity: 1.0
                            }
                        }
                    ]

                    transitions: [
                        Transition {
                            from: "hidden"
                            to: "shown"

                            NumberAnimation {
                                target: searchPage
                                property: "y"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }

                            NumberAnimation {
                                target: searchPage
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InQuad
                            }
                        },
                        Transition {
                            from: "shown"
                            to: "hidden"

                            NumberAnimation {
                                target: searchPage
                                property: "y"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }

                            NumberAnimation {
                                target: searchPage
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.OutQuad
                            }
                        }
                    ]
                }

                Item {
                    id: mainPage
                    visible: !(state == "hidden" && y == -parent.height)
                    width: parent.width
                    height: parent.height
                    anchors.left: parent.left
                    state: searchEdit.text.length? "hidden": "shown"

                    states: [
                        State {
                            name: "hidden"
                            PropertyChanges {
                                target: mainPage
                                y: -parent.height
                                opacity: 0
                            }
                        },
                        State {
                            name: "shown"
                            PropertyChanges {
                                target: mainPage
                                y: 0
                                opacity: 1.0
                            }
                        }
                    ]

                    transitions: [
                        Transition {
                            from: "hidden"
                            to: "shown"

                            NumberAnimation {
                                target: mainPage
                                property: "y"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }

                            NumberAnimation {
                                target: mainPage
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InQuad
                            }
                        },
                        Transition {
                            from: "shown"
                            to: "hidden"

                            NumberAnimation {
                                target: mainPage
                                property: "y"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }

                            NumberAnimation {
                                target: mainPage
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.OutQuad
                            }
                        }
                    ]

                    SplitView {
                        id: mainContentView
                        orientation: Qt.Vertical
                        anchors.left: parent.left
                        width: parent.width
                        height: parent.height
                        state: "allExpanded"

                        onStateChanged: {
                            console.log(mainContentView.state);
                        }

                        Component.onCompleted: {
                            expandedChanged();
                        }

                        states: [
                            State {
                                name: "allCollapsed"
                                PropertyChanges {
                                    target: itemViewContainer
                                    height: expanderHeight
                                }
                                PropertyChanges {
                                    target: recentViewContainer
                                    height: expanderHeight
                                }
                            },
                            State {
                                name: "spaceViewExpanded"
                                PropertyChanges {
                                    target: itemViewContainer
                                    height: parent.height - expanderHeight
                                }
                                PropertyChanges {
                                    target: recentViewContainer
                                    height: expanderHeight
                                }
                            },
                            State {
                                name: "recentViewExpanded"
                                PropertyChanges {
                                    target: itemViewContainer
                                    height: expanderHeight
                                }
                                PropertyChanges {
                                    target: recentViewContainer
                                    height: parent.height - expanderHeight
                                }
                            },
                            State {
                                name: "allExpanded"
                                PropertyChanges {
                                    target: itemViewContainer
                                    height: parent.height * 2/3
                                }
                                PropertyChanges {
                                    target: recentViewContainer
                                    height: parent.height * 1/3
                                }
                            }
                        ]

                        transitions: [
                            Transition {
                                from: "allExpanded"
                                to: "spaceViewExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "spaceViewExpanded"
                                to: "allCollapsed"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "allCollapsed"
                                to: "spaceViewExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "spaceViewExpanded"
                                to: "allExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },

                            Transition {
                                from: "allExpanded"
                                to: "recentViewExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "recentViewExpanded"
                                to: "allCollapsed"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "allCollapsed"
                                to: "recentViewExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            },
                            Transition {
                                from: "recentViewExpanded"
                                to: "allExpanded"

                                NumberAnimation {
                                    targets: [itemViewContainer, recentViewContainer]
                                    properties: "height"
                                    duration: expandDuration
                                }
                            }
                        ]

                        Item {
                            id: itemViewContainer
                            //Layout.fillHeight: true
                            height: parent.height*2/3

                            Item {
                                id: fileItemViewExpander
                                property bool expanded: !spacesSettings.isSpacesCollapsed
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: expanderHeight

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: mainWindow.controlSpacing
                                    anchors.rightMargin: mainWindow.controlSpacing
                                    spacing: mainWindow.controlSpacing
                                    Text {
                                        id: fileItemViewExpanderText
                                        verticalAlignment: Qt.AlignVCenter
                                        text: fileItemViewExpander.expanded? qsTr("Collapse Space"): qsTr("Expand Space")

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                fileItemViewExpander.expanded = !fileItemViewExpander.expanded;
                                                spacesSettings.isSpacesCollapsed = !fileItemViewExpander.expanded;
                                                console.log("item view expanded changed:", fileItemViewExpander.expanded);
                                                expandedChanged()
                                            }
                                        }
                                    }

                                    SpaceTabBar {
                                        id: spaceTabBar
                                        visible: count > 0 && !spacesSettings.isSpacesCollapsed
                                        clip: true
                                        currentIndex: 0
                                        Layout.maximumWidth: parent.width - fileItemViewExpanderText.width - createSpaceTabButton.width - sortMenuButton.width - viewTypeButton.width - 6*mainWindow.controlSpacing
                                    }

                                    Button {
                                        id: createSpaceTabButton
                                        visible: spaceTabBar.visible
                                        Layout.preferredWidth: 36
                                        Layout.preferredHeight: 36
                                        iconName: "list-add-symbolic"
                                        onClicked: {
                                            console.log("create space");
                                            modelController.createSpace();
                                        }
                                    }

                                    Rectangle {
                                        color: "transparent"
                                        Layout.fillWidth: true
                                    }

                                    Button {
                                        id: sortMenuButton
                                        visible: spaceTabBar.visible
                                        Layout.preferredWidth: 60
                                        Layout.preferredHeight: 36
                                        iconName: "view-sort-ascending-symbolic"

                                        menu: Menu {
                                            id: sortMenu

                                            property bool globalSettings: spacesSettings.useGlobalSorting
                                            ExclusiveGroup {
                                                id: sortTypeGroup
                                            }
                                            ExclusiveGroup {
                                                id: sortOrderGroup
                                            }

                                            MenuItem {
                                                text: qsTr("Most Used")
                                                exclusiveGroup: sortTypeGroup
                                                checkable: true
                                                checked: currentView.sortType == 0
                                                onTriggered: {
                                                    console.log("most used triggered", swipeView.currentIndex, sortMenu.globalSettings);
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortType(0);
                                                        spacesSettings.setUserAddedFirst(true);
                                                    } else {
                                                        mainWindow.currentView.sortRequest(0, -1, true);
                                                    }
                                                }
                                            }

                                            Menu {
                                                title: qsTr("Add Mode")
                                                MenuItem {
                                                    text: qsTr("User Added")
                                                    exclusiveGroup: sortTypeGroup
                                                    checkable: true
                                                    checked: currentView.sortType == 1 && currentView.userAddedFirst
                                                    onTriggered: {
                                                        console.log("user added triggered");
                                                        if (sortMenu.globalSettings) {
                                                            spacesSettings.setSortType(1);
                                                            spacesSettings.setUserAddedFirst(true);
                                                        } else {
                                                            mainWindow.currentView.sortRequest(0, -1, true);
                                                        }
                                                    }
                                                }
                                                MenuItem {
                                                    text: qsTr("Auto Added")
                                                    exclusiveGroup: sortTypeGroup
                                                    checkable: true
                                                    checked: currentView.sortType == 1 && !currentView.userAddedFirst
                                                    onTriggered: {
                                                        console.log("auto added triggered");
                                                        if (sortMenu.globalSettings) {
                                                            spacesSettings.setSortType(1);
                                                            spacesSettings.setUserAddedFirst(false);
                                                        } else {
                                                            mainWindow.currentView.sortRequest(0, -1, false);
                                                        }
                                                    }
                                                }
                                            }

                                            MenuItem {
                                                text: qsTr("File Name")
                                                exclusiveGroup: sortTypeGroup
                                                checkable: true
                                                checked: currentView.sortType == 2
                                                onTriggered: {
                                                    console.log("file name triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortType(2);
                                                        spacesSettings.setUserAddedFirst(true);
                                                    } else {
                                                        mainWindow.currentView.sortRequest(2, -1, true);
                                                    }
                                                }
                                            }

                                            MenuItem {
                                                text: qsTr("Date")
                                                exclusiveGroup: sortTypeGroup
                                                checkable: true
                                                checked: currentView.sortType == 3
                                                onTriggered: {
                                                    console.log("date triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortType(3);
                                                        spacesSettings.setUserAddedFirst(true);
                                                    } else {
                                                        mainWindow.currentView.sortRequest(3, -1, true);
                                                    }
                                                }
                                            }

                                            MenuItem {
                                                text: qsTr("File Type")
                                                exclusiveGroup: sortTypeGroup
                                                checkable: true
                                                checked: currentView.sortType == 4
                                                onTriggered: {
                                                    console.log("file type triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortType(4);
                                                        spacesSettings.setUserAddedFirst(true);
                                                    } else {
                                                        mainWindow.currentView.sortRequest(4, -1, true);
                                                    }
                                                }
                                            }

                                            MenuItem {
                                                text: qsTr("File Size")
                                                exclusiveGroup: sortTypeGroup
                                                checkable: true
                                                checked: currentView.sortType == 5
                                                onTriggered: {
                                                    console.log("file size triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortType(5);
                                                        spacesSettings.setUserAddedFirst(true);
                                                    } else {
                                                        mainWindow.currentView.sortRequest(5, -1, true);
                                                    }
                                                }
                                            }

                                            MenuSeparator {}

                                            MenuItem {
                                                text: qsTr("Ascending Order")
                                                exclusiveGroup: sortOrderGroup
                                                checkable: true
                                                checked: currentView.sortOrder == 0
                                                onTriggered: {
                                                    console.log("ascend triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortOrder(0);
                                                    } else {
                                                        currentView.sortRequest(-1, 0, true);
                                                    }
                                                }
                                            }

                                            MenuItem {
                                                text: qsTr("Descending Order")
                                                exclusiveGroup: sortOrderGroup
                                                checkable: true
                                                checked: currentView.sortOrder == 1
                                                onTriggered: {
                                                    console.log("descend triggered");
                                                    if (sortMenu.globalSettings) {
                                                        spacesSettings.setSortOrder(1);
                                                    } else {
                                                        currentView.sortRequest(-1, 1, true);
                                                    }
                                                }
                                            }

                                            MenuSeparator {}

                                            MenuItem {
                                                text: qsTr("Use Global Sorting")
                                                checkable: true
                                                checked: spacesSettings.useGlobalSorting
                                                onTriggered: {
                                                    console.log("global sorting triggered");
                                                    if (checked) {
                                                        spacesSettings.setSortOrder(currentView.sortOrder);
                                                        spacesSettings.setSortType(currentView.sortType);
                                                        spacesSettings.setUserAddedFirst(currentView.userAddedFirst);
                                                    }
                                                    spacesSettings.setUseGlobalSorting(checked);
                                                }
                                            }
                                        }
                                    }

                                    Button {
                                        id: viewTypeButton
                                        visible: spaceTabBar.visible
                                        Layout.preferredWidth: 36
                                        Layout.preferredHeight: 36
                                        property bool isIconMode: !spacesSettings.isSpaceListMode

                                        iconName: isIconMode? "view-grid-symbolic": "view-list-symbolic"

                                        onClicked: {
                                            isIconMode = !isIconMode;
                                            spacesSettings.isSpaceListMode = !isIconMode;
                                            viewTypeChanged();
                                            //mainWindow.currentView.switchView(isIconMode? 1: 2);
                                        }
                                    }
                                }
                            }

                            SwipeView {
                                id: swipeView
                                currentIndex: modelInited? spaceTabBar.currentIndex: spaceTabBar.count - 1
                                anchors.top: fileItemViewExpander.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom

                                onCurrentIndexChanged: {
                                    if (modelInited) {
                                        spaceTabBar.currentIndex = currentIndex;
                                    }
                                }
                            }

                            PageIndicator {
                                id: indicator
                                visible: fileItemViewExpander.expanded

                                count: swipeView.count
                                currentIndex: swipeView.currentIndex

                                anchors.bottom: swipeView.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        Item {
                            id: recentViewContainer
                            //Layout.fillHeight: true
                            height: parent.height*1/3

                            Item {
                                property bool expanded: !spacesSettings.isRecentCollapsed
                                id: recentExpander
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.leftMargin: controlSpacing
                                anchors.right: parent.right
                                height: expanderHeight

                                Text {
                                    anchors.left: parent.left
                                    width: parent.width
                                    height: expanderHeight
                                    verticalAlignment: Qt.AlignVCenter
                                    color: "black"
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: recentExpander.expanded? qsTr("Collapse Recent"): qsTr("Expand Recent")
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onPressed: {
                                        recentExpander.expanded = !recentExpander.expanded;
                                        spacesSettings.isRecentCollapsed = !recentExpander.expanded;
                                        console.log("expanded changed", recentExpander.expanded);
                                        expandedChanged()
                                    }
                                }
                            }

                            FileItemView {
                                anchors.top: recentExpander.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                id: recentFileItemView
                                visible: mainWindow.expandedMode / 2 >= 1
                                acceptDrop: false
                                Component.onCompleted: {
                                    switchView(2);
                                    modelController.recentModelCreated.connect(function (model){
                                        console.log("recent model loaded");
                                        recentFileItemView.setModel(model);
                                        model.filesLoaded.connect(function (){
                                            console.log("recent files loaded");
                                        });
                                    });

                                    modelController.startLoadRecentModel();
                                }

                                onPreviewRequest: {
                                    console.log("request preview from recent item view", path, icon);
                                    previewPage.startPreviewFile(path, icon, userAdded);
                                }

                                onOpenRequest: {
                                    console.log("request open from recent item view", path);
                                    modelController.openFile(path);
                                }

                                onMenuRequest: {
                                    console.log("recent view menu request");
                                    modelController.requestShowMenu(proxyModel, index, globalPos);
                                }
                            }
                        }
                    }
                }
            }

            Item {
                id: previewPageContainer
                //visible: previewPageVisible
                width: parent.width * 1/3
                anchors.margins: 10
                visible: !(!previewPageVisible && width == 0)
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 10
                    color: headerBarContainer.styleBackgroundColor
                    radius: 6
                    PreviewPage {
                        id: previewPage
                        anchors.fill: parent
                    }
                }
            }
        }
    }

    NoSpacePage {
        anchors.fill: parent
        z: 999
        visible: !spacesSettings.isSpacesCollapsed && (mainPage.state != "hidden" && modelInited? (swipeView.count == 0 && mainContentView.state != "recentViewExpanded"): false)
    }

    Component.onCompleted: {
        modelController.allModelCreated.connect(function(){
            modelInited = true;
            console.log("modelInited space tab bar", spaceTabBar.count);
            if (spaceTabBar.count > 0) {
                // ugly
                spaceTabBar.currentIndex = -1;
                spaceTabBar.currentIndex = 0;
            }
        });
        modelController.modelCreated.connect(function(model){
            console.log("on model created", model.getSpaceName());
            console.debug(model.getSpaceName(), model.getSpaceId());

            model.filesLoaded.connect(function (){
                console.log("files loaded");
            });
            model.startLoadFiles();
            var component = Qt.createComponent("FileItemView.qml");
            if (component.status === Component.Ready) {
                var viewInstance = component.createObject(swipeView);
                viewInstance.setModel(model);

                viewInstance.openRequest.connect(function (path){
                    console.log("request open from view", viewInstance, path);
                    modelController.openFile(path);
                });
                viewInstance.previewRequest.connect(function (path, decoration, userAdded){
                    console.log("request preview from view", viewInstance, path, decoration);
                    previewPage.startPreviewFile(path, decoration, userAdded);
                });
                viewInstance.previewSpaceRequest.connect(function (spaceId){
                    console.log("start preview space", spaceId);
                    previewPage.startPreviewSpace(spaceId);
                });
                viewInstance.dropRequest.connect(function (urls){
                    console.log("add files to space");
                    modelController.userAddFilesToSpace(urls, model);
                });
                viewInstance.sortRequest.connect(function (sortType, sortOrder, userAddedFirst){
                    console.log("sort", sortType, sortOrder, userAddedFirst);
                    model.doSort(sortType, sortOrder, userAddedFirst)
                });
                viewInstance.menuRequest.connect(function (proxyModel, index, globalPos){
                    if (viewInstance === mainWindow.currentView)
                        modelController.requestShowMenu(proxyModel, index, globalPos);
                });

                mainWindow.viewTypeChanged.connect(function (){
                    viewInstance.switchView(viewTypeButton.isIconMode? 1: 2);
                });

//                if (swipeView.index < 0) {
//                    console.log("init current index");
//                    mainWindow.currentView = viewInstance;
//                }

                viewInstance.switchView(viewTypeButton.isIconMode? 1: 2);
                swipeView.addItem(viewInstance);

                var menuItem = searchPage.spaceMenu.addItem(viewInstance.viewSpaceName);

                var tabButtonComponent = Qt.createComponent("SpaceTabButton.qml");
                console.log("tab button component status", tabButtonComponent.status, tabButtonComponent.errorString());
                if (tabButtonComponent.status === Component.Ready) {
                    var tabButton = tabButtonComponent.createObject(spaceTabBar);
                    tabButton.viewItem = viewInstance;
                    console.log("tab button created", tabButton, model.getSpaceName());
                    tabButton.text = model.getSpaceName();
                    tabButton.proxyModel = model;
                    spaceTabBar.addItem(tabButton);
                    viewInstance.tabButton = tabButton;
                    if (modelInited) {
                        var originIndex = spaceTabBar.currentIndex;
                        var targetIndex = spaceTabBar.count - 1;
                        spaceTabBar.currentIndex = targetIndex;
                        //spaceTabBar.currentIndex = originIndex;
                    }

                    tabButton.spaceRenameRequest.connect(function (spaceId){
                        console.log("space rename request", spaceId);
                        modelController.requestRenameSpace(spaceId);
                    });
                    tabButton.spaceRemoveRequest.connect(function (spaceId){
                        console.log("space remove request", spaceId);
                        swipeView.removeItem(tabButton.viewItem);
                        spaceTabBar.removeTab(tabButton);
                        searchPage.spaceMenu.removeItem(menuItem);

                        delete viewInstance;
                        delete tabButton;
                        delete menuItem;
                        //delete model;

                        modelController.removeSpace(spaceId);
                    });
                    tabButton.spaceEditRequest.connect(function (spaceId){
                        console.log("space edit request", spaceId);
                        modelController.editSpaceCondition(spaceId);
                    });

                    menuItem.triggered.connect(function(){
                        console.log(tabButton);
                        searchPage.searchInSpace = true;
                        tabButton.toggle();
                    });

                    viewInstance.viewSpaceNameChanged.connect(function(){
                        menuItem.text = viewInstance.viewSpaceName;
                        tabButton.text = viewInstance.viewSpaceName;
                    });
                }
            } else {
                console.log("failed to create component", component.status, component.errorString());
            }
        });

        modelController.startLoadModel();
    }
}
