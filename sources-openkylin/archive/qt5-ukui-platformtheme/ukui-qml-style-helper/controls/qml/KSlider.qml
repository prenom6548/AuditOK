/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KSlider。
 * 配色对齐 org.ukui.style/Slider.qml（非 touchSlider）/ UKUISlider token：
 *  - unGroove：kcomponent-normal（translucent -> kcomponent-alpha-normal）
 *  - groove(cover)：kbrand-normal
 *  - handle：kfont-white + kbrand-normal 描边，尺寸/边框对齐 Slider.qml 非 touchSlider（handleHeight=16，handleBorderWidth=3）
 *  - NodeSlider 默认 tickInterval=10；StepSlider 默认 singleStep=10；
 *    SingleSelectSlider range 0..2，tickInterval=1，且不画 coverLine/coverNode
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Slider {
    id: control

    enum KSliderType { SmoothSlider = 0, StepSlider = 1, NodeSlider = 2, SingleSelectSlider = 3 }

    property int sliderType: KSlider.KSliderType.SmoothSlider
    property int tickInterval: 0
    property bool nodeVisible: true
    property bool translucent: false

    from: 0
    to: 100
    value: 0
    stepSize: sliderType === KSlider.KSliderType.StepSlider ? (tickInterval > 0 ? tickInterval : 10) : 1
    font: app.font
    hoverEnabled: true

    implicitWidth: orientation === Qt.Horizontal ? sliderStyle.normalWidth : sliderStyle.grooveHeight
    implicitHeight: orientation === Qt.Horizontal ? sliderStyle.normalHeight : sliderStyle.normalWidth

    StylePrivate.APPParameter { id: app }
    StylePrivate.UKUISlider { id: sliderStyle }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property var _unGrooveBrush: translucent
        ? (control.enabled ? sliderStyle.normalTransparentUnGrooveColor : sliderStyle.disableTransparentUnGrooveColor)
        : (control.enabled ? sliderStyle.normalUnGrooveColor : sliderStyle.disableUnGrooveColor)
    readonly property var _grooveBrush: control.enabled ? sliderStyle.normalGrooveColor : sliderStyle.disableGrooveColor

    function _handleFillBrush(hovered) {
        if (!control.enabled)
            return sliderStyle.disableHandleColor
        if (control.pressed)
            return sliderStyle.clickHandleColor
        if (hovered)
            return sliderStyle.hoverHandleColor
        return sliderStyle.normalHandleColor
    }

    function _handleBorderBrush(hovered) {
        if (!control.enabled)
            return sliderStyle.disableHandleBorderColor
        if (control.pressed)
            return sliderStyle.clickHandleBorderColor
        if (hovered)
            return sliderStyle.hoverHandleBorderColor
        return sliderStyle.normalHandleBorderColor
    }

    // 几何常量（对齐 kslider.cpp）
    readonly property int _lineWidth: sliderStyle.grooveHeight
    readonly property int _lineRadius: _lineWidth / 2
    readonly property int _margin: 3
    readonly property int _handleSize: sliderStyle.handleHeight
    readonly property int _nodeBarW: 3
    readonly property int _nodeBarH: 10

    // groove 矩形（对齐 baseLineRect）
    readonly property rect _groove: {
        if (orientation === Qt.Horizontal) {
            var x = _margin + _handleSize / 2
            return Qt.rect(x, height / 2 - _lineWidth / 2, width - 2 * x, _lineWidth)
        } else {
            var y = _margin + _handleSize / 2
            return Qt.rect(width / 2 - _lineWidth / 2, y, _lineWidth, height - 2 * y)
        }
    }

    // 节点列表（对齐 kslider.cpp locateNode）
    readonly property var _nodeList: {
        var list = []
        if (!nodeVisible || sliderType === KSlider.KSliderType.SingleSelectSlider)
            return list

        var g = _groove
        if (orientation === Qt.Horizontal) {
            var cy = height / 2
            list.push({ px: g.x, py: cy })
            if (tickInterval > 0 && to !== from) {
                for (var i = from + tickInterval; i < to; i += tickInterval) {
                    var ratio = (i - from) / (to - from)
                    list.push({ px: g.x + g.width * ratio, py: cy })
                }
            }
            list.push({ px: g.x + g.width, py: cy })
        } else {
            var cx = width / 2
            list.push({ px: cx, py: g.y + g.height })
            if (tickInterval > 0 && to !== from) {
                for (var j = from + tickInterval; j < to; j += tickInterval) {
                    var ratioV = (j - from) / (to - from)
                    list.push({ px: cx, py: g.y + g.height * ratioV })
                }
            }
            list.push({ px: cx, py: g.y })
        }
        return list
    }

    readonly property color _nodeBase: parseColor.getDtColor("kcomponent-alpha-normal")
    readonly property int _nodeRadius: (typeof ukui_platformtheme_token !== "undefined"
                                        && ukui_platformtheme_token
                                        && ukui_platformtheme_token.kradiusMin > 0)
                                       ? ukui_platformtheme_token.kradiusMin : 4

    // 当前值位置（像素）
    readonly property real _valuePos: {
        if (to === from) return 0
        var r = (value - from) / (to - from)
        if (orientation === Qt.Horizontal)
            return _groove.x + _groove.width * r
        else
            return _groove.y + _groove.height * (1 - r)
    }

    background: Item {
        implicitWidth: control.orientation === Qt.Horizontal ? 200 : control._lineWidth
        implicitHeight: control.orientation === Qt.Horizontal ? control._lineWidth : 200
        opacity: control.enabled ? 1 : 0.45

        // unGroove
        Item {
            x: control._groove.x
            y: control._groove.y
            width: control._groove.width
            height: control._groove.height

            Rectangle {
                anchors.fill: parent
                radius: control._lineRadius
                color: parseColor.isSolidPattern(control._unGrooveBrush)
                       ? parseColor.startColor(control._unGrooveBrush)
                       : "transparent"
                antialiasing: true
            }

            Rectangle {
                anchors.fill: parent
                radius: control._lineRadius
                visible: !parseColor.isSolidPattern(control._unGrooveBrush)
                gradient: Gradient {
                    GradientStop { position: 0.0; color: parseColor.startColor(control._unGrooveBrush) }
                    GradientStop { position: 1.0; color: parseColor.endColor(control._unGrooveBrush) }
                }
                antialiasing: true
            }
        }

        // cover groove（SingleSelectSlider 不画）
        Item {
            visible: control.sliderType !== KSlider.KSliderType.SingleSelectSlider
            x: control._groove.x
            y: control.orientation === Qt.Horizontal ? control._groove.y : control._valuePos
            width: control.orientation === Qt.Horizontal
                   ? Math.max(0, control._valuePos - control._groove.x) : control._lineWidth
            height: control.orientation === Qt.Horizontal
                    ? control._lineWidth
                    : Math.max(0, control._groove.y + control._groove.height - control._valuePos)

            Rectangle {
                anchors.fill: parent
                radius: control._lineRadius
                color: parseColor.isSolidPattern(control._grooveBrush)
                       ? parseColor.startColor(control._grooveBrush)
                       : "transparent"
                antialiasing: true
            }

            Rectangle {
                anchors.fill: parent
                radius: control._lineRadius
                visible: !parseColor.isSolidPattern(control._grooveBrush)
                gradient: Gradient {
                    GradientStop { position: 0.0; color: parseColor.startColor(control._grooveBrush) }
                    GradientStop { position: 1.0; color: parseColor.endColor(control._grooveBrush) }
                }
                antialiasing: true
            }
        }

        // 节点（对齐 kslider.cpp drawBasePath / drawCoverNode）
        Repeater {
            model: control._nodeList

            delegate: Item {
                x: modelData.px - (control.orientation === Qt.Horizontal ? 1 : 5)
                y: modelData.py - (control.orientation === Qt.Horizontal ? 5 : 3)
                width: control.orientation === Qt.Horizontal ? control._nodeBarW : control._nodeBarH
                height: control.orientation === Qt.Horizontal ? control._nodeBarH : control._nodeBarW

                property bool _covered: control.orientation === Qt.Horizontal
                                        ? (modelData.px <= control._valuePos)
                                        : (modelData.py >= control._valuePos)

                Rectangle {
                    anchors.fill: parent
                    radius: control._nodeRadius
                    color: parent._covered
                           ? parseColor.startColor(control._grooveBrush)
                           : control._nodeBase
                    antialiasing: true
                }
            }
        }
    }

    handle: Item {
        id: handleRoot
        x: control.leftPadding + (control.orientation === Qt.Horizontal
            ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.orientation === Qt.Horizontal
            ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))
        width: control._handleSize
        height: control._handleSize

        property bool handleHovered: handleMouse.containsMouse || control.hovered
        readonly property var fillBrush: control._handleFillBrush(handleHovered)
        readonly property var borderBrush: control._handleBorderBrush(handleHovered)

        MouseArea {
            id: handleMouse
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onPressed: mouse.accepted = false
        }

        Rectangle {
            anchors.fill: handleRec
            visible: !control.enabled
            radius: handleRec.radius
            color: parseColor.startColor(sliderStyle.touchHandleBaseColor)
            antialiasing: true
        }

        Rectangle {
            id: handleRec
            anchors.fill: parent
            opacity: control.enabled ? 1 : 0.45
            radius: width / 2
            color: parseColor.isSolidPattern(handleRoot.fillBrush)
                   ? parseColor.startColor(handleRoot.fillBrush)
                   : "transparent"
            border.width: control.pressed ? sliderStyle.handleBorderWidth * 1.25 : sliderStyle.handleBorderWidth
            border.color: parseColor.startColor(handleRoot.borderBrush)
            scale: 1.0
            antialiasing: true

            states: [
                State {
                    name: "HOVERED"
                    when: control.enabled && !control.pressed && handleRoot.handleHovered
                    PropertyChanges { target: handleRec; scale: 1.125 }
                },
                State {
                    name: "PRESSED"
                    when: control.enabled && control.pressed
                    PropertyChanges {
                        target: handleRec
                        scale: 1.0
                        border.width: sliderStyle.handleBorderWidth * 1.25
                    }
                }
            ]

            transitions: [
                Transition {
                    from: "*"; to: "HOVERED"
                    reversible: true
                    NumberAnimation { property: "scale"; duration: 100; easing.type: Easing.InOutSine }
                },
                Transition {
                    from: "*"; to: "PRESSED"
                    reversible: true
                    ParallelAnimation {
                        NumberAnimation { property: "scale"; duration: 100; easing.type: Easing.InOutSine }
                        NumberAnimation { property: "border.width"; duration: 100; easing.type: Easing.Linear }
                    }
                }
            ]

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                border.width: parent.border.width
                border.color: parent.border.color
                visible: control.enabled && !parseColor.isSolidPattern(handleRoot.fillBrush)
                gradient: Gradient {
                    GradientStop { position: 0.0; color: parseColor.startColor(handleRoot.fillBrush) }
                    GradientStop { position: 1.0; color: parseColor.endColor(handleRoot.fillBrush) }
                }
                antialiasing: true
            }
        }
    }

    function setSliderType(t) {
        sliderType = t
        if (tickInterval === 0 && t === KSlider.KSliderType.NodeSlider) tickInterval = 10
        if (singleStep === 0 && t === KSlider.KSliderType.StepSlider) stepSize = 10
        if (t === KSlider.KSliderType.SingleSelectSlider) { tickInterval = 1; stepSize = 1; from = 0; to = 2 }
    }
    function setTickInterval(i) { tickInterval = i }
    function setNodeVisible(f) { nodeVisible = f }
    function setTranslucent(f) { translucent = f }
}
