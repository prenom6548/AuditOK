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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import org.ukui.panel.impl 1.0 as Panel

WidgetContainerItem {
    id: containerItem
    Accessible.description: "This is a widget container of ukui-panel"
    Accessible.name: "ukui-panel_widget_container"

    property int orientation: WidgetContainer.orientation
    property bool isHorizontal: orientation === Types.Horizontal
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    property int trayAreaMaxLength: containerItem.WidgetContainer.screen === null
                              ? 0 : isHorizontal ? panelBase.width * 0.382 : panelBase.height * 0.382
    property int calendarLength: 0
    property int showDesktopLength: 0
    property int systemTrayLength: Math.max(0, trayAreaMaxLength - mainLayout.nonTrayPluginsTotalLengthFrom1)

    property int containerMargin: 8 * ratio
    property int mainLayoutSpacing: 32 * ratio
    property int appViewSpacing: 4 * ratio
    property int configSpacing: 0
    property double ratio: Math.max(Math.min(panelSize / WidgetContainer.config.panelDefaultSize, 2), 1)
    property int panelSize: 0
    onParentChanged: {
        anchors.fill = parent
    }

    Binding {
        target: containerItem
        property: "panelSize"
        value: isHorizontal ? containerItem.height : containerItem.width
        delayed: true
    }

    ContentWindow {
        id: panelPosIndicator
        screen: containerItem.WidgetContainer.screen
        position: Types.Bottom
        flags: Qt.Window | Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint
        windowType: WindowType.SystemWindow

        property bool hideIndicator: false
        property bool isHorizontal: (position === Types.BottomCenter || position === Types.TopCenter)
        property int  newPanelPos: Types.Center

        enableBlurEffect: !hideIndicator

        DtThemeBackground {
            Accessible.description: "This is a position indicator window of ukui-panel"
            Accessible.name: "ukui-panel position indicator"
            radius: containerItem.WidgetContainer.radius
            backgroundColor: GlobalTheme.brightTextActive
            opacity: panelPosIndicator.hideIndicator ? 0 : 0.3
            width: {
                if (panelPosIndicator.isHorizontal) {
                    return containerItem.WidgetContainer.screen.availableGeometry.width - 6;
                }
                return containerItem.isHorizontal ? containerItem.height : containerItem.width;
            }

            height: {
                if (panelPosIndicator.isHorizontal) {
                    return containerItem.isHorizontal ? containerItem.height : containerItem.width;
                }
                return containerItem.WidgetContainer.screen.availableGeometry.height - 6;
            }
        }
    }

    DtThemeBackground {
        id: panelBase
        anchors.fill: parent
        anchors {
            leftMargin: containerItem.WidgetContainer.margin.left
            topMargin: containerItem.WidgetContainer.margin.top
            rightMargin: containerItem.WidgetContainer.margin.right
            bottomMargin: containerItem.WidgetContainer.margin.bottom
        }
        border.width: 0
        radius: containerItem.WidgetContainer.radius

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            enabled: !panel.lockPanel
            propagateComposedEvents: true
            Accessible.description: "This is a widget container of ukui-panel"
            Accessible.name: "ukui-panel_mousearea"

            property point startPoint: "-1, -1"
            property bool isResize: false
            property real screenAngle: 0
            property point prePoint: Qt.point(-1, -1)
            property bool resizing: false

            // 计算夹角度数，对边b, 邻边a
            function getAngle(b, a) {
                return Math.atan(b / a) * 180 / Math.PI;
            }

            onResizingChanged: {
                if (!resizing) {
                    panel.endResizeCustomPanelSize();
                }
            }
            onClicked: (mouse) => {
                if (mouse.button === Qt.RightButton && Panel.disableContextMenu) {
                    mouse.accepted = true;
                }
                mouse.accepted = false;
            }
            onExited: {
                if (!pressed) {
                    panel.changeCursor("");
                    resizing = false;
                }
            }

            onReleased: {
                panel.changeCursor("");
                let visible = panelPosIndicator.visible;
                panelPosIndicator.hide();
                if (visible && !panel.lockPanel && (panelPosIndicator.newPanelPos !== containerItem.WidgetContainer.position)) {
                    panel.changePanelPos(panelPosIndicator.newPanelPos);
                }
                resizing = false;
            }

            // 计算按下点在本地坐标中出发何种操作
            function checkPointOpt(point) {
                switch (containerItem.WidgetContainer.position) {
                    case Types.Left:
                        return (point.x > (width - 4)) ? "resizeHor" : "";
                    case Types.Top:
                        return (point.y > (height - 4)) ? "resizeVec" : "";
                    case Types.Right:
                        return (point.x < 4) ? "resizeHor" : "";
                    case Types.Bottom:
                        return (point.y < 4) ? "resizeVec" : "";
                    default:
                        break
                }

                return "";
            }

            onPressed: (event) => {
                let localPos = Qt.point(event.x, event.y);
                // 按下的起始点
                startPoint = mapToGlobal(localPos);
                prePoint = mapToGlobal(localPos);

                let opt = checkPointOpt(localPos);
                isResize = (opt !== "");

                let screenSize = containerItem.WidgetContainer.screen.size;
                // 屏幕对角线夹角与右边的夹角，右上角和右下角相同
                screenAngle = getAngle(screenSize.width/2, screenSize.height/2);

                if (!isResize) {
                    if (panel.lockPanel) {
                        event.accepted = false;
                    }
                } else {
                    let edge;
                    switch (containerItem.WidgetContainer.position) {
                        case Types.Left:
                            edge = Qt.RightEdge;
                            break;
                        case Types.Right:
                            edge = Qt.LeftEdge;
                            break;
                        case Types.Top:
                            edge = Qt.BottomEdge;
                            break;
                        case Types.Bottom:
                            edge = Qt.TopEdge;
                            break;
                        default:
                            return;
                    }
                    resizing = true;
                    panel.startResizeCustomPanelSize(edge);
                }
            }

            onPositionChanged: (event) => {
                let localPos = Qt.point(event.x, event.y);
                if (!pressed) {
                    // 悬浮状态，切换鼠标样式
                    if (containsMouse) {
                        panel.changeCursor(checkPointOpt(localPos));
                    }
                    return;
                }

                let globalPos = mapToGlobal(localPos);

                if (!isResize) {
                    // 拖拽换位功能
                    if (Math.abs(startPoint.x - globalPos.x) < 20 || Math.abs(startPoint.y - globalPos.y) < 20) {
                        panelPosIndicator.newPanelPos = containerItem.WidgetContainer.position;
                        panelPosIndicator.hideIndicator = true;
                        return;
                    }

                    // move
                    // 切换鼠标样式
                    panel.changeCursor("move");
                    let screenRect = containerItem.WidgetContainer.screen.geometry;
                    let b = globalPos.x - screenRect.x;
                    let a1 = globalPos.y - screenRect.y;
                    let a2 = screenRect.y + screenRect.height - globalPos.y;

                    if (b < 0 || b > screenRect.width) {
                        return;
                    }

                    if (a1 < 0 || a1 > screenRect.height)  {
                        return;
                    }

                    if (a2 < 0 || a2 > screenRect.height)  {
                        return;
                    }

                    let angleTop = getAngle(b, a1);
                    let angleBottom = getAngle(b, a2);

                    let pos;
                    if (angleTop <= screenAngle && angleBottom < screenAngle) {
                        pos = Types.LeftCenter;
                        panelPosIndicator.newPanelPos = Types.Left;
                    } else if (angleTop > screenAngle && angleBottom <= screenAngle) {
                        pos = Types.TopCenter;
                        panelPosIndicator.newPanelPos = Types.Top;
                    } else if (angleTop >= screenAngle && angleBottom > screenAngle) {
                        pos = Types.RightCenter;
                        panelPosIndicator.newPanelPos = Types.Right;
                    } else if (angleTop < screenAngle && angleBottom >= screenAngle) {
                        pos = Types.BottomCenter;
                        panelPosIndicator.newPanelPos = Types.Bottom;
                    } else {
                        return;
                    }

                    if (panelPosIndicator.newPanelPos === containerItem.WidgetContainer.position) {
                        // TODO: wlcom环境调用hide会导致mouseArea的pressed变为false
                        // panelPosIndicator.hide();
                        panelPosIndicator.hideIndicator = true;
                    } else {
                        panelPosIndicator.hideIndicator = false;
                        panelPosIndicator.show();
                        panelPosIndicator.position = pos;
                    }
                }
                prePoint = mapToGlobal(localPos);
            }
        }

        GridLayout {
            id: mainLayout
            z: 10
            anchors.fill: parent
            anchors {
                leftMargin: containerItem.WidgetContainer.padding.left
                topMargin: containerItem.WidgetContainer.padding.top
                rightMargin: containerItem.WidgetContainer.padding.right
                bottomMargin: containerItem.WidgetContainer.padding.bottom
            }
            rows: containerItem.isHorizontal ? 1 : children.length
            columns: containerItem.isHorizontal ? children.length : 1
            rowSpacing: mainLayoutSpacing
            columnSpacing: mainLayoutSpacing

            // 为了计算taskManager最大长度
            property var otherPluginsLength: {"": 0}      // 全部区域的widget，除taskManager外,对应的instanceId和长度
            property int otherPluginsTotalLength: 0       // 全部区域除taskManager外，其他插件的长度
            // 为了计算托盘最大长度
            property var otherPluginsLengthFrom1: {"": 0} // layoutIndex为1的widget，除托盘外,对应的instanceId和长度
            property int nonTrayPluginsTotalLengthFrom1: 0 // 右侧区域（layoutIndex为1）除托盘外，其他插件的长度

            function computePluginsLength(widgetsLength) {
                let otherPluginsTotalLength = 0;
                for (var key in widgetsLength) {
                    var length = widgetsLength[key];
                    if (typeof length === 'number') {
                        otherPluginsTotalLength += length;
                    }
                }
                return otherPluginsTotalLength;
            }

            /**
             * panel左侧 / 上侧
             * appView: 包括 org.ukui.menu.starter/org.ukui.panel.search/org.ukui.panel.separator/org.ukui.panel.taskManager/org.ukui.panel.taskView插件
             * layoutIndex: 0
             */
            GridLayout {
                id: appView
                z: 5
                Layout.fillWidth: !containerItem.isHorizontal
                Layout.fillHeight: containerItem.isHorizontal
                Layout.maximumWidth: containerItem.isHorizontal ? (mainLayout.width - configView.width - mainLayoutSpacing) : mainLayout.width
                Layout.maximumHeight: containerItem.isHorizontal ? mainLayout.height : (mainLayout.height - configView.height - mainLayoutSpacing)

                Layout.alignment: containerItem.isHorizontal ? Qt.AlignLeft : Qt.AlignTop
                Layout.leftMargin: containerItem.isHorizontal ? containerMargin : 0
                Layout.topMargin: containerItem.isHorizontal ? 0 : containerMargin
                rows: containerItem.isHorizontal ? 1 : appRepeater.count
                columns: containerItem.isHorizontal ? appRepeater.count : 1
                rowSpacing: appViewSpacing
                columnSpacing: appViewSpacing
                Repeater {
                    id: appRepeater
                    model: Panel.WidgetSortModel
                    {
                        // TODO: use config
                        layoutIndex: 0
                        widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                        sourceModel: containerItem.widgetItemModel
                    }
                    delegate: widgetLoaderComponent
                }
            }

            /**
             *  panel右侧 / 下侧
             *  configView: 包括org.ukui.systemTray\org.ukui.panel.calendar\org.ukui.panel.showDesktop插件
             *  layoutIndex: 1
             */
            GridLayout {
                id: configView
                z: 5
                Layout.fillWidth: !containerItem.isHorizontal
                Layout.fillHeight: containerItem.isHorizontal
                Layout.alignment: containerItem.isHorizontal ? Qt.AlignRight : Qt.AlignBottom

                rows: containerItem.isHorizontal ? 1 : configRepeater.count
                columns: containerItem.isHorizontal ? configRepeater.count : 1
                rowSpacing: configSpacing
                columnSpacing: configSpacing

                Repeater {
                    id: configRepeater
                    model: Panel.WidgetSortModel
                    {
                        // TODO: use config
                        layoutIndex: 1
                        widgetOrder: containerItem.WidgetContainer.config.widgetsOrder
                        sourceModel: containerItem.widgetItemModel
                    }
                    delegate: widgetLoaderComponent
                }
            }
        }

        Component {
            id: widgetLoaderComponent

            Item {
                id: widgetParent
                clip: true

                property double ratio: containerItem.ratio
                property int index: model.index
                property Item widgetItem: model.widgetItem
                property string itemId: widgetItem.Widget.id

                Layout.fillWidth: widgetItem && widgetItem.Layout.fillWidth
                Layout.fillHeight: widgetItem && widgetItem.Layout.fillHeight

                Layout.minimumWidth: widgetItem.Layout.minimumWidth
                Layout.minimumHeight: widgetItem.Layout.minimumHeight
                Layout.preferredWidth: widgetItem.Layout.preferredWidth
                Layout.preferredHeight: widgetItem.Layout.preferredHeight
                Layout.maximumWidth: {
                    if (itemId === "org.ukui.panel.taskManager" && isHorizontal) {
                        return mainLayout.width - mainLayout.otherPluginsTotalLength;
                    }
                    if (itemId === "org.ukui.systemTray" && isHorizontal)  {
                        return Math.max(Layout.minimumWidth, containerItem.systemTrayLength);
                    }
                    return widgetItem.Layout.maximumWidth ? widgetItem.Layout.maximumWidth : mainLayout.width;
                }
                Layout.maximumHeight: {
                    if (itemId === "org.ukui.panel.taskManager" && !isHorizontal) {
                        return mainLayout.height - mainLayout.otherPluginsTotalLength;
                    }
                    if (itemId === "org.ukui.systemTray" && !isHorizontal) {
                        return Math.max(Layout.minimumHeight, containerItem.systemTrayLength);
                    }
                    return widgetItem.Layout.maximumHeight ? widgetItem.Layout.maximumHeight : mainLayout.height;
                }

                onWidthChanged: {
                    if (isHorizontal) {
                        if (itemId !== "org.ukui.panel.taskManager") {
                            mainLayout.otherPluginsLength[widgetItem.Widget.instanceId] = width;

                            if (widgetLayoutIndex === 1 && itemId !== "org.ukui.systemTray") {
                                mainLayout.otherPluginsLengthFrom1[widgetItem.Widget.instanceId] = width;
                                mainLayout.nonTrayPluginsTotalLengthFrom1 = mainLayout.computePluginsLength(mainLayout.otherPluginsLengthFrom1) + configSpacing * (configRepeater.count - 1);
                            }

                            // 需要加上appView、configView间距和mainLayout间距
                            mainLayout.otherPluginsTotalLength = mainLayout.computePluginsLength(mainLayout.otherPluginsLength)
                                + (appViewSpacing * (appRepeater.count - 1) + configSpacing * (configRepeater.count - 1) + mainLayoutSpacing + appView.Layout.leftMargin);
                        }
                    }
                }

                onHeightChanged: {
                    if (!isHorizontal) {
                        if (itemId !== "org.ukui.panel.taskManager") {
                            mainLayout.otherPluginsLength[widgetItem.Widget.instanceId] = height;

                            if (widgetLayoutIndex === 1 && itemId !== "org.ukui.systemTray") {
                                mainLayout.otherPluginsLengthFrom1[widgetItem.Widget.instanceId] = height;
                                mainLayout.nonTrayPluginsTotalLengthFrom1 = mainLayout.computePluginsLength(mainLayout.otherPluginsLengthFrom1) + configSpacing * (configRepeater.count - 1);
                            }

                            // 需要加上appView、configView间距和mainLayout间距
                            mainLayout.otherPluginsTotalLength = mainLayout.computePluginsLength(mainLayout.otherPluginsLength)
                                + (appViewSpacing * (appRepeater.count - 1) + configSpacing * (configRepeater.count - 1) + mainLayoutSpacing + appView.Layout.topMargin);
                        }
                    }
                }
                Component.onDestruction: {
                    if (itemId !== "org.ukui.panel.taskManager") {
                        mainLayout.otherPluginsLength[widgetItem.Widget.instanceId] = 0;

                        if (widgetLayoutIndex === 1 && itemId !== "org.ukui.systemTray") {
                            mainLayout.otherPluginsLengthFrom1[widgetItem.Widget.instanceId] = 0;
                            mainLayout.nonTrayPluginsTotalLengthFrom1 = mainLayout.computePluginsLength(mainLayout.otherPluginsLengthFrom1) + configSpacing * (configRepeater.count - 1);
                        }

                        // 需要加上appView、configView间距和mainLayout间距
                        mainLayout.otherPluginsTotalLength = mainLayout.computePluginsLength(mainLayout.otherPluginsLength)
                            + (appViewSpacing * (appRepeater.count - 1) + configSpacing * (configRepeater.count - 1) + mainLayoutSpacing + containerItem.containerMargin);
                    }
                }

                onWidgetItemChanged: {
                    if (widgetItem) {
                        widgetItem.parent = widgetParent
                        widgetItem.anchors.fill = widgetParent

                        if (itemId === "org.ukui.panel.calendar") {
                            containerItem.calendarLength = Qt.binding(function(){
                                return containerItem.isHorizontal ? width : height;
                            })
                        }
                        if (itemId === "org.ukui.panel.showDesktop") {
                            containerItem.showDesktopLength = Qt.binding(function(){
                                return containerItem.isHorizontal ? width : height;
                            })
                        }
                    }
                }
            }
        }
    }
    MouseArea {
        id: fcMouseArea
        anchors.fill: parent
        acceptedButtons: {
            let button = Qt.NoButton;
            if (panel.disableMouseLeftButton) {
                button |= Qt.LeftButton
            }
            if (panel.disableMouseRightButton) {
                button |= Qt.RightButton
            }
            return button;
        }
        enabled: panel.disableMouseLeftButton || panel.disableMouseRightButton || panel.disableContextMenu
    }
}
