import QtQuick 2.15
import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import org.ukui.quick.items 1.0
import org.ukui.quick.platform 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4 as QQC1
import org.ukui.peony.desktop 1.0
import QtQuick.Controls.Material 2.12
import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.15


WidgetItem {
    id: root
    property int instanceId: root.Widget.instanceId
    property int screenId: root.widget.container.instanceId
    property int origanizationType : Widget.config.organizationType
    property bool expand: DesktopOrganizationManager.isExpand(instanceId)
    property bool init: false
    property bool textFieldvisiable: false
    property bool ismultifilemode: false
    property bool islistModel: DesktopOrganizationManager.getIsListMode(instanceId)
    property int  initHeight: 0
    property bool needInitHeight: false
    property int  preheight: 0
    property int  lastExpanedHeight: 0
    property string firsturi: ""
    property bool operationBymenu: false
    readonly property var widgetFocusScope: parent
    readonly property var widgetsLayout: widgetFocusScope.widgetsLayoutRef
    readonly property int currentActiveFocusInstanceId: widgetsLayout.currentActiveFocusinstanceId
    property bool activefocus: widgetFocusScope.activeFocus
    property int m_lastIndex: -1
    property var m_renameStartTime: 0
    property var m_renameTitleStartTime: 0
    property var tooltipsText: ""
    // property int desktopGridWidth: Controller.gridSize.width
    // property int desktopGridHeight: Controller.gridSize.height
    // 两次间隔点击重命名定时器（与桌面一致，3秒超时）
    Timer {
        id: m_renameTimer
        interval: 3000
        onTriggered: {
            m_lastIndex = -1
	}
    }
    Timer {
        id: m_renametitleTimer
        interval: 3000
    }
    onActivefocusChanged: {
        if (activefocus == false && currentActiveFocusInstanceId!=instanceId) {
            selectionModel.clearSelection()
        }
    }
    states: [
        State {
            name: "collapsed"
            when: !expand
            PropertyChanges {
                target: root
                height: title.height
            }
            // PropertyChanges {
            //     target: rootradiusitem
            //     visible: true
            // }
        }
        // State {
        //     name: "expanded"
        //     when: expand /*&& true*/ /*DesktopOrganizationManager.isNeedOriginHeight(instanceId)*/
        //     PropertyChanges {
        //         target: root
        //         height: initHeight
        //     }
        //     // PropertyChanges {
        //     //     target: rootradiusitem
        //     //     visible: true
        //     // }
        // }
    ]
    // 动画过渡
    NumberAnimation{
          id: heightAnimatorcollapsed
          target: root
          duration: 200
          property: "height"
          from: preheight
          to: title.height
          easing.type: Easing.InOutQuad
    }
    NumberAnimation{
          id: heightAnimatorexpanded
          target: root
          duration: 200
          property: "height"
          from: title.height
          to: lastExpanedHeight
          easing.type: Easing.InOutQuad
    }
    // todo: 在addWidget之前通过config设置初始大小
    Component.onCompleted: {
        var init = DesktopUKUIpluginnManager.isNeedInit(instanceId)
         if(init){
             DesktopOrganizationManager.setGirdSizeByModel(desktopItemProxyModel, instanceId)
         }

        initHeight = DesktopOrganizationManager.getInitHeight(instanceId);
        needInitHeight = !expand
    }

    function requestExpandState(newExpand) {
        preheight = root.height
        DesktopOrganizationManager.expandStatusChanged(instanceId, newExpand, preheight)
    }

    function applyExpandState(newExpand) {
        if (expand === newExpand) {
            return
        }

        preheight = root.height
        expand = newExpand
        if (expand) {
            if (needInitHeight) {
                // 首次展开时使用 C++ 侧记录的高度。
                needInitHeight = false
                lastExpanedHeight = initHeight
            } else if (lastExpanedHeight <= title.height) {
                lastExpanedHeight = Math.max(initHeight, preheight)
            }
            heightAnimatorexpanded.start()
	     Qt.callLater(function() {
                       widgetFocusScope.forceActiveFocus()
              })
        } else {
            lastExpanedHeight = preheight
            heightAnimatorcollapsed.start()
        }
    }

    Connections {
        target: DesktopOrganizationManager

        function onExpandStatusChangedRequest(id, newExpand) {
            if (id === instanceId) {
                applyExpandState(newExpand)
            }
        }

        function onRenameFocusRequest(id, posx) {
            if ((posx <= (title.width - titletext.width) / 2
                    || posx >= (title.width / 2 + titletext.width / 2))
                    && !operationBymenu) {
                if (id === instanceId) {
                    requestExpandState(!expand)
                }
            } else {
                if (id === instanceId) {
                    nameField.focus = true
                    textFieldvisiable = true
                }
                if (operationBymenu) {
                    operationBymenu = false
                }
            }
        }

        function onRenameRequest(id, newName, needFocus) {
            if (id === instanceId) {
                nameField.text = newName
                if (needFocus) {
                    nameField.focus = true
                    textFieldvisiable = true
                }
            }
        }

        function onCancelEditRequest() {
            nameField.focus = false
            textFieldvisiable = false
        }

        function onCreateFileAndEditRequest(id, uri) {
            if (id === instanceId) {
                startEditForUri(uri)
            }
        }
    }

    Rectangle {
        id: title
        width: parent.width
        height: 32
        color: expand ? "transparent" : !title.hovered ? "transparent" : "#26FFFFFF"
        radius: GlobalTheme.kRadiusWindow
        clip: true // 启用裁剪
        // 覆盖下半部分的圆角
        property bool hovered: false
        anchors.topMargin: 1
        Rectangle {
            anchors.fill: parent
            radius: GlobalTheme.kRadiusWindow
            opacity: 1 //tabbtn.checked?0.7:1
            anchors.rightMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 0
            anchors.bottomMargin: -10
            color: !title.hovered ? "transparent" : "#26FFFFFF"
            visible: expand
        }
        MouseArea {
            id:titlearea
            anchors.fill: parent
            hoverEnabled: true // 启用悬停检测
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onPressed: {
                widgetFocusScope.forceActiveFocus()
                if (mouse.button === Qt.RightButton) {
                    mouse.accepted = true  // 阻止右键传递到桌面
                } else {
                    mouse.accepted = false
                    if (!m_renametitleTimer.running) {
                        if(!textFieldvisiable){
                            m_renameTitleStartTime = Date.now()
                            m_renametitleTimer.restart()
                        }
                        DesktopOrganizationManager.cancelEditRequest()
                    } else {
                        // 检查是否满足重命名条件
                        DesktopOrganizationManager.cancelEditRequest()
                        if (mouse.button === Qt.LeftButton) {
                            var elapsed = Date.now() - m_renameTitleStartTime
                            var remaining = 3000 - elapsed
                            var renameWindow = 3000 - 400 // 3000ms超时，400ms双击间隔
                            if (remaining > 0 && remaining < renameWindow) {
                                // 触发重命名
                                var posx = mouseX
                                if (!(posx <= (title.width - titletext.width) / 2
                                        || posx >= (title.width / 2 + titletext.width / 2))) {
                                    // if (id === instanceId) {
                                        nameField.focus = true
                                        textFieldvisiable = true
                                    //}
                                }
                                m_renametitleTimer.stop()
                            }
                        }
                    }
                }

            }
            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    mouse.accepted = true  // 阻止右键传递到桌面
                    var pos = titlearea.mapToGlobal(mouse.x, mouse.y)
                    DesktopOrganizationManager.showMenuRequest(pos, instanceId)
                } else {
                    mouse.accepted = false
                }
            }
            // 当鼠标进入时
            onEntered: {
                title.hovered = true
            }

            // 当鼠标离开时
            onExited: {
                title.hovered = false
            }
        }
        Button {
            opacity: 1
            id: expandbtn
            width: 20
            height: 20
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            antialiasing: true
            icon.name: expand ? "ukui-up-symbolic/highlight" : "arrow-down/highlight"
            icon.height: 20
            icon.width: 20
            hoverEnabled: true
            objectName: "Ukui-idm_StyledButton_Icon"
            Accessible.name: objectName
            flat: true
//            background: Rectangle {
//                color: "transparent"
//            }
            // 简化点击处理
            onClicked: {
                requestExpandState(!expand)
            }
        }
        Button {
            opacity: 1
            id: optionbtn
            width: 24
            height: 24
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            antialiasing: true
            icon.name: "open-menu-symbolic/highlight"
            icon.height: 20
            icon.width: 20
            hoverEnabled: true
            objectName: "Ukui-idm_StyledButtonOption_Icon"
            Accessible.name: objectName
            flat: true
            onClicked: {
                var pos = optionbtn.mapToGlobal(0, optionbtn.height)
                operationBymenu =  true
                DesktopOrganizationManager.showMenuRequest(pos, instanceId,false)
            }
        }
        Rectangle {
            id:titletext
            visible: !textFieldvisiable
            radius: 4
            color: "transparent"
            anchors.centerIn: parent
            height: 30
            width: Math.min(name.implicitWidth, title.width - 80)
            Text {
                height: 30
                font.pixelSize: 10
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
                id: name
                text: nameField.text
                color: "white"
                elide: Text.ElideRight
                width: Math.min(name.implicitWidth, title.width - 80)
            }
            DropShadow {
                anchors.fill: name
                source: name
                horizontalOffset: 1
                verticalOffset: 2
                radius: 3
                samples: radius * 2
                color: "#ff000000"
                z: -1
                layer.enabled: true
            }
            ToolTip {
                property int maxWidth: 400
                id: orgnameToolTip
                // 此处绑定方法会导致ukui主题tooltip概率性偏移，需要用其它方式实现
                //text: visible? model.tooltip: ""
                visible: title.hovered
                         && title.width - 80 == nameField.width //&& !dragHovered
                delay: 200
                closePolicy: Popup.CloseOnPressOutside
//                property int textFormat: Qt.RichText
                property var wrapMode: Text.WrapAnywhere
                // 动态更新tooltip内容
                onAboutToShow: {
                    text = nameField.text
                }
            }
        }
        TextField {
            id: nameField
            z: 1
            visible: textFieldvisiable
            text: Widget.config.name
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            height: 30
            width: Math.min(name.implicitWidth + 30, title.width - 80)
            font.pixelSize: 11
            property string prename: Widget.config.name
            implicitHeight: parent.height - 8
            property bool needCleanBtn: false
            property bool truncating: false
            property bool contextMenuEnabled: true
            MouseArea {
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                anchors.fill: parent
                propagateComposedEvents: true
                enabled: true
                hoverEnabled: true
                onPressed: {
                    mouse.accepted = false
                }
                onClicked: {
                    if(mouse.button == Qt.RightButton){
                        mouse.accepted = true
                    }
                }
            }
            //cleanButonforceHide:true
            // background: Rectangle {
            //     implicitHeight: parent.height - 8
            //     radius: 6
            //     border.color: nameField.focus ? "#4E75FF" : "transparent"
            //     border.width: 2
            //     color: nameField.focus ? "red" : "transparent"
            // }
            // UTF-8字节截断函数，与idm-actions-manager.cpp保持一致
            function truncateToByteLimit(text, maxBytes) {
                var utf8Bytes = 0
                var truncatePos = 0
                for (var i = 0; i < text.length; i++) {
                    var code = text.charCodeAt(i)
                    if (code < 0x80) {
                        utf8Bytes += 1
                    } else if (code < 0x800) {
                        utf8Bytes += 2
                    } else if (code < 0x10000) {
                        utf8Bytes += 3
                    } else {
                        utf8Bytes += 4
                    }
                    if (utf8Bytes <= maxBytes) {
                        truncatePos = i + 1
                    }
                }
                if (utf8Bytes <= maxBytes) {
                    return text
                }
                return text.substring(0, truncatePos)
            }
            onTextChanged: {
                if (truncating) return
                var currentText = nameField.text
                var utf8Data = []
                var i = 0
                while (i < currentText.length) {
                    var code = currentText.charCodeAt(i)
                    if (code < 0x80) {
                        utf8Data.push(code)
                    } else if (code < 0x800) {
                        utf8Data.push(0xC0 | (code >> 6))
                        utf8Data.push(0x80 | (code & 0x3F))
                    } else if (code < 0x10000) {
                        utf8Data.push(0xE0 | (code >> 12))
                        utf8Data.push(0x80 | ((code >> 6) & 0x3F))
                        utf8Data.push(0x80 | (code & 0x3F))
                    } else {
                        utf8Data.push(0xF0 | (code >> 18))
                        utf8Data.push(0x80 | ((code >> 12) & 0x3F))
                        utf8Data.push(0x80 | ((code >> 6) & 0x3F))
                        utf8Data.push(0x80 | (code & 0x3F))
                    }
                    i++
                }
                if (utf8Data.length > 255) {
                    truncating = true
                    var truncatedText = truncateToByteLimit(currentText, 255)
                    nameField.text = truncatedText
                    truncating = false
                }
            }
            Keys.onShortcutOverride: {
                if (event.key === Qt.Key_Escape) {
                    console.log("ESC intercepted by onShortcutOverride")
                    nameField.text = prename
                    name.text = prename
                    DesktopOrganizationManager.cancelEdit()
                }
            }
            onAccepted: {
                DesktopOrganizationManager.cancelEdit()
                commitRename(text)
                name.text = text
                prename = text
            }

            onActiveFocusChanged: {
                if (!nameField.activeFocus /*&& editName.visible*/
                        ) {
                    console.log("失去焦点，提交重命名")
                    commitRename(text)
                    prename = text
                    name.text = text
                }
            }
        }
