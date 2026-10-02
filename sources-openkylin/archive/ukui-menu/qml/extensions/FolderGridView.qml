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

import QtQuick 2.0
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.ukui.menu.core 1.0
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.quick.items 1.0 as UkuiItems
import AppUI 1.0 as AppUI

Loader {
    id: root
    active: false
    property var folderModel
    property string folderName: ""
    property int folderX: 0
    property int folderY: 0
    property int viewMaxRow: 0
    property bool isFolderOpened: false

    property real ratio: 1
    property bool isFullScreen: true
    property int margins: isFullScreen? 20 * ratio : 0
    property int animationDuration: 150
    signal turnPageFinished()
    focus: isFolderOpened

    onActiveFocusChanged: {
        if (activeFocus) {
            item.viewItemShowBorder = true;
        }
    }

    DropArea {
        id: folderDropBase
        anchors.fill: parent
        enabled: isFolderOpened
        onDropped: {
            // 在应用组打开时，快速拖拽到外部 - 将应用从组内移除,并放到最后一位
            if (drop.getDataAsString("folder") === "true" && isFolderOpened) {
                favoriteModel.addAppFromFolder(drop.getDataAsString("url").slice(6), favoriteModel.rowCount());
                isFolderOpened = false;
            }
        }
    }
    FolderParam {
        id: folderParam
        ratio: root.ratio
        isFullScreen: root.isFullScreen
        viewRow: viewMaxRow
    }
    function initFolder(id, name, x, y) {
        folderModel.setFolderId(id);
        folderName = name;
        folderX = x;
        folderY = y;
        viewMaxRow = Math.ceil(folderModel.count / 4) > 4 ? 4 : Math.ceil(folderModel.count / 4);
        active = true;
        isFolderOpened = true;
        item.viewModel = folderModel; // 内部Gridview的model，在每次显示时需要重新赋值
        if (!root.activeFocus) {
            forceActiveFocus();
            item.viewItemShowBorder = false;
        }
    }

    sourceComponent: folderComponent

    Component {
        id: folderComponent
        Item {
            id: folderBase
            property var viewModel // 内部Gridview的model
            property alias viewItemShowBorder: content.itemShowBorder

            DropArea {
                id: dropArea
                width: folderBackground.width
                height: folderBackground.height + folderParam.itemIconSize // 图标完全脱离边框 才执行exit操作
                anchors.centerIn: folderIconBase
                enabled: isFolderOpened

                property int edgeThreshold: 20* ratio  + folderParam.itemIconSize // 边缘阈值, 拖拽提示条高度：20 + 图标边缘距离hotSpot距离：48
                property int edgeStatus: 0  // 当前边缘状态 0:无 1:左边缘 2:右边缘
                property bool isExit: false //完全脱离
                property int movedTarget: -1
                property int draggedIndex: -1

                // 边缘检测定时器
                Timer {
                    id: edgeTimer
                    interval: dropArea.isExit ? 1000 : 300
                    onTriggered: {
                        if (content.isDragging) {
                            if (dropArea.isExit) {
                                dropArea.handleExitAction();
                            } else {
                                dropArea.handleEdgeAction();
                            }
                        }
                    }
                }

                function handleEdgeAction() {
                    switch (edgeStatus) {
                        case 1: // 左边缘，向前翻页
                            content.setCurrentPageIndex(content.currentPageIndex - 1);
                            break
                        case 2: // 右边缘,向后翻页
                            content.setCurrentPageIndex(content.currentPageIndex + 1);
                            break
                    }

                    // 重置状态
                    edgeStatus = 0;
                }
                function handleExitAction() {
                    isFolderOpened = false;
                }
                // point 是否在item的中点前
                function beforeMidpoint(item, point) {
                    // 获取一个Item的中点坐标
                    let midPoint = Qt.point(item.x + item.width/2, item.y + item.height/2);
                    return point.x < midPoint.x;
                }

                onEntered: {
                    dropArea.isExit = false;
                    edgeTimer.stop();
                    const obj = JSON.parse(drag.text);
                    if (obj !== null) {
                        draggedIndex = obj.dragStartIndex;
                    }
                }
                onPositionChanged: (dragEvent) => {
                    // 多页情况下，处于边缘，左右跨页移动
                    if (content.pageCount > 1) {
                        const newStatus = (dragEvent.y < edgeThreshold) ? 1 : (dragEvent.y > height - edgeThreshold) ? 2 : 0;
                        if (newStatus !== edgeStatus) {
                            edgeTimer.stop();
                            edgeStatus = newStatus;
                            if (newStatus !== 0) {
                                edgeTimer.restart();
                            }
                        }
                    }

                    // 正常拖拽换位
                    let sourceIndex = dragEvent.source.itemsIndex;
                    let currentPoint = content.getContentPoint(dragEvent.x, dragEvent.y);
                    let targetItem = content.itemAt(currentPoint.x, currentPoint.y);
                    if (targetItem !== null) {
                        let targetIndex = targetItem.DelegateModel.itemsIndex;
                        movedTarget = targetIndex;
                        content.model.items.move(sourceIndex, targetIndex);
                    }
                }

                onExited: {
                    edgeTimer.stop();
                    edgeStatus = 0;

                    dropArea.isExit = true;
                    edgeTimer.restart();

                    // 拖出应用组范围，之前的移动全都回位
                    if (movedTarget > -1 && draggedIndex > -1) {
                        content.model.items.move(movedTarget, draggedIndex);
                        movedTarget = -1;
                        draggedIndex = -1;
                    }
                }

                onDropped: (dragEvent) =>  {
                    content.isDragging = false;
                    edgeTimer.stop();
                    edgeStatus = 0;

                    let currentPoint = content.getContentPoint(dragEvent.x, dragEvent.y);
                    let targetItem = content.itemAt(currentPoint.x, currentPoint.y);
                    const obj = JSON.parse(dragEvent.text);
                    if (targetItem !== null && obj !== null) {
                        folderModel.exchangedOrder(obj.dragStartIndex, targetItem.DelegateModel.itemsIndex);
                    } else {
                        folderModel.exchangedOrder(obj.dragStartIndex, folderModel.count - 1);
                    }
                }
            }
            // 展开应用组底色
            UkuiItems.DtThemeBackground {
                id: folderBackground
                width: folderParam.backgroundWidth
                height: folderParam.backgroundHeight
                anchors.centerIn: folderIconBase

                backgroundColor: Platform.GlobalTheme.kContainAlphaClick
                useStyleTransparency: false
                radius: Platform.GlobalTheme.kRadiusMax

                UkuiItems.DtThemeBackground {
                    id: topDragHintBar
                    width: folderBackground.width
                    height: folderParam.dragHintBarHeight
                    anchors.top: folderBackground.top
                    visible: content.isDragging && (content.currentPageIndex > 0) && isFolderOpened

                    backgroundColor: Platform.GlobalTheme.kFontWhiteSecondaryDisable
                    useStyleTransparency: false

                    corners.topLeftRadius: folderBackground.radius
                    corners.topRightRadius: folderBackground.radius
                    corners.bottomLeftRadius: 0
                    corners.bottomRightRadius: 0
                    UkuiItems.Icon {
                        width: folderParam.dragHintBarIconSize; height: width
                        anchors.centerIn: parent
                        source: "pan-up-symbolic"
                    }
                }

                UkuiItems.DtThemeBackground {
                    id: bottomDragHintBar
                    width: folderBackground.width
                    height: folderParam.dragHintBarHeight
                    anchors.bottom: folderBackground.bottom
                    visible: content.isDragging && content.currentPageIndex < (content.pageCount - 1) && isFolderOpened

                    backgroundColor: Platform.GlobalTheme.kFontWhiteSecondaryDisable
                    useStyleTransparency: false

                    corners.topLeftRadius: 0
                    corners.topRightRadius: 0
                    corners.bottomLeftRadius: folderBackground.radius
                    corners.bottomRightRadius: folderBackground.radius
                    UkuiItems.Icon {
                        width: folderParam.dragHintBarIconSize; height: width
                        anchors.centerIn: parent
                        source: "pan-down-symbolic"
                    }
                }
            }
            Item {
                id: folderIconBase
                clip: true

                state: isFolderOpened ? "folderOpened" : "folderHidden"
                states: [
                    State {
                        name: "folderOpened"
                        PropertyChanges {
                            target: folderIconBase
                            width: folderParam.folderIconBase.width
                            height: folderParam.folderIconBase.height
                            x: (parent.width - width) / 2
                            y: isFullScreen ? (parent.height - height) / 2 : folderParam.folderIconBase.y
                            opacity: 1
                        }
                        PropertyChanges {
                            target: folderBackground
                            width: folderParam.backgroundWidth
                            height: folderParam.backgroundHeight
                        }
                        PropertyChanges {
                            target: content
                            labelSpacing: folderParam.folderLabelSpacing
                            labelMagrins: folderParam.folderLabelMagrins
                        }
                        PropertyChanges {
                            target: contentBase
                            contentMargins: folderParam.folderContentMargins
                        }
                        PropertyChanges {
                            target: folderNameText; opacity: 1
                        }
                    },
                    State {
                        name: "folderHidden"
                        PropertyChanges {
                            target: folderIconBase
                            width: folderParam.normalIconBase.width
                            height: folderParam.normalIconBase.height
                            x: root.mapFromGlobal(folderX, 0).x
                            y: root.mapFromGlobal(0, folderY).y
                            opacity: 0
                        }
                        PropertyChanges {
                            target: folderBackground
                            width: folderParam.normalIconBase.width
                            height: folderParam.normalIconBase.height
                        }
                        PropertyChanges {
                            target: content
                            labelSpacing: folderParam.iconLabelSpacing
                            labelMagrins: folderParam.iconLabelMagrins
                        }
                        PropertyChanges {
                            target: contentBase
                            contentMargins: folderParam.iconContentMargins
                        }
                        PropertyChanges {
                            target: folderNameText; opacity: 0
                        }
                    }
                ]
                transitions: [
                    Transition {
                        to: "folderHidden"
                        SequentialAnimation {
                            ScriptAction {
                                script: content.mouseEnable = false
                            }
                            ParallelAnimation {
                                PropertyAnimation {
                                    target: folderIconBase
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "x, y, width, height, opacity"
                                }
                                PropertyAnimation {
                                    target: folderBackground
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "width, height"
                                }
                                PropertyAnimation {
                                    target: folderNameText
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "opacity"
                                }
                                PropertyAnimation {
                                    target: content
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "labelMagrins, labelSpacing"
                                }
                                PropertyAnimation {
                                    target: contentBase
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "contentMargins"
                                }
                                ScriptAction {
                                    script: content.normalScreenIconSignal()
                                }
                            }
                            ScriptAction {
                                script: {
                                    // 避免应用组内部保持拖拽时，收起应用组后，应用组背景颜色还在（解决bug：拖拽应用组内部应用到外面时，应用组底框会变深）
                                    folderBackground.opacity = 0;
                                    // 点击空白区域实现重命名并收起应用组
                                    folderNameText.textEditingFinished();
                                    content.hideFolder();
                                }
                            }
                        }
                    },
                    Transition {
                        to: "folderOpened"
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    target: folderIconBase
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "x, y, width, height, radius, alpha, opacity"
                                }
                                PropertyAnimation {
                                    target: folderBackground
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "width, height"
                                }
                                PropertyAnimation {
                                    target: folderNameText
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "opacity"
                                }
                                PropertyAnimation {
                                    target: content
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "labelMagrins, labelSpacing"
                                }
                                PropertyAnimation {
                                    target: contentBase
                                    duration: animationDuration; easing.type: Easing.Bezier; easing.bezierCurve: [0.3, 0.2, 0.1, 1.0]
                                    properties: "contentMargins"
                                }
                            }
                            ScriptAction {
                                script: {
                                    folderBackground.opacity = 1;
                                    content.mouseEnable = true;
                                }
                            }
                        }
                    }
                ]

                Item {
                    id: contentBase
                    anchors.fill: parent
                    property real contentMargins
                    anchors.margins: contentMargins

                    FolderContent {
                        id: content
                        width: parent.width
                        height: contentItem.childrenRect.height
                        x: 0; y: 0
                        isFullScreen: root.isFullScreen
                        ratio: root.ratio
                        // 在整体拖拽结束后，Loader的组件才可以销毁
                        onIsDraggingChanged: {
                            if (!isDragging && !isFolderOpened) {
                                root.active = false; // TODO: 应用组变为一个Loader，一直显示
                            }
                        }
                    }
                }
            }
            MouseArea {
                id: baseMouseArea
                anchors.fill: dropArea
                enabled: isFolderOpened
                z: -10
                onWheel: (wheel) => {
                    if (wheel.angleDelta.y < 0) {
                        content.setCurrentPageIndex(content.currentPageIndex + 1); // 滚轮向下,向下翻页
                    } else if (wheel.angleDelta.y > 0) {
                        content.setCurrentPageIndex(content.currentPageIndex - 1); // 滚轮向上,向上翻页
                    }
                }

                property bool isDragging: false
                property real startY: 0
                property real startContentY: 0
                property real yOffset: 0

                onPressed: {
                    startY = mouseY;
                    startContentY = content.y;
                    baseMouseArea.isDragging = true;
                }

                onPositionChanged: {
                    if (pressed && content.pageCount > 1) {
                        // 计算Y轴位移
                        yOffset = mouseY - startY;
                        var newY = startContentY + yOffset;

                        // 限制滚动范围、弹性效果
                        var minY = -(content.cellHeight * content.maxRows * (content.pageCount - 1));
                        var maxY = 0;
                        if (newY > maxY) {
                            newY = maxY + (yOffset) * 0.3;
                        } else if (newY < minY) {
                            newY = minY + (yOffset) * 0.3;
                        }

                        content.y = newY;
                    }
                }

                onReleased: {
                    baseMouseArea.isDragging = false;

                    // 判断翻页方向
                    var pageHeight = content.cellHeight * content.maxRows;
                    var targetPageIndex = content.currentPageIndex;

                    var threshold = pageHeight * 0.3; // 计算翻页阈值: 页面高度的30%
                    if (yOffset > threshold) {
                        // 向下拖动超过阈值 - 上一页
                        targetPageIndex = Math.max(0, content.currentPageIndex - 1);
                    } else if (yOffset < -threshold) {
                        // 向上拖动超过阈值 - 下一页
                        targetPageIndex = Math.min(content.pageCount - 1, content.currentPageIndex + 1);
                    } else {
                        // 根据当前位置决定目标页面
                        var currentPageOffset = -content.y % pageHeight;
                        if (currentPageOffset > pageHeight / 2) {
                            targetPageIndex = Math.floor(-content.y / pageHeight) + 1;
                        } else {
                            targetPageIndex = Math.floor(-content.y / pageHeight);
                        }
                        targetPageIndex = Math.max(0, Math.min(content.pageCount - 1, targetPageIndex));
                    }

                    content.setCurrentPageIndex(targetPageIndex);
                    yOffset = 0;
                }
            }
            // 多页时右侧导航栏
            ListView {
                id: pageIndicator
                width: contentItem.childrenRect.width
                height: contentItem.childrenRect.height
                interactive: false
                model: content.pageCount
                visible: isFolderOpened && (count > 1)

                anchors.right: folderBackground.right
                anchors.rightMargin: folderParam.pageIndicatorMargin
                anchors.verticalCenter: folderBackground.verticalCenter

                delegate: MouseArea {
                    width: folderParam.pageIndicatorSize; height: width
                    hoverEnabled: true
                    onClicked: {
                        content.setCurrentPageIndex(index);
                    }
                    UkuiItems.DtThemeBackground {
                        width: parent.containsMouse ? folderParam.pageIndicatorSizeContainMouse : parent.width / 2; height: width;
                        radius: width / 2
                        anchors.centerIn: parent

                        backgroundColor: (index === content.currentPageIndex) ? Platform.GlobalTheme.kFontStrong : Platform.GlobalTheme.kGrayAlpha7
                        useStyleTransparency: false
                    }
                }
            }

            // 应用组名称及编辑框
            AppUI.EditText {
                id: folderNameText
                anchors.bottom: folderIconBase.top
                anchors.bottomMargin: folderParam.folderNameMargin
                anchors.horizontalCenter: folderIconBase.horizontalCenter

                height: folderParam.folderNameHeight; width: folderIconBase.width
                modelTitle: folderModel.folderName
                textCenterIn: true

                onTextEditingFinished: {
                    if (!folderNameText.editStatus) return;
                    var tmpText = folderNameText.title;
                    if (tmpText.trim() === "") {
                        folderNameText.title = qsTr("Folder");
                    }
                    if (folderNameText.title !== folderNameText.modelTitle) {
                        folderModel.renameFolder(folderNameText.title);
                    }
                    folderNameText.editStatus = false;
                    content.forceActiveFocus();
                    content.itemShowBorder = false;
                }
            }
        }
    }
}
