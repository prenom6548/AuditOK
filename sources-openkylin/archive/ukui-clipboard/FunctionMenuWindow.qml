import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import ContentMenu 1.0
import org.ukui.quick.platform 1.0 as Platform

ApplicationWindow {
    id: contentWindow
    maximumWidth: 140
    minimumWidth: 140
    maximumHeight: optionMenu.minContextHeight + topPaddings * 2
    minimumHeight: maximumHeight
    height: maximumHeight
    width: 140
    title: "Function Menu Window"
    property int leftpaddings: 4
    property int topPaddings: 6
    // 设置窗口背景颜色为透明，以便于显示圆角效果
    color: "transparent"
    visible: false

    Connections {
        target: backend
        function onContentMenuVisibleChanged(visible) {
            contentWindow.visible = visible
        }
    }

    // onActiveChanged: {
    //     if (active) {
    //         backend.processClipboardData()
    //     }
    // }

    onFocusObjectChanged: {
        if (object === null) {
            visible = false
        }
    }

    Rectangle {
        anchors.fill: parent
        color: contentWindow.palette.window
        opacity: backend.transparency
    }

    ListView {
        id: optionMenu
        width: 132
        property int minContextHeight: contentHeight < 32 ? 32:contentHeight
        anchors.centerIn: parent
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.topMargin: 6
        model: backend.contentMenus
        visible: true
        clip: true
        spacing: 1

        delegate: MenuItem {
            id: menuItem
            text: modelData.itemName
            width: 132
            height: 32

            onTriggered: {
                contentWindow.visible = false
                var currentType =  backend.contentMenus[index].contentType
                if (currentType === ContentType.Original) {
                    backend.performPasteOperation()
                } else {
                    backend.setDisplayingType(currentType)
                    backend.showClipboardWindow()
                }
            }
        }
    }
}
