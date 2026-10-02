import QtQuick 2.15
import QtQuick.Layouts 1.15
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
    readonly property var desktopShell: root.parent
        && root.parent.peonyShell !== undefined ? root.parent.peonyShell : null
    readonly property int instanceId: (typeof Widget !== "undefined"
        && Widget.instanceId !== undefined) ? Widget.instanceId : -1
    readonly property string configuredImagePath: {
        if (previewMode || instanceId < 0
                || typeof Widget === "undefined" || !Widget.config)
            return ""
        var path = Widget.config.imageLocalFile
        if (path === undefined || path === null)
            return ""
        return String(path)
    }
    function restoreConfiguredImage() {
        backend.initializeImage(root.configuredImagePath)
    }
    readonly property bool canRemoveFromDesktop: !previewMode
        && instanceId >= 0
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
        Component.onCompleted: root.restoreConfiguredImage()
        onPersistedImagePathChanged: (localFile) => {
            if (root.previewMode || root.instanceId < 0
                    || typeof Widget === "undefined" || !Widget.config)
                return
            Widget.config["imageLocalFile"] = localFile
        }
    }
    onInstanceIdChanged: root.restoreConfiguredImage()
    onConfiguredImagePathChanged: root.restoreConfiguredImage()

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
                text: qsTr("Open failed")
                visible: backend.openFailed
                color: "#ffb3b3"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAnywhere
                font.pixelSize: 12
            }
        }
    }

    readonly property var sizeVariants: [
        { id: "small",  w: 136, h: 136 },
        { id: "medium", w: 296, h: 136 },
        { id: "large",  w: 296, h: 296 }
    ]
    property bool _smallVariantAvailable: false
    property bool _mediumVariantAvailable: false
    property bool _largeVariantAvailable: false

    function canSwitchToVariant(variantId) {
        if (previewMode) return false
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        if (sizeMode === variantId) return false
        if (!desktopShell || !_hasApi(desktopShell, "canResizeWidgetTo")) return false
        return desktopShell.canResizeWidgetTo(root, v.w, v.h)
    }

    function refreshVariantAvailability() {
        _smallVariantAvailable = canSwitchToVariant("small")
        _mediumVariantAvailable = canSwitchToVariant("medium")
        _largeVariantAvailable = canSwitchToVariant("large")
        syncSizeActionChecks()
    }

    function syncSizeActionChecks() {
        smallSizeAction.checked = Qt.binding(function() {
            return root.sizeMode === "small"
        })
        mediumSizeAction.checked = Qt.binding(function() {
            return root.sizeMode === "medium"
        })
        largeSizeAction.checked = Qt.binding(function() {
            return root.sizeMode === "large"
        })
    }

    function activateVariant(variantId) {
        if (sizeMode === variantId) {
            syncSizeActionChecks()
            return
        }

        Qt.callLater(function() {
            root.switchToVariant(variantId)
            root.syncSizeActionChecks()
            root.refreshVariantAvailability()
        })
    }

    function switchToVariant(variantId) {
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        if (!canSwitchToVariant(variantId)) return false

        if (typeof Widget !== "undefined" && Widget.config) {
            Widget.config["variantId"] = variantId
            Widget.config["width"]     = v.w
            Widget.config["height"]    = v.h
        }
        root.width = v.w
        root.height = v.h
        return true
    }

    Connections {
        target: (typeof Widget !== "undefined" && Widget.container)
                ? Widget.container : null

        function onActiveChanged() {
            if (target && target.active)
                root.refreshVariantAvailability()
        }
    }

    Component.onCompleted: Qt.callLater(root.refreshVariantAvailability)

    // 右键事件由桌面的 WidgetContainerItem 接收，并根据这些 Action
    // 使用 ukui-quick 的共享菜单机制统一显示。
    Widget.actions: [
        Action {
            text: qsTr("More plugins")
            iconName: "preferences-desktop-apps-symbolic"
            enabled: root.desktopShell !== null
            onTriggered: {
                if (root.desktopShell
                        && root._hasApi(root.desktopShell, "showWidgetManager")) {
                    root.desktopShell.showWidgetManager()
                }
            }
        },
        Action {
            text: qsTr("Change image")
            iconName: "folder-pictures-symbolic"
            enabled: !root.previewMode
            onTriggered: {
                backend.openImage()
            }
        },
        Action { isSeparator: true },
        Action {
            id: smallSizeAction
            text: qsTr("Small size")
            iconName: "small-symbolic"
            checkable: true
            checked: root.sizeMode === "small"
            enabled: !root.previewMode
                     && (root.sizeMode === "small"
                         || root._smallVariantAvailable)
            onTriggered: root.activateVariant("small")
        },
        Action {
            id: mediumSizeAction
            text: qsTr("Medium size")
            iconName: "medium-symbolic"
            checkable: true
            checked: root.sizeMode === "medium"
            enabled: !root.previewMode
                     && (root.sizeMode === "medium"
                         || root._mediumVariantAvailable)
            onTriggered: root.activateVariant("medium")
        },
        Action {
            id: largeSizeAction
            text: qsTr("Large size")
            iconName: "large-symbolic"
            checkable: true
            checked: root.sizeMode === "large"
            enabled: !root.previewMode
                     && (root.sizeMode === "large"
                         || root._largeVariantAvailable)
            onTriggered: root.activateVariant("large")
        },
        Action { isSeparator: true },
        Action {
            text: qsTr("Remove this widget")
            iconName: "zoom-out-symbolic"
            enabled: root.canRemoveFromDesktop
            onTriggered: {
                if (!root.canRemoveFromDesktop)
                    return
                root.desktopShell.removeWidgetInstance(root.instanceId)
            }
        }
    ]
}
