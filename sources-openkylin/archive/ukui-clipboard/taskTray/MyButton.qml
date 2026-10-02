import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
//import MyComponents 1.0

Rectangle {
    id: myButton
    visible: true
    width: 352
    height: 106
    radius: 8
    // border.width: 1
    // border.color: isPressed ? "1D72F2" : "gray"

    property bool qmlHasImage: ClipboardData.hasImage
    //property date time: ClipboardData.sourceTime
    property string text1 : ClipboardData.sourceText
    property url imageSource: ClipboardData.imageUrl
    property bool isCollected : ClipboardData.isCollected
    property bool isPressed: false

    property string myBtnNolColor: tray.dtColor("myBtnNolCol")
    property string myBtnHovColor: tray.dtColor("myBtnHovCol")
    property string myBtnClkColor: tray.dtColor("myBtnClkCol")
    property string myBtnTextColor: tray.dtColor("textCol")

    color: myBtnNolColor

    Connections {
        target: tray
        function onStyleThemeChanged() {
            //myButton.update()
            myBtnNolColor = tray.dtColor("myBtnNolCol")
            myBtnHovColor = tray.dtColor("myBtnHovCol")
            myBtnClkColor = tray.dtColor("myBtnClkCol")
            myBtnTextColor = tray.dtColor("textCol")
            myButton.color = myBtnNolColor
        }
    }

    function buttonEnter() {
        myButton.color = myBtnHovColor
        timeLabel.visible = false
        copyButton.visible = true
        collectionButton.visible = true
    }

    function buttonExited() {
        myButton.color = myBtnNolColor
        copyButton.visible = false
        collectionButton.visible = false
        timeLabel.visible = true
    }

    MouseArea{
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onEntered: {
            myButton.buttonEnter()
        }
        onExited: {
            myButton.buttonExited()
        }
        onPressed: (mouse) =>{
            if (mouse.button === Qt.RightButton) {
                console.log("右键按下")
                contextMenu.popup(mouse.x,mouse.y)
            }
        }
        onDoubleClicked: (mouse) =>{
            if (mouse.button === Qt.LeftButton) {
                ClipboardData.sendNeedMoveToFirst()
            }
        }
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("copy")
            onTriggered: ClipboardData.sendNeedMoveToFirst()
        }

        MenuItem {
            text: qsTr("delete")
            onTriggered: ClipboardData.sendNeedToDelete()
        }

        MenuItem {
            text: qsTr("delete all")
            onTriggered: {
                contextMenu.visible = false
                ClipboardData.needToDeleteAll()
            }
        }

        MenuItem {
            text: qsTr("open image")
            visible: ClipboardData.getMode() === 1
            onTriggered: tray.openImageOrFile(ClipboardData.getFilePath())
        }

        MenuItem {
            text: qsTr("open file")
            visible: ClipboardData.getMode() === 2
            onTriggered: tray.openImageOrFile(ClipboardData.getFilePath())
        }

        MenuItem {
            text: qsTr("open file location")
            visible: ClipboardData.getMode() === 1 || ClipboardData.getMode() === 2
            onTriggered: tray.openDir(ClipboardData.getFilePath())
        }
    }

    Rectangle {
        id: topLable
        visible: true
        width: 320
        height: 20
        color: "transparent"

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 5
        anchors.top: parent.top

        RelativeTimeLabel {
            id: timeLabel
            targetTime: ClipboardData.sourceTime
            fontClo: myBtnTextColor
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 10

            width: 150
        }

        Button {
            id: copyButton
            visible: false
            width: 16
            height: 16
            anchors.right: topLable.right
            ToolTip.visible: hovered
            ToolTip.text: qsTr("copy")
            flat: true  //设置底色为透明

            onHoveredChanged: {
                if (hovered) {
                    myButton.buttonEnter()
                } else {
                    myButton.buttonExited()
                }
            }

            icon{
                name:"edit-copy-symbolic"
            }

            onClicked: {
                ClipboardData.sendNeedMoveToFirst()
            }
        }

        Button {
            id: collectionButton
            visible: false
            width: 16
            height: 16
            anchors.right: copyButton.left
            anchors.rightMargin: 8
            ToolTip.visible: hovered
            ToolTip.text: ClipboardData.isCollected? qsTr("unfavorite") : qsTr("favorite")
            flat: true  //设置底色为透明

            onHoveredChanged: {
                if (hovered) {
                    myButton.buttonEnter()
                } else {
                    myButton.buttonExited()
                }
            }

            icon{
                name: ClipboardData.isCollected? "starred-symbolic" : "non-starred-symbolic"
            }

            onClicked: {
                ClipboardData.setIsCollected(!ClipboardData.isCollected)
            }
        }
    }

    function calculateIconComponentWidth() {
        var intWidth = 0
        if(qmlHasImage) {
            if (text1 == "scaledImageString") {
                intWidth = 0
            } else {
                intWidth = 48
            }
        } else {
            intWidth = 0
        }
        return intWidth
    }

    function calculateImageComponentWidth() {
        var intWidth = 0
        if(qmlHasImage) {
            if (text1 == "scaledImageString") {
                intWidth = 332
            } else {
                intWidth = 0
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
                intWidth = 252
            }
        } else {
            intWidth = 332
        }
        return intWidth;
    }

    RowLayout {
        anchors.fill: parent
        width: 336
        height: 68
        anchors.left: myButton.left
        anchors.leftMargin: 11
        anchors.top: topLable.bottom
        anchors.topMargin: 8
        spacing: 0

        Label {
            id: iconComponent
            height: 48
            width: calculateIconComponentWidth()
            UkuiItems.Icon {
                height: 48
                width: calculateIconComponentWidth()
                anchors.centerIn: parent
                source: ClipboardData.getIconName()
            }

        }

        Image {
            id: imageComponent
            source: ClipboardData.imageUrl
            Layout.preferredWidth: calculateImageComponentWidth()
            Layout.preferredHeight: 48
            visible: true
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
        }

        Item {
            id: textItem
            Layout.preferredHeight: 68
            //Layout.preferredWidth: calculateTextItemWidth()
            //visible: true
            Layout.preferredWidth: ClipboardData.isLinkMode? 0 : calculateTextItemWidth()
            visible: !ClipboardData.isLinkMode

            Label {
                id: textComponent
                text: text1
                wrapMode: Text.WrapAnywhere
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter // 确保垂直居中
                horizontalAlignment: Text.AlignLeft // 确保水平左对齐
                width: parent.width
                height: parent.height
                //clip: true // 确保内容不会溢出容器边界

                ToolTip {
                    property var maxWidth: 400
                    property var wrapMode: Text.WrapAnywhere
                    //visible: textItemMouseArea.containsMouse ? textComponent.truncated : false
                    visible: textItemMouseArea.containsMouse ? (ClipboardData.getMode() === 2 || (textComponent.truncated && ClipboardData.getMode() === 0)) : false
                    text: ClipboardData.getToolTipString()
                }

                MouseArea {
                    id: textItemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: {
                        myButton.buttonEnter()
                    }

                    onExited:  {
                        myButton.buttonExited()
                    }

                    onDoubleClicked: {
                        ClipboardData.sendNeedMoveToFirst()
                    }
                }

                onFontChanged: {
                    linkComponent.font = font
                    console.log("size=",font.pixelSize)
                }
            }
        }

        Item {
            id: linkitem
            Layout.preferredHeight: 68
            Layout.preferredWidth: ClipboardData.isLinkMode? calculateTextItemWidth() : 0
            visible: ClipboardData.isLinkMode

            Text {
                id: linkComponent
                text: ClipboardData.isLinkMode? text1 : ""
                font: textComponent.font
                color: tray.dtColor("link-active")
                width: ClipboardData.isLinkMode? 332 : 0
                onYChanged: {
                    if (y < 0) {
                        y = 0
                    }
                }
                onHeightChanged: {
                    if (linkComponent.height > 68) {
                        linkComponent.height = 68
                    }
                }

                ToolTip {
                    property var maxWidth: 400
                    property var wrapMode: Text.WrapAnywhere
                    visible: textMouseArea.containsMouse ? linkComponent.truncated : false
                    text: linkComponent.text
                }
                anchors.centerIn: parent
                wrapMode: Text.WrapAnywhere
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                MouseArea {
                    id: textMouseArea
                    anchors.fill: linkComponent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    propagateComposedEvents: true

                    onEntered: {
                        linkComponent.font.underline = true
                        myButton.buttonEnter()
                    }

                    onExited:  {
                        linkComponent.font.underline = false
                        myButton.buttonExited()
                    }

                    onClicked: {
                        tray.xdgOpen(text1)
                        mouse.accepted = true;
                    }
                }
            }
        }
    }
}
