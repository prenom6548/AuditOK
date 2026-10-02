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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import org.ukui.panel.taskManager 1.0 as UkuiTaskManager

WidgetItem {
    id: root
    Layout.objectName: "layout"

    property bool isHorizontal: Widget.orientation === Types.Horizontal
    property int maxLength: {
        if (isHorizontal) {
            if (parent.Layout.maximumWidth > 0) {
                return parent.Layout.maximumWidth;
            } else {
                if (parent.width > 0) {
                    return parent.width;
                } else {
                    return height + buttonBase.width;
                }
            }
        } else {
            return parent.Layout.maximumHeight ? parent.Layout.maximumHeight : parent.height;
        }
    }
    property bool isIntervactive: {
        if (isHorizontal) {
            return taskManagerView.view.contentWidth > maxLength;
        } else {
            return taskManagerView.view.contentHeight > maxLength;
        }
    }
    property real ratio: parent.ratio ? parent.ratio : 1
    property int spacing: parent ? (parent.ratio ? 4 * parent.ratio : 4) : 4
    property bool isIslandMode: parent.isIslandMode ? parent.isIslandMode : false

    Layout.fillWidth: !isHorizontal
    Layout.fillHeight: isHorizontal
    Layout.preferredWidth: isHorizontal ? childrenRect.width : width
    Layout.preferredHeight: isHorizontal ? height : childrenRect.height

    // 最小宽高：保证taskManager在滚动状态时，能够至少显示一个图标和翻页按钮
    Layout.minimumWidth: isHorizontal ? Math.min((taskManagerView.view.contentWidth + taskManagerView.view.taskItemSize + spacing), (height + buttonBase.width)) : 0

    onWidgetWindowChanged: {
        if (widgetWindow) {
            widgetWindow.onVisibleChanged.connect(updateWindowVisible);
        }
    }

    function updateWindowVisible() {
        taskManagerView.view.windowVisible = widgetWindow.visible;
    }

    function flickView(distance) {
        // 计算轻弹速度
        let velocity = Math.round(Math.sqrt(2 * Math.abs(distance) * taskManagerView.view.flickDeceleration));
        // 确定轻弹方向
        let direction = (distance > 0) ? 1 : -1
        taskManagerView.view.maximumFlickVelocity = velocity;
        if (isHorizontal) {
            taskManagerView.view.flick(velocity * direction, 0);
        } else {
            taskManagerView.view.flick(0, velocity * direction);
        }
    }

    // 向前滚动view，进行Overshoot时的处理
    function flickViewForward(distance) {
        // 想要滚动的距离，采取向上取整，避免剩余0.XX时还需要再进行翻页
        let preDistance = Math.ceil(distance);
        // 剩余的距离
        let remainDistance = isHorizontal ? taskManagerView.view.contentX : taskManagerView.view.contentY;
        if (preDistance < remainDistance) {
            flickView(preDistance);
        } else {
            flickView(remainDistance + taskManagerView.viewOvershoot);
        }
    }

    // 向后滚动view，进行Overshoot时的处理
    function flickViewBackward(distance) {
        // 想要滚动的距离，采取向上取整，避免剩余0.XX时还需要再进行翻页
        let preDistance = Math.ceil(distance);
        // 剩余的距离，采取向下取整，原因同上
        let remainDistance = isHorizontal ? Math.max(0, taskManagerView.view.contentItem.childrenRect.width - taskManagerView.view.contentX - taskManagerView.view.width)
            : Math.max(0, taskManagerView.view.contentItem.childrenRect.height - taskManagerView.view.contentY - taskManagerView.view.height);
        remainDistance = Math.floor(remainDistance);
        if (preDistance < remainDistance) {
            flickView(preDistance * -1);
        } else {
            flickView((remainDistance + taskManagerView.viewOvershoot) * -1);
        }
    }

    GridLayout {
        id: gridLayout
        width: isHorizontal ? (isIntervactive ? maxLength : taskManagerView.width) : parent.width
        height: isHorizontal ? parent.height : (isIntervactive ? maxLength : taskManagerView.height)
        columnSpacing: root.spacing
        rowSpacing: root.spacing
        columns: taskManagerView.view.isHorizontal ? 2 : 1
        rows: taskManagerView.view.isHorizontal ? 1 : 2

        // taskManager
        MouseArea {
            id: wheelArea
            Accessible.name: "ukui-panel_taskManager_wheelArea"
            Layout.preferredWidth: isHorizontal ? taskManagerView.width : parent.width
            Layout.preferredHeight: isHorizontal ? parent.height : taskManagerView.height

            enabled: isIntervactive
            onWheel: (wheel) => {
                flickView(wheel.angleDelta.y);
            }

            UkuiTaskManager.TaskManagerView {
                id: taskManagerView
                // 三岛、经典模式下，taskManager.width都留出一个item的空白宽度，并且可以接受drop事件
                property int visualWidth: isIntervactive ? childrenRect.width : childrenRect.width + view.taskItemSize + spacing
                // 超长滚动时，内容可超过边界的距离
                property int viewOvershoot: 120

                width: isHorizontal ? visualWidth : parent.width
                height: isHorizontal ? parent.height : childrenRect.height
                ratio: root.ratio

                view.clip: isIntervactive
                view.spacing: root.spacing
                view.width: {
                    if (isHorizontal) {
                        if (maxLength < view.contentItem.childrenRect.width) {
                            return maxLength - buttonBase.width - root.spacing;
                        } else {
                            return isIslandMode ? view.contentItem.childrenRect.width : maxLength;
                        }
                    } else {
                        return parent.width
                    }
                }
                view.height: {
                    if (isHorizontal) {
                        return parent.height
                    } else {
                        if (maxLength < view.contentItem.childrenRect.height) {
                            return maxLength - buttonBase.height - root.spacing
                        } else {
                           return isIslandMode ? view.contentItem.childrenRect.height : maxLength;
                        }
                    }
                }
            }
        }

        // 超长滚动按钮 按钮宽高比为 0.8
        StyleBackground {
            id: buttonBase
            Layout.preferredHeight: isHorizontal ? (parent.height - spacing*2) : (buttonBase.width*0.8*2 + 1)
            Layout.preferredWidth: isHorizontal ? (buttonBase.height*0.8*2 + 1) : (parent.width - spacing*2)
            Layout.alignment: isHorizontal ? Qt.AlignVCenter : Qt.AlignHCenter
            Layout.margins: 2
            property int iconSize: isHorizontal ? (buttonBase.height / 2) : (buttonBase.width / 2)

            paletteRole: Theme.Text
            useStyleTransparency: false
            alpha: 0.1
            radius: 12

            GridLayout {
                anchors.fill: parent
                rows: isHorizontal ? 1 : children.length
                columns: isHorizontal ? children.length : 1
                rowSpacing: 0
                columnSpacing: 0

                // 上一页
                MouseArea {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Accessible.name: "ukui-panel_taskManager_widget_upPage"
                    property string iconEnabled: "://org.ukui.panel.taskManager/qml/icon/ukui-start-symbolic.svg"
                    property string iconDisabled: "://org.ukui.panel.taskManager/qml/icon/ukui-start-symbolic-disable.svg"
                    property bool isAtBeginning: isHorizontal ? taskManagerView.view.atXBeginning : taskManagerView.view.atYBeginning
                    onClicked: {
                         if (!isAtBeginning) {
                             flickViewForward(isHorizontal ? taskManagerView.view.width : taskManagerView.view.height);
                         }
                    }
                    Icon {
                        width: buttonBase.iconSize; height: width
                        anchors.centerIn: parent
                        mode: Icon.AutoHighlight
                        rotation: isHorizontal ? 0 : 90
                        source: parent.isAtBeginning ? parent.iconDisabled : parent.iconEnabled
                    }
                }

                StyleBackground {
                    Layout.fillHeight: isHorizontal
                    Layout.fillWidth: !isHorizontal
                    Layout.preferredHeight: 1
                    Layout.preferredWidth: 1

                    paletteRole: Theme.Text
                    useStyleTransparency: false
                    alpha: 0.15
                }

                // 下一页
                MouseArea {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Accessible.name: "ukui-panel_taskManager_widget_downPage"
                    property string iconEnabled: "://org.ukui.panel.taskManager/qml/icon/ukui-end-symbolic.svg"
                    property string iconDisabled: "://org.ukui.panel.taskManager/qml/icon/ukui-end-symbolic-disable.svg"
                    property bool isAtEnd: isHorizontal ? taskManagerView.view.atXEnd : taskManagerView.view.atYEnd

                    onClicked: {
                        if (!isAtEnd) {
                            flickViewBackward(isHorizontal ? taskManagerView.view.width : taskManagerView.view.height);
                        }
                    }
                    Icon {
                        width: buttonBase.iconSize; height: width
                        anchors.centerIn: parent
                        mode: Icon.AutoHighlight
                        rotation: isHorizontal ? 0 : 90
                        source: parent.isAtEnd ? parent.iconDisabled : parent.iconEnabled
                    }
                }
            }
        }
    }
}
