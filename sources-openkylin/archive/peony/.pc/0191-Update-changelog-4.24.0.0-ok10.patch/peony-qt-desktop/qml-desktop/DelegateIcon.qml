//DelegateIcon.qml
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import org.ukui.desktop 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0

Item {
    id: viewIconRect
    Accessible.ignored: true
    anchors.top: parent.top
    anchors.topMargin: 5
    anchors.horizontalCenter: parent.horizontalCenter
    property int emblemSize : 16
    property var icon
    property var emblems
    property string uri

    Icon {
        id: itemIcon
        anchors.fill: parent
        source: icon
        Binding {
            target: itemIcon
            property: "addUnifiedIconBox"
            value: true
            when: isValidDesktopApp(uri) // 调用校验函数
        }
    }

    // 右下角
    Icon {
        source: emblems && emblems[0] ? emblems[0] : ""
        width: emblemSize; height: emblemSize
        visible: emblems && emblems[0] ? true : false
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 2
    }

    // 左下角
    Icon {
        source: emblems && emblems[1] ? emblems[1] : ""
        width: emblemSize; height: emblemSize
        visible: emblems && emblems[1] ? true : false
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 2
    }

    // 右上角
    Icon {
        source: emblems && emblems[2] ? emblems[2] : ""
        width: emblemSize; height: emblemSize
        visible: emblems && emblems[2] ? true : false
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 2
    }

    // 左上角
    Icon {
        source: emblems && emblems[3] ? emblems[3] : ""
        width: emblemSize; height: emblemSize
        visible: emblems && emblems[3] ? true : false
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 2
    }

    function isValidDesktopApp(uri) {
        return uri.endsWith(".desktop")
    }
}
