
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.3
import org.ukui.shortcut.bluetooth 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform

ListView {
    id: listPairView
    model: BtInterface.deviceList
    spacing: 0

    Connections {
        target: BtInterface
        onUpdatePairedDevice : {
            console.log(ipos, device)
            if(-1 != ipos) {
                var d = listPairView.itemAtIndex(ipos);
                if(d) {
                    d.updatePairedDeviceAttrs(device, attrs)
                }
            }
        }
    }

    // 定义每个项的显示方式
    delegate: ItemDelegate {
        width: listPairView.width
        height: 45
        Layout.leftMargin: 8
        checkable: true
        property bool devConnecting:  modelData.Connecting
        property bool devConnected:   modelData.Connected
        property var  devBattery:     modelData.Battery
        property var  devName:        modelData.Name
        property var  devType:        modelData.Type

        RowLayout {
            anchors.fill: parent 
                
            Item {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 16
                width: 36
                height: 36
                
                UkuiItems.IconButton {
                    visible: !devConnecting
                    iconSource: BtInterface.getBluetoothIcon(devType)
                    anchors.fill: parent
                    id: typeicon
                    radius: parent.width / 2
                    isHighLight: devConnected
                    width: parent.width
                    height: parent.height
                }

                UkuiItems.IconButton {
                    visible: devConnecting
                    iconSource: "ukui-loading-" + String(loadingicon.loading_num % 8) + "-symbolic"
                    anchors.fill: parent
                    id: loadingicon
                    radius: parent.width / 2
                    isHighLight: devConnected
                    width: parent.width
                    height: parent.height
                    property int  loading_num : 0
                }
                Timer {
                    interval: 100
                    running: devConnecting
                    repeat: true
                    onTriggered:{
                        loadingicon.loading_num += 1;
                        loadingicon.iconSource =  "ukui-loading-" + String(loadingicon.loading_num % 8) + "-symbolic";
                    }
                }
            }

            EllipsisLabel {
                id: btname
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 8
                Layout.preferredWidth: 296 - (batteryItem.visible ? batteryItem.width : 0)
                text: devName
            }

            Item { Layout.fillWidth: true }

            Item {
                id: batteryItem
                visible: (devBattery >= 0 && devConnected)  ? true : false
                Layout.preferredWidth: batterytext.contentWidth + batteryicon.width + 4
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin:24


                Label {
                    anchors.right: batteryicon.left
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    id: batterytext
                    text: devBattery + "%"
                }

                UkuiItems.Icon {
                    id: batteryicon
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    mode: UkuiItems.Icon.AutoHighlight
                    source: BtInterface.getBluetoothBatteryIcon(devBattery);
                    width: 16
                    height: 16

                }
            }
        }

       onClicked: {
            // // 设置当前选中项
            var key1 = devConnected ? "devDisconnect" : "devConnect";
            console.log(key1)
            var mapData = {};
            mapData[key1] = modelData.Addr;
            BtInterface.setBluetoothConfig(mapData);
            if(devConnected) {
                devConnecting = true;
            }
        }

        function updatePairedDeviceAttrs(device, attrs) {
            if(device === modelData.Addr) {
                if(attrs.hasOwnProperty("Connecting")) {
                    devConnecting = attrs.Connecting;
                }
                if(attrs.hasOwnProperty("Battery")) {
                    devBattery = attrs.Battery;
                }
                if(attrs.hasOwnProperty("Name")) {
                    devName = attrs.Name;
                }
                if(attrs.hasOwnProperty("Type")) {
                    devType = attrs.Type;
                }
            }
            else {
                console.log("error:",  device, modelData.Addr)
            }
        }
    }
}
