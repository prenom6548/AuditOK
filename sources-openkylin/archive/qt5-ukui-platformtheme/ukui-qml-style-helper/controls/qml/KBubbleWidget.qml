/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBubbleWidget.
 * 带三角形尾部的气泡容器。尾部方向/位置、圆角、透明度、高亮背景色可配。
 * 背景默认 window-active，highlightBackground 时用 kbrand-normal。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.qqc2style.private 1.1 as StylePrivate

T.Control {
    id: control

    enum TailDirection { TopDirection = 0, LeftDirection = 1, BottomDirection = 2, RightDirection = 3, None = 4 }
    enum TailLocation { LeftLocation = 0, MiddleLocation = 1, RightLocation = 2 }

    property int tailDirection: KBubbleWidget.TailDirection.None
    property int tailLocation: KBubbleWidget.TailLocation.MiddleLocation
    property size tailSize: Qt.size(16, 16)
    property int borderRadius: (typeof ukui_platformtheme_token !== "undefined"
                                && ukui_platformtheme_token
                                && ukui_platformtheme_token.kradiusNormal > 0)
                               ? ukui_platformtheme_token.kradiusNormal : 6
    property bool enableBlur: false
    property real bubbleOpacity: 1.0
    property bool highlightBackground: false
    property alias contentItem: contentHost.sourceComponent
    property Component content

    font: app.font
    palette: app.palette

    implicitWidth: 200
    implicitHeight: 80

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property color _bgColor: highlightBackground
            ? parseColor.getDtColor("kbrand-normal")
            : parseColor.getDtColor("window-active")

    background: Item {
        anchors.fill: parent
        opacity: control.bubbleOpacity

        // 尾部三角形
        Canvas {
            id: tail
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = control._bgColor
                var w = control.tailSize.width
                var h = control.tailSize.height
                var W = width, H = height
                ctx.beginPath()
                if (control.tailDirection === KBubbleWidget.TailDirection.TopDirection) {
                    var x = control.tailLocation === KBubbleWidget.TailLocation.LeftLocation ? w
                            : control.tailLocation === KBubbleWidget.TailLocation.RightLocation ? W - 2 * w
                            : W / 2 - w / 2
                    ctx.moveTo(x, h)
                    ctx.lineTo(x + w / 2, 0)
                    ctx.lineTo(x + w, h)
                } else if (control.tailDirection === KBubbleWidget.TailDirection.BottomDirection) {
                    var xb = control.tailLocation === KBubbleWidget.TailLocation.LeftLocation ? w
                            : control.tailLocation === KBubbleWidget.TailLocation.RightLocation ? W - 2 * w
                            : W / 2 - w / 2
                    ctx.moveTo(xb, H - h)
                    ctx.lineTo(xb + w / 2, H)
                    ctx.lineTo(xb + w, H - h)
                } else if (control.tailDirection === KBubbleWidget.TailDirection.LeftDirection) {
                    var y = control.tailLocation === KBubbleWidget.TailLocation.LeftLocation ? h
                            : control.tailLocation === KBubbleWidget.TailLocation.RightLocation ? H - 2 * h
                            : H / 2 - h / 2
                    ctx.moveTo(h, y)
                    ctx.lineTo(0, y + h / 2)
                    ctx.lineTo(h, y + h)
                } else if (control.tailDirection === KBubbleWidget.TailDirection.RightDirection) {
                    var yr = control.tailLocation === KBubbleWidget.TailLocation.LeftLocation ? h
                            : control.tailLocation === KBubbleWidget.TailLocation.RightLocation ? H - 2 * h
                            : H / 2 - h / 2
                    ctx.moveTo(W - h, yr)
                    ctx.lineTo(W, yr + h / 2)
                    ctx.lineTo(W - h, yr + h)
                }
                ctx.closePath()
                ctx.fill()
            }
            visible: control.tailDirection !== KBubbleWidget.TailDirection.None
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: control.tailDirection !== KBubbleWidget.TailDirection.None ? control.tailSize.height / 2 : 0
            radius: control.borderRadius
            color: control._bgColor
            border.width: 1
            border.color: parseColor.getDtColor("kline-window-inactive")
            antialiasing: true
            opacity: control.bubbleOpacity
        }
    }

    onTailDirectionChanged: tail.requestPaint()
    onTailLocationChanged: tail.requestPaint()
    onHighlightBackgroundChanged: tail.requestPaint()

    contentItem: Item {
        id: contentHost
        anchors.fill: parent
        anchors.margins: control.tailDirection !== KBubbleWidget.TailDirection.None ? control.tailSize.height / 2 + 4 : 4
        property Component sourceComponent: control.content
        Loader { sourceComponent: contentHost.sourceComponent }
    }

    function setTailSize(s) { tailSize = s }
    function setTailPosition(dir, loc) { tailDirection = dir; tailLocation = loc }
    function setBorderRadius(r) { borderRadius = r }
    function setEnableBlur(flag) { enableBlur = flag }
    function setOpacity(o) { bubbleOpacity = o }
    function setHighlightBackground(flag) { highlightBackground = flag }
}
