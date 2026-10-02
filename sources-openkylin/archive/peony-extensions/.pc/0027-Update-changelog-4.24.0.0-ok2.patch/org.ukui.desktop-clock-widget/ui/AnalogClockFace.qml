import QtQuick 2.15

// 刻度 + SVG 指针（资源见 desktopclock.qrc），供 main.qml 与 preview 共用，保证预览与桌面一致。
// 枢轴取自 SVG viewBox 内圆心/装饰圆心（与缩放无关的相对比例），保证时针、分针末端圆同心叠在表盘中心。
Item {
    id: root
    property int hour: 0
    property int minute: 0
    property int second: 0
    property string sizeMode: "small"

    readonly property bool isLarge: sizeMode === "large"
    readonly property bool isMedium: sizeMode === "medium"
    readonly property real shapePow: isLarge ? 14.0 : 12.0
    readonly property real contourScale: isLarge ? 0.95 : 0.94
    readonly property real cx: width / 2
    readonly property real cy: height / 2
    readonly property real r: Math.min(width, height) / 2 - 2

    readonly property int majorTickLen: isLarge ? 16 : (isMedium ? 10 : 9)
    readonly property int minorTickLen: isLarge ? 10 : (isMedium ? 6 : 5)
    readonly property real hubRadius: isLarge ? 7.5 : (isMedium ? 4.2 : 3.8)

    readonly property real hourA: ((hour % 12) + (minute + second / 60) / 60) * Math.PI / 6 - Math.PI / 2
    readonly property real minuteA: (minute + second / 60) * Math.PI / 30 - Math.PI / 2
    readonly property real secondA: second * Math.PI / 30 - Math.PI / 2
    readonly property real hourReach: r * contourScale * dialRadiusFactor(hourA, shapePow)
    readonly property real minuteReach: r * contourScale * dialRadiusFactor(minuteA, shapePow)
    readonly property real secondReach: r * contourScale * dialRadiusFactor(secondA, shapePow)
    readonly property real hourTipLen: Math.max(4, hourReach * 0.45)
    readonly property real minuteTipLen: Math.max(4, minuteReach * 0.72)
    readonly property real secondTipLen: Math.max(4, secondReach * 0.88)

    readonly property string srcHour: "qrc:/org/ukui/desktopclock/hands/clock_hand_hour.svg"
    readonly property string srcMinute: "qrc:/org/ukui/desktopclock/hands/clock_hand_minute.svg"
    readonly property string srcSecond: "qrc:/org/ukui/desktopclock/hands/clock_hand_second.svg"

    // viewBox 内枢轴相对位置（0~1），与 QML Image 缩放后像素一致
    readonly property real hourPivotRelX: 2.5 / 5
    readonly property real hourPivotRelY: 1.0
    readonly property real minutePivotRelX: 4 / 8
    readonly property real minutePivotRelY: 52.091129541397095 / 56.0078125
    readonly property real secondPivotRelX: 1.73828125 / 4
    readonly property real secondPivotRelY: 64.5 / 72

    readonly property real hourAspectFallback: 5 / 35.005249
    readonly property real minuteAspectFallback: 8 / 56.0078125
    readonly property real secondAspectFallback: 4 / 72

    function dialRadiusFactor(angle, p) {
        var ca = Math.abs(Math.cos(angle))
        var sa = Math.abs(Math.sin(angle))
        return Math.pow(Math.pow(ca, p) + Math.pow(sa, p), -1 / p)
    }

    Canvas {
        id: tickCanvas
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            var w = width
            var h = height
            var cx = w / 2
            var cy = h / 2
            var r = Math.min(w, h) / 2 - 2
            var isLarge = root.isLarge
            var isMedium = root.isMedium
            var shapePow = root.shapePow
            var contourScale = root.contourScale
            var majorTickLen = root.majorTickLen
            var minorTickLen = root.minorTickLen
            ctx.setTransform(1, 0, 0, 1, 0, 0)
            ctx.clearRect(0, 0, w, h)

            for (var i = 0; i < 60; ++i) {
                var major = (i % 5) === 0
                var a = i * Math.PI / 30 - Math.PI / 2
                var tickLen = major ? majorTickLen : minorTickLen
                var outerR = r * contourScale * root.dialRadiusFactor(a, shapePow)
                var innerR = Math.max(0, outerR - tickLen)
                var x1 = cx + Math.cos(a) * innerR
                var y1 = cy + Math.sin(a) * innerR
                var x2 = cx + Math.cos(a) * outerR
                var y2 = cy + Math.sin(a) * outerR
                ctx.strokeStyle = major ? "#30415C" : "#B9BDC6"
                ctx.lineWidth = major ? (isLarge ? 1.8 : 1.4) : 1
                ctx.beginPath()
                ctx.moveTo(x1, y1)
                ctx.lineTo(x2, y2)
                ctx.stroke()
            }
        }
    }

    Item {
        anchors.fill: parent
        visible: width > 2 && height > 2

        // 盖在刻度之上、指针之下，不挡住时针/分针根部圆饰
        Rectangle {
            anchors.centerIn: parent
            width: root.hubRadius * 2
            height: width
            radius: width / 2
            color: "#2B4F8A"
            z: 1
        }

        Item {
            id: hourHost
            x: root.cx
            y: root.cy
            width: 0
            height: 0
            z: 2
            Image {
                id: hourImage
                height: root.hourTipLen
                readonly property real ar: sourceSize.height > 0 ? sourceSize.width / sourceSize.height : root.hourAspectFallback
                width: height * ar
                x: -root.hourPivotRelX * width
                y: -root.hourPivotRelY * height
                source: root.srcHour
                smooth: true
                antialiasing: true
                transform: [
                    Rotation {
                        origin.x: root.hourPivotRelX * hourImage.width
                        origin.y: root.hourPivotRelY * hourImage.height
                        angle: root.hourA * 180 / Math.PI + 90
                    }
                ]
            }
        }

        Item {
            id: minuteHost
            x: root.cx
            y: root.cy
            width: 0
            height: 0
            z: 3
            Image {
                id: minuteImage
                height: root.minuteTipLen
                readonly property real ar: sourceSize.height > 0 ? sourceSize.width / sourceSize.height : root.minuteAspectFallback
                width: height * ar
                x: -root.minutePivotRelX * width
                y: -root.minutePivotRelY * height
                source: root.srcMinute
                smooth: true
                antialiasing: true
                transform: [
                    Rotation {
                        origin.x: root.minutePivotRelX * minuteImage.width
                        origin.y: root.minutePivotRelY * minuteImage.height
                        angle: root.minuteA * 180 / Math.PI + 90
                    }
                ]
            }
        }

        Item {
            id: secondHost
            x: root.cx
            y: root.cy
            width: 0
            height: 0
            z: 4
            Image {
                id: secondImage
                height: root.secondTipLen
                readonly property real ar: sourceSize.height > 0 ? sourceSize.width / sourceSize.height : root.secondAspectFallback
                width: height * ar
                x: -root.secondPivotRelX * width
                y: -root.secondPivotRelY * height
                source: root.srcSecond
                smooth: true
                antialiasing: true
                transform: [
                    Rotation {
                        origin.x: root.secondPivotRelX * secondImage.width
                        origin.y: root.secondPivotRelY * secondImage.height
                        angle: root.secondA * 180 / Math.PI + 90
                    }
                ]
            }
        }
    }

    function repaintTicks() {
        tickCanvas.requestPaint()
    }

    onSizeModeChanged: Qt.callLater(function() {
        tickCanvas.requestPaint()
    })

    Component.onCompleted: tickCanvas.requestPaint()
}
