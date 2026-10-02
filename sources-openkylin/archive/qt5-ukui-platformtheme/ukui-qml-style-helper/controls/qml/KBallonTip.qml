/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBallonTip。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Window 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

// 外层 Popup 负责保持与宿主窗口的关系；实际提示内容在内层 Window 中绘制。
// 结构与主题 Menu.qml 保持一致，窗口移动时可重新同步全局坐标。
T.Popup {
    id: root

    enum TipType { Nothing = 0, Normal = 1, Info = 2, Warning = 3, Error = 4, Variable = 5, Loading = 6 }

    property int tipType: KBallonTip.TipType.Nothing
    property string text: ""
    property string variableIconName: ""
    property int tipTime: 1000
    property int contentLeftMargin: 24
    property int contentTopMargin: 18
    property int contentRightMargin: 24
    property int contentBottomMargin: 18
    property bool transparent: false
    property Item anchorItem: null

    padding: 0
    margins: 0
    modal: false
    focus: false
    closePolicy: T.Popup.NoAutoClose

    implicitWidth: Math.max(1, contentLeftMargin + (_hasIcon ? 22 + contentRow.spacing : 0)
                            + textLabel.implicitWidth + contentRightMargin)
    implicitHeight: Math.max(22, textLabel.implicitHeight)
                    + contentTopMargin + contentBottomMargin
    width: implicitWidth
    height: implicitHeight

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    property color _backgroundColor: "transparent"
    readonly property color _surfaceColor: transparent
        ? Qt.rgba(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b,
                  Math.max(0, Math.min(1, app.menuTransparency)))
        : _backgroundColor
    readonly property int _radius: {
        var r = typeof ukui_platformtheme_token !== "undefined" && ukui_platformtheme_token
                ? ukui_platformtheme_token.kradiusNormal : 6
        return r < 0 ? 6 : r
    }

    property int _loadingIndex: 0
    readonly property string _iconName: {
        if (tipType === KBallonTip.TipType.Normal)
            return "ukui-dialog-success"
        if (tipType === KBallonTip.TipType.Info)
            return "dialog-info"
        if (tipType === KBallonTip.TipType.Warning)
            return "dialog-warning"
        if (tipType === KBallonTip.TipType.Error)
            return "dialog-error"
        if (tipType === KBallonTip.TipType.Variable)
            return variableIconName
        if (tipType === KBallonTip.TipType.Loading)
            return "ukui-loading-" + _loadingIndex + ".symbolic"
        return ""
    }
    readonly property bool _hasIcon: tipType !== KBallonTip.TipType.Nothing

    function _refreshTheme() {
        _backgroundColor = parseColor.getDtColor("window-active")
    }

    function _refreshLoading() {
        if (tipType === KBallonTip.TipType.Loading)
            loadingTimer.start()
        else {
            loadingTimer.stop()
            _loadingIndex = 0
        }
    }

    function _syncWindowGeometry() {
        if (!popupWindow || !anchorItem)
            return

        var w = Math.max(1, popupWindow.width)
        var p = anchorItem.mapToGlobal((anchorItem.width - w) / 2,
                                       anchorItem.height + 8)
        var x = Math.round(p.x)
        var y = Math.round(p.y)
        if (popupWindow.x !== x)
            popupWindow.x = x
        if (popupWindow.y !== y)
            popupWindow.y = y
        popupWindow.requestUpdate()
    }

    function _scheduleWindowSync() {
        if (visible)
            Qt.callLater(_syncWindowGeometry)
    }

    function showInfo(item) {
        if (item)
            anchorItem = item
        _refreshLoading()
        // 先定位独立窗口，避免首次显示时落在默认的左上角坐标。
        _syncWindowGeometry()
        visible = true
        hideTimer.restart()
        _scheduleWindowSync()
    }

    function hide() {
        visible = false
        hideTimer.stop()
        loadingTimer.stop()
    }

    function setTipType(type) { tipType = type }
    function setText(value) { text = value }
    function setTipTime(value) { tipTime = value }
    function setContentsMargins(left, top, right, bottom) {
        contentLeftMargin = left
        contentTopMargin = top
        contentRightMargin = right
        contentBottomMargin = bottom
    }
    function setVariableIcon(name) { variableIconName = name }
    function setTransparent(value) { transparent = value }
    function setAnchorItem(item) { anchorItem = item }

    Component.onCompleted: {
        _refreshTheme()
        _refreshLoading()
    }
    onTipTypeChanged: _refreshLoading()
    onVisibleChanged: _scheduleWindowSync()
    onWidthChanged: _scheduleWindowSync()
    onHeightChanged: _scheduleWindowSync()
    onAnchorItemChanged: _scheduleWindowSync()

    Timer {
        id: hideTimer
        interval: Math.max(0, root.tipTime)
        repeat: false
        onTriggered: root.hide()
    }

    Timer {
        id: loadingTimer
        interval: 100
        repeat: true
        onTriggered: root._loadingIndex = (root._loadingIndex + 1) % 8
    }

    // 外层 Popup 不绘制内容，避免在宿主窗口内出现第二份提示。
    contentItem: Item {
        visible: false
        implicitWidth: 0
        implicitHeight: 0
    }

    background: Item {
        implicitWidth: 0
        implicitHeight: 0

        Window {
            id: popupWindow
            width: Math.max(1, Math.ceil(root.width))
            height: Math.max(1, Math.ceil(root.height))
            visible: root.visible
            color: "transparent"
            flags: Qt.ToolTip | Qt.FramelessWindowHint

            onWidthChanged: root._scheduleWindowSync()
            onHeightChanged: root._scheduleWindowSync()
            onVisibleChanged: {
                if (visible)
                    root._scheduleWindowSync()
            }

            // 平台窗口管理器提供阴影、边框和圆角。
            Private.UKUIPopupHandle {
                anchors.fill: parent
                visible: parent.visible
                titlebarVisible: false
                acceptFocus: false
                windowStaysOnTopHint: true
                radius: root._radius
            }

            Rectangle {
                id: surface
                anchors.fill: parent
                // 圆角由 UKUIPopupHandle 的窗口装饰统一处理，避免透明状态下重复抗锯齿。
                radius: 0
                color: root._surfaceColor
            }

            RowLayout {
                id: contentRow
                anchors.fill: parent
                anchors.leftMargin: root.contentLeftMargin
                anchors.topMargin: root.contentTopMargin
                anchors.rightMargin: root.contentRightMargin
                anchors.bottomMargin: root.contentBottomMargin
                spacing: 12

                Item {
                    visible: root._hasIcon
                    Layout.preferredWidth: 22
                    Layout.minimumWidth: 22
                    Layout.maximumWidth: 22
                    Layout.preferredHeight: 22
                    Layout.alignment: Qt.AlignVCenter

                    Private.KyIcon {
                        anchors.fill: parent
                        iconName: root._iconName
                    }
                }

                Text {
                    id: textLabel
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    Layout.preferredWidth: implicitWidth
                    Layout.alignment: Qt.AlignVCenter
                    text: root.text
                    font: app.font
                    color: parseColor.getDtColor("kfont-primary")
                    wrapMode: Text.NoWrap
                }
            }
        }
    }

    // 锚点自身的布局、滚动或尺寸变化时更新独立窗口位置。
    Connections {
        target: root.anchorItem
        function onXChanged() { root._scheduleWindowSync() }
        function onYChanged() { root._scheduleWindowSync() }
        function onWidthChanged() { root._scheduleWindowSync() }
        function onHeightChanged() { root._scheduleWindowSync() }
    }

    // 与 Menu.qml 一样，主窗口移动后同步内层 Window 的全局坐标。
    Connections {
        target: Window.window
        function onXChanged() { root._scheduleWindowSync() }
        function onYChanged() { root._scheduleWindowSync() }
    }

    Connections {
        target: app
        function onParametryChanged() { root._refreshTheme() }
    }
}
