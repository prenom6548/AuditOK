/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KProgressCircle.
 * 圆形进度条控件，支持百分比显示、三种状态（正常/成功/失败）。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Templates 2.12 as T
import QtGraphicalEffects 1.0
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Control {
    id: control

    palette: app.palette
    font: app.font

    // 进度值属性
    property int minimum: 0
    property int maximum: 100
    property int value: 0

    // 文本显示
    property bool textVisible: true
    property string displayText: {
        if (maximum === 0 && minimum === 0)
            return ""
        var totalSteps = maximum - minimum
        if (totalSteps === 0)
            return "100%"
        var progress = Math.round((value - minimum) * 100.0 / totalSteps)
        return progress + "%"
    }

    // 状态枚举：0=Normal, 1=Success, 2=Failed
    property int state: 0

    // 圆环宽度
    readonly property int circleWidth: 6

    // 起始角度（12点钟方向为90度）
    readonly property int startAngle: 90

    readonly property bool isClassicTheme: app.widgetThemeName === "classical"

    // 基础圆环颜色（对齐 changeTheme()）
    property color baseColor: app.isDark ? getDtColor("kgray-16") : getDtColor("kgray-6")
    property color baseCircleColor: app.isDark ? getDtColor("kgray-17") : getDtColor("kgray-10")

    // 进度颜色（对齐 setState + dawColoredCircle）
    readonly property color progressColor: {
        switch (state) {
        case 2: return "#FF4D4F"
        case 1: return "#52C429"
        default: return getDtColor("kbrand-normal")
        }
    }

    // 固定尺寸
    implicitWidth: 60
    implicitHeight: 60

    // 计算进度角度
    readonly property real angleSpan: {
        if (maximum === 0)
            return 0
        var span = value * 360 / maximum
        return Math.min(360, Math.max(0, span))
    }

    // 中心图标尺寸
    readonly property int centerPixSize: 20

    onStateChanged: canvas.requestPaint()
    onValueChanged: syncStateWithValue()
    onMaximumChanged: syncStateWithValue()
    onTextVisibleChanged: canvas.requestPaint()

    Connections {
        target: app
        function onParametryChanged() {
            updateThemeColors()
            canvas.requestPaint()
        }
        function onIconThemeChanged() {
            canvas.requestPaint()
        }
        function onIsDarkChanged() {
            updateThemeColors()
            canvas.requestPaint()
        }
    }

    StylePrivate.APPParameter {
        id: app
    }

    StylePrivate.ParseColorInterface {
        id: parseColor
    }

    function getDtColor(colorName) {
        return parseColor.getDtColor(colorName)
    }

    function lanhuHSL(l) {
        return Qt.hsla(0, 0, l / 100, 1)
    }

    function syncStateWithValue() {
        // 对齐 C++ valueChanged：满值切成功；未满值时回到正常（失败态保持，需 setState/reset 清除）
        if (state === 2)
            return
        if (value === maximum)
            state = 1
        else if (state === 1)
            state = 0
        canvas.requestPaint()
    }

    function updateThemeColors() {
        baseColor = app.isDark ? getDtColor("kgray-16") : getDtColor("kgray-6")
        baseCircleColor = app.isDark ? getDtColor("kgray-17") : getDtColor("kgray-10")
    }

    function progressLineCap() {
        return isClassicTheme ? "butt" : "round"
    }

    function classicOuterRingColor() {
        return app.isDark ? lanhuHSL(35) : lanhuHSL(73)
    }

    function classicFillColor() {
        return app.isDark ? lanhuHSL(27) : lanhuHSL(98)
    }

    // 绘制画布
    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var centerX = width / 2
            var centerY = height / 2
            var arcRadius = (Math.min(width, height) - circleWidth) / 2 - 1
            if (isClassicTheme)
                arcRadius -= 1

            if (textVisible) {
                if (isClassicTheme) {
                    ctx.beginPath()
                    ctx.arc(centerX, centerY, arcRadius, 0, Math.PI * 2)
                    ctx.strokeStyle = classicOuterRingColor()
                    ctx.lineWidth = 8
                    ctx.lineCap = "butt"
                    ctx.stroke()
                }

                ctx.beginPath()
                ctx.arc(centerX, centerY, arcRadius, 0, Math.PI * 2)
                ctx.strokeStyle = baseColor
                ctx.lineWidth = circleWidth
                ctx.lineCap = progressLineCap()
                ctx.stroke()

                if (angleSpan > 0) {
                    var startRad = (startAngle - 90) * Math.PI / 180
                    var endRad = startRad - angleSpan * Math.PI / 180
                    ctx.beginPath()
                    ctx.arc(centerX, centerY, arcRadius, startRad, endRad, true)
                    ctx.strokeStyle = progressColor
                    ctx.lineWidth = circleWidth
                    ctx.lineCap = progressLineCap()
                    ctx.stroke()
                }
            } else {
                var outerRadius = arcRadius + circleWidth / 2
                var innerRadius = arcRadius - circleWidth / 2

                if (isClassicTheme) {
                    ctx.beginPath()
                    ctx.arc(centerX, centerY, outerRadius, 0, Math.PI * 2)
                    ctx.fillStyle = classicFillColor()
                    ctx.fill()
                    ctx.strokeStyle = classicOuterRingColor()
                    ctx.lineWidth = 1
                    ctx.stroke()
                } else {
                    ctx.beginPath()
                    ctx.arc(centerX, centerY, outerRadius, 0, Math.PI * 2)
                    ctx.fillStyle = baseColor
                    ctx.fill()
                }

                ctx.beginPath()
                ctx.arc(centerX, centerY, innerRadius, 0, Math.PI * 2)
                if (isClassicTheme) {
                    ctx.fillStyle = classicFillColor()
                    ctx.fill()
                }
                ctx.strokeStyle = baseCircleColor
                ctx.lineWidth = circleWidth
                ctx.lineCap = progressLineCap()
                ctx.stroke()

                if (angleSpan > 0) {
                    var innerStartRad = (startAngle - 90) * Math.PI / 180
                    var innerEndRad = innerStartRad - angleSpan * Math.PI / 180
                    ctx.beginPath()
                    ctx.arc(centerX, centerY, innerRadius, innerStartRad, innerEndRad, true)
                    ctx.strokeStyle = progressColor
                    ctx.lineWidth = circleWidth
                    ctx.lineCap = progressLineCap()
                    ctx.stroke()
                }
            }
        }
    }

    // 文本/图标显示区域（互斥展示，避免重叠）
    Item {
        id: contentArea
        anchors.centerIn: parent
        visible: textVisible

        Text {
            id: percentText
            anchors.centerIn: parent
            visible: control.state === 0
            text: displayText
            font: control.font
            color: control.palette.windowText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            anchors.centerIn: parent
            width: centerPixSize
            height: centerPixSize
            visible: control.state === 1

            Image {
                id: successIcon
                anchors.fill: parent
                source: "image://imageProvider/object-select-symbolic/normal"
                fillMode: Image.PreserveAspectFit
                cache: false
                visible: false
            }

            ColorOverlay {
                anchors.fill: parent
                source: successIcon
                color: getDtColor("ksuccess-normal")
            }
        }

        Item {
            anchors.centerIn: parent
            width: centerPixSize
            height: centerPixSize
            visible: control.state === 2

            Image {
                id: failedIcon
                anchors.fill: parent
                source: "image://imageProvider/window-close-symbolic/normal"
                fillMode: Image.PreserveAspectFit
                cache: false
                visible: false
            }

            ColorOverlay {
                anchors.fill: parent
                source: failedIcon
                color: getDtColor("kerror-normal")
            }
        }
    }

    function setValue(val) {
        if (val === value)
            return
        if ((val > maximum || val < minimum) && (maximum !== 0 || minimum !== 0))
            return
        value = val
    }

    function setRange(min, max) {
        minimum = min
        maximum = Math.max(min, max)
        if (value < minimum || value > maximum)
            reset()
    }

    function reset() {
        value = minimum - 1
        state = 0
    }

    function setState(newState) {
        state = newState
    }
}
