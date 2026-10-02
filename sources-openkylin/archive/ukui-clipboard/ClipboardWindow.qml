import QtQuick 2.15
import QtGraphicalEffects 1.0
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0 as Platform
import ContentMenu 1.0

ApplicationWindow {
    id: clipboardWindow
    visible: false
    title: "Preview Window"

    property int paddings: 4
    property int shadowWidth: 8
    property int windowWidth: 568
    property int windowHeight: 412
    property int maxContentWidth: 540
    property int maxContentHeight: 360
    property int windowRadius: 18
    property var themeColor: Platform.Theme.themeColor
    property var highlightColor: Platform.Theme.color(Platform.Theme.Highlight, Platform.Theme.Active, 1)

    width: windowWidth
    height: windowHeight
    maximumWidth: width
    maximumHeight: height
    minimumWidth: width
    minimumHeight: height
    // 设置窗口背景颜色为透明，以便于显示圆角效果
    color: "transparent"
    onVisibleChanged: {
        if (visible) {
            var pos = backend.getGlobalMousePosition()
            clipboardWindow.x = (Screen.width - clipboardWindow.width) / 2
            clipboardWindow.y = (Screen.height - clipboardWindow.height) / 2
            // 删除下拉菜单里多余的选项并切换到指定界面
            buttonMenu.removeItem();
            var currentType = backend.displayingType
            stack.switchToSpecifiedPage(currentType)
        }
    }

    onThemeColorChanged: {
        highlightColor = Platform.Theme.color(Platform.Theme.Highlight, Platform.Theme.Active, 1)
    }

    Connections {
        target: backend
        function onClipboardWindowVisibleChanged(visible) {
            clipboardWindow.visible = visible
        }
        function onIconThemeChanged() {
            menuBack.restoreIconName = backend.getIcon(backend.displayedIconName)
        }
        function onDisplayedIconNameChanged() {
            menuBack.restoreIconName = backend.getIcon(backend.displayedIconName)
        }
    }

    // 鼠标拖动
    MouseArea {
        anchors.fill: parent
        onPressed: clipboardWindow.startSystemMove()
    }

    Rectangle {
        anchors.fill: parent
        //radius: 8
        color: backend.backgroundColor
        opacity: backend.transparency
    }

    Rectangle {
        id: menuBack
        width: windowWidth
        height: windowHeight
        anchors.centerIn: parent
        color: "transparent"
        //color: backend.backgroundColor // Qt.rgba(255, 255, 255, 0.95)
        //opacity: backend.transparency
        //radius: windowRadius
        clip : true

        property int lastDisplayedType: -1
        property var restoreIconName: backend.getIcon(backend.displayedIconName)

        focus: true
        Keys.enabled: true
        Keys.onPressed: {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter){
                pasteButton.pasted();
            }
        }

        Row {
            id: topLine
            spacing: 0
            Layout.alignment: Qt.AlignRight
            anchors.top: parent.top
            anchors.right: parent.right
            topPadding: 4
            rightPadding: 8
            Rectangle {
                id: restoreBtn
                width: 28
                height: 28
                radius: width / 2
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, 0.05)
                color: backend.displayingType !== backend.originalType ? highlightColor : Qt.rgba(0, 0, 0, 0.03)
                // 深色主题下反色
                UkuiItems.Icon {
                    height: 14
                    width: 14
                    anchors.centerIn: parent
                    mode: backend.displayingType !== backend.originalType ? UkuiItems.Icon.Highlight : UkuiItems.Icon.Normal
                    source: menuBack.restoreIconName
                }

                ToolTip {
                    text: backend.displayingType !== backend.originalType ? qsTr("click to cancel conversion") : qsTr("click to switch to the original format")
                    visible: restoreBtnArea.containsMouse
                }

                MouseArea {
                    id: restoreBtnArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        var originalType = backend.originalType
                        if (originalType === backend.displayingType){
                            // setDisplayingType和switchToSpecifiedPage对顺序有要求
                            backend.setDisplayingType(menuBack.lastDisplayedType)
                            stack.switchToSpecifiedPage(menuBack.lastDisplayedType)
                            return
                        }

                        menuBack.lastDisplayedType = backend.displayingType
                        backend.setDisplayingType(originalType)
                        stack.switchToSpecifiedPage(originalType)
                    }
                }
            }


            Rectangle {
                id: menuBtn
                width: 20
                height: 28
                color: "transparent"

                // 深色主题下反色
                UkuiItems.Icon {
                    height: 16
                    width: 16
                    anchors.centerIn: parent
                    source: backend.getIcon("ukui-down-symbolic");
                    rotation: buttonMenu.visible ? 180 : 0
                }

                ToolTip {
                    text: qsTr("more options")
                    visible: moreOptionArea.containsMouse && !buttonMenu.visible
                }

                MouseArea {
                    id: moreOptionArea
                    anchors.fill: parent
                    hoverEnabled: true
                    property bool menuVisible: false
                    onPressed: {
                        if (!menuVisible) {
                            buttonMenu.open()
                            menuVisible = true
                        } else {
                            menuVisible = false
                        }
                    }
                }
            }
        }

        Item {
            id: menuBackground
            anchors.right: parent.right
            anchors.top: topLine.bottom
            width: buttonMenu.width + paddings * 2
            height: buttonMenu.height + paddings * 2
            visible: buttonMenu.visible

            Repeater {
                id: control
                model: backend.contentMenus
                visible: false

                property int menuItemWidth: 120

                delegate: MenuItem {
                    id: menuItem
                    text: modelData.itemName
                    icon.source:modelData.iconName.toString()
                    highlighted: buttonMenu.currentIndex === index

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: buttonMenu.currentIndex = index
                        onExited: buttonMenu.currentIndex = -1
                        onClicked: menuItem.triggered()
                    }
                    onTriggered: {
                        console.log("ontriggeredddd")
                        var currentType = backend.contentMenus[index].contentType
                        if (currentType === backend.displayingType){
                            return
                        }
                        backend.setDisplayingType(currentType)
                        stack.switchToSpecifiedPage(currentType)
                    }
                    Component.onCompleted: {
//                        var contentItemWidth = 16 + label.contentWidth +
//                                control.count*paddings + paddings * 3
//                        control.menuItemWidth = control.menuItemWidth >= contentItemWidth ?
//                                    control.menuItemWidth : contentItemWidth
                        console.log("additemmmmmmm")
                        buttonMenu.addItem(menuItem)
                    }
                }
            }

            // 关联菜单
            Menu {
                id: buttonMenu
//                width: control.menuItemWidth
                anchors.centerIn: menuBackground
                onCountChanged: {

                    console.log("menu count changedddd", count)
                    for(var i = 0; i < count; i++){
                    console.log("iiiiiiiiii", i, buttonMenu.itemAt(i).text)
                    }
                }

                function removeItem() {
                    for (var i = 0; i < control.model.length; i++) {
                        if(control.model[i].contentType === ContentType.Original) {
                            backend.removeContentMenu(i); // 删除项
                            break; // 只删除第一个匹配的项
                        }
                    }
                }
            }
        }

        StackView {
            id: stack
            replaceEnter: null
            replaceExit: null
            anchors.top: topLine.bottom
            anchors.bottom: buttomLine.top
            anchors.left: parent.left
            anchors.right: parent.right

            function switchToSpecifiedPage(currentType)
            {
                if (currentType === ContentType.PlainText) {
                    replace(currentItem, plainTextComponent.createObject())
                } else if (currentType === ContentType.RichText) {
                    replace(currentItem, richTextComponent.createObject())
                } else if (currentType === ContentType.Image){
                    replace(currentItem, imageComponent.createObject())
                }
            }
        }

        Row {
            id: buttomLine
            spacing: 0
            Layout.alignment: Qt.AlignRight
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            rightPadding: 8
            bottomPadding: 12
            Rectangle {
                id: operateButton
                width: 81 + 2 * paddings
                height: 32 + 2 * paddings
                color: "transparent"

                Button {
                    id: canceBtn
                    anchors.right: seperator.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 32
                    icon{
                        name:"application-exit-symbolic"
                    }

                    ToolTip {
                        text: qsTr("cancel")
                        visible: canceBtn.hovered
                    }

                    onClicked: {
                        clipboardWindow.visible = false
                        backend.setClipboardWindowVisible(false)
                        backend.clearContentMenu()
                        backend.clearImages()
                    }
                }

                Rectangle {
                    id: seperator
                    width: 1
                    height: 30
                    color: "transparent" //Qt.rgba(0, 0, 0, 0.18)
                    anchors.centerIn: parent
                }

                Button {
                    id: pasteButton
                    anchors.left: seperator.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 32
                    icon{
                        name:"object-select-symbolic"
                    }

                    ToolTip {
                        text: qsTr("confirm pasting")
                        visible: pasteButton.hovered
                    }

                    function pasted() {
                        var currentType = backend.displayingType
                        // 使用TextEdit自带的函数将文本拷贝到剪切板
                        if (currentType === ContentType.Image) {
                            backend.copyToClipboard(currentType)
                        } else {
                            stack.currentItem.copyToClipboard()
                        }
                        clipboardWindow.visible = false
                        backend.setClipboardWindowVisible(false)
                        backend.performPasteOperation()
                        backend.clearContentMenu()
                        backend.clearImages()
                    }

                    onClicked: {
                        pasteButton.pasted();
                    }
                }
            }
        }

        Component {
            id: richTextComponent // 富文本项的Component
            TextEditArea {
                visibleWidth: maxContentWidth
                visibleHeight: maxContentHeight
                textFormat: Qt.RichText
            }
        }

        Component {
            id: imageComponent // 图片项的Component
            ImagePreviewArea {
                visibleWidth: maxContentWidth
                visibleHeight: maxContentHeight
            }
        }

        Component {
            id: plainTextComponent // 纯文本项的Component
            TextEditArea {
                visibleWidth: maxContentWidth
                visibleHeight: maxContentHeight
                textFormat: Qt.PlainText
            }
        }
    }
}
