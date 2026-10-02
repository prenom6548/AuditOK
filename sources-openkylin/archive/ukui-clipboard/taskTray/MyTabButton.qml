import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: myTabButton
    width: 58
    height: 32
    checkable: true

    property bool isEnter: false
    property bool isPressed: false
    property string checkColor : tray.dtColor("checkCol")
    property string clickColor : tray.dtColor("clickCol")
    property string hoverColor : tray.dtColor("hoverCol")

    property string buttonText

    contentItem: Label {
        text: buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        wrapMode: Text.NoWrap
        property bool isElided: truncated
    }

    ToolTip.visible: hovered && contentItem.isElided
    ToolTip.text: buttonText

    Connections {
        target: tray
        function onStyleThemeChanged() {
            checkColor = tray.dtColor("checkCol")
            clickColor = tray.dtColor("clickCol")
            hoverColor = tray.dtColor("hoverCol")
            myTabButton.update()
        }
    }

    signal clicked()

    background: Rectangle{
        id: rec
        color: {
            if (isPressed) {
                return clickColor
            } else if (checked) {
                return checkColor
            } else if (isEnter) {
                return hoverColor
            } else {
                return "transparent"
            }
        }
        radius: 5
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            isPressed = true
            myTabButton.clicked()
            checked = true
        }

        onReleased: {
            isPressed = false
        }

        onEntered: {
            myTabButton.isEnter = true
        }

        onExited: {
            myTabButton.isEnter = false
        }
    }

}
