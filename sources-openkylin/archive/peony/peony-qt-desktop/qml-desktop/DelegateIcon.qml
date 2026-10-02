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
    Loader {
        id: bottomRightEmblemLoader
        property string emblemSource: emblems && emblems[0] ? emblems[0] : ""
        active: emblemSource !== ""
        width: emblemSize; height: emblemSize
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 2
        sourceComponent: Icon {
            anchors.fill: parent
            source: bottomRightEmblemLoader.emblemSource
        }
    }

    // 左下角
    Loader {
        id: bottomLeftEmblemLoader
        property string emblemSource: emblems && emblems[1] ? emblems[1] : ""
        active: emblemSource !== ""
        width: emblemSize; height: emblemSize
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 2
        sourceComponent: Icon {
            anchors.fill: parent
            source: bottomLeftEmblemLoader.emblemSource
        }
    }

    // 右上角
    Loader {
        id: topRightEmblemLoader
        property string emblemSource: emblems && emblems[2] ? emblems[2] : ""
        active: emblemSource !== ""
        width: emblemSize; height: emblemSize
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 2
        sourceComponent: Icon {
            anchors.fill: parent
            source: topRightEmblemLoader.emblemSource
        }
    }

    // 左上角
    Loader {
        id: topLeftEmblemLoader
        property string emblemSource: emblems && emblems[3] ? emblems[3] : ""
        active: emblemSource !== ""
        width: emblemSize; height: emblemSize
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 2
        sourceComponent: Icon {
            anchors.fill: parent
            source: topLeftEmblemLoader.emblemSource
        }
    }

    function isValidDesktopApp(uri) {
        return uri.endsWith(".desktop")
    }
}
