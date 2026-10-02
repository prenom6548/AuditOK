import QtQuick 2.0

//标题部分
Item {
    id:titleWidget
    anchors.fill: parent
    width: 100
    height: 40

    Image{
        x:12
        y:12
        id:iconLabelImage
        height: 24
        width:24
        source: "/Icon/new.png"
    }

    Text{
        x:47
        y:12
        z:3
        id:iconLabelText
        text: qsTr("Introduction to new functions")
        font.pointSize: 12
    }


}
