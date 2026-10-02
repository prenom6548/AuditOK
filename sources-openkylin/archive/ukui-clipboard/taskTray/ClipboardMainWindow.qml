import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems

ApplicationWindow {
    id: mainwindow
    visible: tray.windowVisible
    title: "Ukui-clipboard MainWindow"
    color: "transparent"
    // opacity: 0.3

    width: 384
    height: 600
    minimumWidth: 384
    minimumHeight: 600
    maximumWidth: 384
    maximumHeight: 600

    // onClosing: {
    //     tray.setWindowVisible(false)
    //     return false
    // }

    Rectangle {
        id: backWindow
        anchors.fill: parent
        color: mainwindow.palette.window
        opacity: tray.transparency
    }

    onActiveChanged: {
        if (active) {
            backWindow.opacity = tray.transparency
        } else {
            backWindow.opacity = 1.0
        }
        tray.setWindowBlurEffect(active)
    }

    Rectangle {
        id: titleBar
        height: 40
        color: "transparent"

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        MouseArea {
            anchors.fill: parent
            onPressed: mainwindow.startSystemMove()
        }

        Row {
            id: leftContent
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            layoutDirection: tray.isMirror ? Qt.RightToLeft : Qt.LeftToRight

            UkuiItems.Icon {
                height: 24
                width: 24
                anchors.verticalCenter: parent.verticalCenter
                source: "ukui-clipboard"
            }

            Label {
                id: appTitle
                text: qsTr("ukui-clipboard")
                anchors.verticalCenter: parent.verticalCenter
                property var windowLabel: true
            }
        }

        Row {
            id: rightContent
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2
            layoutDirection: tray.isMirror ? Qt.RightToLeft : Qt.LeftToRight
            Button {
                id: fixedButton
                width: 32
                height: 32
                Layout.preferredHeight: 32
                Layout.preferredWidth: 32
                Layout.maximumHeight: 32
                Layout.maximumWidth: 32
                Layout.minimumHeight: 32
                Layout.minimumWidth: 32
                property var buttonType: "windowTitleBarWindowButton"

                flat: true
                checkable: true
                icon{
                    name: fixedButton.checked ? "ukui-fixed-symbolic" : "ukui-unfixed-symbolic"
                }

                ToolTip.visible: hovered
                ToolTip.text: tray.keepAbove ? qsTr("cancel keep above") : qsTr("keep above")

                onClicked: tray.setkeepAbove(!tray.keepAbove)
            }

            // Button {
            //     width: 32
            //     height: 32
            //     Layout.preferredHeight: 32
            //     Layout.preferredWidth: 32
            //     Layout.maximumHeight: 32
            //     Layout.maximumWidth: 32
            //     Layout.minimumHeight: 32
            //     Layout.minimumWidth: 32

            //     flat: true
            //     icon{
            //         name:"open-menu-symbolic"
            //     }

            //     //onClicked: tray.setWindowVisible(false)
            // }

            Button {
                width: 32
                height: 32
                Layout.preferredHeight: 32
                Layout.preferredWidth: 32
                Layout.maximumHeight: 32
                Layout.maximumWidth: 32
                Layout.minimumHeight: 32
                Layout.minimumWidth: 32
                property var buttonType: "windowTitleBarCloseButton"

                flat: true
                icon{
                    name:"window-close-symbolic"
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("close")

                onClicked: tray.setWindowVisible(false)
            }
        }

        Component.onCompleted: {
            if (!tray.isMirror) {
                leftContent.anchors.left = titleBar.left
                leftContent.anchors.leftMargin = 8

                rightContent.anchors.right = titleBar.right
                rightContent.anchors.rightMargin = 5
            } else {
                leftContent.anchors.right = titleBar.right
                leftContent.anchors.rightMargin = 8

                rightContent.anchors.left = titleBar.left
                rightContent.anchors.leftMargin = 5
            }
        }
    }

    ButtonGroup {
        id: buttonGroup
        exclusive: true
        buttons: rowlayout.children
    }

    Row {
        id: rowlayout
        width: 341
        height: 32
        spacing: 0
        anchors.top: titleBar.bottom
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 16

        MyTabButton {
            id: allButt
            buttonText: qsTr("All")
            checked: true

            onClicked: tray.setCurrentModel(tray.models[0])
            ButtonGroup.group: buttonGroup
        }
        MyTabButton {
            id: textButt
            buttonText: qsTr("Text")

            onClicked: tray.setCurrentModel(tray.models[1])
            ButtonGroup.group: buttonGroup
        }
        MyTabButton {
            id: imageButt
            buttonText: qsTr("Image")

            onClicked: tray.setCurrentModel(tray.models[2])
            ButtonGroup.group: buttonGroup
        }
        MyTabButton {
            id: fileButt
            buttonText: qsTr("File")

            onClicked: tray.setCurrentModel(tray.models[3])
            ButtonGroup.group: buttonGroup
        }
        MyTabButton {
            id: linkButt
            buttonText: qsTr("Link")

            onClicked: tray.setCurrentModel(tray.models[4])
            ButtonGroup.group: buttonGroup
        }
        MyTabButton {
            id: collectionButt
            buttonText: qsTr("Collection")

            onClicked: tray.setCurrentModel(tray.models[5])
            ButtonGroup.group: buttonGroup
        }
    }

    ListView {
        id: mainview
        width: 384
        height: 500
        visible: true

        anchors.top: rowlayout.bottom
        anchors.topMargin: 14
        spacing: 10

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            visible:  false
        }

        onContentHeightChanged: { //动态显示滑动条
            scrollBar.visible = Qt.binding(function() { return mainview.model.getModelCount() > 4 ? true : false})
        }

        clip: true
        model: tray.currentModel
        delegate: buttoncomponent

        Component {
            id: buttoncomponent
            MyButton {
                id: mybutton
                anchors.left: parent.left
                anchors.leftMargin: 16
            }
        }
    }
}


