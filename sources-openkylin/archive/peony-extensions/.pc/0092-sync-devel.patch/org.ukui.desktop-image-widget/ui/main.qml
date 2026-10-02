import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.desktopimage 1.0

WidgetItem {
    id: root
    z: 2

    readonly property bool previewMode: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode
    function _hasApi(o, name) {
        if (!o) return false
        var fn = o[name]
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    function _hasRemoveWidgetApi(o) {
        if (!o)
            return false
        var fn = o.removeWidgetInstance
        if (fn === undefined || fn === null)
            return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    function _findRemoveShell() {
        if (root.parent && root.parent.peonyShell && _hasRemoveWidgetApi(root.parent.peonyShell))
            return root.parent.peonyShell
        if (typeof platformWindow !== "undefined" && platformWindow && _hasRemoveWidgetApi(platformWindow))
            return platformWindow
        var win = root.window
        if (win && _hasRemoveWidgetApi(win))
            return win
        if (typeof island !== "undefined" && island && _hasRemoveWidgetApi(island))
            return island
        var p = root.parent
        while (p) {
            if (_hasRemoveWidgetApi(p))
                return p
            if (p.peonyShell && _hasRemoveWidgetApi(p.peonyShell))
                return p.peonyShell
            if (p.island && _hasRemoveWidgetApi(p.island))
                return p.island
            if (typeof p.platformWindow !== "undefined" && p.platformWindow
                    && _hasRemoveWidgetApi(p.platformWindow))
                return p.platformWindow
            p = p.parent
        }
        return null
    }
    readonly property var desktopShell: _findRemoveShell()
    readonly property int resolvedInstanceId: {
        if (typeof Widget !== "undefined" && Widget.instanceId !== undefined && Widget.instanceId >= 0)
            return Widget.instanceId
        if (root.parent && root.parent.instanceid !== undefined)
            return root.parent.instanceid
        return -1
    }
    readonly property bool canRemoveFromDesktop: !previewMode
        && resolvedInstanceId >= 0
        && desktopShell !== null
    readonly property string variantFromConfig: {
        if (typeof Widget === "undefined" || !Widget.config)
            return ""
        var v = Widget.config.variantId
        if (v === undefined || v === null)
            return ""
        return String(v)
    }
    readonly property string variantFromGeometry: {
        var w = Math.round(root.width)
        var h = Math.round(root.height)
        if (w < 32 || h < 32)
            return ""
        if (w <= 180 && h <= 180)
            return "small"
        if (w >= 260 && h <= 180)
            return "medium"
        if (w >= 260 && h >= 260)
            return "large"
        return ""
    }
    readonly property string sizeMode: variantFromGeometry.length > 0
        ? variantFromGeometry
        : variantFromConfig
    Layout.fillWidth: true
    Layout.fillHeight: true

    ImageBackend {
        id: backend
    }

    readonly property bool showEmptyState: !backend.hasImage || backend.openFailed

    Rectangle {
        id: frameRect
        anchors.fill: parent
        radius: 16
        color: Qt.rgba(0.10, 0.16, 0.30, 0.92)
        border.color: Qt.rgba(1, 1, 1, 0.75)
        border.width: 2
        clip: true

        Item {
            id: imageViewport
            anchors.fill: parent
            anchors.margins: 2
            visible: backend.hasImage && !backend.openFailed

            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0.08, 0.12, 0.22, 1.0)
            }

            Image {
                id: sourceImage
                anchors.centerIn: parent
                width: parent.width
                height: parent.height
                source: backend.imageSource
                fillMode: Image.PreserveAspectCrop
                cache: false
                asynchronous: true
                smooth: true
                mipmap: true
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter

                onStatusChanged: {
                    if (status === Image.Error)
                        backend.reportImageLoadFailed()
                }
            }

            layer.enabled: true
            layer.smooth: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: imageViewport.width
                    height: imageViewport.height
                    radius: Math.max(0, frameRect.radius - 2)
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: Math.max(0, frameRect.radius - 2)
            color: Qt.rgba(0, 0, 0, 0.25)
            visible: root.showEmptyState
        }

        Column {
            anchors.centerIn: parent
            width: Math.min(parent.width - 24, 240)
            spacing: 10
            visible: root.showEmptyState

            Rectangle {
                width: parent.width
                height: 36
                radius: 8
                color: openMouse.pressed ? Qt.rgba(1, 1, 1, 0.26) : Qt.rgba(1, 1, 1, 0.2)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.8)

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Open")
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                }

                MouseArea {
                    id: openMouse
                    anchors.fill: parent
                    enabled: !root.previewMode
                    onClicked: backend.openImage()
                }
            }

            Text {
                width: parent.width
                text: backend.openFailed ? qsTr("Open failed") : backend.statusText
                visible: backend.openFailed || backend.statusText.length > 0
                color: "#ffb3b3"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAnywhere
                font.pixelSize: 12
            }
        }
    }

    property bool _smallVariantAvailable: false
    property bool _mediumVariantAvailable: false
    property bool _largeVariantAvailable: false
    readonly property var sizeVariants: [
        { id: "small",  w: 136, h: 136 },
        { id: "medium", w: 296, h: 136 },
        { id: "large",  w: 296, h: 296 }
    ]

    function _hasCanResizeApi(o) {
        if (!o) return false
        var fn = o.canResizeWidgetTo
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    function canSwitchToVariant(variantId) {
        if (previewMode) return false
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        if (sizeMode === variantId) return false
        if (!desktopShell || !_hasCanResizeApi(desktopShell)) return true
        return desktopShell.canResizeWidgetTo(root, root, v.w, v.h)
    }
    function refreshVariantAvailability() {
        _smallVariantAvailable = canSwitchToVariant("small")
        _mediumVariantAvailable = canSwitchToVariant("medium")
        _largeVariantAvailable = canSwitchToVariant("large")
    }
    function _hasClampPositionApi(o) {
        if (!o) return false
        var fn = o.clampWidgetPositionToScreen
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    function switchToVariant(variantId) {
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return
        if (!canSwitchToVariant(variantId)) return

        if (typeof Widget !== "undefined" && Widget.config) {
            Widget.config["variantId"] = variantId
            Widget.config["width"]     = v.w
            Widget.config["height"]    = v.h
        }
        root.width = v.w
        root.height = v.h

        if (desktopShell && _hasClampPositionApi(desktopShell)) {
            Qt.callLater(function() {
                desktopShell.clampWidgetPositionToScreen(root, v.w, v.h)
            })
        }
    }

    Loader {
        id: ctxMenuLoader
        width: 0
        height: 0
        active: false
        asynchronous: false
        source: Qt.resolvedUrl("image-widget-context-menu.qml")
        onLoaded: {
            item.host = root
            item.backend = backend
        }
    }

    function openImageContextMenu(mx, my) {
        refreshVariantAvailability()
        if (!ctxMenuLoader.active)
            ctxMenuLoader.active = true
        if (ctxMenuLoader.status !== Loader.Ready || !ctxMenuLoader.item)
            return
        ctxMenuLoader.item.host = root
        ctxMenuLoader.item.backend = backend

        var menuW = 260
        var menuH = (backend.hasImage && !backend.openFailed) ? 300 : 260
        if (ctxMenuLoader.item.menuImplicitWidth > 0)
            menuW = Math.max(ctxMenuLoader.item.menuImplicitWidth, 220)
        if (ctxMenuLoader.item.menuImplicitHeight > 0)
            menuH = Math.max(ctxMenuLoader.item.menuImplicitHeight, (backend.hasImage && !backend.openFailed) ? 280 : 240)
        var margin = 6

        // 与时钟插件保持一致：mapToGlobal → 可用屏幕几何裁剪（去任务栏）→ mapFromGlobal
        var globalClick = root.mapToGlobal(mx, my)
        var sLeft, sTop, sRight, sBottom
        if (root.desktopShell && _hasApi(root.desktopShell, "getScreenAvailableGeometry")) {
            var avail = root.desktopShell.getScreenAvailableGeometry(root)
            sLeft   = avail.x
            sTop    = avail.y
            sRight  = sLeft + avail.width
            sBottom = sTop  + avail.height
        } else {
            sLeft   = Screen.virtualX
            sTop    = Screen.virtualY
            sRight  = sLeft + Screen.desktopAvailableWidth
            sBottom = sTop  + Screen.desktopAvailableHeight
        }

        var gx = Math.max(sLeft + margin, Math.min(globalClick.x, sRight  - menuW - margin))
        var gy = Math.max(sTop  + margin, Math.min(globalClick.y, sBottom - menuH - margin))

        var localPos = root.mapFromGlobal(gx, gy)
        ctxMenuLoader.item.openAt(localPos.x, localPos.y)
    }

    MouseArea {
        anchors.fill: parent
        z: 100000
        enabled: !root.previewMode
        acceptedButtons: Qt.RightButton
        onPressed: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                openImageContextMenu(mouse.x, mouse.y)
                mouse.accepted = true
            }
        }
    }
}
