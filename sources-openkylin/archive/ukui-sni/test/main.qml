import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import IconItemModule 1.0

Window {
    width: 800
    height: 50
    visible: true
    title: qsTr("tray demo")

    ListView {
        id: trayview
        anchors.fill: parent
        model: itemModel
        orientation: ListView.Horizontal
        spacing: 5
        delegate: Rectangle {
            id: trayitem
            property bool isSelect: false
            color: isSelect ? "lightblue" : "white"
            width: 50
            height: 25

            Loader {
                anchors.centerIn: parent
                sourceComponent: ((model.Status === "NeedsAttention") && (model.AttentionMovieName !== "")) ? movieComponent : iconComponent
            }

            Component {
                id: iconComponent
                IconItem {
                    width: 20
                    height: 20
                    icon: (model.Status === "NeedsAttention") ? model.AttentionIcon : model.Icon
                    overlayIcon: model.OverlayIcon
                    isInverseColor: false

                    MouseArea {
                        id: control
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if (mouse.button === Qt.LeftButton) {
                                trayview.model.onActivate(index);
                            }
                            if (mouse.button === Qt.RightButton) {
                                trayview.model.onContextMenu(index);
                            }
                        }
                        ToolTip.visible: control.containsMouse
                        ToolTip.text: model.ToolTipTitle
                        ToolTip.delay: 500
                    }
                }
            }

            Component {
                id: movieComponent
                AnimatedImage {
                    width: 20
                    height: 20
                    source: model.AttentionMovieName
                }
            }
        }
    }
//    AnimatedSprite {
//        frameX: 0
//        frameY: 0
//        frameWidth: 64
//        frameHeight: 64
//        frameDuration: 1000
//        frameCount: 10
//        interpolate: false
//        source: "://res/icon/text.png"
//    }
}
