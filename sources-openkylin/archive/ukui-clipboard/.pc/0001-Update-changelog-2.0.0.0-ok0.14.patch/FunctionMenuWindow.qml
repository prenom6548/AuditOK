import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import ContentMenu 1.0
import org.ukui.quick.platform 1.0 as Platform

ApplicationWindow {
    id: contentWindow
    width: optionMenu.width + leftpaddings * 2
    maximumWidth: optionMenu.width + leftpaddings * 2
    minimumWidth: optionMenu.width + leftpaddings * 2
    maximumHeight: optionMenu.minContextHeight + topPaddings * 2
    minimumHeight: maximumHeight
    height: maximumHeight
    title: "Function Menu Window"
    property int leftpaddings: 4
    property int topPaddings: 6
    // 设置窗口背景颜色为透明，以便于显示圆角效果
    color: "transparent"


    Connections {
        target: backend
        function onContentMenuVisibleChanged(visible) {
            contentWindow.visible = visible
        }
    }
    onVisibleChanged: {
        if (visible) {
            optionMenu.visible = true
        }
    }
    onActiveChanged: {
        if (active) {
            // 将移送窗口位置放到onActiveChanged中执行，因为在onVisibleChanged中
            // 移动位置低概率在X11下不生效，从而显示到位置(0, 0)
            var pos = backend.getGlobalMousePosition()
            x = pos.x + 2
            y = pos.y + 2
            backend.processClipboardData()
        }
    }
    onFocusObjectChanged: {
        if (object === null) {
            visible = false
            optionMenu.currentIndex = -1
        }
    }

    background: Rectangle {
        id: windowback
        anchors.fill: parent
        color: "white"
        radius: Platform.Theme.windowRadius
    }

    Menu {
        id: optionMenu
        anchors.centerIn: parent
        width: control.menuItemWidth
        visible: true
        currentIndex: -1
        property int minContextHeight: contentHeight < 32 ? 32 : contentHeight
        background: Rectangle {
            anchors.fill: parent
            color: windowback.color
            radius: Platform.Theme.windowRadius
        }
        onVisibleChanged: {
            if (!visible) {
                contentWindow.visible = false
                currentIndex = -1
            }
        }

        Repeater {
            id: control
            model: backend.contentMenus
            width: menuItemWidth

            property int menuItemWidth: 140 - leftpaddings * 2

            delegate: MenuItem {
                id: menuItem
                width: control.menuItemWidth
                height: 32
                highlighted: optionMenu.currentIndex === index
                contentItem: Label {
                    id: label
                    text: modelData.itemName
                    font: menuItem.font
                    verticalAlignment: Text.AlignVCenter
                    leftInset: 10
                }
                background: Rectangle {
                    anchors.fill: menuItem
                    radius: contentWindow.leftpaddings
                    anchors.centerIn: parent
                    color: menuItem.highlighted ? Qt.rgba(0, 0, 0, 0.05) : "transparent"
                }
                onTriggered: {
                    contentWindow.visible = false
                    optionMenu.currentIndex = -1
                    var currentType =  backend.contentMenus[index].contentType
                    if (currentType === ContentType.Original) {
                        backend.performPasteOperation()
                    } else {
                        backend.setDisplayingType(currentType)
                        backend.showClipboardWindow()
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: optionMenu.currentIndex = index
                    onExited: optionMenu.currentIndex = -1
                    onClicked: menuItem.triggered()
                }
                Component.onCompleted: {
                    control.menuItemWidth = control.menuItemWidth >= (label.contentWidth + leftPadding*2) ?
                                control.menuItemWidth : (label.contentWidth + leftPadding*2)
                }
            }
        }
    }
}
