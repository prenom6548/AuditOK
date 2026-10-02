import QtQuick 2.12
import QtQuick.Window 2.12

Window {
    id: root
    width: 112
    height: 112
    visible: keepWindowVisible
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.Tool | Qt.WindowStaysOnTopHint | Qt.NoDropShadowWindowHint
    title: "voice-kyling"

    property bool keepWindowVisible: false

    x: {
        var screenRect = Screen.desktopAvailableGeometry
        return screenRect.x + screenRect.width - width - 28
    }
    y: {
        var screenRect = Screen.desktopAvailableGeometry
        return screenRect.y + screenRect.height - height - 92
    }

    onVisibleChanged: {
        if (visible) {
            raise()
            requestActivate()
        }
    }

    Connections {
        target: aiBallController
        onBallVisibleChanged: {
            if (aiBallController.ballVisible) {
                keepWindowVisible = true
            }
        }
    }

    AIBall {
        id: orb
        anchors.centerIn: parent
        stateName: aiBallController.uiState
        shown: aiBallController.ballVisible
        audioLevel: Math.max(aiBallController.audioLevel, aiBallController.speechPulse)
        onHideFinished: {
            keepWindowVisible = false
            aiBallController.hideAnimationFinished()
        }
    }
}
