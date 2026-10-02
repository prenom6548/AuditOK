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

import QtQuick 2.12
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.ukui.sidebar.ui 1.0
import org.ukui.sidebar.core 1.0
import org.ukui.sidebar.shortcut.core 1.0
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

UkuiItems.DtThemeBackground {
    id: root
    readonly property int padding: mainWindow.isTabletMode ? 16 : 0
    property bool focusFrameVisible: false
    property int sliderPointerInteractionId: 0
    readonly property bool focusFrameRevealPending: !panelParent.isSwiping
                                                    && panelParent.currentIndex === 0
                                                    && !focusFrameVisible
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    backgroundColor: Platform.GlobalTheme.baseActive
    radius: Platform.GlobalTheme.kRadiusWindow
    focus: true

    function showKeyboardFocusFrame() {
        if (panelParent.currentIndex === 0) {
            focusFrameVisible = true;
        }
    }

    //保存鼠标点击滑动条之前，焦点框的位置
    function beginSliderPointerInteraction(focusItem) {
        sliderPointerInteractionId += 1;
        var interactionId = sliderPointerInteractionId;
        var focusFrameWasVisible = focusFrameVisible;
        focusFrameVisible = false;

        Qt.callLater(function() {
            if (interactionId !== sliderPointerInteractionId || !focusItem) {
                return;
            }

            try {
                if (focusItem.visible && focusItem.enabled) {
                    focusItem.forceActiveFocus();
                }
            } catch (error) {
                focusFrameVisible = focusFrameWasVisible;
            }
        });
    }

    function clearSliderPointerInteraction() {
        sliderPointerInteractionId += 1;
        focusFrameVisible = false;
    }

    //首次按Tab或鼠标操作滑动条后，不移动焦点。
    Shortcut {
        enabled: root.focusFrameRevealPending
        sequences: ["Tab", "Shift+Tab"]
        context: Qt.WindowShortcut
        autoRepeat: false
        onActivated: root.showKeyboardFocusFrame()
    }

    //页面切换期间或不显示焦点框时不响应Return，Enter
    Shortcut {
        enabled: panelParent.isSwiping || !root.focusFrameVisible
        sequences: ["Return", "Enter"]
        context: Qt.WindowShortcut
        autoRepeat: false
        onActivated: {}
    }

    Keys.onEscapePressed: {
        if (mainWindow.visible) {
            //焦点还给任务栏
            mainWindow.requestPanelFocusRestore();
            if (mainWindow.isTabletMode) {
                root.activePanelOnTablet(false);
            } else {
                mainWindow.hide();
            }
        }
    }

    function moveShortcutPanel(dx) {
        if (LayoutMirroring.enabled) {
            let newX = dx - panelParent.width - padding;
            panel.animationX = newX > 0 ? 0 : newX;
        } else {
            let newX = panelParent.width + padding - dx;
            panel.animationX = newX < 0 ? 0 : newX;
        }
    }

    function initTabletProp() {
        panel.animationX = LayoutMirroring.enabled
                ? -(panelParent.width + padding)
                : panelParent.width + padding;
    }

    function activePanelOnTablet(active) {
        if (panelAnimation.running) {
            return;
        }

        panelAnimation.active = active;
        panelAnimation.from = panel.animationX;
        panelAnimation.to = active
                ? 0
                : LayoutMirroring.enabled
                  ? -(panelParent.width + root.padding)
                  : (panelParent.width + root.padding);
        panelAnimation.start();
    }

    NumberAnimation {
        id: panelAnimation
        property bool active: true
        target: panel
        property: "animationX"
        duration: 300
        easing.type: Easing.InOutQuad

        onFinished: {
            if (!active) {
                mainWindow.hide();
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (mainWindow.isTabletMode) {
                root.activePanelOnTablet(false);
            } else {
                //mainWindow.hide();
            }
        }
    }

    Binding {
        target: root
        property: "width"
        when: !mainWindow.isTabletMode
        value: panelParent.width + root.padding*2
        restoreMode: Binding.RestoreNone
    }

    Binding {
        target: root
        property: "height"
        when: !mainWindow.isTabletMode
        value: panelParent.height + root.padding*2
        restoreMode: Binding.RestoreNone
    }

    SwipeView {
        id: panelParent

        property bool isSwiping: false
        property int targetIndex: 0
        property real targetContentX: 0
        activeFocusOnTab: false
        z: 10
        x: mainWindow.isTabletMode
           ? LayoutMirroring.enabled
             ? root.padding : (root.width - width - root.padding) : root.padding
        y: mainWindow.isTabletMode ? (root.height - height - root.padding) : root.padding
        clip: true
        interactive: false

        //关闭listview自动定位
        function disableContentItemAutoPositioning() {
            contentItem.highlightFollowsCurrentItem = false;
            contentItem.highlightRangeMode = ListView.NoHighlightRange;
            contentItem.snapMode = ListView.NoSnap;
            contentItem.highlightMoveDuration = 0;
            contentItem.highlightMoveVelocity = -1;
        }

        //计算目标页面坐标
        function contentXForIndex(index) {
            var item = contentItem.itemAtIndex(index);
            if (item) {
                return item.x;
            }
            return root.LayoutMirroring.enabled
                    ? -(index + 1) * width
                    : index * width;
        }

        function syncPageVisibility() {
            if (isSwiping) {
                panel.visible = true;
                widgetPage.visible = true;
                return;
            }
            panel.visible = currentIndex === 0;
            widgetPage.visible = currentIndex === 1;
        }

        //翻页动画结束时设置页面的显示状态和焦点状态
        function finishPageTransition() {
            contentItem.contentX = targetContentX;
            setCurrentIndex(targetIndex);
            isSwiping = false;
            syncPageVisibility();

            if (targetIndex === 1) {
                widgetPage.focusAfterShow();
            } else if (widgetPage.restoreFocusAfterSwipe) {
                widgetPage.restoreFocus();
            }
        }

        function switchToPage(index, duration) {
            disableContentItemAutoPositioning();
            pageTransitionAnimation.stop();
            targetIndex = index;
            targetContentX = contentXForIndex(index);

            if (duration <= 0) {
                finishPageTransition();
                return;
            }

            isSwiping = true;
            panel.visible = true;
            widgetPage.visible = true;

            pageTransitionAnimation.from = contentItem.contentX;
            pageTransitionAnimation.to = targetContentX;
            pageTransitionAnimation.duration = duration;
            pageTransitionAnimation.start();
        }

        NumberAnimation {
            id: pageTransitionAnimation
            target: panelParent.contentItem
            property: "contentX"
            easing.type: Easing.InOutQuad

            onFinished: panelParent.finishPageTransition()
        }

        onIsSwipingChanged: {
            syncPageVisibility();
        }

        Component.onCompleted: {
            disableContentItemAutoPositioning();
            syncPageVisibility();
        }

        ShortcuPanel {
            id: panel

            property real animationX: LayoutMirroring.enabled ? -(panelParent.width + root.padding) : 0
            x: 0
            Binding on x {
                when: mainWindow.isTabletMode
                value: panel.animationX
                restoreMode: Binding.RestoreValue
            }

            width: mainWindow.isTabletMode ? 540 : 396

            onWidthChanged: {
                panelParent.width = width;
                widgetPage.width = width;
            }

            onHeightChanged: {
                panelParent.height = height;
                widgetPage.height = height;
            }

            onXChanged: {
                let w = width + root.padding;
                mainWindow.setBlurStrength(Math.floor(4000*(1 - x/w)));
            }
        }

        Item {
            id: widgetPage
            property bool showReturnButton
            property string returnButtonName

            property Item returnFocusItem: null
            property bool restoreFocusAfterSwipe: false
            property bool returnButtonKeyboardFocus: false

            RowLayout {
                id: pageHeader
                anchors.top: parent.top
                anchors.topMargin: 16
                anchors.left: parent.left
                anchors.leftMargin: widgetPage.showReturnButton ? 16 : 24
                height: 24
                spacing: 4

                FocusScope {
                    id: backButton
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignVCenter

                    visible: widgetPage.showReturnButton
                    enabled: visible
                    activeFocusOnTab: visible

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("back to Shortcuts")

                    UkuiItems.DtThemeBackground {
                        anchors.fill: parent
                        useStyleTransparency: false
                        backgroundColor: Platform.GlobalTheme.kContainGeneralAlphaNormal
                        border.width: backButton.activeFocus ? 2 : 0
                        borderColor: Platform.GlobalTheme.kBrandFocus
                        radius: mainWindow.isTabletMode ? Platform.GlobalTheme.kRadiusMax : Platform.GlobalTheme.kRadiusNormal

                        UkuiItems.Icon {
                            anchors.fill: parent
                            source: LayoutMirroring.enabled ? "ukui-end-symbolic" : "pan-start-symbolic"
                            mode: UkuiItems.Icon.AutoHighlight
                        }
                    }

                    Keys.onPressed: {
                        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            widgetPage.hideWidget(250);
                        }
                    }
                }

                UkuiItems.DtThemeText {
                    id: backText
                    Layout.fillHeight: true

                    pointSizeOffset: 2
                    font.weight: Font.Bold
                    text: widgetPage.returnButtonName === "" ?
                              qsTr("back to Shortcuts") : widgetPage.returnButtonName
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
            }

            WidgetDelegate {
                id: widget

                anchors.fill: parent
                quickWidgetId: ""
            }

            MouseArea {
                enabled: widgetPage.showReturnButton
                anchors.fill: pageHeader
                onClicked: {
                    widgetPage.hideWidget(250);
                }
            }

            function showWidget(id, show, name, duration, fromKeyboard = false) {
                returnFocusItem = mainWindow.activeFocusItem;
                restoreFocusAfterSwipe = false;
                returnButtonKeyboardFocus = show && fromKeyboard;
                panel.exitEdit();
                mainWindow.currentId = id;
                if (widget.quickWidgetId !== id) widget.quickWidgetId = id;
                widgetPage.showReturnButton = show;
                widgetPage.returnButtonName = name;
                panelParent.switchToPage(1, duration);
            }

            //动画完成后，对二级页面的焦点处理
            function focusAfterShow() {
                Qt.callLater(function() {
                    if (panelParent.currentIndex !== 1 || panelParent.isSwiping) {
                        return;
                    }

                    if (widgetPage.returnButtonKeyboardFocus) {
                        backButton.forceActiveFocus();
                    } else {
                        root.forceActiveFocus();
                    }
                });
            }

            //返回快捷面板界面时恢复焦点
            function restoreFocus() {
                var item = returnFocusItem;
                returnFocusItem = null;
                restoreFocusAfterSwipe = false;

                if (item && item.visible && item.enabled) {
                    item.forceActiveFocus();
                    if (item.activeFocus) {
                        return;
                    }
                }

                panel.focusFirstItem();
            }

            function hideWidget(duration = 0, shouldRestoreFocus = true) {
                mainWindow.currentId = "";
                if (backButton.activeFocus) {
                    root.forceActiveFocus();
                }
                returnButtonKeyboardFocus = false;
                restoreFocusAfterSwipe = shouldRestoreFocus && duration !== 0;
                if (!shouldRestoreFocus) {
                    returnFocusItem = null;
                }
                panelParent.switchToPage(0, duration);
                if (duration === 0) {
                    if (shouldRestoreFocus) {
                        restoreFocus();
                    }
                }
            }
        }

        Connections {
            target: mainWindow

            function onShowMenuWidget(id, show, name) {
                widgetPage.showWidget(id, show, name, 0, false);
            }

            function onHideMenuWidget() {
                widgetPage.hideWidget();
            }

            function onVisibleChanged() {
                root.clearSliderPointerInteraction();
                if (!mainWindow.visible) {
                    widgetPage.hideWidget(0, false);
                    widget.quickWidgetId = "";
                } else if (panelParent.currentIndex === 0) {
                    Qt.callLater(panel.focusFirstItem);
                }
            }
        }
    }
}
