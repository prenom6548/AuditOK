import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: myTabButton
    width: 56
    height: 32
    hoverEnabled: true
    property string colorValue : "#99FFFFFF"

    contentItem: Text {
        text: myTabButton.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 14
        color: "#1A1A1A"
    }

    //myTabButton.text会被下方设置的属性遮住
    background: Rectangle{
        id: myTabButtonRec
        border.width: 1
        radius: 5
        border.color: "transparent"
        color: colorValue
    }

//    Component.onCompleted: {
//        myTabButtonRec.color = Qt.binding(function() { return MouseArea.entered()?  "#0f000000" : "#99FFFFFF"})
//    }

//    MouseArea{
//        //anchors.fill: parent会覆盖外部的onClicked函数，得在MouseArea中重新实现
//        anchors.fill: parent
//        hoverEnabled: true

//        onEntered: {
//            myTabButtonRec.color = "#0f000000"
//        }

//        onExited: {
//            myTabButtonRec.color = "#99FFFFFF"
//        }
//    }
}
