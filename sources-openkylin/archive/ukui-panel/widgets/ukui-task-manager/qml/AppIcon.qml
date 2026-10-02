/*
 *
 *  Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12
import org.ukui.quick.widgets 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager

Item {
    property alias appIcon: icon
    property alias appBackgroud: backgroud
    property bool isOpenGL: true
    property real ratio: 1
    property real inset: 4 * ratio // background与控件的边距
    function updateIconGeometry(winIdList) {
        Platform.WindowManager.setMinimizedGeometry(winIdList, backgroud);
    }

    Timer {
        id: iconDragTimer
        interval: 100
        onTriggered: {
            if (iconDropArea.containsDrag) {
                updateIconGeometry(taskItemData.CurrentWinIdList);
                if (taskItemData.CurrentWinIdList[0] !== "") {
                    UkuiTaskManager.TaskManager.activateWindow(taskItemData.CurrentWinIdList[0])
                    taskManager.hidethumbnail();
                }
            }
        }
    }
    DropArea {
        id: iconDropArea
        anchors.fill: parent
        enabled: UkuiTaskManager.TaskManager.enableDragToApp

        function showTooltipIfNeeded() {
            if (tooltip.active && iconDropArea.containsDrag && taskItemData.CurrentWinIdList.length < 2) {
                tooltip.showTooltip();
            }
        }

        onContainsDragChanged: showTooltipIfNeeded()  //实现拖拽到未打开的固定图标上，显示tooltip
        onEntered: (dragEvent) => {
            // 表示图标内部是否接收到了拖拽事件（功能: 拖拽文件到已打开的应用图标上，有三态显示并进行激活窗口操作）
            if (taskItemListRoot.dragEventSource(dragEvent) !== "DropToAppIcon") {
                dragEvent.accepted = false;
                return;
            }

            if (taskItemData.CurrentWinIdList.length === 1) {
                iconDragTimer.restart();
            } else {
                mouseArea.aboutToShowThumbnailView(true, thumbnailData, mouseArea)
            }
        }

        onPositionChanged: {
            // 拖拽到固定未打开的图标上，鼠标标识为禁用状态
            if (taskItemData.CurrentWinIdList.length < 1) {
                drag.accepted = false;
            }
        }
        onExited: {
            iconDragTimer.stop();
            tooltip.hideTooltip();
            taskItemListRoot.requestThumbnailView(false, thumbnailData, mouseArea)
        }
    }

    UkuiItems.Tooltip {
        id: tooltip
        anchors.fill: parent
        mainText: taskItemData.Name
        posFollowCursor: false
        // 固定在任务栏的应用显示tooltip (taskItemData.CurrentWinIdList.length < 1)
        // 接收拖拽文件的图标显示tooltip (iconDropArea.containsDrag)
        active: iconDropArea.containsDrag ? true : taskItemData.CurrentWinIdList ? taskItemData.CurrentWinIdList.length < 1 : false

        onActiveChanged: iconDropArea.showTooltipIfNeeded() // 用于在有拖拽事件，并且只有一个打开窗口时的情况
        location: {
            switch(Widget.container.position) {
                case UkuiItems.Types.Bottom:
                    return UkuiItems.Dialog.BottomEdge;
                case UkuiItems.Types.Left:
                    return UkuiItems.Dialog.LeftEdge;
                case UkuiItems.Types.Top:
                    return UkuiItems.Dialog.TopEdge;
                case UkuiItems.Types.Right:
                    return UkuiItems.Dialog.RightEdge;
            }
        }
        margin: 8
    }

    UkuiItems.DtThemeBackground {
        id: backgroud
        Accessible.role: Accessible.Button
        Accessible.description: "This is task manager button of ukui-panel"
        Accessible.name: taskItemData.Id
        anchors.fill: parent
        anchors {
            leftMargin: isHorizontal? 0 : inset
            topMargin: isHorizontal? inset : 0
            rightMargin: isHorizontal? 0 : inset
            bottomMargin: isHorizontal? inset : 0
        }
        useStyleTransparency: false
        // id: mouseArea --> TaskManager.qml
        backgroundColor: iconDropArea.containsDrag ? Platform.GlobalTheme.kComponentAlphaHover :
                            mouseArea.containsPress ? Platform.GlobalTheme.kComponentAlphaClick :
                            mouseArea.containsMouse ?  Platform.GlobalTheme.kComponentAlphaHover : Platform.GlobalTheme.kContainGeneralAlphaNormal

        radius: Platform.GlobalTheme.kRadiusNormal

        Rectangle {
            id: demandsAttention
            anchors.fill: parent
            color: "#FF9100"
            z: -1
            radius: backgroud.radius
            visible: taskItemData.DemandsAttentionWinIdList ? (taskItemData.DemandsAttentionWinIdList.length > 0) : false

            NumberAnimation on opacity {
                running: demandsAttention.visible
                loops: 4
                from: 0.25
                to: 0.6
                duration: 450
            }
        }

        UkuiItems.Icon {
            id: icon
            anchors.margins: inset
            anchors.fill: parent
            Behavior on scale {
                NumberAnimation { duration: 100 }
            }
        }

        Loader {
            anchors.right: icon.right
            anchors.rightMargin: -3
            anchors.top: icon.top
            anchors.topMargin: -3
            active: taskItemData.UnreadMessagesNum > 0
            sourceComponent: Rectangle {
                id: unreadMessagesBase
                height: icon.height / 2
                width: height + additionalWidth
                radius: height / 2
                property int additionalWidth: {
                    if (taskItemData.UnreadMessagesNum < 10) {
                        return 0;
                    } else if (taskItemData.UnreadMessagesNum < 100){
                        return 6;
                    } else {
                        return 12;
                    }
                }
                color: Platform.GlobalTheme.kErrorNormal.pureColor
                layer.enabled: isOpenCV
                layer.effect: DropShadow {
                    radius: 2
                    samples: 5
                    color: Platform.GlobalTheme.kGrayAlpha10.pureColor
                }
                UkuiItems.DtThemeText {
                    id: unreadMessagesNum
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    textColor: Platform.GlobalTheme.kFontWhite
                    pointSizeOffset: -2
                    font.pixelSize: unreadMessagesBase.height * 5 / 8 //按设计稿比例设置固定字体大小
                    text: {
                        if (taskItemData.UnreadMessagesNum < 100) {
                            return taskItemData.UnreadMessagesNum;
                        } else {
                            return "99+"
                        }
                    }
                }
            }
        }
    }
    UkuiItems.DtThemeBackground {
        height: 4 * ratio
        width: taskItemData.CurrentWinIdList ? Math.min(taskItemData.CurrentWinIdList.length * 8 * ratio, 16 * ratio) : 16
        radius: 2 * ratio

        anchors.verticalCenter: backgroud.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        useStyleTransparency: false
        backgroundColor: taskItemData.HasActiveWindow? Platform.GlobalTheme.kBrandNormal : Platform.GlobalTheme.kGrayAlpha9
        Behavior on width {
            NumberAnimation { duration: 200 }
        }
    }
}
