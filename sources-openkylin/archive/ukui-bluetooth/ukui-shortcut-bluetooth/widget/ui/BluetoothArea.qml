import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.3
import org.ukui.shortcut.bluetooth 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform


UkuiItems.DtThemeBackground {
    id: root
    backgroundColor: Platform.GlobalTheme.kContainSecondaryAlphaNormal
    radius: Platform.GlobalTheme.kRadiusWindow


    Component.onCompleted: {
        console.log("ukui shortcut bluetooth start")
        BtInterface.registerClient(1)
    }

    Component.onDestruction: {
        console.log("ukui shortcut bluetooth stop")
        BtInterface.registerClient(0)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            id: bttop
            width: parent.width
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: 56

            Item { Layout.fillWidth: true }

            SwitchDelegate {
                id: btpower
                checked: BtInterface.btPower
                onClicked: {
                    BtInterface.btPower = !BtInterface.btPower
                }
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 16
                width: 50
                height: 24
            }
        }
        
        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            height: 1
            color: "#0f000000"
        }


        Item {
            visible: !BtInterface.btPower
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            UkuiItems.DtThemeText {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
                text: qsTr("Bluetooth is disabled")
                textColor: Platform.GlobalTheme.kFontSecondary
            }
        }

        

        ScrollView {
            visible: BtInterface.btPower
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            clip: true

            Flickable {
                id: myflickable
                anchors.fill: parent
                contentHeight: (listPairView.contentHeight>0 ? mydeviceslabel.height : 0) + listPairView.contentHeight + listNotPairView.contentHeight + 40
                //contentWidth : parent.width

                ColumnLayout {
                    Rectangle {
                        id: mydeviceslabel
                        visible: BtInterface.btPower && (listPairView.contentHeight>0 ? true : false)
                        Layout.alignment: Qt.AlignTop
                        height: 40
                        Label {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("My Devices")
                        }
                    }

                    BluetoothPairedDevList {
                        id : listPairView
                        Layout.alignment: Qt.AlignTop
                        interactive: false
                        implicitHeight: listPairView.contentHeight
                        width: 396
                    }

                    Rectangle {
                        visible: BtInterface.btPower
                        Layout.alignment: Qt.AlignTop
                        height: 40
                        Label {
                            id: newDeviceLabel
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("New Devices")
                        }

                        BusyIndicator{
                            anchors.left: newDeviceLabel.right
                            anchors.leftMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            width: 12
                            height: 12
                            running: BtInterface.btDiscovering
                        }
                    }

                    BluetoothNotPairedDevlist {
                        id : listNotPairView
                        Layout.alignment: Qt.AlignBottom
                        interactive: false
                        implicitHeight: listNotPairView.contentHeight
                        srollmoving: myflickable.moving
                        width: 396
                    }
                }
            }
        }


        Rectangle {
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            height: 1
            color: Qt.rgba(0, 0, 0, 0.06)
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            Layout.alignment: Qt.AlignBottom
            opacity: Theme.windowOpacity
            UkuiItems.DtThemeText {
                id: settingLabel
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("More Bluetooth Settings")
                textColor: Platform.GlobalTheme.kFontStrong

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        BtInterface.openBluetoothSetting()
                    }
                    onEntered: {
                        settingLabel.textColor = Platform.GlobalTheme.highlightActive
                    }
                    onExited: {
                        settingLabel.textColor = Platform.GlobalTheme.kFontStrong
                    }
                }
            }
        }

            
    }
}
