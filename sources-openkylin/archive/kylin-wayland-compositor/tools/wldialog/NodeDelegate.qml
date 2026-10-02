// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.15

ColumnLayout {
    id: root

    property var widgetModel: null

    Layout.fillWidth: true
    spacing: 10

    ColumnLayout {
        id: contentColumn

        Layout.fillWidth: true
        spacing: 8

        Loader {
            id: mainWidgetLoader

            Layout.fillWidth: true
            Layout.minimumHeight: item ? item.implicitHeight : 0
            active: widgetModel !== undefined && widgetModel.type !== undefined
            sourceComponent: {
                switch (widgetModel.type) {
                case "CheckBox":
                    return checkBoxComp;
                case "Custom":
                    return customGroupComp;
                case "TabPage":
                    return tabPageGroupComp;
                case "tabPageContainer":
                    return pageContainerGroupComp;
                case "ListView":
                    return listViewGroupComp;
                case "GridView":
                    return gridViewGroupComp;
                default:
                    return null;
                }
            }
        }
    }

    Component {
        id: checkBoxComp

        RowLayout {
            CheckBox {
                text: widgetModel.data.text
                checked: {
                    var name = widgetModel.data.name;
                    dynamicModel.updateCheckBoxState(name, widgetModel.data.default);
                    return widgetModel.data.default;
                }
                onCheckedChanged: {
                    var name = widgetModel.data.name;
                    if (name) {
                        dynamicModel.updateCheckBoxState(name, checked);
                    }
                }
            }
        }
    }

    Component {
        id: customGroupComp

        GroupBox {
            id: groupBox

            Layout.fillWidth: true

            ColumnLayout {
                id: customLayout

                anchors.fill: parent
                spacing: 8

                Repeater {
                    model: widgetModel.child

                    delegate: Loader {
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            setSource("NodeDelegate.qml", {
                                "widgetModel": modelData
                            });
                        }
                    }
                }
            }

            background: Rectangle {
                y: groupBox.topPadding - groupBox.bottomPadding
                width: parent.width
                height: parent.height - groupBox.topPadding + groupBox.bottomPadding
                color: gsettingsManager.themeColor
                border.width: 1
                border.color: "#d7d9db"
                radius: 8
            }

            label: Label {
                visible: widgetModel.data.title
                text: widgetModel.data.title ? widgetModel.data.title : ""
                wrapMode: Text.WordWrap
                x: groupBox.leftPadding
                width: groupBox.availableWidth
            }
        }
    }

    Component {
        id: tabPageGroupComp

        ColumnLayout {
            property string tabPageName: widgetModel && widgetModel.data ? widgetModel.data.name : ""

            Layout.fillWidth: true
            spacing: 0

            TabBar {
                id: tabBar

                property var currentTabData: null

                width: parent.width
                onCurrentIndexChanged: {
                    if (widgetModel && widgetModel.child && widgetModel.child.length > currentIndex) {
                        tabBar.currentTabData = widgetModel.child[currentIndex];
                        dynamicModel.updateCurrentTabPage(tabPageName, tabBar.currentTabData.data.name);
                    } else {
                        tabBar.currentTabData = null;
                    }
                }

                Repeater {
                    model: widgetModel.child

                    delegate: TabButton {
                        text: modelData.data.title || "未命名页面"

                        background: Rectangle {
                            height: parent.height + 10
                            color: gsettingsManager.themeColor
                            radius: 8
                            border.width: parent.checked ? 2 : 1
                            border.color: parent.checked ? gsettingsManager.highLight : "#d7d9db"
                        }
                    }
                }
            }

            StackLayout {
                id: stackItem

                property int maxContentHeight: 200

                function updateMaxContentHeight(cHeight) {
                    if (cHeight > maxContentHeight)
                        maxContentHeight = cHeight;
                }

                Layout.fillWidth: true
                Layout.minimumWidth: 460
                currentIndex: tabBar.currentIndex

                Repeater {
                    model: widgetModel.child

                    delegate: Item {
                        id: pageContaint
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 332

                        Rectangle {
                            id: bgRectangle
                            anchors.fill: parent
                            color: gsettingsManager.themeColor
                            border.width: 1
                            border.color: "#d7d9db"
                        }

                        Flickable {
                            id: pageFlickable
                            anchors.fill: parent
                            anchors.margins: 8
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds

                            contentWidth: Math.max(loaderItem.implicitWidth, pageFlickable.width)
                            contentHeight: Math.max(loaderItem.implicitHeight, pageFlickable.height)

                            Item {
                                id: contentItem

                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    spacing: 8

                                    Loader {
                                        id: loaderItem
                                        Layout.fillWidth: true
                                        Layout.minimumHeight: Math.max(item ? item.implicitHeight : pageFlickable.height, stackItem.maxContentHeight)
                                        Layout.minimumWidth: Math.max(item ? item.implicitWidth : pageFlickable.width, pageFlickable.width)
                                        Component.onCompleted: {
                                            setSource("NodeDelegate.qml", {
                                                "widgetModel": modelData
                                            });
                                        }
                                        onLoaded: {
                                            if (item) {
                                                stackItem.updateMaxContentHeight(item.implicitHeight);
                                            }
                                        }
                                    }
                                }
                            }
                            ScrollBar.vertical: ScrollBar {
                                id: verticalBar

                                visible: pageFlickable.contentHeight > parent.height

                                contentItem: Rectangle {
                                    implicitWidth: 8
                                    color: "#bdbebf"
                                    opacity: 0.6
                                    radius: 4
                                }
                            }

                            ScrollBar.horizontal: ScrollBar {
                                id: horizontalBar

                                visible: pageFlickable.contentWidth > parent.width

                                contentItem: Rectangle {
                                    implicitHeight: 8
                                    color: "#bdbebf"
                                    opacity: 0.6
                                    radius: 4
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: pageContainerGroupComp

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Repeater {
                model: Array.isArray(widgetModel.child) ? widgetModel.child : []

                delegate: Loader {
                    Layout.fillWidth: true
                    Component.onCompleted: {
                        setSource("NodeDelegate.qml", {
                            "widgetModel": modelData
                        });
                    }
                }
            }
        }
    }

    function initListModel(sourceWidgetModel, targetListModel) {
        if (!sourceWidgetModel || !sourceWidgetModel.child) {
            return;
        }
        targetListModel.clear();
        for (var i = 0; i < sourceWidgetModel.child.length; i++) {
            var childItem = sourceWidgetModel.child[i];
            targetListModel.append({
                "title": String(childItem.title ? childItem.title : ""),
                "listIcon": String(childItem.icon ? childItem.icon : ""),
                "subTitle": String(childItem.subTitle ? childItem.subTitle : ""),
                "key": String(childItem.key ? childItem.key : ""),
                "textItem": String(childItem.text ? childItem.text : ""),
                "selected": childItem.selected ? childItem.selected : false
            });
        }
    }

    function multiSelectionUpdate(name, sourceListModel) {
        var selectedKeys = [];
        for (var i = 0; i < sourceListModel.count; i++) {
            if (sourceListModel.get(i).selected) {
                selectedKeys.push(sourceListModel.get(i).key);
            }
        }

        dynamicModel.updateListItems(name, selectedKeys);
    }

    function updateListModel(newDataArray, listModel) {
        if (!newDataArray.child) {
            return;
        }

        var existingMap = {};
        for (var j = 0; j < listModel.count; j++) {
            var oldItem = listModel.get(j);
            if (oldItem.key) {
                existingMap[oldItem.key] = j;
            }
        }

        for (var i = 0; i < newDataArray.child.length; i++) {
            var item = newDataArray.child[i];
            if (!item.key)
                continue;

            if (existingMap[item.key] !== undefined) {
                var index = existingMap[item.key];
                var oldData = listModel.get(index);

                listModel.set(index, {
                    "title": (item.title !== undefined) ? item.title : oldData.title,
                    "listIcon": (item.icon !== undefined) ? item.icon : oldData.listIcon,
                    "subTitle": (item.sub_title !== undefined) ? item.subTitle : oldData.subTitle,
                    "key": oldData.key,
                    "textItem": (item.text !== undefined) ? item.text : oldData.textItem,
                    "selected": (item.selected !== undefined) ? item.selected : oldData.selected
                });
                delete existingMap[item.key];
            } else {
                listModel.append({
                    "title": item.title || "",
                    "listIcon": item.icon || "",
                    "subTitle": item.subTitle || "",
                    "key": item.key,
                    "textItem": item.text || "",
                    "selected": item.selected || false
                });
            }
        }

        var indicesToRemove = Object.values(existingMap).sort((a, b) => b - a);
        for (var k = 0; k < indicesToRemove.length; k++) {
            var indexToRemove = indicesToRemove[k];
            var removedItem = listModel.get(indexToRemove);
            listModel.remove(indexToRemove);
        }
    }

    Component {
        id: listViewGroupComp

        ColumnLayout {
            id: listItemLayout

            Label {
                id: listLabel

                visible: false
            }
            TextMetrics {
                id: listMetrics

                font: listLabel.font
                text: "display"
            }
            readonly property int textRow: (widgetModel && widgetModel.data && widgetModel.data.textRow) ? widgetModel.data.textRow : 1
            property int cachedCellHeight: Math.max(48, listMetrics.height * textRow + 32)
            property int selectionMode: widgetModel.multiple ? 1 : 0   // 0: SingleSelection 1: MultiSelection

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            ListModel {
                id: listModel
            }
            Component.onCompleted: {
                initListModel(widgetModel, listModel);
            }

            Connections {
                target: nodeDataUpdate
                function onListChanged() {
                    if (nodeDataUpdate.list && nodeDataUpdate.list.data && nodeDataUpdate.list.type === "ListView" && (widgetModel.data.name.endsWith('|' + nodeDataUpdate.list.data.name) || (widgetModel.data.name === nodeDataUpdate.list.data.name))) {
                        updateListModel(nodeDataUpdate.list, listModel);
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 300
                Layout.minimumWidth: 400
                Layout.margins: 8

                ListView {
                    id: listViewItem

                    anchors.fill: parent
                    clip: true
                    focus: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: listModel
                    currentIndex: -1
                    delegate: CommonViewItem {
                        width: ListView.view.width
                        Layout.fillWidth: true
                        height: listItemLayout.cachedCellHeight
                        highlighted: ListView.isCurrentItem

                        selectionMode: listItemLayout.selectionMode

                        view: listViewItem
                        title: model.title
                        subTitle: model.subTitle
                        bodyText: model.textItem
                        iconSource: model.listIcon

                        checked: (listItemLayout.selectionMode === 1) ? model.selected : false
                        onCheckedChanged: {
                            if (listItemLayout.selectionMode === 1) {
                                listModel.setProperty(index, "selected", checked);

                                var name = widgetModel.data.name;
                                multiSelectionUpdate(name, listModel);
                            }
                        }

                        Component.onCompleted: {
                            if (listItemLayout.selectionMode === 0 && model.selected === true) {
                                listViewItem.currentIndex = index;
                            }
                        }
                    }

                    onCurrentIndexChanged: {
                        if (listItemLayout.selectionMode === 0) {
                            var name = widgetModel.data.name;
                            if (currentIndex === -1) {
                                dynamicModel.updateListItems(name, "");
                            } else {
                                dynamicModel.updateListItems(name, listModel.get(currentIndex).key);
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        visible: listViewItem.contentHeight > parent.height

                        contentItem: Rectangle {
                            implicitWidth: 6
                            color: "#bdbebf"
                            opacity: 0.6
                            radius: 4
                        }
                    }
                }
            }
        }
    }

    Component {
        id: gridViewGroupComp

        ColumnLayout {
            id: gridItemLayout

            Label {
                id: gridLabel

                visible: false
            }
            TextMetrics {
                id: gridMetrics

                font: gridLabel.font
                text: "display"
            }
            property var cellRows: widgetModel.data ? (widgetModel.data.textRow ? widgetModel.data.textRow : 2) : 2
            property int cachedCellHeight: Math.max(48, gridMetrics.height * cellRows + 32)
            property bool isSingleItem: gridModel.count === 1
            property int selectionMode: isSingleItem ? 0 : (widgetModel.multiple ? 1 : 0)   // 0: SingleSelection 1: MultiSelection

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            ListModel {
                id: gridModel
            }
            Component.onCompleted: {
                initListModel(widgetModel, gridModel);
            }

            Connections {
                target: nodeDataUpdate
                function onListChanged() {
                    console.log(widgetModel.data.name, nodeDataUpdate.list.data.name);
                    if (nodeDataUpdate.list && nodeDataUpdate.list.data && nodeDataUpdate.list.type === "GridView" && (widgetModel.data.name.endsWith('|' + nodeDataUpdate.list.data.name) || (widgetModel.data.name === nodeDataUpdate.list.data.name))) {
                        updateListModel(nodeDataUpdate.list, gridModel);
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 300
                Layout.minimumWidth: 400
                Layout.margins: 8

                GridView {
                    id: gridViewItem

                    anchors.fill: parent
                    cellWidth: gridItemLayout.isSingleItem ? gridViewItem.width : gridViewItem.width / 2
                    cellHeight: gridItemLayout.isSingleItem ? gridViewItem.height : Math.max(80, gridItemLayout.cachedCellHeight + 24)
                    model: gridModel
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    focus: true
                    currentIndex: -1
                    delegate: CommonViewItem {
                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight
                        highlighted: GridView.isCurrentItem

                        selectionMode: gridItemLayout.selectionMode
                        isSingleItem: gridItemLayout.isSingleItem

                        view: gridViewItem
                        isGridMode: true
                        title: model.title
                        subTitle: model.subTitle
                        bodyText: model.textItem
                        iconSource: model.listIcon

                        checked: (gridItemLayout.selectionMode === 1) ? model.selected : false
                        onCheckedChanged: {
                            if (gridItemLayout.selectionMode === 1) {
                                gridModel.setProperty(index, "selected", checked);

                                var name = widgetModel.data.name;
                                multiSelectionUpdate(name, gridModel);
                            }
                        }

                        Component.onCompleted: {
                            if (gridItemLayout.selectionMode === 0 && model.selected === true) {
                                gridViewItem.currentIndex = index;
                            }
                        }
                    }

                    onCurrentIndexChanged: {
                        if (gridItemLayout.selectionMode === 0) {
                            var name = widgetModel.data.name;
                            if (currentIndex === -1) {
                                dynamicModel.updateListItems(name, "");
                            } else {
                                dynamicModel.updateListItems(name, gridModel.get(currentIndex).key);
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        visible: gridViewItem.contentHeight > parent.height

                        contentItem: Rectangle {
                            implicitWidth: 6
                            color: "#bdbebf"
                            opacity: 0.6
                            radius: 4
                        }
                    }
                }
            }
        }
    }
}
