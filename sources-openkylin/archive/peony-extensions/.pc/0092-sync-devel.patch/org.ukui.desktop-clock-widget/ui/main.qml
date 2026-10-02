import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "."
import org.ukui.quick.widgets 1.0
import org.ukui.quick.items 1.0
import org.ukui.desktopclock 1.0

WidgetItem {
    id: root
    z: 2

    FontLoader {
        id: dmSerifDisplayFont
        // 随 libdesktopclockbackend.so 内嵌资源加载，避免 main.qml 在 ui/ 或任意 URL 下相对路径失效
        source: "qrc:/org/ukui/desktopclock/fonts/DMSerifDisplay-Regular.ttf"
    }

    readonly property bool previewMode: (typeof widgetPreviewMode !== "undefined") && widgetPreviewMode
    // C++ Q_INVOKABLE 在 QML 里用 typeof === "function" 判断经常为 false，导致 desktopShell 恒为 null、菜单灰色不可用
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
        // WidgetsLayout 桌面 delegate 上暴露的 island（PlatformWindow），插件 QML 引擎无 platformWindow 时仍可用
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
    // 时区持久化仅使用 Widget.instanceId，避免 parent.instanceid 在新增实例阶段误关联到其它实例配置
    readonly property int timezoneInstanceId: {
        if (typeof Widget !== "undefined" && Widget.instanceId !== undefined && Widget.instanceId >= 0)
            return Widget.instanceId
        return -1
    }
    readonly property bool canRemoveFromDesktop: !previewMode
        && resolvedInstanceId >= 0
        && desktopShell !== null
    readonly property string previewVariantId: (typeof widgetPreviewVariantId !== "undefined") ? widgetPreviewVariantId : ""
    readonly property real shortSide: Math.min(width, height)
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
    // 像素矩形能明确区分三档时以几何为准；若优先 config，易出现「配置仍为 large 而实际已是小格」，
    // 导致 sizeMode 误判为大尺寸，右键只剩「缩到小」可用。
    readonly property string resolvedVariant: {
        if (variantFromGeometry.length > 0)
            return variantFromGeometry
        if (variantFromConfig.length > 0)
            return variantFromConfig
        return ""
    }
    readonly property string autoMode: {
        if (variantFromGeometry.length > 0)
            return variantFromGeometry
        return shortSide >= 180 ? "large" : (shortSide >= 96 ? "medium" : "small")
    }
    readonly property string sizeMode: (previewMode && previewVariantId.length > 0) ? previewVariantId
        : (resolvedVariant.length > 0 ? resolvedVariant : autoMode)
    readonly property int innerChromeMargin: (sizeMode === "small") ? 4 : 8
    readonly property int dialInset: (sizeMode === "small") ? 4 : 8
    readonly property int mediumTimeFontPx: 50
    readonly property int mediumDateFontPx: 12
    readonly property string mediumLocalCaption: Qt.locale().name.indexOf("zh") === 0
        ? qsTr("Beijing Time")
        : qsTr("Local Time")
    readonly property string fontMediumTime: (dmSerifDisplayFont.status === FontLoader.Ready)
        ? dmSerifDisplayFont.name
        : "DM Serif Display"
    readonly property string fontAnalogDigits: "OPPOSans"
    // 中等尺寸：文字区相对卡片内缘的左边距（与 innerChromeMargin 叠加后对齐到像素）
    readonly property int mediumTextLeftFromCard: 21
    readonly property bool mediumModeActive: sizeMode === "medium"

    Layout.fillWidth: true
    Layout.fillHeight: true

    ClockBackend {
        id: backend
        Component.onCompleted: backend.setWidgetInstanceId(root.timezoneInstanceId)
    }
    onTimezoneInstanceIdChanged: backend.setWidgetInstanceId(root.timezoneInstanceId)

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: "#F4F5F7"
        border.width: 1
        border.color: "#D9DEE7"
    }

    Item {
        anchors.fill: parent
        anchors.margins: innerChromeMargin

        Item {
            anchors.fill: parent
            opacity: mediumModeActive ? 0 : 1
            scale: mediumModeActive ? 0.985 : 1
            visible: opacity > 0.001

            Behavior on opacity {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on scale {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutCubic
                }
            }

            AnalogClockFace {
                id: analogMain
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height) - dialInset
                height: width
                sizeMode: root.sizeMode
                hour: backend.hour
                minute: backend.minute
                second: backend.second
            }

            Text {
                text: "12"
                anchors.horizontalCenter: analogMain.horizontalCenter
                anchors.top: analogMain.top
                anchors.topMargin: Math.round(analogMain.height * (sizeMode === "large" ? 0.15 : 0.12))
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: sizeMode === "large" ? 28 : 12
                font.bold: true
            }
            Text {
                text: "3"
                anchors.verticalCenter: analogMain.verticalCenter
                anchors.right: analogMain.right
                anchors.rightMargin: Math.round(analogMain.width * (sizeMode === "large" ? 0.15 : 0.12))
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: sizeMode === "large" ? 28 : 12
                font.bold: true
            }
            Text {
                text: "6"
                anchors.horizontalCenter: analogMain.horizontalCenter
                anchors.bottom: analogMain.bottom
                anchors.bottomMargin: Math.round(analogMain.height * (sizeMode === "large" ? 0.15 : 0.12))
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: sizeMode === "large" ? 28 : 12
                font.bold: true
            }
            Text {
                text: "9"
                anchors.verticalCenter: analogMain.verticalCenter
                anchors.left: analogMain.left
                anchors.leftMargin: Math.round(analogMain.width * (sizeMode === "large" ? 0.15 : 0.12))
                color: "#2F3440"
                font.family: root.fontAnalogDigits
                font.pixelSize: sizeMode === "large" ? 28 : 12
                font.bold: true
            }
        }

        Row {
            id: mediumRow
            anchors.left: parent.left
            anchors.leftMargin: Math.max(0, mediumTextLeftFromCard - innerChromeMargin)
            anchors.right: parent.right
            anchors.rightMargin: innerChromeMargin
            anchors.verticalCenter: parent.verticalCenter
            spacing: Math.max(8, width * 0.03)
            opacity: mediumModeActive ? 1 : 0
            scale: mediumModeActive ? 1 : 0.985
            visible: opacity > 0.001

            Behavior on opacity {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on scale {
                NumberAnimation {
                    duration: 220
                    easing.type: Easing.OutCubic
                }
            }

            Column {
                id: mediumTextColumn
                width: mediumRow.width - mediumRow.spacing - mediumDial.width
                spacing: Math.max(4, mediumRow.height * 0.03)
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    text: backend.zoneText.length > 0 ? backend.zoneText : root.mediumLocalCaption
                    color: "#A2A2A2"
                    font.pixelSize: Math.max(10, Math.round(mediumRow.width * 0.048))
                    font.bold: true
                    elide: Text.ElideRight
                    width: parent.width
                }
                Text {
                    text: backend.timeDisplayText
                    color: "#3F3F3F"
                    font.family: root.fontMediumTime
                    font.pixelSize: mediumTimeFontPx
                    // 仅内置 Regular 时 bold 易触发替换成系统默认字体族
                    font.bold: false
                    elide: Text.ElideRight
                    width: parent.width
                }
                Row {
                    width: parent.width
                    spacing: Math.max(8, width * 0.06)
                    Text {
                        text: backend.weekText
                        color: "#3F3F3F"
                        font.pixelSize: mediumDateFontPx
                        font.bold: true
                    }
                    Text {
                        text: backend.monthDayText
                        color: "#4B4B4B"
                        font.pixelSize: mediumDateFontPx
                    }
                }
            }

            Item {
                id: mediumDial
                width: Math.min(parent.height - 2, parent.width * 0.44)
                height: width
                anchors.verticalCenter: parent.verticalCenter

                AnalogClockFace {
                    id: analogMedium
                    anchors.fill: parent
                    sizeMode: "medium"
                    hour: backend.hour
                    minute: backend.minute
                    second: backend.second
                }

                Text { text: "12"; anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; anchors.topMargin: Math.round(parent.height * 0.14); color: "#22252D"; font.pixelSize: Math.max(9, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "3"; anchors.verticalCenter: parent.verticalCenter; anchors.right: parent.right; anchors.rightMargin: Math.round(parent.width * 0.14); color: "#22252D"; font.pixelSize: Math.max(9, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "6"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: Math.round(parent.height * 0.14); color: "#22252D"; font.pixelSize: Math.max(9, Math.round(parent.width * 0.14)); font.bold: true }
                Text { text: "9"; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: Math.round(parent.width * 0.14); color: "#22252D"; font.pixelSize: Math.max(9, Math.round(parent.width * 0.14)); font.bold: true }
            }
        }
    }

    function _repaintDialsAfterLayout() {
        analogMain.repaintTicks()
        analogMedium.repaintTicks()
        Qt.callLater(function() {
            analogMain.repaintTicks()
            analogMedium.repaintTicks()
        })
    }

    Connections {
        target: root
        function onSizeModeChanged() {
            _repaintDialsAfterLayout()
        }
    }

    property bool _smallVariantAvailable: false
    property bool _mediumVariantAvailable: false
    property bool _largeVariantAvailable: false

    // 三种尺寸对应 metadata.json ManagerVariants 中的 desktopSize
    readonly property var sizeVariants: [
        { id: "small",  w: 136, h: 136 },
        { id: "medium", w: 296, h: 136 },
        { id: "large",  w: 296, h: 296 }
    ]

    // 判断 desktopShell 是否支持指定 API
    function _hasApi(o, name) {
        if (!o) return false
        var fn = o[name]
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }

    // 判断 desktopShell 是否支持 canResizeWidgetTo API
    function _hasCanResizeApi(o) {
        if (!o) return false
        var fn = o.canResizeWidgetTo
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    // 检查是否可以切换到指定变体（有空间且不是当前尺寸）
    function canSwitchToVariant(variantId) {
        if (previewMode) return false
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        // 已经是当前尺寸则不需要切换
        if (sizeMode === variantId) return false
        // 没有 shell 或不支持 API 则默认允许（降级）
        if (!desktopShell || !_hasCanResizeApi(desktopShell)) return true
        // 第一个参数需非空：个别时机下 root.parent 尚未就绪会导致 C++ 侧直接 return false；
        // 插件根 root 恒为有效 QQuickItem，且与第二参数同为 WidgetItem，便于 C++ 侧 qobject_cast 排除自身。
        return desktopShell.canResizeWidgetTo(root, root, v.w, v.h)
    }

    // 每次打开菜单前按当前位置重算尺寸切换可用性，避免拖拽后菜单沿用旧状态
    function refreshVariantAvailability() {
        _smallVariantAvailable = canSwitchToVariant("small")
        _mediumVariantAvailable = canSwitchToVariant("medium")
        _largeVariantAvailable = canSwitchToVariant("large")
    }

    // 判断 desktopShell 是否支持 clampWidgetPositionToScreen API
    function _hasClampPositionApi(o) {
        if (!o) return false
        var fn = o.clampWidgetPositionToScreen
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }

    // 切换到指定变体尺寸，持久化配置
    // 尺寸切换动画由桌面层（WidgetsLayout.qml）的 Behavior on width/height 统一驱动，
    // 此处只需直接赋值，无需在插件内部启动动画
    function switchToVariant(variantId) {
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return
        if (!canSwitchToVariant(variantId)) return

        // 先更新 variantId，使 sizeMode 在尺寸变化前切换到目标 UI 模式，
        // variantFromGeometry 在中间尺寸时返回 ""，自动回退到 variantFromConfig，
        // 保证内容布局与目标尺寸一致，动画过程中不会出现错误的布局状态。
        if (typeof Widget !== "undefined" && Widget.config) {
            Widget.config["variantId"] = variantId
            Widget.config["width"]     = v.w
            Widget.config["height"]    = v.h
        }

        root.width  = v.w
        root.height = v.h

        // 切换后钳夹位置，避免在边缘时新尺寸超出屏幕
        if (desktopShell && _hasClampPositionApi(desktopShell)) {
            Qt.callLater(function() {
                desktopShell.clampWidgetPositionToScreen(root, v.w, v.h)
            })
        }
    }

    // 右键菜单独立 QML + Loader 延迟加载，避免 loadWidget 时即创建 Menu 导致添加/拖放失败。
    Loader {
        id: ctxMenuLoader
        width: 0
        height: 0
        active: false
        asynchronous: false
        source: Qt.resolvedUrl("clock-widget-context-menu.qml")
        onLoaded: {
            item.host = root
            item.backend = backend
        }
    }

    function openClockContextMenu(mx, my) {
        refreshVariantAvailability()
        if (!ctxMenuLoader.active)
            ctxMenuLoader.active = true
        if (ctxMenuLoader.status !== Loader.Ready || !ctxMenuLoader.item)
            return
        ctxMenuLoader.item.host = root
        ctxMenuLoader.item.backend = backend

        var menuW = 260
        var menuH = 280
        if (ctxMenuLoader.item.menuImplicitWidth > 0)
            menuW = Math.max(ctxMenuLoader.item.menuImplicitWidth, 220)
        if (ctxMenuLoader.item.menuImplicitHeight > 0)
            menuH = Math.max(ctxMenuLoader.item.menuImplicitHeight, 240)
        var margin = 6

        // 边界裁剪基于 mapToGlobal（正确折算镜像 Translate）+ 可用屏幕几何。
        // 优先从 desktopShell.getScreenAvailableGeometry() 获取去掉任务栏后的可用区域，
        // 这样即使插件在底部也不会被任务栏遮挡；降级时退回全屏幕尺寸。
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
                openClockContextMenu(mouse.x, mouse.y)
                mouse.accepted = true
            }
        }
    }
}