//        MouseArea {
//            z: -1
//            anchors.fill: parent
//            propagateComposedEvents: true
//            enabled: false
//            hoverEnabled: true
//            onPressed: {
//                mouse.accepted = false
//                DesktopOrganizationManager.cancelEdit()
//                nameField.focus = false
//            }
//            onDoubleClicked: {
//                nameField.focus = true
//                textFieldvisiable = true
//            }
//        }
    }
    SelectionModel {
        id: selectionModel
        targetView: gridView.visible ? gridView:tableView
    }
    Connections {
        target: desktopItemProxyModel
        function onModelReset() {
            selectionModel.pruneSelection()
        }
        function onRowsRemoved(parent, first, last) {
            selectionModel.pruneSelection()
        }
    }

    Item {
        id: orgShortcuts

        function shortcutsActive() {
            return root.shortcutsEnabled()
        }

        Shortcut {
            sequences: [StandardKey.SelectAll]
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutSelectAll()
        }
        Shortcut {
            sequences: [StandardKey.Copy]
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutCopy()
        }
        Shortcut {
            sequence: "Ctrl+X"
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutCut()
        }
        Shortcut {
            sequences: [StandardKey.Paste]
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutPaste()
        }
        Shortcut {
            sequences: ["Ctrl+D", "Delete"]
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutDelete(null)
        }
        Shortcut {
            sequence: "Shift+Delete"
            enabled: orgShortcuts.shortcutsActive()
            onActivated: root.shortcutRemovePermanent()
        }
        Shortcut {
            sequence: "Escape"
            enabled: widgetFocusScope.activeFocus
            onActivated: root.shortcutEscape()
        }
        Shortcut {
            sequences: ["Alt+E", "F2"]
            enabled: widgetFocusScope.activeFocus
            onActivated: {
                Controller.editUris(selectionModel.getSelections());
            }
        }
        Shortcut {
            sequences: ["Alt+Enter", "Alt+Return"]
            enabled: widgetFocusScope.activeFocus
            onActivated: Controller.showProperties(selectionModel.getSelections())
        }
        Shortcut {
            sequences: ["Enter", "Return"]
            enabled: widgetFocusScope.activeFocus
            onActivated: Controller.openFileByUri(selectionModel.getSelections())
        }
    }

    ListModel {
        id: dragitemModel
    }

    Item {
        id: dragIconView
        //anchors.fill: parent
        visible: false
        width: bounds.width
        height: bounds.height
        clip: true // 启用裁剪
        z: 100
        // 动态计算边界
        property rect bounds: calculateContentBounds()
        property real offsetToBoundsTopLeftX: 0
        property real offsetToBoundsTopLeftY: 0
        layer.textureSize: Qt.size(width / Screen.devicePixelRatio,
                                   height / Screen.devicePixelRatio)
        Drag.supportedActions: Qt.MoveAction
        Drag.dragType: Drag.Automatic
        Drag.onDragStarted: {
            console.log("item drag start")
            isDraging = true
        }
        Drag.onDragFinished: {
            console.log("item drag finish======")
            isDraging = false
        }

        function calculateContentBounds() {
            var minX = 0, minY = 0
            var maxX = 0, maxY = 0
            if (dragitemModel.count > 0) {
                minX = dragitemModel.get(0).x
                minY = dragitemModel.get(0).y
            }
            for (var i = 0; i < dragitemModel.count; i++) {
                var item = dragitemModel.get(i)
                minX = Math.min(minX, item.x)
                minY = Math.min(minY, item.y)
                maxX = Math.max(maxX, item.x)
                maxY = Math.max(maxY, item.y)
                console.log("minX, minY, maxX - minX, maxY - minY", minX, minY,
                            maxX - minX, maxY - minY, item.x, item.y,
                            dragitemModel.count)
            }
            return Qt.rect(minX, minY, maxX - minX + gridView.cellWidth,
                           maxY - minY + gridView.cellHeight)
        }

        Repeater {
            model: dragitemModel
            delegate: Rectangle {
                id: deleitem
                x: model.x - parent.bounds.x //+ gridView.contentX
                y: model.y - parent.bounds.y //- gridView.contentY
                width: gridView.cellWidth
                height: gridView.cellHeight
                color: "transparent"
                Rectangle {
                    id: gridViewIconRectdrag
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 32
                    height: 32
                    color: "transparent"
                    //color: selected ? "#E6E6E6" : "transparent"
                    radius: 6
                    objectName: "Ukui-idm_IconViewItem_icon" + model.displayname
                    Accessible.name: objectName
                    Icon {
                        id: itemIcondrag
                        anchors.fill: parent
                        source: Controller.icon(model.uri)
                        smooth: true
                    }
                }

                // Text {
                //     id: draggridviewitemtxt
                //     anchors.topMargin: 8
                //     anchors.bottomMargin: 4
                //     anchors.top: parent.top
                //     width: parent.width
                //     text: model.displayname
                //     wrapMode: Text.WrapAnywhere
                //     elide: Text.ElideRight
                //     horizontalAlignment: Text.AlignHCenter
                //     verticalAlignment: Text.AlignTop
                //     maximumLineCount: 2
                //     color: selected?"white":"black"
                //     Component.onCompleted: {
                //         var sssddddd = model.displayname
                //         var ssssss = 11
                //     }
                // }
                Rectangle {
                    anchors.top: gridViewIconRectdrag.bottom
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    clip: true
                    color: "transparent"
                    DelegateText {
                        id: gridviewitemtxtdrag
                        anchors.fill: parent
                        display: model.displayname
                        // selected: model.selected
                        isdrag: true
                        objectName: "Ukui-idm_IconViewItem_text" + model.displayname
                    }
                }
            }
        }
    }

    Item {
        id: draglistView
        //anchors.fill: parent
        visible: false
        width: root.width
        height: bounds.height
        clip: true // 启用裁剪
        z: 100
        // 动态计算边界
        layer.textureSize: Qt.size(width * Screen.devicePixelRatio,
                                       height * Screen.devicePixelRatio)
        property rect bounds: calculateContentBounds()
        property real offsetToBoundsTopLeftX: 0
        property real offsetToBoundsTopLeftY: 0
        Drag.supportedActions: Qt.MoveAction
        Drag.dragType: Drag.Automatic
        Drag.onDragStarted: {
            console.log("item drag start")
            isDraging = true
        }
        Drag.onDragFinished: {
            console.log("item drag finish======")
            isDraging = false
        }

        function calculateContentBounds() {
            var minX = 0, minY = 0
            var maxX = 0, maxY = 0
            if (dragitemModel.count > 0) {
                minX = dragitemModel.get(0).x
                minY = dragitemModel.get(0).y
            }
            for (var i = 0; i < dragitemModel.count; i++) {
                var item = dragitemModel.get(i)
                minX = Math.min(minX, item.x)
                minY = Math.min(minY, item.y)
                maxX = Math.max(maxX, item.x)
                maxY = Math.max(maxY, item.y)
                console.log("minX, minY, maxX - minX, maxY - minY", minX, minY,
                            maxX - minX, maxY - minY, item.x, item.y,
                            dragitemModel.count)
            }
            return Qt.rect(minX, minY, maxX - minX + draglistView.width,
                           maxY - minY + 32)
        }

        Repeater {
            model: dragitemModel
            delegate: Rectangle {
                id: delelistitem
                x: model.x - parent.bounds.x //+ gridView.contentX
                y: model.y - parent.bounds.y //- gridView.contentY
                width: root.width - tableView.leftMargin - tableView.rightMargin
                height: 32
                color: ThemeStyle.highlight()
                radius: 6
                Rectangle {
                    id: tableViewIconRectdrag
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin:4
                    anchors.left: parent.left
                    width: 18
                    height: 18
                    color: "transparent"
                    visible: true
                    //color: selected ? "#E6E6E6" : "transparent"
                    radius: 6
                    objectName: "Ukui-idm_IconViewItem_icon" + model.displayname
                    Accessible.name: objectName
                    Icon {
                        id: itemlistIcondrag
                        anchors.fill: parent
                        source: Controller.icon(model.uri)
                        smooth: true
                        //verticalAlignment: Text.AlignVCenter
                    }
                }
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: tableViewIconRectdrag.right
                    // anchors.topMargin: 5
                    width: 1/3*parent.width
                    clip: true
                    color: "transparent"
                    anchors.leftMargin: 2
                    height: displaydragtxt.height
                    DelegateText {
                        id: displaydragtxt
                        anchors.fill: parent
                        display: model.displayname
                        selected: model.selected
                        isdrag: true
                        islistview: true
                    }
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right:typetext.left
                    // anchors.topMargin: 5
                    width:  1/6 * root.width
                    clip: true
                    height: displaydragtxt.height
                    color: "transparent"
                    anchors.leftMargin: 2
                    visible: model.isfile
                    DelegateText {
                        id: sizedragtxt
                        anchors.fill: parent
                        display: model.size
                        selected: model.selected
                        isdrag: true
                        islistview: true
                    }
                }

                Rectangle {
                    id:typetext
                    //anchors.top: parent.top
                     anchors.verticalCenter: parent.verticalCenter
                    anchors.right: delelistitem.right
                    anchors.rightMargin: 6
                    width: 1/4 * root.width
                    clip: true
                    height: typedragtxt.height
                    color: "transparent"
                    DelegateText {
                        id: typedragtxt
                        anchors.fill: parent
                        display: model.type
                        selected: model.selected
                        isdrag: true
                        islistview: true
                    }
                }
            }
        }
    }
    ToolTip {
        id: delegateToolTip
        // 此处绑定方法会导致ukui主题tooltip概率性偏移，需要用其它方式实现
        //text: visible? model.tooltip: ""
        visible:  tooltipsText !== null && tooltipsText !== ""
        delay: 200
        closePolicy: Popup.CloseOnPressOutside
//                property int textFormat: Qt.RichText
        property int maxWidth: 400
        property var wrapMode: Text.WrapAnywhere
        // 动态更新tooltip内容
        onAboutToShow: {
            text = tooltipsText
        }
    }

    Item {
        id: viewsContainer
        anchors.top: title.bottom
        width: parent.width
        height: parent.height - title.height

        GridView {
        z: 5
        id: gridView
        anchors.fill: parent
        topMargin: 2
        leftMargin: 18
        contentWidth: parent.contentWidth
        visible: expand && !islistModel
        focus: visible
        clip: true
        cellWidth: 66
        cellHeight: 64
        objectName: "Ukui-idm_GridView_gridView"
        Accessible.name: objectName
        interactive: verticalScrollBar.visible
        model: DesktopItemProxyModel {
            id: desktopItemProxyModel
            screenId: root.screenId
            instanceId: root.instanceId
            sourceModel: DesktopItemModelInstance
            origanizationType:root.origanizationType
        }

        property bool modelResetTrigger: false
        property int currentEditIndex: -1
        property alias isMultifileMode: root.ismultifilemode
        property bool menuVisible: false
        readonly property int columnCount: Math.max(1, Math.floor(width / cellWidth))

        function positionForIndex(itemIndex) {
            return Qt.point(Math.floor(itemIndex / columnCount), itemIndex % columnCount)
        }

        function indexForPosition(row, column) {
            if (row < 0 || column < 0 || column >= columnCount) {
                return -1
            }
            var itemIndex = row * columnCount + column
            return itemIndex < count ? itemIndex : -1
        }

        function dragPositionForIndex(itemIndex) {
            var itemPos = positionForIndex(itemIndex)
            return Qt.point(itemPos.y * cellWidth, itemPos.x * cellHeight)
        }

        //cacheBuffer: 60000
        anchors.leftMargin: 2
        delegate: ItemDelegate {
            id: dele
            z: selected ? parent.z + 1 : parent.z
            property int itemIndex: index
            highlighted: GridView.isCurrentItem
            width: 60
            height: 58
            visible: true
            objectName: "Ukui-idm_IconViewItem_" + displayname
            anchors.topMargin: 2
            Accessible.name: objectName
            //property bool hovered: gridViewDelegateMouseArea.containsMouse && gridViewDelegateMouseArea.reallyContainsMouse
            property string uri: model.uri
            property bool selected: selectionModel.isSelected(uri)
            property point delegateGlobalPos
            property alias pressPos1: gridViewDelegateMouseArea.pressPos1
            property var displayname: model.display
            property var icondecoration: model ? model.decoration : ""
            property alias delegateText: delegateText
            property int xpos: x
            clip: false
            background: Rectangle {
                id: backgroundrec
                objectName: "Ukui-idm_IconViewItem_" + dele.displayname
                Accessible.name: objectName
                radius: 6 // 设置圆角半径为6
                color: selected
                       && !delegateText.editing ? ThemeStyle.highlight(
                                                      ) : hovered
                                                  && !delegateText.editing ? ThemeStyle.midlight(
                                                                                 ) : "transparent"
            }
            Keys.onShortcutOverride: event.accepted = false

            Rectangle {
                id: gridViewIconRect
                anchors.top: parent.top
                anchors.topMargin: 6
                anchors.horizontalCenter: parent.horizontalCenter
                width: 32
                height: 32
                color: "transparent"
                //color: selected ? "#E6E6E6" : "transparent"
                radius: 6
                objectName: "Ukui-idm_IconViewItem_icon" + model.display
                Accessible.name: objectName
                DelegateIcon {
                    anchors.top: gridViewIconRect.top
                    id: dragviewIconRect
                    anchors.fill: parent
                    icon : Controller.icon(model.uri)
                    emblems: model.emblems
                    uri: model.uri
                }
                // Icon {
                //     id: itemIcon
                //     anchors.fill: parent
                //     source: model.decoration
                // }
            }
            Rectangle {
                anchors.top: gridViewIconRect.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width + 8
                radius: 6
                clip: false
                color: "transparent"
                border.width: 1
                DelegateText {
                    id: delegateText
                    anchors.top: parent.top
                    display: model.display // 显示的文本内容
                    editing: gridView.currentEditIndex === index
                             && gridView.currentEditIndex !== -1 // 控制是否进入编辑状态
                    showFullText: !ismultifilemode /*&& !isBoxSelection && !(desktopIconView.menuVisible && desktopIconView.hasMultipleFilesInMenu)*/
                    selected: dele.selected
                    isDir: model.isDir
                    // isSoftLink: model.softLink
                    maxVisibleHeight: {
                        return 100
                    }
                }
                Connections {
                    target: delegateText
                    function onEditFinish(selections, newText) {
                        selectionModel.clearSelection()
                        DesktopOrganizationManager.renameByorgTool()
                        Controller.rename(selections, newText)
                        gridView.currentEditIndex = -1
                    }
                }
            }
            // Text {
            //     id: gridviewitemtxt
            //     anchors.topMargin: 8
            //     anchors.bottomMargin: 4
            //     anchors.top: parent.top
            //     width: parent.width
            //     text: model.display
            //     wrapMode: Text.WrapAnywhere
            //     elide: Text.ElideRight
            //     horizontalAlignment: Text.AlignHCenter
            //     verticalAlignment: Text.AlignTop
            //     maximumLineCount: 2
            //     color: selected?"white":"black"
            //     objectName: "Ukui-idm_IconViewItem_text" + gridviewitemtxt.text
            //     Accessible.name: objectName
            // }
            // MouseArea {
            //     anchors.fill: parent
            //     hoverEnabled: true
            // }
            //}
            MouseArea {
                id: gridViewDelegateMouseArea
                z: 8
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                anchors.fill: parent
                propagateComposedEvents: true
                property point pressPos: Qt.point(0, 0)
                drag.target: dragIconView
                property bool allowDrag: true
                property point pressPos1: Qt.point(0, 0)
                property bool reallyContainsMouse: false
                property point dragstartpos: Qt.point(0, 0)
                hoverEnabled: true
                drag.onActiveChanged: {
                    if (drag.active && allowDrag) {
                        // editTriggerTimer.stop()
                        // desktopIconView.closeEdit()
                        // hotspot的计算要在bounds更新之后
                        dragIconView.Drag.source = dele
                        dele.delegateGlobalPos = mapToItem(gridView, 0, 0)
                        var selections = selectionModel.getSelections()
                        var selectionpos = selectionModel.getSelectionsPositions()
                        dragIconView.Drag.mimeData = {
                            "text/uri-list": selectionModel.getSelections().map(
                                                 uri => uri.trim(
                                                     )) // 去除每个URI两端的空格
                            .filter(uri => uri !== "") // 过滤空URI
                            .join("\r\n"),
                            "peony-qt/encoded-uris": selectionModel.getSelections(
                                                         ).filter(
                                                         uri => uri.trim(
                                                             ) !== "").join(
                                                         " "),
                            "peony-qt/pos-list": selectionModel.getSelectionsPositions(
                                                     ).map(
                                                     point => `${point.x},${point.y}`) // 将QPoint转为"x,y"格式
                            .join(" "),
                            "text/ukui_desktop"// 用空格分隔多个坐标
                            : true,
                            "text/plain": selectionModel.getSelections().map(
                                              uri => uri.trim()) // 去除每个URI两端的空格
                            .filter(uri => uri !== "").join(" "),
                            "application/x-qabstractitemmodeldatalist": selectionModel.getSelections().map(uri => uri.trim()).filter(
                                                                            uri => uri !== "").map(
                                                                            uri => ({
                                                                                        "0": {
                                                                                            "text/uri-list": uri
                                                                                        }
                                                                                    })).join(","),
                            "text/oganization_tool": "oganization_tool",
                            "text/org_instanceid":instanceId,
                            "text/drag-global-start-pos": `${dragstartpos.x},${dragstartpos.y}`
                        }

                        // grabToImage不会立即返回所以在回调中处理
                        var items = selectionModel.getSelectedItems()
                        dragitemModel.clear()
                        for (var i = 0; i < items.length; i++) {
                            dragitemModel.append(items[i])
                        }
                        // 设置hotSpot为鼠标在按下相对于选中（bounds）左上角区域的位置
                        // dragIconView.Drag.hotSpot.x = gridViewDelegateMouseArea.pressPos1.x
                        //         + dragIconView.offsetToBoundsTopLeftX //- dragIconView.bounds.x
                        // dragIconView.Drag.hotSpot.y = gridViewDelegateMouseArea.pressPos1.y
                        //         + dragIconView.offsetToBoundsTopLeftY //- dragIconView.bounds.y
                        dragIconView.Drag.hotSpot.x = gridView.cellWidth/2 + gridView.leftMargin/2

                        dragIconView.Drag.hotSpot.y = gridView.cellHeight/2

                        dragIconView.grabToImage(function (result) {
                            dragIconView.Drag.imageSource = result.url
                            dragIconView.Drag.active = true
                            // var s    = Screen.devicePixelRatio
                            // var ssss    =Screen.devicePixelRatio
                        } /*,Qt.size(dragIconView.width * Screen.devicePixelRatio, dragIconView.height * Screen.devicePixelRatio)*/
                        )
                    } else if (drag.active && !allowDrag) {
                        // 如果不允许拖拽但拖拽被激活，则立即停止
                        drag.active = false
                        dragIconView.Drag.active = false
                    } else {
                        dragIconView.Drag.active = false
                        dragIconView.x = 0
                        dragIconView.y = 0
                        pressPos1 = Qt.point(0, 0)
                        dragIconView.offsetToBoundsTopLeftX = 0
                        dragIconView.offsetToBoundsTopLeftY = 0
                        console.log("更新当前视图")
                        gridView.model.invalidateModel()
                    }
                }
                onPressed: {
                    widgetFocusScope.forceActiveFocus()
                    var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier
                    var controlKeyPressed = mouse.modifiers & Qt.ControlModifier

                    allowDrag = mouse.button !== Qt.RightButton
                    var clickedOnIcon = false
                    var item = gridView.itemAtIndex(index)
                    dragstartpos = mapToGlobal(Qt.point(mouse.x, mouse.y))
                    if (item) {
                        var iconLeft = backgroundrec.x
                        var iconRight = iconLeft + backgroundrec.width
                        var iconTop = gridViewIconRect.y
                        var iconBottom = iconTop + gridViewIconRect.height

                        // 手动检查点是否在矩形内
                        if (mouseX >= iconLeft && mouseX <= iconRight
                                && mouseY >= iconTop && mouseY <= iconBottom) {
                            clickedOnIcon = true
                            console.log("点击在Icon上:", index, model.display)
                        }
                    }
                    if (clickedOnIcon && item.delegateText
                            && item.delegateText.editing) {
                        // 点击在正在编辑的图标上，应该触发拖拽而不是框选
                        pressPos = Qt.point(-1, -1) // 设置特殊值表示不进行框选
                        // selectionRect.visible = false
                        // selectionRect.width = 0
                        // selectionRect.height = 0
                        // // 不清除选择，保持当前选择状态
                        closeEdit() // 关闭编辑状态
                        // controller.closeMenu()
                        // mouse.accepted = false // 允许事件传递到图标组件
                        return
                    }
                    if (shiftKeyPressed
                            || controlKeyPressed /*|| (isBoxSelection && dele.selected)*/
                            ) {
                        console.log("Ctrl or shift key is pressed!")
                        ismultifilemode = true
                        if (shiftKeyPressed) {
                            var startIndex = selectionModel.firstIndex()
                            selectionModel.selectBetween(startIndex, index)
                        }
                    } /*else {
                                            ismultifilemode = false
                                                                }*/
                    if (mouse.button == Qt.RightButton) {
                        if(delegateText.editing){
                            mouse.accepted = false
                            return
                        }
                        if (!ismultifilemode) {
                            selectionModel.clearSelection()
                        }
                        var globalPos = mapToGlobal(mouse.x, mouse.y)
                        selectionModel.updateSelection(index, true)
                        var selections = selectionModel.getSelections()
                        gridView.menuVisible = true
                        // DesktopOrganizationManager.setOperationInstanceId(
                        //             instanceId)
                        console.log("open menu", globalPos, selections,
                                    mouse.x, mouse.y)
                        Controller.setIsOperationByPartition(true)
                        Controller.showMenu(selections, globalPos)
                        return
                    }
                    if (!ismultifilemode) {

                        if (delegateText.editing) {
                            console.log("editing........")
                            mouse.accepted = false
                            return
                        }
                        closeEdit()
                        if (dele.selected) {

                            //editTriggerTimer.start()
                        } else {
                            selectionModel.clearSelection()
                        }
                    }
                    if (controlKeyPressed && dele.selected) {
                        selectionModel.updateSelection(index, false)
                    } else {
                        selectionModel.updateSelection(index, true)
                    }
                    gridView.menuVisible = false
                }
                onReleased: {
                    var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier
                    var controlKeyPressed = mouse.modifiers & Qt.ControlModifier
                    if (!(shiftKeyPressed || controlKeyPressed)
                            && gridView.currentEditIndex === -1) {
                        ismultifilemode = false
                        // if (!desktopIconView.menuVisible) {
                        if (!gridView.menuVisible) {
                            selectionModel.selectCurrent(index)
                        }
                    }
                }
                onClicked: {
                    if(mouse.button == Qt.RightButton){
                        if(gridView.currentEditIndex == index){
                            return
                        }
                    }
                    DesktopOrganizationManager.cancelEdit()
                    // 两次间隔点击进入重命名（与桌面保持一致）
                    if (!m_renameTimer.running || index !== m_lastIndex) {
                        // 点击不同图标，重置定时器
                        m_renameStartTime = Date.now()
                        m_renameTimer.restart()
                        m_lastIndex = index
                    } else {
                        // 检查是否满足重命名条件
                        if (mouse.button === Qt.LeftButton) {
                            var elapsed = Date.now() - m_renameStartTime
                            var remaining = 3000 - elapsed
                            var renameWindow = 3000 - 400 // 3000ms超时，400ms双击间隔
                            if (remaining > 0 && remaining < renameWindow) {
                                // 触发重命名
                                m_renameTimer.stop()
                                gridView.currentEditIndex = index
                                dele.delegateText.selectedUris = [model.uri]
                                dele.delegateText.forceActiveFocus()
                            }
                        }
                    }
                }
                onDoubleClicked: {
                    if (mouse.button == Qt.LeftButton) {
                        var item = gridView.itemAtIndex(index)
                        Controller.openFileByUri(item.uri)
                        console.log("on double clicked", item.uri)
                        mouse.accepted = true
                        selectionModel.clearSelection()
                    }
                }
                onEntered: {
                    tooltipsText = model.tooltip
                    reallyContainsMouse = false
                }
                onExited: {
                    tooltipsText = ""
                    reallyContainsMouse = false
                }
                onPositionChanged: {
                    reallyContainsMouse = true
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: verticalScrollBar
            minimumSize: 0.1
            visible: Math.ceil((gridView.count / Math.floor(
                                    (gridView.width / gridView.cellWidth))))
                     * gridView.cellHeight > gridView.height ? true : false
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            propagateComposedEvents: false
            id: gridViewMouseArea
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            // onPressed: {
            //     if(mouse.button == Qt.RightButton){
            //         var pos = gridViewMouseArea.mapToGlobal(mouse.x, mouse.y)
            //         DesktopOrganizationManager.showMenuRequest(pos,instanceId)
            //     }
            // }
            onReleased: {
                gridView.menuVisible = false
            }
            onClicked: {
                widgetFocusScope.forceActiveFocus()
                closeEdit()
                DesktopOrganizationManager.cancelEdit()
                selectionModel.clearSelection()
                if (mouse.button == Qt.RightButton) {
                    gridView.menuVisible = true
                    var pos = gridViewMouseArea.mapToGlobal(mouse.x, mouse.y)
                    operationBymenu =  true
                    DesktopOrganizationManager.showMenuRequest(pos, instanceId)
                    return
                }
                gridView.menuVisible = false
            }
        }
    }

    ListView {
        z: 5
        id: tableView
        anchors.fill: parent
        leftMargin: 12
        rightMargin: 13
        focus: visible
        visible: expand && islistModel
        // 不允许横向滚动
        //contentWidth: parent.width
        //anchors.fill: parent
        //model: sqlitemodel
        clip: true
        interactive: listviewVerticalScrollBar.visible
        model: desktopItemProxyModel
        Component.onCompleted: {
            //disorselectedAll(false)
        }
        objectName: "Ukui-idm_ListView_tableView"
        Accessible.name: objectName
        property int currentEditIndex: -1
        property bool menuVisible:false
        property alias isMultifileMode: root.ismultifilemode

        function positionForIndex(itemIndex) {
            return Qt.point(itemIndex, 0)
        }

        function indexForPosition(row, column) {
            return row >= 0 && column === 0 && row < count ? row : -1
        }

        function dragPositionForIndex(itemIndex) {
            return Qt.point(0, itemIndex * 32)
        }

        //cacheBuffer: 60000
        delegate: ItemDelegate {
            id:listdele
            z: selected ? parent.z + 1 : parent.z
            width: root.width - tableView.leftMargin - tableView.rightMargin
            height: 32
            //highlighted: ListView.isCurrentItem
            //focus: visible
            property string path: model.path
            property var decoration: model.decoration
            //property string parentPath: model.parentPath
            property string uri: model.uri
            property bool selected: selectionModel.isSelected(uri)
            property alias delegateText: delegateTextlist
            property int  itemIndex:index
            property point delegateGlobalPos
            property alias pressPos1: tableViewDelegateMouseArea.pressPos1
            property var displayname: model.display
            property var size: model.fileSize
            property var type: model.fileType
            property bool isfile: !model.isDesktop && !model.isDir

            objectName: "Ukui-idm_listViewItem_" + displaytxt.text
            Accessible.name: objectName
            // background: Rectangle{
            //     anchors.fill:parent
            //     radius: 6
            //     color: "red"
            // }
            Rectangle {
                height:listdele.height
                width: root.width
                anchors.fill:parent
                color: selected /*&& !delegateTextlist.editing*/ ? ThemeStyle.highlight() : hovered
                                                  && !delegateTextlist.editing ? ThemeStyle.midlight(
                                                                                 ) : "transparent"
                radius: 6
            }
            Item {
                id: listViewIconRect
                anchors.left: listdele.left
                anchors.verticalCenter: parent.verticalCenter
                width: 18
                height: 18
                // objectName: "Ukui-idm_listViewItem_icon" + displaytxt.text
                Accessible.name: objectName
                anchors.leftMargin: 8
                // Icon {
                //     id: itemIconlist
                //     anchors.fill: parent
                //     source: model.decoration
                // }
                DelegateIcon {
                    id: itemIconlist
                    // width:  listViewIconRect.width
                    // height:  listViewIconRect.height
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.fill: parent
                    icon : Controller.icon(model.uri)
                    emblems: model.emblems
                    emblemSize: 8
                    uri: model.uri
                }
            }

            // Text {
            //     id: displaytxt
            //     anchors.left: listViewIconRect.right
            //     anchors.leftMargin: 4
            //     anchors.rightMargin: 4
            //     height: parent.height
            //     text: model.display
            //     elide: Text.ElideRight
            //     verticalAlignment: Qt.AlignVCenter
            //     objectName: "Ukui-idm_listViewItem_icon" + displaytxt.text
            //     Accessible.name: objectName
            //     color:"white"
            //     maximumLineCount: 1
            //     font.pixelSize: 11
            //     visible: false
            //     width: displaytxt.implicitWidth > 1/3 * root.width ?  1/3 * root.width : displaytxt.implicitWidth
            //     MouseArea {
            //         //anchors.fill: parent
            //         hoverEnabled: true
            //         width: displaytxt.implicitWidth
            //         height: displaytxt.implicitHeight
            //         onEntered: {

            //         }
            //         onExited: {

            //         }
            //     }
            // }

            DelegateText {
                id: delegateTextlist
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 4
                anchors.left: listViewIconRect.right
                display: model.display // 显示的文本内容
                editing: tableView.currentEditIndex === index
                         && tableView.currentEditIndex !== -1 // 控制是否进入编辑状态
                showFullText: !ismultifilemode /*&& !isBoxSelection && !(desktopIconView.menuVisible && desktopIconView.hasMultipleFilesInMenu)*/
                selected: listdele.selected
                isDir: model.isDir
                islistview: true
                // isSoftLink: model.softLink
                maxwidth: 1/3*root.width
                maxVisibleHeight: {
                    return 32
                }
                // MouseArea{
                //     anchors.fill:parent
                //     onEntered: {

                //     }
                // }
            }
            Connections {
                target: delegateTextlist
                function onEditFinish(selections, newText) {
                    selectionModel.clearSelection()
                    DesktopOrganizationManager.renameByorgTool()
                    Controller.rename(selections, newText)
                    tableView.currentEditIndex = -1
                }
            }

            Text {
                id: tableViewSizeText
                objectName: "Ukui-idm_listViewItem_text_size" + text
                Accessible.name: objectName
                anchors.left: delegateTextlist.right
                //anchors.right: tableViewTypeText.left
                width: 1/6 * root.width
                height: parent.height
                text: model.fileSize
                color: "white"
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignLeft
                font.pixelSize: 11
                visible:!model.isDesktop && !model.isDir && root.width >120
                elide: Text.ElideRight
                //     anchors.fill: parent
                //     hoverEnabled: true
                //     onEntered: {

                //     }
                //     onExited: {
                //     }
                // }
            }
            DropShadow {
                visible: tableViewSizeText.visible
                anchors.fill: tableViewSizeText
                source: tableViewSizeText
                horizontalOffset: 1
                verticalOffset: 2
                radius: 3
                samples: radius * 2
                color: "#ff000000"
                z: -1
                layer.enabled: true
            }
            Text {
                id: tableViewTypeText
                visible: !collapse
                objectName: "Ukui-idm_listViewItem_text_size" + text
                Accessible.name: objectName
                anchors.left: tableViewSizeText.right
                anchors.right: parent.right
                // anchors.rightMargin: 12
                width: 1/4 * root.width
                height: parent.height
                text: model.fileType
                color: "white"
                elide: Text.ElideRight
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignRight
                font.pixelSize: 11
                // MouseArea {
                //     anchors.fill: parent
                //     hoverEnabled: true
                //     onEntered: {

                //     }
                //     onExited: {
                //     }
                // }
            }
            DropShadow {
                visible: tableViewTypeText.visible
                anchors.fill: tableViewTypeText
                source: tableViewTypeText
                horizontalOffset: 1
                verticalOffset: 2
                radius: 3
                samples: radius * 2
                color: "#ff000000"
                z: -1
                layer.enabled: true
            }
            MouseArea {
                id: tableViewDelegateMouseArea
                z: 8
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                propagateComposedEvents: true
                property point pressPos: Qt.point(0, 0)
                property bool allowDrag: true
               // onPressed: pressPos = Qt.point(mouseX, mouseY)
                //onPositionChanged: console.log(mouseX)
                drag.target: draglistView
                property point pressPos1: Qt.point(0, 0)
                property point dragstartpos: Qt.point(0, 0)
                hoverEnabled: true
                drag.onActiveChanged: {
                    console.log("mouse area drag active", drag.active,
                                allowDrag)
                    if (drag.active && allowDrag) {
                        // editTriggerTimer.stop()
                        // desktopIconView.closeEdit()
                        // hotspot的计算要在bounds更新之后
                        draglistView.Drag.source = listdele
                        listdele.delegateGlobalPos = mapToItem(gridView, 0, 0)
                        var selections = selectionModel.getSelections()
                        var selectionpos = selectionModel.getSelectionsPositions()
                        draglistView.Drag.mimeData = {
                            "text/uri-list": selectionModel.getSelections().map(
                                                 uri => uri.trim(
                                                     )) // 去除每个URI两端的空格
                            .filter(uri => uri !== "") // 过滤空URI
                            .join("\r\n"),
                            "peony-qt/encoded-uris": selectionModel.getSelections(
                                                         ).filter(
                                                         uri => uri.trim(
                                                             ) !== "").join(
                                                         " "),
                            "peony-qt/pos-list": selectionModel.getSelectionsPositions(
                                                     ).map(
                                                     point => `${point.x},${point.y}`) // 将QPoint转为"x,y"格式
                            .join(" "),
                            "text/ukui_desktop"// 用空格分隔多个坐标
                            : true,
                            "text/plain": selectionModel.getSelections().map(
                                              uri => uri.trim()) // 去除每个URI两端的空格
                            .filter(uri => uri !== "").join(" "),
                            "application/x-qabstractitemmodeldatalist": selectionModel.getSelections().map(uri => uri.trim()).filter(
                                                                            uri => uri !== "").map(
                                                                            uri => ({
                                                                                        "0": {
                                                                                            "text/uri-list": uri
                                                                                        }
                                                                                    })).join(","),
                            "text/oganization_tool": "oganization_tool",
                            "text/org_instanceid":instanceId,
			   "text/drag-global-start-pos": `${dragstartpos.x},${dragstartpos.y}`
                        }

                        // grabToImage不会立即返回所以在回调中处理
                        var items = selectionModel.getSelectedItems()
                        dragitemModel.clear()
                        for (var i = 0; i < items.length; i++) {
                            dragitemModel.append(items[i])
                        }
                        // 设置hotSpot为鼠标在按下相对于选中（bounds）左上角区域的位置
                        var ss = mouseX
                        draglistView.Drag.hotSpot.x = mouseX /*tableViewDelegateMouseArea.pressPos1.x
                                + draglistView.offsetToBoundsTopLeftX*/ /*- listdele.width*/
                        draglistView.Drag.hotSpot.y =  mouseY
                        draglistView.grabToImage(function (result) {
                            draglistView.Drag.imageSource = result.url
                            draglistView.Drag.active = true
                            // var s    = Screen.devicePixelRatio
                            // var ssss    =Screen.devicePixelRatio
                        } /*,Qt.size(dragIconView.width * Screen.devicePixelRatio, dragIconView.height * Screen.devicePixelRatio)*/
                        )
                    } else if (drag.active && !allowDrag) {
                        // 如果不允许拖拽但拖拽被激活，则立即停止
                        drag.active = false
                        draglistView.Drag.active = false
                    } else {
                        draglistView.Drag.active = false
                        draglistView.x = 0
                        draglistView.y = 0
                        pressPos1 = Qt.point(0, 0)
                        draglistView.offsetToBoundsTopLeftX = 0
                        draglistView.offsetToBoundsTopLeftY = 0
                        console.log("更新当前视图")
                        tableView.model.invalidateModel()
                    }
                }
                onPressed: {
                    widgetFocusScope.forceActiveFocus()
                    var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier
                    var controlKeyPressed = mouse.modifiers & Qt.ControlModifier
                    allowDrag = mouse.button !== Qt.RightButton
                    var clickedOntext = false
                    var item = tableView.itemAtIndex(index)
                    dragstartpos = mapToGlobal(Qt.point(mouse.x, mouse.y))
                    if (item) {
                        var iconLeft = delegateTextlist.x
                        var iconRight = iconLeft + delegateTextlist.width
                        var iconTop = delegateTextlist.y
                        var iconBottom = iconTop + delegateTextlist.height

                        // 手动检查点是否在文本内
                        if (mouseX >= iconLeft && mouseX <= iconRight
                                && mouseY >= iconTop && mouseY <= iconBottom) {
                            clickedOntext = true
                            console.log("点击在Icon上:", index, model.display)
                        }
                    }
                    if (clickedOntext && item.delegateText
                            && item.delegateText.editing) {
                        // 点击在正在编辑的图标上，应该触发拖拽而不是框选
                        // pressPos = Qt.point(-1, -1) // 设置特殊值表示不进行框选
                        // // selectionRect.visible = false
                        // // selectionRect.width = 0
                        // // selectionRect.height = 0
                        // // // 不清除选择，保持当前选择状态
                        // closeEdit() // 关闭编辑状态
                        // // controller.closeMenu()
                        mouse.accepted = false // 允许事件传递到图标组件
                        return
                    }
                    if (shiftKeyPressed
                            || controlKeyPressed /*|| (isBoxSelection && dele.selected)*/
                            ) {
                        console.log("Ctrl or shift key is pressed!")
                        ismultifilemode = true
                        if (shiftKeyPressed) {
                            var startIndex = selectionModel.firstIndex()
                            selectionModel.selectBetween(startIndex, index)
                        }
                    } /*else {
                                            ismultifilemode = false
                                                                }*/
                    // pressPos = Qt.point(mouseX, mouseY)
                    if (mouse.button == Qt.RightButton) {
                        if (!ismultifilemode) {
                            selectionModel.clearSelection()
                        }
                        var globalPos = mapToGlobal(mouse.x, mouse.y)
                        selectionModel.updateSelection(index, true)
                        var selections = selectionModel.getSelections()
                        tableView.menuVisible = true
                        // DesktopOrganizationManager.setOperationInstanceId(
                        //             instanceId)
                        console.log("open menu", globalPos, selections,
                                    mouse.x, mouse.y)
                        Controller.setIsOperationByPartition(true)
                        Controller.showMenu(selections, globalPos)
                        return
                    }
                    if (!ismultifilemode) {

                        // if (delegateTextlist.editing) {
                        //     console.log("editing........")
                        //     mouse.accepted = false
                        //     return
                        // }
                        closeEdit()
                        if (listdele.selected) {

                            //editTriggerTimer.start()
                        } else {
                            selectionModel.clearSelection()
                        }
                    }
                    if (controlKeyPressed && listdele.selected) {
                        selectionModel.updateSelection(index, false)
                    } else {
                        selectionModel.updateSelection(index, true)
                    }
                    tableView.menuVisible = false
                }
                onClicked: {
                    if (mouse.button == Qt.RightButton
                            && tableView.currentEditIndex == index) {
                        return
                    }
                    DesktopOrganizationManager.cancelEdit()
                    // 两次间隔点击进入重命名（与桌面保持一致）
                    if (!m_renameTimer.running || index !== m_lastIndex) {
                        // 点击不同图标，重置定时器
                        m_renameStartTime = Date.now()
                        m_renameTimer.restart()
                        m_lastIndex = index
                    } else {
                        // 检查是否满足重命名条件
                        if (mouse.button === Qt.LeftButton) {
                            var elapsed = Date.now() - m_renameStartTime
                            var remaining = 3000 - elapsed
                            var renameWindow = 3000 - 400 // 3000ms超时，400ms双击间隔
                            if (remaining > 0 && remaining < renameWindow) {
                                // 触发重命名
                                m_renameTimer.stop()
                                tableView.currentEditIndex = index
                                listdele.delegateText.selectedUris = [model.uri]
                                listdele.delegateText.forceActiveFocus()
                            }
                        }
                    }
                }
                // onDoubleClicked: {
                //     // tableView.currentIndex = index
                //     // console.log("on double clicked", model.path)
                //     // if (ismultifilemode)
                //     //     return
                //     // openRequest(model.path)
                //     // mouse.accepted = true

                // }
                onDoubleClicked: {
                    if (mouse.button == Qt.LeftButton) {
                        var item = tableView.itemAtIndex(index)
                        Controller.openFileByUri(item.uri)
                        console.log("on double clicked", item.uri)
                        mouse.accepted = true
                        selectionModel.clearSelection()
                    }
                }
                onReleased: {
                    var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier
                    var controlKeyPressed = mouse.modifiers & Qt.ControlModifier
                    if (!(shiftKeyPressed || controlKeyPressed)
                            && tableView.currentEditIndex === -1) {
                        ismultifilemode = false
                        // if (!desktopIconView.menuVisible) {
                        if (!tableView.menuVisible) {
                            selectionModel.selectCurrent(index)
                        }
                    }
                }
                onEntered: {
                    tooltipsText = model.tooltip
                }
                onExited: {
                    tooltipsText = ""
                }
            }
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            propagateComposedEvents: false
            id:tableViewMouseArea
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                widgetFocusScope.forceActiveFocus()
                closeEdit()
                DesktopOrganizationManager.cancelEdit()
                selectionModel.clearSelection()
                if (mouse.button == Qt.RightButton) {
                    tableView.menuVisible = true
                    var pos = tableViewMouseArea.mapToGlobal(mouse.x, mouse.y)
                    operationBymenu =  true
                    DesktopOrganizationManager.showMenuRequest(pos, instanceId)
                    return
                }
                tableView.menuVisible = false
            }
        }
        ScrollBar.vertical: ScrollBar {
            objectName: "Ukui-idm_ScrollBar_Listview"
            Accessible.name: objectName
            id: listviewVerticalScrollBar
            minimumSize: 0.3
            visible: tableView.count * 36 > tableView.height ? true : false
        }
    }

    }

    DropArea {
        anchors.fill: parent
        visible: true
        onEntered: {
            var hasUriList = drag.formats.indexOf("text/uri-list") >= 0
            var hasUrl = drag.formats.indexOf("url") >= 0
        }
       onPositionChanged: {
           var orgstr = drag.getDataAsString("text/org_instanceid")
           var dragFrominstanceid = Number(orgstr)
           if(orgstr && dragFrominstanceid == instanceId){
               drag.accepted = false
               return
           }
       }
        onDropped: {
            var srcUris = []
            var peonyUriData = drop.getDataAsString("peony-qt/encoded-uris")
            if (peonyUriData && peonyUriData.length > 0) {
                var uris = peonyUriData.split(" ")
                for (var i = 0; i < uris.length; i++) {
                    if (!uris[i].startsWith("recent://")) {
                        srcUris.push(uris[i])
                    }
                }
            } else {
                var uriString = drop.getDataAsString("text/uri-list")
                var uris = uriString.split(/\r?\n/).map(
                            uri => uri.trim()).filter(uri => uri !== "")
                if (uris.length === 0) {
                    var appUrl = drop.getDataAsString("url")
                    if (appUrl && appUrl.length > 0) {
                        uris = [appUrl]
                    }
                }
            }
            DesktopOrganizationManager.addFileFromDesktopToOrgTool(uris, instanceId)
            var orgstr = drop.getDataAsString("text/org_instanceid")
            var dragFrominstanceid = Number(orgstr)
            if(orgstr && dragFrominstanceid != -1){
                DesktopOrganizationManager.updateOrganizationView(dragFrominstanceid)
            }
            firsturi = srcUris[0]
            delayTimer.start()
        }
    }
    Timer {
         id: delayTimer
         interval: 50
         onTriggered: {
            DesktopOrganizationManager.selectAndJumptoUriRequest(instanceId, firsturi)
         }
    }

    function shortcutsEnabled() {
        if (!expand || !Controller.enableShortcutKeys()) {
            return false
        }
        if (textFieldvisiable) {
            return false
        }
        var targetView = gridView.visible ? gridView : tableView
        if (targetView.currentEditIndex !== -1) {
            return false
        }
        return widgetFocusScope.activeFocus
    }

    function shortcutSelectAll() {
        if (!shortcutsEnabled()) {
            return
        }
        Controller.setIsOperationByPartition(true)
        selectionModel.selectAll()
    }

    function shortcutCopy() {
        if (!shortcutsEnabled()) {
            return
        }
        Controller.setIsOperationByPartition(true)
        Controller.fileCopy(selectionModel.getSelections())
    }

    function shortcutCut() {
        if (!shortcutsEnabled()) {
            return
        }
        Controller.setIsOperationByPartition(true)
        Controller.fileCut(selectionModel.getSelections())
    }

    function shortcutPaste() {
        if (!shortcutsEnabled()) {
            return
        }
        DesktopOrganizationManager.pasteFiles(instanceId)
    }

    function shortcutDelete(event) {
        if (!shortcutsEnabled()) {
            return
        }
        closeEdit()
        Controller.setIsOperationByPartition(true)
        var selections = selectionModel.getSelections()
        if (event && (event.modifiers & Qt.ShiftModifier) !== 0) {
            Controller.remove(selections)
        } else {
            Controller.deleteToTrash(selections)
        }
    }

    function shortcutRemovePermanent() {
        if (!shortcutsEnabled()) {
            return
        }
        closeEdit()
        Controller.setIsOperationByPartition(true)
        Controller.remove(selectionModel.getSelections())
    }

    function shortcutEscape() {
        if (textFieldvisiable) {
            DesktopOrganizationManager.cancelEdit()
            return
        }
        closeEdit()
    }

    function commitRename(name) {
        DesktopOrganizationManager.renameSpace(instanceId, name)
    }

    function closeEdit() {
        var targetview = gridView.visible ? gridView :tableView
        console.log("closeEdit", targetview.currentEditIndex)
        if (targetview.currentEditIndex !== -1) {
            var item = targetview.itemAtIndex(targetview.currentEditIndex)
            if (item && item.delegateText) {
                item.delegateText.updateFileName()
            }
            targetview.currentEditIndex = -1
        }
    }

    function scrollToIndex(index) {
         if (gridView.visible) {
             gridView.positionViewAtIndex(index, ListView.Center)
         } else if (tableView.visible) {
             tableView.positionViewAtIndex(index, ListView.Center)
         }
     }

    function runWithDelegate(itemIndex, callback, attempts) {
        var targetView = gridView.visible ? gridView : tableView
        var item = targetView.itemAtIndex(itemIndex)
        if (item) {
            callback(item)
            return
        }
        if (attempts <= 0) {
            return
        }
        Qt.callLater(function() {
            runWithDelegate(itemIndex, callback, attempts - 1)
        })
    }

    function startEditForUri(uri) {
        var itemIndex = desktopItemProxyModel.indexOfUri(uri)
        if (itemIndex < 0) {
            return
        }

        var targetView = gridView.visible ? gridView : tableView
        selectionModel.selectCurrent(itemIndex)
        targetView.currentEditIndex = itemIndex
        scrollToIndex(itemIndex)
        runWithDelegate(itemIndex, function(item) {
            if (!item.delegateText) {
                return
            }
            item.delegateText.selectedUris = selectionModel.getSelections()
            item.delegateText.forceActiveFocus()
        }, 5)
    }
    Connections {
        target: DesktopOrganizationManager
        function onViewModelChanged(instanceid,flag) {
            if(instanceId === instanceid){
                islistModel = flag
            }
        }
    }
    Connections {
        target: DesktopOrganizationManager
        function onUpdateInitHeight(id,height) {
            if(instanceId === id){
                initHeight = height
            }
        }
    }
    Connections {
        target: DesktopOrganizationManager
        function onSelectAndJumptoUriRequest(id,uri) {
            if(instanceId === id){
                var itemIndex = desktopItemProxyModel.indexOfUri(uri)
                if (itemIndex >= 0) {
                    scrollToIndex(itemIndex)
                    selectionModel.selectCurrent(itemIndex)
                }
            }
        }
    }
    Connections {
        target: Controller
        // function onClickedScreenIdChanged(screenId) {
        //     if (screenId !== desktopIconView.currentScreentId) {
        //         selectionModel.clearSelection();
        //     }
        //     console.log("Click on screen", screenId);
        // }
        function onEdit(uri) {
            startEditForUri(uri)
        }
        function onSelectAll() {
            if (widgetFocusScope.activeFocus) {
                selectionModel.invertSelections()
            }
        }
    }
}
