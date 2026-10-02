import QtQuick 2.15

Item {
    id: root
    width: voiceWindow.hostSize
    height: voiceWindow.hostSize

    readonly property int pulseEndSize: voiceWindow.windowSize + 6

    Item {
        id: pulseLayer
        anchors.fill: parent
        z: 0
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

    Component {
        id: pulseRingComponent
        Rectangle {
            id: pulseRing
            anchors.centerIn: parent
            width: voiceWindow.windowSize
            height: voiceWindow.windowSize
            radius: width / 2
            color: voiceWindow.themeColor
            opacity: 1.0

            ParallelAnimation {
                running: true
                NumberAnimation {
                    target: pulseRing
                    property: "width"
                    from: voiceWindow.windowSize
                    to: root.pulseEndSize
                    duration: 200
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: pulseRing
                    property: "height"
                    from: voiceWindow.windowSize
                    to: root.pulseEndSize
                    duration: 200
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: pulseRing
                    property: "opacity"
                    from: 1.0
                    to: 0.5
                    duration: 200
                    easing.type: Easing.OutQuad
                }
                onStopped: pulseRing.destroy()
            }
        }
    }

    Connections {
        target: voiceWindow
        function onPulseTokenChanged() {
            pulseRingComponent.createObject(pulseLayer)
        }
    }

    Connections {
        target: voiceWindow
        function onThemeColorChanged() {
            mainCircle.color = voiceWindow.themeColor
        }
    }
}
