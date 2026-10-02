/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KColorButton.
 * 三种样式 Circle/RoundedRect/CheckedRect，hover/checked 时绘制白色外圈或对勾。
 * 颜色对齐 kcolorbutton.cpp：默认 highlight-active，禁用 buttontext-disable。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.AbstractButton {
    id: control

    enum ButtonType { Circle = 0, RoundedRect = 1, CheckedRect = 2 }

    property int buttonType: KColorButton.ButtonType.RoundedRect
    property var backgroundColor: "transparent" // var：兼容 Qt::GlobalColor(int)/字符串/QColor
    property bool useCustomColor: false
    property int borderRadius: 6
    property color _fillColor: getDtColor("highlight-active")

    checkable: true
    hoverEnabled: true
    focusPolicy: Qt.NoFocus

    implicitWidth: buttonType === KColorButton.ButtonType.Circle ? 20 : 24
    implicitHeight: buttonType === KColorButton.ButtonType.Circle ? 20 : 24

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    Connections {
        target: app
        function onParametryChanged() {
            updateFillColor()
            updateBorderRadius()
        }
        function onIsDarkChanged() {
            updateFillColor()
        }
    }

    onEnabledChanged: updateFillColor()
    onUseCustomColorChanged: updateFillColor()
    onBackgroundColorChanged: updateFillColor()

    Component.onCompleted: {
        updateFillColor()
        updateBorderRadius()
    }

    function getDtColor(colorName) {
        return parseColor.getDtColor(colorName)
    }

    // 对齐 C++：setBackgroundColor(QColor) 可接收 Qt::GlobalColor(QColor 隐式构造)。
    // QML 中 Qt.darkYellow 等为 int，property color 无法直接赋值，故先转成 Qt 真实 RGB hex。
    // 注意：不用 SVG 颜色名，因为 SVG "green"=#008000 ≠ Qt::green=#00ff00，且 "darkyellow" 非标准名。
    function coerceColor(c) {
        if (typeof c === "number") {
            var rgb = [
                "#000000", "#ffffff", "#808080", "#a0a0a0", "#c0c0c0",
                "#ff0000", "#00ff00", "#0000ff", "#00ffff", "#ff00ff", "#ffff00",
                "#800000", "#008000", "#000080", "#008080", "#800080", "#808000"
            ]
            if (c === 19) return "transparent" // Qt::transparent
            if (c === 17) return "#000000"     // Qt::color0
            if (c === 18) return "#ffffff"     // Qt::color1
            return (c >= 0 && c < rgb.length) ? rgb[c] : "transparent"
        }
        return c
    }

    function updateFillColor() {
        if (!enabled)
            _fillColor = getDtColor("buttontext-disable")
        else if (useCustomColor)
            _fillColor = coerceColor(backgroundColor)
        else
            _fillColor = getDtColor("highlight-active")
    }

    function updateBorderRadius() {
        if (typeof ukui_platformtheme_token !== "undefined"
                && ukui_platformtheme_token
                && ukui_platformtheme_token.kradiusNormal > 0)
            borderRadius = ukui_platformtheme_token.kradiusNormal
    }

    readonly property bool _highlighted: checked || hovered

    background: Item {
        anchors.fill: parent

        // 外圈白边（仅 RoundedRect 在 hover/checked 时）
        Rectangle {
            id: outerRing
            anchors.fill: parent
            visible: control.buttonType === KColorButton.ButtonType.RoundedRect
                     && control._highlighted && control.enabled
            radius: control.borderRadius
            color: typeof ukui_platformtheme_token !== "undefined" ? ukui_platformtheme_token.kFontWhite : "white"
            antialiasing: true
        }

        Rectangle {
            id: fillRect
            anchors.centerIn: parent
            width: outerRing.visible ? parent.width - 4 : parent.width
            height: outerRing.visible ? parent.height - 4 : parent.height
            visible: control.buttonType !== KColorButton.ButtonType.Circle
            radius: outerRing.visible ? Math.max(0, control.borderRadius / 2) : control.borderRadius
            color: control._fillColor
            antialiasing: true

            // CheckedRect 选中时绘制白色对勾
            // 对齐 kcolorbutton.cpp：C++ 整数除法 (rect.width()*2/11) + QPen 默认 SquareCap/BevelJoin
            Canvas {
                id: checkMark
                anchors.fill: parent
                visible: control.buttonType === KColorButton.ButtonType.CheckedRect && control.checked && control.enabled
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.strokeStyle = typeof ukui_platformtheme_token !== "undefined" ? ukui_platformtheme_token.kFontWhite : "white"
                    ctx.lineWidth = 2
                    ctx.lineCap = "square"   // 对应 QPainter 默认 Qt::SquareCap，端点外延半个线宽
                    ctx.lineJoin = "bevel"   // 对应 QPainter 默认 Qt::BevelJoin
                    ctx.beginPath()
                    ctx.moveTo(Math.floor(width * 2 / 11), Math.floor(height * 6 / 11))
                    ctx.lineTo(Math.floor(width * 5 / 11), Math.floor(height * 8 / 11))
                    ctx.lineTo(Math.floor(width * 9 / 11), Math.floor(height * 4 / 11))
                    ctx.stroke()
                }
            }
        }

        // Circle 类型（对齐 kcolorbutton.cpp: rect.adjusted(1,1,0,0) + 内圆 adjusted(w/4,h/4,-w/4,-h/4)）
        Rectangle {
            id: circleFill
            x: 1
            y: 1
            width: parent.width - 1
            height: parent.height - 1
            visible: control.buttonType === KColorButton.ButtonType.Circle
            radius: Math.min(width, height) / 2
            color: control._fillColor
            antialiasing: true

            // 内白圆：复刻 C++ 整数除法 adjusted(w/4,...) ，外19 -> 内11 -> 环厚4
            Rectangle {
                x: Math.floor(parent.width / 4)
                y: Math.floor(parent.height / 4)
                width: parent.width - 2 * Math.floor(parent.width / 4)
                height: parent.height - 2 * Math.floor(parent.height / 4)
                radius: width / 2
                color: typeof ukui_platformtheme_token !== "undefined" ? ukui_platformtheme_token.kFontWhite : "white"
                visible: control._highlighted && control.enabled
            }
        }
    }

    contentItem: Item {}

    function setBackgroundColor(color) { useCustomColor = true; backgroundColor = color }
    function setButtonType(type) { buttonType = type }
    function setBorderRadius(r) { borderRadius = r }
}
