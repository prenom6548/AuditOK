/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Rectangle {
    id: root

    property var model
    property int dragIndex: -1
    property int hoverIndex: -1
    property bool autoScrollUp: false
    property bool autoScrollDown: false
    property int scrollSpeed: 6
    property int scrollInterval: 16

    signal itemMoved(int fromIndex, int toIndex)

    function handleDeleteIM(index) {
        imModel.deleteInputMethod(index);
        imModel.updateCurrentGroupInputMethodList();
    }

    function handleDragStart(index) {
        dragIndex = index;
    }

    function updateScrollAction(yPos, yEndPox) {
        var threshold = flickable.height * 0.2;
        //        console.log("flickable.contentY:",flickable.contentY,"yPos:",yPos,",threshold:",threshold,",flickable.height:",flickable.height);
        // 检查是否接近顶部边界
        if (yPos - allImsTitle.height - fillItem.height < flickable.contentY - threshold)
            autoScrollUp = true;
        else
            autoScrollUp = false;
        // 检查是否接近底部边界
        if (yEndPox > flickable.contentY + flickable.height - threshold)
            autoScrollDown = true;
        else
            autoScrollDown = false;
    }

    function handleDragging(imitem, yPos) {
        hoverIndex = -1;
        var yEndPox = yPos + imitem.height;
        var topper = -1;
        var lowwer = -1;
        for (var i = 0; i < repeater.count; ++i) {
            if (i === dragIndex)
                continue;

            var item = repeater.itemAt(i);
            var itemBeginPosY = item.mapToItem(flickable, 0, 0).y;
            var itemMidPosY = itemBeginPosY + item.height / 2;
            var itemEndPosY = itemBeginPosY + item.height;
            //            print("handleDragging,itemBeginPosY:" + itemBeginPosY + ", itemMidPosY:" + itemMidPosY + ", itemEndPosY:" + itemEndPosY, ", yPos:" + yPos);
            // 判断上边缘
            if (itemBeginPosY <= yPos && yPos <= itemEndPosY) {
                if (yPos <= itemMidPosY) {
                    highlight.y = itemEndPosY;
                    hoverIndex = i;
                    highlight.visible = true;
                    break;
                }
            }
            // 判断下边缘
            if (itemBeginPosY <= yEndPox && yEndPox <= itemEndPosY) {
                if (yEndPox >= itemMidPosY) {
                    highlight.y = itemEndPosY;
                    hoverIndex = i;
                    highlight.visible = true;
                    break;
                }
            }
        }
        if (hoverIndex !== -1) {
            highlight.visible = true;
            repeater.itemAt(hoverIndex).opacity = 0.6;
        } else {
            highlight.visible = false;
            for (var j = 0; j < repeater.count; ++j) {
                if (j === dragIndex)
                    continue;

                repeater.itemAt(j).opacity = 1;
            }
        }
        // 拖拽条目在边界时，滚动视图
        updateScrollAction(yPos, yEndPox);
    }

    function handleDragEnd(fromIndex) {
        if (highlight.visible) {
            highlight.visible = false;
            if (hoverIndex !== fromIndex && hoverIndex != -1) {
                imModel.moveTo(fromIndex, hoverIndex);
                imModel.updateCurrentGroupInputMethodList();
            }
        }
        dragIndex = -1;
        autoScrollUp = false;
        autoScrollDown = false;
    }

    color: "transparent"
    width: parent.width
    height: parent.height
    clip: true

    // 自动滚动计时器
    Timer {
        id: scrollTimer

        interval: scrollInterval
        repeat: true
        running: autoScrollUp || autoScrollDown
        onTriggered: {
            if (autoScrollUp && layout.height > flickable.height)
                flickable.contentY = Math.max(0, flickable.contentY - scrollSpeed);

            if (autoScrollDown && layout.height > flickable.height)
                flickable.contentY = Math.min(flickable.contentHeight - flickable.height, flickable.contentY + scrollSpeed);

        }
    }

    Flickable {
        id: flickable

        anchors.fill: parent
        contentWidth: width
        contentHeight: layout.height + highlight.height + 6
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        onContentYChanged: {
            highlight.visible = false;
        }
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: layout

            width: parent.width
            spacing: 2

            UkuiItems.DtThemeText {
                id: allImsTitle

                anchors.left: parent.left
                anchors.leftMargin: 16
                text: qsTr("All Input Methods")
                Accessible.name: text
                pointSizeOffset: 2
            }

            Item {
                id: fillItem

                height: 4
                width: parent.width
            }

            Repeater {
                id: repeater

                model: root.model

                delegate: Item {
                    readonly property bool isFirst: model.index === 0
                    readonly property bool isLast: model.index === (root.model.length - 1)

                    width: layout.width
                    height: 60

                    InputMethodItem {
                        id: imitem

                        width: parent.width
                        height: parent.height
                        imIcon: modelData.icon
                        itemText: modelData.name
                        uniqueName: modelData.uniqueName
                        itemIndex: index
                        itemCount: root.model.length
                        configurable: modelData.configurable
                        parentContainer: flickable
                        onDragStarted: {
                            handleDragStart(index);
                        }
                        onDragging: {
                            imitem.opacity = 0;
                            handleDragging(imitem, yPos);
                            background.visible = false;
                        }
                        onDragEnded: {
                            imitem.opacity = 1;
                            handleDragEnd(index);
                            background.visible = true;
                        }
                        onDeleteIM: {
                            handleDeleteIM(index);
                        }
                    }

                    UkuiItems.DtThemeBackground {
                        id: background

                        useStyleTransparency: false
                        z: -1
                        anchors.fill: imitem
                        radius: GlobalTheme.kRadiusNormal
                        clip: true

                        Rectangle {
                            anchors.fill: parent
                            radius: GlobalTheme.kRadiusNormal
                            color: parent.color
                            anchors.topMargin: isFirst ? 0 : -parent.radius
                            anchors.bottomMargin: isLast ? 0 : -parent.radius
                        }

                    }

                }

            }

            Item {
                height: 2
                width: parent.width
            }

        }

        Rectangle {
            id: highlight

            width: layout.width
            height: 2
            color: GlobalTheme.highlightActive.pureColor
            visible: false
            z: 1
            parent: flickable
        }

        MouseArea {
            onWheel: () => {
                flickable.flick(0, wheel.angleDelta.y * 5);
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: vScrollBar

            Accessible.name: qsTr("DraggableList ScrollBar")
            hoverEnabled: true
            active: hovered || pressed
            orientation: Qt.Vertical
            size: flickable.height / flickable.contentHeight
            anchors.rightMargin: -20
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: flickable.contentHeight > flickable.height
        }

    }

}
