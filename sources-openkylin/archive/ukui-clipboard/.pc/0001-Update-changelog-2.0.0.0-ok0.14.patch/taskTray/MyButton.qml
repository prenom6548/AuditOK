import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
//import MyComponents 1.0

Rectangle {
    id: myButton
    visible: true
    width: 340
    height: 106
    color: "#99FFFFFF"
    radius: 8
    border.width: 1
    border.color: "gray"

    property bool qmlHasImage: ClipboardData.hasImage
    property date time: ClipboardData.sourceTime
    property string text1 : ClipboardData.sourceText
    property url imageSource: ClipboardData.imageUrl
    property bool isCollected : ClipboardData.isCollected

//    Component.onCompleted: {
//        console.log("hasImage=",hasImage)
//    }

    anchors.margins: {
        top: 10
    }

    Rectangle {
        id: topLable
        visible: true
        width: 320
        height: 18
        //color: "yellow"

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 1
        anchors.top: parent.top

        RelativeTimeLabel {
            id: timeLabel
            targetTime: time
            anchors.right: parent.right
            anchors.rightMargin: 25
            anchors.top: parent.top
            anchors.topMargin: 10
        }

        Rectangle {
            id: copyButton
            visible: false
            width: 16
            height: 16
            anchors.right: topLable.right

            Image {
                id: iconCopy
                source: "./data/copy.svg"
                anchors.fill: copyButton
            }
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    ClipboardData.copyToClipboard()
                }
            }
        }

        Rectangle {
            id: collectionButton
            visible: false
            width: 16
            height: 16
            anchors.right: copyButton.left
            anchors.rightMargin: 8

            Image {
                id: iconColl
                source: ClipboardData.isCollected? "./data/collection.svg" : "./data/uncollection.svg"
                anchors.fill: collectionButton
            }

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    ClipboardData.setIsCollected(!ClipboardData.isCollected)
                }
            }
        }
    }

    function calculateImageComponentWidth() {
        var intWidth = 0
        if(qmlHasImage) {
            if (text1 == "scaledImageString") {
                intWidth = 324
            } else {
                intWidth = 48
            }
        } else {
            intWidth = 0
        }
        return intWidth;
    }

    function calculateTextItemWidth() {
        var intWidth = 0
        if(qmlHasImage) {
            if (text1 == "scaledImageString") {
                intWidth = 0
            } else {
                intWidth = 240
            }
        } else {
            intWidth = 324
        }
        return intWidth;
    }

    RowLayout {
        anchors.centerIn: parent
        width: 324
        height: 68
        anchors.left: myButton.left
        anchors.leftMargin: 11
        anchors.top: topLable.bottom

        Image {
            id: imageComponent
            source: imageSource
            Layout.preferredWidth: calculateImageComponentWidth()
            Layout.preferredHeight: 48
            visible: true
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
        }

        Item {
            id: textItem
            Layout.preferredHeight: 68
            Layout.preferredWidth: calculateTextItemWidth()

            Text {
                id: textComponent
                text: text1
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter // 确保垂直居中
                horizontalAlignment: Text.AlignLeft // 确保水平左对齐
                width: parent.width
                height: parent.height
                clip: true // 确保内容不会溢出容器边界
            }
        }

    }

    //myButton的MouseArea必须放在后面
    //不然收藏和复制按钮会出现闪动现象
    MouseArea{
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true

        onEntered: {
            parent.border.color = "#1D72F2"
            timeLabel.visible = false
            copyButton.visible = true
            collectionButton.visible = true
        }
        onExited: {
            parent.border.color = "gray"
            copyButton.visible = false
            collectionButton.visible = false
            timeLabel.visible = true
        }
    }
}
