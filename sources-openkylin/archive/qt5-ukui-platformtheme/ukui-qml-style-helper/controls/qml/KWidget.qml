/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * QML counterpart of kdk::KWidget.
 * 含图标栏（iconBar）、窗口按钮栏（windowButtonBar）、侧边栏（sideBar）、主内容区（baseBar），
 * 支持 VerticalType / HorizontalType / MixedType 三种布局，背景响应主题与窗口激活状态。
 * sideBar 由 layoutType 控制显隐；baseBar 可由 baseBarVisible 控制，关闭后侧栏铺满。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0

ApplicationWindow {
    id: control

    enum LayoutType { VerticalType = 0, HorizontalType = 1, MixedType = 2 }

    /// 作为独立顶层窗，避免嵌在其它 Window/Item 下时被设为 transient，导致点主窗无法失焦
    flags: Qt.Window

    property int layoutType: KWidget.LayoutType.VerticalType
    property url iconSource: ""
    property string iconName: ""
    property string widgetName: ""
    /// 对齐 C++ sidebarFollowMode，默认 true；为 true 时侧栏/图标栏宽度跟随 sideBarWidth
    property bool sidebarFollowMode: true
    property bool loadingStatus: false
    /// 侧栏与图标栏宽度，对齐 PM_Widget_SideWidget_Width（PC: 200）
    property int sideBarWidth: 200
    /// 标题栏高度，对齐 PM_IconbarHeight（PC: 40）
    property int iconBarHeight: 40
    /// 图标尺寸，对齐 PM_IconBarIconSize（PC: 24）
    property int iconBarIconSize: 24

    property bool menuButtonVisible: true
    property bool minButtonVisible: true
    property bool maxButtonVisible: true
    property bool closeButtonVisible: true
    /// 主内容区（baseBar）是否显示；为 false 时侧栏铺满整窗（需侧栏可见）
    property bool baseBarVisible: true

    property bool isNormal: visibility !== Window.Maximized

    property alias sideBar: sideBarContainer
    property alias baseBar: baseBarContainer
    property alias iconBar: iconBarItem
    property alias windowButtonBar: windowButtonBarItem
    property alias menuButton: menuBtn
    property alias minButton: minBtn
    property alias maxButton: maxBtn
    property alias closeButton: closeBtn

    /// 透传菜单项，便于应用直接访问/改可见性（对齐 KMenuButton::*Action）
    property alias settingItem: menuBtn.settingItem
    property alias helpItem: menuBtn.helpItem
    property alias aboutItem: menuBtn.aboutItem
    property alias feedbackItem: menuBtn.feedbackItem
    property alias quitItem: menuBtn.quitItem
    property alias themeItem: menuBtn.themeItem
    property alias themeAutoItem: menuBtn.autoItem
    property alias themeLightItem: menuBtn.lightItem
    property alias themeDarkItem: menuBtn.darkItem
    property alias themeMenuVisible: menuBtn.themeMenuVisible

    signal minBtnClicked()
    signal maxOrRestoryBtnClicked(bool isNormal)
    signal closeBtnClicked()
    signal titleDoubleClicked()

    // 标题栏菜单响应（对齐 C++ QAction::triggered，供应用连接）
    signal settingTriggered()
    signal helpTriggered()
    signal aboutTriggered()
    signal feedbackTriggered()
    signal quitTriggered()
    signal themeAutoTriggered()
    signal themeLightTriggered()
    signal themeDarkTriggered()

    width: 800
    height: 600
    minimumWidth: 200
    title: widgetName
    color: "transparent"

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token

    readonly property color _baseColor: _hasToken
        ? (control.active ? ukui_platformtheme_token.kContainGeneralNormal
                          : ukui_platformtheme_token.kContainGeneralInactive)
        : palette.base
    readonly property color _sideColor: _hasToken
        ? (control.active ? ukui_platformtheme_token.kContainSecondaryNormal
                          : ukui_platformtheme_token.kContainSecondaryInactive)
        : palette.window
    /// HorizontalType：图标栏与侧栏同色；VerticalType / MixedType：图标栏用 base 色
    readonly property color _iconBarColor: {
        if (layoutType === KWidget.LayoutType.HorizontalType)
            return _sideColor
        return _baseColor
    }

    readonly property bool _sideBarVisible: layoutType !== KWidget.LayoutType.VerticalType
                                            && !loadingStatus
    readonly property bool _baseBarReallyVisible: baseBarVisible && !loadingStatus
    /// 仅侧栏：内容区关闭且侧栏开启时，侧栏占满
    readonly property bool _sideBarOnly: _sideBarVisible && !_baseBarReallyVisible

    function _iconUrl() {
        if (iconSource.toString() !== "")
            return iconSource
        if (iconName === "")
            return ""
        return "image://imageProvider/" + iconName + "/"
                + (control.active ? "normal" : "disenable")
    }

    onWidgetNameChanged: title = widgetName
    onVisibilityChanged: {
        isNormal = visibility !== Window.Maximized
        maxBtn.i = isNormal ? 0 : 1
    }

    header: Item {
        height: control.iconBarHeight
        width: parent.width
        RowLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: rec1
                Layout.fillHeight: true
                Layout.fillWidth: control._sideBarOnly
                Layout.preferredWidth: {
                    if (!sideBarContainer.visible)
                        return 0
                    if (control._sideBarOnly)
                        return parent.width
                    return sideBarContainer.width
                }
                Layout.maximumWidth: {
                    if (!sideBarContainer.visible)
                        return 0
                    if (control._sideBarOnly)
                        return 16777215
                    return sideBarContainer.width
                }
                Layout.minimumWidth: {
                    if (!sideBarContainer.visible)
                        return 0
                    if (control._sideBarOnly)
                        return 0
                    return sideBarContainer.width
                }
                color: sideBarContainer.color
            }
            Rectangle {
                visible: !control._sideBarOnly
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: control._baseColor
            }
        }

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // 图标栏 — 对齐 KIconBar，宽度与侧栏一致
            Item {
                id: iconBarItem
                Layout.preferredWidth: control.sideBarWidth
                Layout.maximumWidth: control.sideBarWidth
                Layout.minimumWidth: Math.min(control.sideBarWidth, parent.width / 2)
                Layout.fillHeight: true
                //color: control._iconBarColor

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    spacing: 8

                    Item {
                        Layout.preferredWidth: control.iconBarIconSize
                        Layout.preferredHeight: control.iconBarIconSize
                        Layout.alignment: Qt.AlignVCenter
                        visible: titleImage.source !== ""

                        Image {
                            id: titleImage
                            anchors.fill: parent
                            source: control._iconUrl()
                            sourceSize.width: control.iconBarIconSize
                            sourceSize.height: control.iconBarIconSize
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            cache: false
                        }
                    }

                    Label {
                        id: titleLabel
                        property bool windowLabel: true
                        text: control.widgetName
                        visible: control.widgetName !== ""
                        font: app.font
                        elide: Text.ElideRight
                        maximumLineCount: 1
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        Layout.minimumWidth: 0
                        ToolTip.visible: truncated && titleHover.containsMouse
                        ToolTip.text: control.widgetName

                        MouseArea {
                            id: titleHover
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }
                    }

                    Item { Layout.fillWidth: true; Layout.preferredWidth: 0 }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: {
                        control._toggleMaximized()
                        control.titleDoubleClicked()
                    }
                }
            }

            // 窗口按钮栏 — 对齐 KWindowButtonBar：菜单 / 最小化 / 最大化 / 关闭
            Item {
                id: windowButtonBarItem
                Layout.fillWidth: true
                Layout.fillHeight: true
//                color: control._baseColor

                RowLayout {
                    anchors.fill: parent
                    anchors.rightMargin: 4
                    spacing: 2

                    Item {
                        Layout.fillWidth: true
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onDoubleClicked: {
                                if (control.maxButtonVisible && maxBtn.enabled) {
                                    control._toggleMaximized()
                                    control.titleDoubleClicked()
                                }
                            }
                        }
                    }

                    Private.KHeadBarMenuBtn {
                        id: menuBtn
                        visible: control.menuButtonVisible
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: false
                        onSettingTriggered: control.settingTriggered()
                        onHelpTriggered: control.helpTriggered()
                        onAboutTriggered: control.aboutTriggered()
                        onFeedbackTriggered: control.feedbackTriggered()
                        onQuitTriggered: control.quitTriggered()
                        onThemeAutoTriggered: control.themeAutoTriggered()
                        onThemeLightTriggered: control.themeLightTriggered()
                        onThemeDarkTriggered: control.themeDarkTriggered()
                    }
                    Private.KWindowButton {
                        id: minBtn
                        visible: control.minButtonVisible
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: false
                        property string type: "minimize"
                        onClicked: {
                            control.showMinimized()
                            control.minBtnClicked()
                        }
                    }
                    Private.KWindowButton {
                        id: maxBtn
                        visible: control.maxButtonVisible
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: false
                        property string type: "maximize"
                        onClicked: {
                            control._toggleMaximized()
                            control.maxOrRestoryBtnClicked(control.isNormal)
                        }
                        onHoveredChanged: {
                            maxBtnFlyoutTimer.stop()
                            if (hovered)
                                maxBtnFlyoutTimer.start()
                        }
                    }
                    Private.KWindowButton {
                        id: closeBtn
                        visible: control.closeButtonVisible
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: false
                        property string type: "close"
                        onClicked: {
                            control.closeBtnClicked()
                            control.close()
                        }
                    }
                }
            }
        }
    }

    StackLayout {
        id: stackedLayout
        anchors.fill: parent
        currentIndex: control.loadingStatus ? 1 : 0

        // 主内容：侧栏 + base 区
        RowLayout {
            spacing: 0

            Rectangle {
                id: sideBarContainer
                visible: control._sideBarVisible
                Layout.fillHeight: true
                Layout.fillWidth: control._sideBarOnly
                Layout.preferredWidth: control._sideBarOnly ? -1 : control.sideBarWidth
                Layout.maximumWidth: control._sideBarOnly ? 16777215 : control.sideBarWidth
                Layout.minimumWidth: control._sideBarOnly ? 0 : control.sideBarWidth
                color:  {
                    var baccolor = control._sideColor
                    var alphaValue = control.active ? app.menuTransparency : 1.0;
                    return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, alphaValue)
                }
                clip: true
            }

            Rectangle {
                id: baseBarContainer
                visible: control._baseBarReallyVisible
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                color: control._baseColor
            }
        }

        // 加载预览页 — 对齐 setLoadingStatus
        Rectangle {
            color: control._baseColor

            Image {
                anchors.centerIn: parent
                width: 96
                height: 96
                source: control._iconUrl()
                sourceSize.width: 96
                sourceSize.height: 96
                fillMode: Image.PreserveAspectFit
                smooth: true
                cache: false
                visible: source !== ""
            }
        }
    }

    Timer {
        id: maxBtnFlyoutTimer
        interval: Qt.styleHints.mousePressAndHoldInterval
        repeat: false
        onTriggered: {
            if (!maxBtn.hovered)
                return
            var p = maxBtn.mapToItem(control.header, 0, 0)
            kdialog.showTileFlyout(p.x, p.y, Math.round(maxBtn.width), Math.round(maxBtn.height))
        }
    }

    Private.KyQuickDialog {
        id: kdialog
    }
    property alias kyQuickDialog: kdialog

    APPParameter {
        id: app
        onParametryChanged: {
            titleImage.source = ""
            titleImage.source = control._iconUrl()
        }
        onIconThemeChanged: {
            titleImage.source = ""
            titleImage.source = control._iconUrl()
        }
    }

    Private.UKUIWindow {
        id: ukuiWindow
        standaloneWindow: true
        blurEnabled: control.active
        shadowEnabled: true
        borderEnabled: true
        roundCornerEnabled: true
    }

    Component.onCompleted: {
        ukuiWindow.setUseStyleWindowManager(true)
        ukuiWindow.clearTransientParent()
    }
    onVisibleChanged: {
        if (visible)
            ukuiWindow.clearTransientParent()
    }

    onActiveChanged: ukuiWindow.blurEnabled = control.active
    function _toggleMaximized() {
        if (control.visibility === Window.Maximized)
            control.showNormal()
        else
            control.showMaximized()
        isNormal = visibility !== Window.Maximized
        maxBtn.i = isNormal ? 0 : 1
    }

    function setIcon(nameOrUrl) {
        var s = "" + nameOrUrl
        if (s.indexOf(":/") >= 0 || s.indexOf("image://") === 0 || s.indexOf("file:") === 0
                || s.indexOf("qrc:") === 0 || s.indexOf("/") === 0) {
            iconSource = nameOrUrl
        } else {
            iconName = s
            iconSource = ""
        }
        titleImage.source = ""
        titleImage.source = _iconUrl()
    }
    function setWidgetName(name) { widgetName = name }
    function setLayoutType(t) { layoutType = t }
    function setSidebarFollowMode(flag) { sidebarFollowMode = flag }
    function setLoadingStatus(flag) { loadingStatus = flag }
    function setMenuButtonVisible(flag) { menuButtonVisible = flag }
    function setMinButtonVisible(flag) { minButtonVisible = flag }
    function setMaxButtonVisible(flag) { maxButtonVisible = flag }
    function setCloseButtonVisible(flag) { closeButtonVisible = flag }
    function setBaseBarVisible(flag) { baseBarVisible = flag }
}
