import QtQuick 2.15
import QtQuick.Layouts 1.15
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
    // C++ Q_INVOKABLE 在 QML 中可能表现为 object，不能只判断 typeof === "function"。
    function _hasApi(o, name) {
        if (!o) return false
        var fn = o[name]
        if (fn === undefined || fn === null) return false
        var t = typeof fn
        return t === "function" || t === "object"
    }
    // WidgetsLayout delegate 为桌面实例提供 peonyShell；管理器预览没有该属性。
    readonly property var desktopShell: root.parent
        && root.parent.peonyShell !== undefined ? root.parent.peonyShell : null
    readonly property int instanceId: (typeof Widget !== "undefined"
        && Widget.instanceId !== undefined) ? Widget.instanceId : -1
    readonly property bool canRemoveFromDesktop: !previewMode
        && instanceId >= 0
        && desktopShell !== null
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
    readonly property string sizeMode: variantFromGeometry.length > 0 ? variantFromGeometry
        : (variantFromConfig.length > 0 ? variantFromConfig
                                        : (shortSide >= 180 ? "large" : (shortSide >= 96 ? "medium" : "small")))
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
        Component.onCompleted: backend.setWidgetInstanceId(root.instanceId)
    }
    onInstanceIdChanged: backend.setWidgetInstanceId(root.instanceId)

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

    // 三种尺寸对应 Peony ManagerSizes 的标准桌面尺寸
    readonly property var sizeVariants: [
        { id: "small",  w: 136, h: 136 },
        { id: "medium", w: 296, h: 136 },
        { id: "large",  w: 296, h: 296 }
    ]
    property bool _smallVariantAvailable: false
    property bool _mediumVariantAvailable: false
    property bool _largeVariantAvailable: false

    // 检查是否可以切换到指定变体（有空间且不是当前尺寸）
    function canSwitchToVariant(variantId) {
        if (previewMode) return false
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        if (sizeMode === variantId) return false
        if (!desktopShell || !_hasApi(desktopShell, "canResizeWidgetTo")) return false
        return desktopShell.canResizeWidgetTo(root, v.w, v.h)
    }

    // WidgetContainerItem 在共享菜单 popup() 前会激活容器。
    // 此时重新计算，保证置灰状态使用插件当前位置和最新占用区域。
    function refreshVariantAvailability() {
        _smallVariantAvailable = canSwitchToVariant("small")
        _mediumVariantAvailable = canSwitchToVariant("medium")
        _largeVariantAvailable = canSwitchToVariant("large")
        syncSizeActionChecks()
    }

    // QAction 再次触发已选项时会自动反选；重新建立 checked 绑定，
    // 让菜单始终准确反映插件当前尺寸。
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

    // 切换到指定变体尺寸，持久化配置
    // 直接更新 WidgetItem，桌面 delegate 的尺寸绑定会同步跟随。
    function switchToVariant(variantId) {
        var v = sizeVariants.find(function(x) { return x.id === variantId })
        if (!v) return false
        if (!canSwitchToVariant(variantId)) return false

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

    // 由 ukui-quick 的 WidgetContainerItem 统一创建并弹出菜单。
    // 插件不再创建 QtQuick.Controls Menu、Loader 或顶层窗口。
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
            text: qsTr("Switch timezone")
            iconName: "preferences-system-time-symbolic"
            onTriggered: backend.openTimezoneChooser()
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
                backend.removeInstanceConfig()
                root.desktopShell.removeWidgetInstance(root.instanceId)
            }
        }
    ]
}
