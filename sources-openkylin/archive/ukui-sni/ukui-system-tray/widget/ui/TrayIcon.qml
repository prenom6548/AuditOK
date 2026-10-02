import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.systemTray 1.0
import org.ukui.quick.items 1.0 as UkuiItems

MouseArea {
    id: control

    property string appStatus: ""
    property string appAttentionMovieName: ""
    property var appIcon
    property var appAttentionIcon
    property var appOverlayIcon
    property int iconRotation: 0
    property real iconRatio: dropArea.onTopBar ? 24 : 16
    property bool showControlPanel: false

    Item {
        id: trayitem
        anchors.fill: parent
        rotation: control.iconRotation

        Loader {
            anchors.centerIn: parent
            sourceComponent: showControlPanel ?
                                 controlPanelComponent : ((control.appStatus === "NeedsAttention") && (control.appAttentionMovieName !== ""))
                                 ? movieComponent : iconComponent
        }

        Component {
            id: iconComponent
            UkuiItems.Icon {
                width: Math.round(dropArea.scaleFactor * iconRatio)
                height: Math.round(dropArea.scaleFactor * iconRatio)
                source: (control.appStatus === "NeedsAttention") ? control.appAttentionIcon : control.appIcon
                mode: UkuiItems.Icon.AutoHighlight
//                overlayIcon: appOverlayIcon
//                isInverseColor: false
            }
        }

        Component {
            id: movieComponent
            AnimatedImage {
                width: Math.round(dropArea.scaleFactor * iconRatio)
                height: Math.round(dropArea.scaleFactor * iconRatio)
                source: control.appAttentionMovieName
            }
        }

        Component {
            id: controlPanelComponent
            ControlPanel {}
        }
    }
}
