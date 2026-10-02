import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent

    RowLayout {
        id: contentRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.topMargin: 20
        spacing: 12

        Image {
            Layout.alignment: Qt.AlignTop
            width: 48
            height: 48
            source: muteDialog.infoIconSource
            smooth: true
            fillMode: Image.PreserveAspectFit
        }

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Microphone is muted. Adjust the input volume to use voice input.")
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            color: "#262626"
        }
    }

    Row {
        id: buttonRow
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 16
        anchors.bottomMargin: 16
        spacing: 8

        Button {
            text: qsTr("Cancel")
            onClicked: muteDialog.onCancelClicked()
        }

        Button {
            text: qsTr("Settings")
            highlighted: true
            onClicked: muteDialog.onSettingsClicked()
        }
    }
}
