import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.3
import org.ukui.shortcut.bluetooth 1.0

import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform


ListView {
    id: listNotPairView
    model: BtInterface.notPairDeviceList
    spacing: 0
    property bool srollmoving: false

    Connections {
        target: BtInterface
        onUpdateNotPairedDevice : {
            console.log("ipos:", ipos, device)
            if(-1 !== ipos) {
                var d = listNotPairView.itemAtIndex(ipos);
                if(d) {
                    d.updateNotPairedDeviceAttrs(device, attrs)
                }
            }
        }
    }

    // 定义每个项的显示方式
    delegate: ItemDelegate {
        width: listNotPairView.width
        height: 45
        Layout.leftMargin: 8
        checkable: true
        hoverEnabled: !listNotPairView.srollmoving
        property bool devConnecting:  modelData.Connecting
        property bool devConnected:   modelData.Connected
        property int  devType:        modelData.Type
        property var  devName:        modelData.Name

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
                    anchors.centerIn: parent
                    isHighLight: devConnected
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
                Layout.preferredWidth: 296
                text: devName
            }

            Item { Layout.fillWidth: true }

        }

        onClicked: {
            // // 设置当前选中项
            var key1 = devConnected ? "devDisconnect" : "devConnect";
            console.log(key1)
            var mapData = {};
            mapData[key1] = modelData.Addr;
            BtInterface.setBluetoothConfig(mapData);
        }

        function updateNotPairedDeviceAttrs(device, attrs) {
            if(device === modelData.Addr) {
                if(attrs.hasOwnProperty("Connecting")) {
                    devConnecting = attrs.Connecting;
                }
                if(attrs.hasOwnProperty("Type")) {
                    devType = attrs.Type;
                }
                if(attrs.hasOwnProperty("Name")) {
                    btname.text = attrs.Name
                }

            }
            else {
                console.log("error:",  device, modelData.Addr)
            }
        }
    }
}





