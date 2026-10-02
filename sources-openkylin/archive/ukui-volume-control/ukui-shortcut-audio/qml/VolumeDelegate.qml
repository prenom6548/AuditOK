import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems

Item {
    id: volumeDelegate
    width: volumeDelegateWidth
    height: volumeDelegateHeight

    property real volumeDelegateWidth: 396
    property real volumeDelegateHeight: 56
    property real volumeDelegateTopMargin: 0

    property alias volumeValue: volumeDelegateSlider.value
    property alias volumeDelegateImgSource: volumeDelegateImg.source
    property alias volumeEnabled: volumeDelegateSlider.enabled
    property string volumeAppName: ""

    signal valueChanged(real value)

    ItemDelegate {
        anchors.fill: parent

        contentItem: Item {
            anchors.fill: parent

            UkuiItems.ThemeIcon {
                id: volumeDelegateImg
                width: 32
                height: 32
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                source: ""

                visible: volumeDelegateImg.source === "" ? false : true
            }

            Slider {
                id: volumeDelegateSlider
                Accessible.name: qsTr("Application volume")
                width: 268
                height: 16
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: volumeDelegateImg.right
                anchors.leftMargin: 8
                from: 0
                to: 100
                stepSize: 1
                enabled: true

                property var sliderType: "touchSlider"

                function notifyVolumeChanged() {
                    console.log("VolumeDelegate.qml Slider onMoved:", volumeDelegateSlider.value)
                    if (volumeDelegateSlider.enabled) {
                        volumeDelegate.valueChanged(volumeDelegateSlider.value)
                    }
                }

                onMoved: notifyVolumeChanged()

                Keys.onPressed: {
                    if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                        notifyVolumeChanged()
                    }
                }
            }

            Label {
                id: volumePercentLabel
                width: 48
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.left: volumeDelegateSlider.right
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr(volumeValue + "%")
            }
        }
    }
}
