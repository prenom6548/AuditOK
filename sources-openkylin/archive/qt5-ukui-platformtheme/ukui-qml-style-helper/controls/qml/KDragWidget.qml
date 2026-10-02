/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KDragWidget.
 * 外观、边框状态和文件过滤行为对齐 kdragwidget.cpp。
 */

import QtQuick 2.12
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: root

    property string iconName: "list-add.symbolic"
    property string text: qsTr("将文件拖拽到此处或点击“+”添加文件")
    property string nameFilter: ""
    property var nameFilters: []
    property bool dropActive: false
    property bool dropInvalid: false

    signal getPath(string path)
    signal getPathList(var paths)

    // 与 KDragWidget::iconButton/textLabel 对应的可访问对象。
    property alias iconButton: addButton
    property alias textLabel: textLabelItem

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property color _hoverLine: parseColor.getDtColor("tooltiptext-active")
    readonly property color _dragLine: parseColor.getDtColor("highlight-active")
    readonly property color _leaveLine: parseColor.getDtColor("placeholdertext-active")
    readonly property color _plusBackground: parseColor.getDtColor("kcomponent-alpha-normal")
    readonly property color _plusHoverBackground: parseColor.getDtColor("kcomponent-alpha-hover")
    readonly property color _plusClickBackground: parseColor.getDtColor("kcomponent-alpha-click")
    readonly property color _plusDisableBackground: parseColor.getDtColor("kcomponent-alpha-disable")
    readonly property int _radius: {
        var r = typeof ukui_platformtheme_token !== "undefined" && ukui_platformtheme_token
                ? ukui_platformtheme_token.kradiusNormal : 6
        return r < 0 ? 6 : r
    }

    // 初始状态与鼠标离开后保持一致，避免首次显示的边框颜色突变。
    property color _lineColor: _leaveLine

    implicitWidth: 332
    implicitHeight: 114
    activeFocusOnTab: true
    enabled: true

    function _filterMatches(url) {
        var filters = nameFilters
        if (nameFilter !== "")
            filters = nameFilter.split(/;;|\n/)
        if (!filters || filters.length === 0)
            return true

        var path = String(url).split("/").pop()
        for (var i = 0; i < filters.length; ++i) {
            var filter = String(filters[i])
            if (filter.indexOf("(*)") !== -1)
                return true
            var matches = filter.match(/\*\.([A-Za-z0-9_+-]+)/g)
            if (!matches || matches.length === 0)
                continue
            for (var j = 0; j < matches.length; ++j) {
                var suffix = matches[j].slice(1).toLowerCase()
                if (path.toLowerCase().slice(-suffix.length) === suffix)
                    return true
            }
        }
        return false
    }

    function _handleUrls(urls) {
        var accepted = []
        for (var i = 0; i < urls.length; ++i) {
            var url = String(urls[i])
            if (_filterMatches(url))
                accepted.push(url.indexOf("file://") === 0 ? url.slice(7) : url)
        }
        dropInvalid = accepted.length !== urls.length
        if (accepted.length === 1)
            getPath(accepted[0])
        else if (accepted.length > 1)
            getPathList(accepted)
    }

    function _urlsAreValid(urls) {
        if (!urls || urls.length === 0)
            return false
        for (var i = 0; i < urls.length; ++i) {
            if (!_filterMatches(urls[i]))
                return false
        }
        return true
    }

    function _handlePaths(paths) {
        var accepted = []
        for (var i = 0; i < paths.length; ++i) {
            if (_filterMatches(paths[i]))
                accepted.push(paths[i])
        }
        if (accepted.length === 1)
            getPath(accepted[0])
        else if (accepted.length > 1)
            getPathList(accepted)
    }

    function setNameFilter(filter) {
        nameFilter = filter
        nameFilters = filter === "" ? [] : filter.split(/;;|\n/)
    }

    function setNameFilters(filters) {
        nameFilters = filters
        nameFilter = filters.join(";;")
    }

    function openFile() {
        fileDialog.nameFilters = nameFilters
        fileDialog.open()
    }

    // 用 Canvas 绘制可随控件尺寸变化的圆角虚线边框。
    Canvas {
        id: borderCanvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var inset = 1
            var w = width - inset * 2
            var h = height - inset * 2
            if (w <= 0 || h <= 0)
                return

            var r = Math.min(root._radius, Math.min(w, h) / 2)
            ctx.beginPath()
            ctx.moveTo(inset + r, inset)
            ctx.lineTo(inset + w - r, inset)
            ctx.quadraticCurveTo(inset + w, inset, inset + w, inset + r)
            ctx.lineTo(inset + w, inset + h - r)
            ctx.quadraticCurveTo(inset + w, inset + h, inset + w - r, inset + h)
            ctx.lineTo(inset + r, inset + h)
            ctx.quadraticCurveTo(inset, inset + h, inset, inset + h - r)
            ctx.lineTo(inset, inset + r)
            ctx.quadraticCurveTo(inset, inset, inset + r, inset)
            ctx.closePath()
            ctx.setLineDash([4, 4])
            ctx.lineWidth = 1
            ctx.strokeStyle = root.enabled
                    ? (root.dropInvalid ? root._leaveLine : root._lineColor) : root._leaveLine
            ctx.stroke()
        }
    }

    MouseArea {
        id: interactionArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            root._lineColor = root._hoverLine
            borderCanvas.requestPaint()
        }
        onExited: {
            root._lineColor = root._leaveLine
            borderCanvas.requestPaint()
        }
        onPressed: {
            root._lineColor = root._hoverLine
            borderCanvas.requestPaint()
        }
        onClicked: root.openFile()
    }

    // 11px 是控件间距；加上 Text 的字体留白后，视觉间距约为 16px。
    Item {
        id: contentLayout
        anchors.centerIn: parent
        width: parent.width
        height: addButton.height + 11 + textLabelItem.implicitHeight
    }

    Rectangle {
        id: addButton
        parent: contentLayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: 36
        height: 36
        color: "transparent"

        Rectangle {
            anchors.fill: parent
            radius: 18
            color: !root.enabled ? root._plusDisableBackground
                                  : interactionArea.pressed ? root._plusClickBackground
                                  : interactionArea.containsMouse ? root._plusHoverBackground
                                  : root._plusBackground

            Private.KyIcon {
                anchors.centerIn: parent
                width: 16
                height: 16
                iconName: root.iconName
            }
        }
    }

    Text {
        id: textLabelItem
        parent: contentLayout
        anchors.top: addButton.bottom
        anchors.topMargin: 11
        anchors.left: parent.left
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        color: root.enabled ? parseColor.getDtColor("kfont-primary") : root._leaveLine
        text: root.text
    }

    DropArea {
        anchors.fill: parent
        keys: ["text/uri-list"]

        onEntered: {
            root.dropActive = true
            root.dropInvalid = !root._urlsAreValid(drag.urls)
            // 拒绝无效文件，让 Qt 显示禁止拖放光标。
            drag.accepted = !root.dropInvalid
            root._lineColor = root._dragLine
            borderCanvas.requestPaint()
        }
        onPositionChanged: {
            root.dropInvalid = !root._urlsAreValid(drag.urls)
            drag.accepted = !root.dropInvalid
            borderCanvas.requestPaint()
        }
        onExited: {
            root.dropActive = false
            root.dropInvalid = false
            root._lineColor = root._leaveLine
            borderCanvas.requestPaint()
        }
        onDropped: {
            root.dropActive = false
            root._lineColor = root._leaveLine
            root._handleUrls(drop.urls)
            borderCanvas.requestPaint()
        }
    }

    Private.KDragFileDialog {
        id: fileDialog
        onAccepted: root._handlePaths(paths)
    }

    Connections {
        target: app
        function onParametryChanged() { borderCanvas.requestPaint() }
    }

    Component.onCompleted: borderCanvas.requestPaint()
    onEnabledChanged: borderCanvas.requestPaint()
    onDropInvalidChanged: borderCanvas.requestPaint()
}
