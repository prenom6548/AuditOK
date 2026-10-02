import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

Item {
    id: systemVolume
    width: systemVolumeWidth
    height: systemVolumeHeight

    property real systemVolumeWidth: 396
    property real systemVolumeHeight: 56
    property real systemVolumeTopMargin: 0
    property bool systemVolumeMuteStatus: false

    property alias systemVolumeValue: systemVolumeSlider.value
    property alias systemVolumeSliderEnabled: systemVolumeSlider.enabled
    property alias systemVolumeSliderRange: systemVolumeSlider.to
    property alias systemVolumeSliderPressed: systemVolumeSlider.pressed

    property alias systemVolumeBtnSource: systemVolumeBtn.icon.source
    property alias systemVolumeBtnEnabled: systemVolumeBtn.enabled

    signal systemVolumeBtnClick()
    signal valueChanged(real value)

    Item {
        id: muteFocusItem
        width: 36
        height: 36
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 24

        UkuiItems.DtThemeButton {
            id: systemVolumeBtn
            anchors.fill: parent
            enabled: true

            background.radius: width / 2
            property var kTransparent: Platform.GradientColor {
                gradientDeg: 0
                gradientStart: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
                gradientEnd: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
                gradientBackground: Qt.rgba(0 / 255, 0 / 255, 0 / 255, 0)
            }
            background.backgroundColor: containsPress ? Platform.GlobalTheme.kContainAlphaClick
                                                      : (containsMouse || muteFocusItem.activeFocus)
                                                        ? Platform.GlobalTheme.kContainAlphaHover
                                                        : kTransparent
            icon.source: "audio-volume-medium-symbolic"
            icon.mode: UkuiItems.Icon.AutoHighlight

            onClicked: {
                systemVolume.systemVolumeBtnClick()
            }

            onEnabledChanged: {
                console.log("systemVolume.qml DtThemeButton onEnabledChanged:", enabled)
                systemVolumeBtn.icon.mode = (enabled ? UkuiItems.Icon.AutoHighlight : UkuiItems.Icon.Disabled)
            }
        }
    }

    Item {
        id: systemVolumeSliderContainer
        width: 268
        height: 16
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: muteFocusItem.right
        anchors.leftMargin: 6

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: systemVolumeSlider.activeFocus ? Platform.GlobalTheme.kContainAlphaHover : "transparent"
            z: -1
        }

        Slider {
            id: systemVolumeSlider
            Accessible.name: qsTr("System volume")
            anchors.fill: parent
            from: 0
            to: 100
            stepSize: 1
            enabled: true

            property var sliderType: "touchSlider"

            function notifyVolumeChanged() {
                console.log("systemVolume.qml Slider onMoved:", systemVolumeSlider.value)
                systemVolume.valueChanged(systemVolumeSlider.value)
            }

            onMoved: notifyVolumeChanged()

            Keys.onPressed: {
                if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                    notifyVolumeChanged()
                }
            }
        }
    }

    Label {
        id: systemVolumePercentLabel
        width: 48
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.left: systemVolumeSliderContainer.right
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr(systemVolumeValue + "%")
        enabled: systemVolumeBtn.enabled
    }

    Component.onCompleted: {
    }
}
