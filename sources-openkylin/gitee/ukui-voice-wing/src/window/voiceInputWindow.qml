import QtQuick 2.15

Item {
    id: root
    width: voiceWindow.hostSize
    height: voiceWindow.hostSize

    readonly property real pulseScale: (voiceWindow.windowSize + 6) / voiceWindow.windowSize
    readonly property int pulseDuration: 500
    readonly property real pulseOpacityHigh: 0.8
    readonly property real pulseOpacityLow: 0.4

    property bool pulseVisible: false

    function resetPulseRing() {
        pulseRing.scale = 1.0
        pulseRing.opacity = pulseOpacityHigh
    }

    function startPulseIfNeeded() {
        if (!voiceWindow.isRecognizing) {
            return
        }
        pulseVisible = true
        if (!pulseCycleAnim.running) {
            pulseCycleAnim.start()
        }
    }

    Rectangle {
        id: pulseRing
        z: 0
        anchors.centerIn: parent
        transformOrigin: Item.Center
        width: voiceWindow.windowSize
        height: width
        radius: width / 2
        color: voiceWindow.themeColor
        opacity: pulseOpacityHigh
        scale: 1.0
        visible: pulseVisible

        SequentialAnimation {
            id: pulseCycleAnim
            loops: 1

            onStopped: {
                resetPulseRing()
                if (voiceWindow.isRecognizing) {
                    start()
                } else {
                    pulseVisible = false
                }
            }

            ParallelAnimation {
                NumberAnimation {
                    target: pulseRing
                    property: "scale"
                    from: 1.0
                    to: root.pulseScale
                    duration: root.pulseDuration
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: pulseRing
                    property: "opacity"
                    from: root.pulseOpacityHigh
                    to: root.pulseOpacityLow
                    duration: root.pulseDuration
                    easing.type: Easing.OutCubic
                }
            }

            ParallelAnimation {
                NumberAnimation {
                    target: pulseRing
                    property: "scale"
                    from: root.pulseScale
                    to: 1.0
                    duration: root.pulseDuration
                    easing.type: Easing.InCubic
                }
                NumberAnimation {
                    target: pulseRing
                    property: "opacity"
                    from: root.pulseOpacityLow
                    to: root.pulseOpacityHigh
                    duration: root.pulseDuration
                    easing.type: Easing.InCubic
                }
            }
        }
    }

    Rectangle {
        id: mainCircle
        z: 1
        anchors.centerIn: parent
        width: voiceWindow.windowSize
        height: width
        radius: width / 2
        color: voiceWindow.themeColor

        Image {
            anchors.centerIn: parent
            width: voiceWindow.windowSize * 0.6
            height: width
            source: voiceWindow.iconSource
            smooth: true
            fillMode: Image.PreserveAspectFit
        }
    }

    Connections {
        target: voiceWindow
        function onIsRecognizingChanged() {
            if (voiceWindow.isRecognizing) {
                startPulseIfNeeded()
            } else if (!pulseCycleAnim.running) {
                pulseVisible = false
                resetPulseRing()
            }
        }
        function onThemeColorChanged() {
            mainCircle.color = voiceWindow.themeColor
            pulseRing.color = voiceWindow.themeColor
        }
    }
}
