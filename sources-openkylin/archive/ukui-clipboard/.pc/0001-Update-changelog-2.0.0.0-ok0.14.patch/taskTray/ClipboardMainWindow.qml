import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainwindow
    visible: tray.windowVisible
    title: qsTr("Ukui-clipboardd MainWindow")
    color: "#99FFFFFF"

    //去不掉最小化按钮
    flags: Qt.Window |
           Qt.CustomizeWindowHint |
           Qt.WindowTitleHint |
           Qt.WindowStaysOnTopHint |
           Qt.WindowCloseButtonHint &
           ~Qt.WindowMinimizeButtonHint &
           ~Qt.WindowMaximizeButtonHint

    width: 372
    height: 600
    minimumWidth: 372
    minimumHeight: 600
    maximumWidth: 372
    maximumHeight: 600

    property string enterColor : "#0f000000"
    property string exitColor : "#99FFFFFF"

    onClosing: {
        tray.setWindowVisible(false)
        return false
    }

    Row {
        id: rowlayout
        width: 341
        height: 32
        spacing: 0
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 16

        MyTabButton {
            id: allButt
            text: qsTr("All")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tray.setCurrentModel(tray.models[0])
                }
                onEntered: {
                    allButt.colorValue = enterColor
                }

                onExited: {
                    allButt.colorValue = exitColor
                }
            }

        }
        MyTabButton {
            id: textButt
            text: qsTr("Text")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tray.setCurrentModel(tray.models[1])
                }
                onEntered: {
                    textButt.colorValue = enterColor
                }

                onExited: {
                    textButt.colorValue = exitColor
                }
            }
        }
        MyTabButton {
            id: imageButt
            text: qsTr("Image")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tray.setCurrentModel(tray.models[2])
                }
                onEntered: {
                    imageButt.colorValue = enterColor
                }

                onExited: {
                    imageButt.colorValue = exitColor
                }
            }
        }
        MyTabButton {
            id: fileButt
            text: qsTr("File")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tray.setCurrentModel(tray.models[3])
                }
                onEntered: {
                    fileButt.colorValue = enterColor
                }

                onExited: {
                    fileButt.colorValue = exitColor
                }
            }
        }
        MyTabButton {
            id: linkButt
            text: qsTr("Link")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    tray.setCurrentModel(tray.models[4])
                }
                onEntered: {
                    linkButt.colorValue = enterColor
                }

                onExited: {
                    linkButt.colorValue = exitColor
                }
            }
        }
        MyTabButton {
            id: collectionButt
            text: qsTr("Collection")
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    console.log("collectionButt is Clicked")
                    tray.setCurrentModel(tray.models[5])
                }
                onEntered: {
                    collectionButt.colorValue = enterColor
                }

                onExited: {
                    collectionButt.colorValue = exitColor
                }
            }
        }
    }

    ListView {
        id: mainview
        width: 372
        height: 545
        visible: true

        anchors.top: rowlayout.bottom
        anchors.topMargin: 14
        spacing: 10

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            visible: true
            //policy: ScrollBar.AlwaysOn
            background: Control {
                background: Rectangle {
                    color: "lightgray"
                    radius: 4
                    width: 6
                }
            }
        }

//        Component.onCompleted: {
//            scrollBar.visible = Qt.binding(function() { return mainview.model.count > 4 ? true : false})
//        }

        clip: true
        model: tray.currentModel
        //model: 100
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


