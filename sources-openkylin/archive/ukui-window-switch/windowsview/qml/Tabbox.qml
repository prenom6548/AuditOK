/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import "Controller.js" as Controller
import org.ukui.windowThumbnail 1.0 as UkuiThumbnail
//ukui-quick
import org.ukui.quick.items 1.0 as UkuiItem
import org.ukui.quick.platform 1.0 as UkuiPlatform
//flow
import org.ukui.positioner 1.0

FocusScope {
    id: root
    property int currentIndex: -1
    property real yOffset: tabboxShadow.shadowSize
    width: tabbox.width
    height: tabbox.height + 2 * tabboxShadow.shadowSize
    focus: true

UkuiItem.DtThemeBackground {
    id : tabbox
    anchors.centerIn: parent
    property var controller : new Controller.AppController(appWindowListModel, tabbox, multitaskViewManager, ["none"]);
    property rect screenGeometry : multitaskViewManager.currentScreenRect
    property bool isOpenGL : true
    property int maxHeight : screenGeometry.height * 0.85
    property var scaleFactor : devicePixelRatio
    property int viewMargin : 20 * scaleFactor
    property bool isLayoutLeftToRight: Qt.application.layoutDirection === Qt.LeftToRight

    focus : true
    useStyleTransparency : true
    backgroundColor : UkuiPlatform.GlobalTheme.kComponentNormal
    width : screenGeometry.width
    height : tabboxFlickable.height
    Keys.enabled : true
    border.width : UkuiPlatform.GlobalTheme.normalLine * scaleFactor
    borderColor : UkuiPlatform.GlobalTheme.kLineWindowActive

    // multi-shadow item
    UkuiItem.DtDropShadow {
        id: tabboxShadow
        anchors.fill: tabbox
        shadowData: UkuiPlatform.GlobalTheme.kShadowMenu
    }

    onActiveFocusChanged : {
        if(!tabbox.activeFocus) {
            multitaskViewManager.close()
        } else {
            if (multitaskViewManager.isBackTab) {
                currentIndex = appWindowListModel.rowCount() > 1 ? appWindowListModel.rowCount() - 1 : 0;
            } else {
                currentIndex = appWindowListModel.rowCount() > 1 ? 1 : 0;
            }

             /**
             1. wlcom环境特定情况（快速操作按下alt+tab并松开）下，会出现本界面不隐藏问题，原因如下：
             由于wlcom合成器与窗口选择器是两个独立的进程，使得窗口选择器被快捷键拉起时，向合成器发出的GrabKeyboard请求未被及时响应
             导致在特定情况下窗口选择器无法收到Key_Alt的Released信号，进而无法通过响应alt released事件来关闭此界面
             2. 当前实现，此界面显示时可以认为Key_Alt必定处于按下状态，因此针对特定情况处理如下：
             在此界面显示并获取键盘焦点后，立即查询Key_Alt是否处于按下状态
             如果按未处于按下状态，则激活选定窗口并退出；否则，不做特殊处理
             **/
            // fix https://gitee.com/openkylin/ukui-window-switch/issues/IAID8L?from=project-issue
            if (!multitaskViewManager.isAltKeyPressed()) {
                activateSelectedWindow();
                return;
            }
        }
    }

    Flickable {
        id: tabboxFlickable
        anchors.leftMargin: tabbox.viewMargin //注意：间距为0则无法显示滚动条
        anchors.rightMargin: tabbox.viewMargin
        width: contentWidth
        height: Math.min(contentHeight, tabbox.maxHeight)
        contentWidth: parent.width - 2 * tabbox.viewMargin
        contentHeight: tabboxFlow.height
        LayoutMirroring.enabled: !tabbox.isLayoutLeftToRight
        LayoutMirroring.childrenInherit: true
        clip: true

        UkuiItem.DtThemeScrollBar.vertical: UkuiItem.DtThemeScrollBar {
            id: tabboxScrollBar
            Accessible.role: Accessible.ScrollBar
            Accessible.name: "tabbox-vertical-scrollbar"
            Accessible.description: "a scroll bar, which allows the user to scroll the visible area"
            parent: tabboxFlickable.parent
            anchors.top: tabboxFlickable.top
            anchors.left: tabboxFlickable.right
            anchors.bottom: tabboxFlickable.bottom
            policy: UkuiItem.DtThemeScrollBar.AsNeeded
            visual: tabboxFlow.height > height
        }

        NumberAnimation {
            id : scrollMoveAnimation
            target : tabboxScrollBar
            property : "position"
            from : tabboxScrollBar.position
            duration : 300
            easing.type : Easing.InOutQuart
        }

        UkuiFlow {
            id: tabboxFlow
            spacing: 8 * tabbox.scaleFactor
            padding: tabbox.viewMargin
            width: parent.width
            flow: Flow.LeftToRight
            layoutDirection: Qt.LeftToRight
            lineHorizontalCenterAlignment: true

            Connections {
                target: previewItemRepeater
                function onCountChanged() {
                    tabboxFlow.forceLayout();
                    if (currentIndex >= previewItemRepeater.count) {
                        // 特殊情况处理：当某个窗口在界面显示后退出，并且此时currentIndex指向最后一个item
                        // 该item不存在，为避免界面无法退出的情况，需要更新currentIndex
                        // 窗口新增则不会出现找不到item的情况，不需要额外处理
                        currentIndex = previewItemRepeater.count - 1
                    }
                }
            }

            Repeater {
                id : previewItemRepeater
                model: appWindowListModel

                delegate: UkuiItem.DtThemeBackground {
                    id: previewItem
                    property string uuid: model.Uuid
                    property int spaceWidth: 4 * tabbox.scaleFactor
                    property int outerBorderWidth: UkuiPlatform.GlobalTheme.focusLine * tabbox.scaleFactor
                    property int innerBorderWidth: UkuiPlatform.GlobalTheme.focusLine * tabbox.scaleFactor
                    property int maxThumbnailWidth: (tabbox.screenGeometry.width - 2 * tabboxFlow.padding) / 4
                    property int thumbnailHeight: tabbox.screenGeometry.height / 22 * 5
                    property int minWidth: Math.floor(tabbox.width / 10)
                    width: Math.max(thumbnailBorderItem.width + 2 * (spaceWidth + outerBorderWidth), minWidth)
                    height: captionRow.height + thumbnailHeight + 5 * spaceWidth + 2 * (innerBorderWidth + outerBorderWidth)
                    color: "transparent"
                    borderColor: UkuiPlatform.GlobalTheme.kFontPrimary
                    border.width: index === currentIndex ? outerBorderWidth : 0
                    radius: tabbox.isOpenGL ? UkuiPlatform.GlobalTheme.kRadiusWindow : 0

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        Accessible.role: Accessible.Button
                        Accessible.name: "active-window-" + model.caption
                        Accessible.description: "preview-window, click to active the app window"

                        onEntered : {
                            thumbnailBorderItem.border.width = previewItem.innerBorderWidth;
                        }

                        onExited : {
                            thumbnailBorderItem.border.width = 0;
                        }

                        onClicked : {
                            currentIndex = index;
                            tabbox.controller.onWindowClicked(model.Uuid);
                        }
                    }

                    function getThumbnailSize()
                    {
                        var thumbHeight = thumbnailHeight
                        var max_width = maxThumbnailWidth
                        var clientWidth = model.Geometry.width
                        var clientHeight = model.Geometry.height
                        var thumbWidth = Math.floor(thumbHeight * clientWidth / clientHeight)
                        if (thumbWidth > max_width) {
                            thumbWidth = max_width
                            thumbHeight = Math.floor(thumbWidth * clientHeight / clientWidth)
                        }
                        return [thumbWidth, thumbHeight]
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: previewItem.spaceWidth
                        spacing: 4 * tabbox.scaleFactor

                        RowLayout {
                            id: captionRow
                            spacing: 8 * tabbox.scaleFactor
                            Layout.preferredHeight: Math.max(iconItem.height, captionText.height)
                            Layout.maximumWidth: thumbnailBorderItem.width
                            Layout.alignment: Qt.AlignCenter
                            Layout.leftMargin: 2 * previewItem.spaceWidth

                            UkuiItem.Icon {
                                id: iconItem
                                width: 24 * tabbox.scaleFactor
                                height: width
                                source: model.Icon
                            }

                            UkuiItem.DtThemeText {
                                id: captionText
                                verticalAlignment: Qt.AlignVCenter
                                Layout.fillWidth: true
                                Layout.preferredHeight: contentHeight
                                text: model.Caption
                                horizontalAlignment: Qt.AlignLeft
                                elide: tabbox.isLayoutLeftToRight ? Text.ElideRight : Text.ElideLeft
                            }
                        } // RowLayout

                        UkuiItem.DtThemeBackground {
                            id: thumbnailBorderItem
                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredWidth: getThumbnailSize()[0] + 2 * (previewItem.innerBorderWidth + previewItem.spaceWidth)
                            Layout.preferredHeight: previewItem.thumbnailHeight + 2 * (previewItem.innerBorderWidth + previewItem.spaceWidth)
                            color: "transparent"
                            borderColor: UkuiPlatform.GlobalTheme.kFontPrimary
                            border.width: 0
                            radius: tabbox.isOpenGL ? UkuiPlatform.GlobalTheme.kRadiusWindow : 0
                            // 预览图阴影
                            shadowData: UkuiPlatform.GlobalTheme.KShadowMin
                            Rectangle {
                                id: thumbnailBackItem
                                width: getThumbnailSize()[0]
                                height: getThumbnailSize()[1]
                                anchors.centerIn: parent
                                color: "transparent"
                                clip: true
                                layer.enabled: tabbox.isOpenGL ? true : false
                                layer.effect: OpacityMask {
                                    maskSource: Rectangle {
                                        width: thumbnailBackItem.width
                                        height: thumbnailBackItem.height
                                        radius: tabbox.isOpenGL ? UkuiPlatform.GlobalTheme.kRadiusMenu : 0
                                    }
                                }
                                UkuiThumbnail.WindowThumbnailItem {
                                    id: thumbnailItem
                                    anchors.fill: parent
                                    uuid: model.Uuid
                                }
                            }
                        }
                    } // ColumnLayout
                } // Repeater.delegate
            } // Repeater
        } // Flow
    } // Flickable

    Keys.onDeletePressed : {
        multitaskViewManager.close()
    }

    Keys.onEscapePressed : {
        multitaskViewManager.close()
    }

    Keys.onBacktabPressed : {
        if (!multitaskViewManager.isAltKeyPressed()) {
            return;
        }
        if (isLayoutLeftToRight) {
            currentIndex = tabboxFlow.moveCurrentIndexLeft(currentIndex);
        } else {
            currentIndex = tabboxFlow.moveCurrentIndexRight(currentIndex);
        }
    }

    Keys.onTabPressed : {
        if (isLayoutLeftToRight) {
            currentIndex = tabboxFlow.moveCurrentIndexRight(currentIndex);
        } else {
            currentIndex = tabboxFlow.moveCurrentIndexLeft(currentIndex);
        }
    }

    Keys.onPressed : {
        if (event.key === Qt.Key_Left) {
            currentIndex = tabboxFlow.moveCurrentIndexLeft(currentIndex);
        } else if (event.key === Qt.Key_Right) {
            currentIndex = tabboxFlow.moveCurrentIndexRight(currentIndex);
        } else if (event.key === Qt.Key_Up) {
            currentIndex = tabboxFlow.moveCurrentIndexUp(currentIndex);
        } else if (event.key === Qt.Key_Down) {
            currentIndex = tabboxFlow.moveCurrentIndexDown(currentIndex);
        } else {
            return;
        }
    }

    Keys.onReleased : {
        if (event.key === Qt.Key_Alt) {
            activateSelectedWindow();
        } else if (event.key === Qt.Key_Shift && !multitaskViewManager.isAltKeyPressed()) {
            activateSelectedWindow();
        }
    }

    function activateSelectedWindow() {
        if (!previewItemRepeater.itemAt(currentIndex)) {
            return;
        }
        controller.onWindowClicked(previewItemRepeater.itemAt(currentIndex).uuid);
    }
}
    onCurrentIndexChanged: {
        if (!multitaskViewManager.altTabVisible) {
            return;
        }

        scrollToTabItem()
    }

    function scrollToTabItem() {
        var currentNextY = previewItemRepeater.itemAt(currentIndex).mapToItem(tabboxFlickable, 0, 0).y
        var previewWindowHeight = previewItemRepeater.itemAt(currentIndex).height
        if (currentNextY < 0) {
            scrollMoveAnimation.to = tabboxScrollBar.position - (-currentNextY + tabbox.viewMargin) / tabboxFlickable.contentHeight
            scrollMoveAnimation.start()
        } else if (currentNextY + previewWindowHeight > tabboxFlickable.height) {
            scrollMoveAnimation.to = tabboxScrollBar.position + (currentNextY + previewWindowHeight - tabboxFlickable.height + tabbox.viewMargin) / tabboxFlickable.contentHeight
            scrollMoveAnimation.start()
        }
    }

    Connections {
        target: multitaskViewManager
        function onAltTabVisibleChanged() {
            if (!multitaskViewManager.altTabVisible) {
                root.currentIndex = -1
            }
        }
    }
}
