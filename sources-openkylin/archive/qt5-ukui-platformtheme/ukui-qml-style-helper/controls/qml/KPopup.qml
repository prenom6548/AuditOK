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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate
import org.ukui.platformtheme.controls 1.0 as PlatformthemeControls

/// 带窗管毛玻璃效果的通用 Popup。
/// 内容通过 default property 放入独立 Window，由 PopupHandle 调用 compositor 模糊。
T.Popup {
    id: control

    palette: app.palette
    font: app.font

    default property alias content: windowContent.data

    /// true 时使用 transparentBC 半透明色（配合毛玻璃）；false 时使用实色 backColor
    property bool transparent: true

    /// 背景不透明度，未设置时跟随 app.menuTransparency
    property real backOpacity: -1

    /// false 时毛玻璃 Window 使用 ToolTip 标志，不抢键盘焦点（适合补全/搜索下拉）
    property bool acceptFocus: false//focus

    /// acceptFocus 为 false 时，popup 显示后将焦点还回此控件
    property Item focusTarget: null

    /// false 时关闭/打开无渐变动画（适合搜索补全等需即时消失的场景）
    property bool animated: true

    readonly property bool isTransaprent: transparent

    readonly property real _opacity: backOpacity >= 0 ? backOpacity : app.menuTransparency

    padding: ukuiPopup.padding
    topMargin: 6
    bottomMargin: 6
    clip: false
    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    readonly property int _contentWidth: Math.max(windowContent.implicitWidth, windowContent.width)
    readonly property int _contentHeight: Math.max(windowContent.implicitHeight, windowContent.height)

    implicitWidth: _contentWidth > 0 ? _contentWidth + leftPadding + rightPadding : 0
    implicitHeight: _contentHeight > 0 ? _contentHeight + topPadding + bottomPadding : 0

    // 外层 Popup 仅负责定位与生命周期，实际内容由 windowContent 承载
    contentItem: Item {
        visible: false
        width: 0
        height: 0
    }

    function getStartColor(dtcolor) {
        return parseInterface.startColor(dtcolor)
    }

    function syncWindowPosition() {
        if (!visible || !parent)
            return
        var p = parent.mapToGlobal(x, y)
        var wx = p.x - (control.mirrored ? popupWindow.width : 0)
        if (popupWindow.x !== wx)
            popupWindow.x = wx
        if (popupWindow.y !== p.y)
            popupWindow.y = p.y
    }

    function syncWindowSize() {
        var w = control.width > 0 ? control.width
              : Math.max(implicitWidth, _contentWidth + leftPadding + rightPadding, 1)
        var h = control.height > 0 ? control.height
              : Math.max(implicitHeight, _contentHeight + topPadding + bottomPadding, 1)
        if (popupWindow.width !== w)
            popupWindow.width = w
        if (popupWindow.height !== h)
            popupWindow.height = h
    }

    function constrainToScreen() {
        var size = popHandle.getScreenSize()
        var p = popHandle.getScreenPoint()

        if (popupWindow.x + popupWindow.width > size.width + p.x) {
            var nx = size.width + p.x - popupWindow.width
            if (nx >= p.x)
                popupWindow.x = nx
        }
        if (popupWindow.y + popupWindow.height >= size.height + p.y) {
            var maxH = size.height + p.y - popupWindow.y
            if (maxH > 0 && control.height <= 0 && popupWindow.height > maxH)
                popupWindow.height = maxH
        }
    }

    onVisibleChanged: {
        if (visible) {
            syncWindowSize()
            syncWindowPosition()
            popupWindow.show()
            if (!acceptFocus)
                Qt.callLater(restoreFocusTarget)
        } else {
            popupWindow.hide()
        }
        if (popupWindow.visible !== visible)
            popupWindow.visible = visible
    }

    function restoreFocusTarget() {
        if (focusTarget) {
            focusTarget.forceActiveFocus()
            return
        }
        var item = control.parent
        while (item) {
            if (item.hasOwnProperty("forceActiveFocus")) {
                item.forceActiveFocus()
                return
            }
            item = item.parent
        }
    }

    onXChanged: syncWindowPosition()
    onYChanged: syncWindowPosition()

    onWidthChanged: {
        if (visible && control.width > 0 && popupWindow.width !== control.width)
            popupWindow.width = control.width
    }

    onHeightChanged: {
        if (visible && control.height > 0 && popupWindow.height !== control.height)
            popupWindow.height = control.height
    }

    Connections {
        target: windowContent
        function onImplicitWidthChanged() {
            if (control.visible && control.width <= 0)
                control.syncWindowSize()
        }
        function onImplicitHeightChanged() {
            if (control.visible && control.height <= 0)
                control.syncWindowSize()
        }
    }

    Connections {
        target: popupWindow
        function onWidthChanged() {
            if (control.parent && control.visible)
                control.syncWindowPosition()
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: control.animated ? 0 : 1
            to: 1
            easing.type: Easing.InOutQuad
            duration: control.animated ? 250 : 0
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: control.animated ? 0 : 1
            easing.type: Easing.InOutQuad
            duration: control.animated ? 250 : 0
        }
    }

    onAboutToHide: popupWindow.hide()

    StylePrivate.UKUIPopup {
        id: ukuiPopup
    }

    StylePrivate.APPParameter {
        id: app
    }

    StylePrivate.ParseColorInterface {
        id: parseInterface
    }

    background: Item {
        implicitWidth: popupWindow.width
        implicitHeight: popupWindow.height

        Window {
            id: popupWindow
            visible: control.visible
            color: "transparent"

            onWidthChanged: control.constrainToScreen()
            onHeightChanged: control.constrainToScreen()

            onVisibleChanged: {
                if (visible) {
                    control.syncWindowSize()
                    if (width <= 0)
                        width = 1
                    if (height <= 0)
                        height = 1
                }
                if (control.visible !== visible)
                    control.visible = visible
            }

            PlatformthemeControls.UKUIPopupHandle {
                id: popHandle
//                blur: true
                titlebarVisible: false
                acceptFocus: control.acceptFocus
                anchors.fill: parent
                visible: parent.visible
                radius: ukuiPopup.radius
            }

            Rectangle {
                id: popRec
                anchors.fill: parent
                radius: _opacity >= 1 ? ukuiPopup.radius : 0
                color: {
                    var baccolor = getStartColor(isTransaprent ? ukuiPopup.transparentBC : ukuiPopup.backColor)
                    return Qt.rgba(baccolor.r, baccolor.g, baccolor.b, _opacity)
                }
            }

            Item {
                id: windowContent
                anchors.fill: parent
                anchors.leftMargin: control.leftPadding
                anchors.rightMargin: control.rightPadding
                anchors.topMargin: control.topPadding
                anchors.bottomMargin: control.bottomPadding
                visible: popupWindow.visible
                focus: false
            }

            Shortcut {
                sequence: "Escape"
                onActivated: control.close()
            }

            Connections {
                target: Qt.application
                function onStateChanged() {
                    if (!Qt.application.active)
                        control.close()
                }
            }
        }
    }
}
