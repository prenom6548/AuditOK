/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KPushButton.
 * 根植于 QtQuick.Controls Button（org.ukui.style 样式自动提供 content/图标/文本色/loading），
 * 仅覆盖 background：用 Canvas 按 C++ QPainterPath（kpushbutton.cpp:842-851）画四角独立圆角。
 * 配色与焦点逻辑完全照搬 BackGroundRectangle.changeColor（normal/highlighted/close/window/import/transparent
 * 全分支，用 UKUIButton 的 *BC/*Border* token + ParseColorInterface.startColor/endColor 取渐变首末色）。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

Button {
    id: control

    // 对齐 Button.qml 的 buttonType 字符串约定（closeButton/windowButton/importButton）
    property string buttonType: ""
    property bool transparent: false

    // 四角独立圆角（对齐 C++ m_topLeft/m_bottomLeft/m_topRight/m_bottomRight）
    property int topLeftRadius: 0
    property int topRightRadius: 0
    property int bottomLeftRadius: 0
    property int bottomRightRadius: 0

    property color backgroundColor: "transparent"
    property bool useCustomColor: false

    // 分割线画在按钮自身边缘上（无间隙）：dividerRight 右边竖线、dividerBottom 底边横线
    // 由 KButtonBox 按位置设；色 kline-normal，距两端各 5px
    property bool dividerRight: false
    property bool dividerBottom: false

    // 加载态：沿用样式 Button 的 isLoadBtn/isLoading 约定（ButtonIconLabelContent 自带 BusyIndicator）
    property bool isLoadBtn: true
    property bool isLoading: false

    // 菜单按钮：hasMenu=true 时样式 Button 显下拉箭头（ukui-down-symbolic）；菜单弹出由调用方 onClicked 触发
    property bool hasMenu: false

    // 焦点状态机（对齐 BackGroundRectangle：visualFocus 进入置 true，按下抑制）
    property bool focusEnable: false
    readonly property bool showFocusBorder: app.focusEnable && control.focusEnable

    StylePrivate.APPParameter { id: app }
    StylePrivate.UKUIButton { id: btn }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property string _stateKey: (control.enabled ? 1 : 0) + "," + (control.hovered ? 1 : 0) + ","
            + (control.pressed ? 1 : 0) + "," + (control.checked ? 1 : 0) + "," + (control.checkable ? 1 : 0) + ","
            + (control.visualFocus ? 1 : 0) + "," + (control.focusEnable ? 1 : 0) + "," + (control.showFocusBorder ? 1 : 0) + ","
            + (control.highlighted ? 1 : 0) + "," + (control.flat ? 1 : 0) + "," + control.buttonType + ","
            + (control.transparent ? 1 : 0) + "," + (control.useCustomColor ? 1 : 0) + "," + control.backgroundColor + ","
            + (control.isLoading ? 1 : 0) + "," + (control.dividerRight ? 1 : 0) + "," + (control.dividerBottom ? 1 : 0)

    function setButtonType(type) { buttonType = type }
    function setTranslucent(flag) { transparent = flag }
    function setBackgroundColor(c) { useCustomColor = true; backgroundColor = c }
    function setLoaingStatus(flag) { isLoading = flag }

    // 对齐 C++ 重载：setBorderRadius(r) 四角同值；setBorderRadius(bl,tl,tr,br) 四角独立
    function setBorderRadius(bl, tl, tr, br) {
        if (tl === undefined) {
            var r = Math.max(0, bl)
            topLeftRadius = r
            topRightRadius = r
            bottomLeftRadius = r
            bottomRightRadius = r
        } else {
            bottomLeftRadius = Math.max(0, bl)
            topLeftRadius = Math.max(0, tl)
            topRightRadius = Math.max(0, tr)
            bottomRightRadius = Math.max(0, br)
        }
    }

    background: Canvas {
        id: bgCanvas
        anchors.fill: parent
        antialiasing: true

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        // 样式 Button 的 onEnabledChanged 会调 back.changeColor(...)；back 现在是本 Canvas，提供空实现兜住
        function changeColor() {}

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var w = width
            var h = height
            if (w <= 0 || h <= 0)
                return

            var enable = control.enabled
            var click = control.pressed
            var hover = control.hovered
            var checked = control.checkable && control.checked
            var flat = control.flat

            // ---- 照搬 BackGroundRectangle.changeColor：按 buttonType 选 token 集，再按状态选首末色/边框色 ----
            var closeBtn = (control.buttonType === "closeButton" || control.buttonType === "windowTitleBarCloseButton")
            var windowBtn = (control.buttonType === "windowButton" || control.buttonType === "windowTitleBarWindowButton")
            var importBtn = (control.buttonType === "importButton")
            var hl = control.highlighted || importBtn

            // 给定一组 normal/clicked/hovered/disable 的填充与边框 token，按状态返回 [start,end,border]
            function pick(nBC, cBC, hBC, dBC, nB, cB, hB, dB) {
                if (!enable)
                    return [parseColor.startColor(dBC), parseColor.endColor(dBC), parseColor.startColor(dB)]
                if (click || checked)
                    return [parseColor.startColor(cBC), parseColor.endColor(cBC), parseColor.startColor(cB)]
                if (hover)
                    return [parseColor.startColor(hBC), parseColor.endColor(hBC), parseColor.startColor(hB)]
                return [parseColor.startColor(nBC), parseColor.endColor(nBC), parseColor.startColor(nB)]
            }

            var s
            if (hl)
                s = pick(btn.normalHBC, btn.clickedHBC, btn.hoveredHBC, btn.disableHBC,
                         btn.normalBorderHColor, btn.clickBorderHColor, btn.hoverBorderHColor, btn.disableBorderHColor)
            else if (closeBtn)
                s = pick(btn.normalCloseBC, btn.clickedCloseBC, btn.hoveredCloseBC, btn.disableCloseBC,
                         btn.normalBorderCloseColor, btn.clickBorderCloseColor, btn.hoverBorderCloseColor, btn.disableBorderCloseColor)
            else if (windowBtn && !control.transparent)
                s = pick(btn.normalWindowBC, btn.clickedWindowBC, btn.hoveredWindowBC, btn.disableWindowBC,
                         btn.normalBorderWindowColor, btn.clickBorderWindowColor, btn.hoverBorderWindowColor, btn.disableBorderWindowColor)
            else if (windowBtn && control.transparent)
                s = pick(btn.normalWindowTransparentBC, btn.clickedWindowTransparentBC, btn.hoveredWindowTransparentBC, btn.disableWindowTransparentBC,
                         btn.normalBorderWindowTransparentColor, btn.clickedBorderWindowTransparentColor, btn.hoveredBorderWindowTransparentColor, btn.disableBorderWindowTransparentColor)
            else if (control.transparent)
                s = pick(btn.normalTransparentBC, btn.clickedTransparentBC, btn.hoveredTransparentBC, btn.disableTransparentBC,
                         btn.normalBorderTransparentBC, btn.clickedBorderTransparentBC, btn.hoveredBorderTransparentBC, btn.disableBorderTransparentBC)
            else
                s = pick(btn.normalBC, btn.clickedBC, btn.hoveredBC, btn.disableBC,
                         btn.normalBorderColor, btn.clickBorderColor, btn.hoverBorderColor, btn.disableBorderColor)

            var startC = s[0]
            var endC = s[1]
            var borderC = s[2]
            var focusC = parseColor.startColor(btn.focusBorderColor)

            // flat 且未交互：透明填充（对齐 BackGroundRectangle）
            if (flat && !click && !hover && !checked) {
                startC = "transparent"
                endC = "transparent"
            }
            // 自定义背景色覆盖填充
            if (control.useCustomColor) {
                startC = control.backgroundColor
                endC = control.backgroundColor
            }

            // ---- 四角路径（对齐 C++ QPainterPath）----
            var max = Math.min(w, h) / 2
            var rtl = Math.min(control.topLeftRadius, max)
            var rtr = Math.min(control.topRightRadius, max)
            var rbl = Math.min(control.bottomLeftRadius, max)
            var rbr = Math.min(control.bottomRightRadius, max)

            function buildPath(inset) {
                var x0 = inset, y0 = inset, x1 = w - inset, y1 = h - inset
                ctx.beginPath()
                ctx.moveTo(x0, y0 + rtl)
                ctx.lineTo(x0, y1 - rbl)
                ctx.quadraticCurveTo(x0, y1, x0 + rbl, y1)
                ctx.lineTo(x1 - rbr, y1)
                ctx.quadraticCurveTo(x1, y1, x1, y1 - rbr)
                ctx.lineTo(x1, y0 + rtr)
                ctx.quadraticCurveTo(x1, y0, x1 - rtr, y0)
                ctx.lineTo(x0 + rtl, y0)
                ctx.quadraticCurveTo(x0, y0, x0, y0 + rtl)
                ctx.closePath()
            }

            // 填充：铺满矩形 + 垂直渐变（对齐 BackGroundRectangle 的 QML Gradient）
            buildPath(0)
            var grad = ctx.createLinearGradient(0, 0, 0, h)
            grad.addColorStop(0.0, startC)
            grad.addColorStop(1.0, endC)
            ctx.fillStyle = grad
            ctx.fill()

            // 描边：内缩 lw/2，居中描边落在内侧 [0,lw]，粗细 = lw（对齐 Rectangle.border）
            var lw = control.showFocusBorder ? btn.focusBorderWidth : btn.borderWidth
            if (lw > 0) {
                buildPath(lw / 2)
                ctx.lineWidth = lw
                ctx.strokeStyle = control.showFocusBorder ? focusC : borderC
                ctx.stroke()
            }

            // 分割线画在按钮自身边缘（无间隙）：右边竖线 / 底边横线，色 kline-normal，距两端 5px
            var divC = parseColor.getDtColor("kline-normal")
            ctx.fillStyle = divC
            if (control.dividerRight && h > 10)
                ctx.fillRect(w - 1, 5, 1, h - 10)
            if (control.dividerBottom && w > 10)
                ctx.fillRect(5, h - 1, w - 10, 1)
        }
    }

    onVisualFocusChanged: focusEnable = control.visualFocus
    on_StateKeyChanged: bgCanvas.requestPaint()
    onTopLeftRadiusChanged: bgCanvas.requestPaint()
    onTopRightRadiusChanged: bgCanvas.requestPaint()
    onBottomLeftRadiusChanged: bgCanvas.requestPaint()
    onBottomRightRadiusChanged: bgCanvas.requestPaint()

    Connections {
        target: control
        function onPressedChanged() {
            if (control.pressed)
                focusEnable = false
            bgCanvas.requestPaint()
        }
    }

    Connections {
        target: app
        function onParametryChanged() { bgCanvas.requestPaint() }
        function onFocusEnableChanged() { bgCanvas.requestPaint() }
    }
}
