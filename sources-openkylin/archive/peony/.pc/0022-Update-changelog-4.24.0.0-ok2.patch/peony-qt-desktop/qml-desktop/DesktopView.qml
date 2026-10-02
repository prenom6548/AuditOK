// Main.qml
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import org.ukui.peony.desktop 1.0
import org.ukui.quick.items 1.0 as UKUIQuickItems
import org.ukui.quick.platform 1.0 as UKUIQuickPlatform
import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import QtGraphicalEffects 1.15

import "." as ComponentsManager

WidgetContainerItem {
    id : window
    objectName: "rootRectangle"
    anchors.fill: parent
    property int number: Screen.serialNumber
    visible: true
    // color: "transparent"
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    property int spaceId: -1
    property bool acceptDrop: true
    property bool isDraging: false
    property bool ismultifilemode: false
    property bool isBoxSelection: false
    property point pressPos: Qt.point(0, 0)
    property bool sorting: false
    property bool initDesktop: false
    property bool hasInitAnima: false
    /// DropArea 的 onPositionChanged 里 drag.formats 可能未定义，用 onEntered 结果作回退
    property bool pendingPeonyWidgetDrop: false
    property bool modelViewReady: false

    // PAG 首帧超时保护：3 秒内未就绪则强制显示图标
    Timer {
        id: pagWaitSafetyTimer
        interval: 3000
        running: backgroundItem.isPagWallpaper && !backgroundItem.pagFirstFrameReady
        repeat: false
        onTriggered: {
            console.warn("DesktopView: PAG firstFrameReady timeout, forcing initDesktop")
            if (!initDesktop && modelViewReady)
                initDesktop = true
        }
    }
    signal dropRequest(var urls)
    signal addItem();

    function dropMimeListHas(list, mime) {
        if (list === undefined || list === null)
            return false;
        if (typeof list.indexOf !== "function")
            return false;
        return list.indexOf(mime) >= 0;
    }

    function isLikelyPeonyWidgetPayload(payload) {
        var text = String(payload || "").replace(/^\uFEFF/, "").trim();
        if (text.length === 0 || text.charAt(0) !== "{")
            return false;
        return text.indexOf("\"widgetId\"") >= 0;
    }

    Binding {
        target: island
        property: "currentWidgetsLayoutWidth"
        value: widgetsLayout.width
    }

    Binding {
        target: island
        property: "currentWidgetsLayoutHeight"
        value: widgetsLayout.height
    }

    function cancelActiveDesktopBoxSelect() {
        if (!selectionRect.visible)
            return
        pressPos = Qt.point(-1, -1)
        selectionRect.visible = false
        selectionRect.width = 0
        selectionRect.height = 0
        isBoxSelection = false
        selectionModel.clearSelection()
        ismultifilemode = false
        desktopIconView.menuVisible = false
    }

    // 滚轮缩放事件处理 - 直接调用控制器的缩放方法
    WheelHandler {
        id: wheelHandler
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedModifiers: Qt.ControlModifier  // 只在按住Ctrl时触发
        onWheel: (event) => {
            if (event.modifiers & Qt.ControlModifier) {
                // 根据滚轮方向调用对应的缩放方法
                if (event.angleDelta.y > 0) {
                    // 滚轮向上 - 放大
                    controller.zoomIn();
                } else {
                    // 滚轮向下 - 缩小
                    controller.zoomOut();
                }
                event.accepted = true; // 阻止事件继续传播
            }
        }
    }

    BackgroundItem {
        id: backgroundItem
        anchors.fill: parent
        backImagePath: backgroundManager.backImagePath
        frontImagePath: backgroundManager.frontImagePath
        backImageSourceSize: backgroundManager.backImageSourceSize
        frontImageSourceSize: backgroundManager.frontImageSourceSize
        backgroundOption: backgroundManager.backgroundOption
        usePureColor: backgroundManager.usePureColor
        bgColor: backgroundManager.color
        screenGeometry: Qt.rect(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height)
    }

    Connections {
        target: Qt.application
        function onScreensChanged() {backgroundItem.handleSpanned()}
    }


    Connections{
       target: windowManager
       function onInitAnima() {
           if(!hasInitAnima){
               hasInitAnima = true
           }
       }
    }

    // 解锁重播：将动效状态归零，等 PAG 首帧就绪后重新触发图标入场动效。
    Connections {
        target: windowManager
        function onStartInitAnimaRequest() {
            hasInitAnima = false
            if (backgroundItem.isPagWallpaper)
                initDesktop = false   // 由 onPagFirstFrameReadyChanged 重新置 true
        }
    }

    SelectionModel {
        id: selectionModel
        targetView: desktopIconView
        function onClearAllStatus() {
            desktopIconView.closeEdit()
        }
    }

    ShortcutManager {
        id: shortcutManager
    }

    function layoutchange(){
        window.isDraging = false
        window.sorting = true
//        for (var i = 0; i < desktopIconView.count; i++) {
//            var item = desktopIconView.itemAt(i);
//            if (item) {
//                desktopIconView.addItemPosition(item.uri, item.pos.x, item.pos.y);
//                var tmpX = desktopIconView.transtogridposx(item.pos.x*desktopIconView.gridWidth,desktopIconView.gridWidth)
//                var tmpY = desktopIconView.transtogridposy(item.pos.y*desktopIconView.gridHeight,desktopIconView.gridHeight)
//                var tmp = Qt.point(tmpX, tmpY)
//                // note: 这里不直接设置item的x和y，避免behavior的过渡效果失效
//                //item.x = desktopIconView.transtogridposx(item.pos.x*desktopIconView.gridWidth,desktopIconView.gridWidth)
//                //item.y = desktopIconView.transtogridposy(item.pos.y*desktopIconView.gridHeight,desktopIconView.gridHeight)
//            }
//            item.startSortanima()
//            var uri = item.uri
//            desktopIconView.preitemsPos[item.uri] = Qt.point(item.pos.x, item.pos.y)
//        }
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
        clip: true  // 启用裁剪
        z: 100

        // 动态计算边界
        property rect bounds: calculateContentBounds()
        property real offsetToBoundsTopLeftX: 0
        property real offsetToBoundsTopLeftY: 0

        Drag.supportedActions:Qt.MoveAction
        Drag.dragType: Drag.Automatic
        Drag.onDragStarted: {
            console.log("item drag start")
            isDraging = true
        }

        Drag.onDragFinished: {
            console.log("item drag finish======")
            isDraging = false
            window.ismultifilemode = false
        }

        function calculateContentBounds() {
            var minX = 0, minY = 0;
            var maxX = 0, maxY = 0;
            if (dragitemModel.count > 0) {
                minX = dragitemModel.get(0).x
                minY = dragitemModel.get(0).y
            }
            for (var i = 0; i < dragitemModel.count; i++) {
                var item = dragitemModel.get(i);
                minX = Math.min(minX, item.x);
                minY = Math.min(minY, item.y);
                maxX = Math.max(maxX, item.x);
                maxY = Math.max(maxY, item.y);
                console.log("minX, minY, maxX - minX, maxY - minY", minX, minY, maxX - minX, maxY - minY,item.x,item.y,dragitemModel.count)
            }
            return Qt.rect(minX, minY, maxX - minX + desktopIconView.gridWidth , maxY - minY + desktopIconView.gridHeight );
        }

        Repeater {
            model: dragitemModel
            delegate: Rectangle {
                id: deleitem
                x: model.x - parent.bounds.x
                y: model.y - parent.bounds.y
                width: desktopIconView.gridWidth - 10
                height: desktopIconView.gridHeight - 10
                color: "transparent"
                DelegateIcon {
                    id: dragviewIconRect
                    width:  controller.iconSize.width
                    height:  controller.iconSize.height
                    icon : controller.icon(model.uri)
                    emblems: model.emblems
                    emblemSize: controller.emblemSize
                    uri: model.uri
                }
                DelegateText {
                    id: dragdelegateText
                    anchors.horizontalCenter: dragviewIconRect.horizontalCenter
                    anchors.top: dragviewIconRect.bottom
                    display: model.display
                    editing: false  // 是否处于编辑状态
                }
            }
        }
    }

    //绘制网格虚线辅助  可注释
    //    Repeater {
    //        model: Math.ceil(parent.height / desktopIconView.gridHeight)
    //        delegate: Rectangle {
    //            width: parent.width
    //            height: 1
    //            y: index * desktopIconView.gridHeight
    //            color: "transparent"
    //            border.color: Qt.rgba(0.5, 0.5, 0.5, 0.3)
    //            border.width: 1
    //            layer.enabled: true
    //            //layer.effect: DashLineEffect {} // 需要自定义DashLineEffect
    //        }
    //    }

    //    // 垂直虚线
    //    Repeater {
    //        model: Math.ceil(parent.width / desktopIconView.gridWidth)
    //        delegate: Rectangle {
    //            width: 1
    //            height: parent.height
    //            x: index *  desktopIconView.gridWidth
    //            color: "transparent"
    //            border.color: Qt.rgba(0.5, 0.5, 0.5, 0.3)
    //            border.width: 1
    //            layer.enabled: true
    //            //.effect: DashLineEffect {}
    //        }
    //    }

    Item {
        id: contentArea
        anchors.fill: parent
        anchors.topMargin: globalSettings.panelMarginTop
        anchors.rightMargin: window.LayoutMirroring.enabled? globalSettings.panelMarginLeft : globalSettings.panelMarginRight
        anchors.bottomMargin: globalSettings.panelMarginBottom
        anchors.leftMargin: window.LayoutMirroring.enabled? globalSettings.panelMarginRight : globalSettings.panelMarginLeft // 关键：留出边框宽度
        property string searchBuffer: ""
        property Timer searchResetTimer: Timer {
            interval: 1000 // 输入间隔超过1秒则清空搜索缓冲
            onTriggered: contentArea.searchBuffer = ""
        }
        // 添加一个专门的容器来设置图标与桌面边缘的内边距
        Item { // iconContainer start
            id: iconContainer
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.topMargin: 5
            anchors.rightMargin: 10
            anchors.bottomMargin: -5

        // 拖拽插入指示线
        Rectangle {
            id: dropIndicator
            visible: false
            width: desktopIconView.gridWidth
            height: 4
            color: "white"
            z: -10
        }

        Repeater {
            id: desktopIconView
            model: myProxyModel
            property int gridWidth: controller.gridSize.width
            property int gridHeight: controller.gridSize.height
            property int currentEditIndex: -1 // 当前编辑项的索引
            property int initDataCount: 0
            property int currentScreentId: model.id()
            property bool autoArrange: globalSettings.useAutoLayout()
            property bool menuVisible: false
            property bool hasMultipleFilesInMenu: false
            //property bool initDesktop: false
            property var resolutionItemPos: []
            property var itemsPos: []
            property var urilist: []
            property point delegateGlobalPos
            property alias isMultifileMode: window.ismultifilemode

            function transtogridposx(x,width){
                var centerX = x + width/2;
                // 计算最近网格索引
                var gridX = Math.round(centerX /gridWidth);
                gridX = Math.abs(centerX -gridWidth*(gridX-0.5))<Math.abs(gridWidth*(gridX+0.5) -centerX)? gridX-1:gridX
                gridX = Math.max(0, gridX);
                return (gridX)*gridWidth
            }
            function transtogridposy(y,height){
                var centerY = y + height/2;
                // 计算最近网格索引
                var gridY = Math.round(centerY /gridHeight);
                gridY = Math.abs(centerY -gridHeight*(gridY-0.5))<Math.abs(gridHeight*(gridY+0.5) -centerY)? gridY-1:gridY
                gridY = Math.max(0, gridY);
                return gridY*gridHeight
            }
            function closeEdit() {
                console.log("closeEdit",desktopIconView.currentEditIndex);
                if (desktopIconView.currentEditIndex !== -1) {
                    var item = desktopIconView.itemAt(desktopIconView.currentEditIndex)
                    if (item && item.delegateText) {
                        item.delegateText.updateFileName();
                    }
                    desktopIconView.currentEditIndex = -1
                }
            }

            function indexAt(x, y) {
                console.log("获取index 通过x，y", x,  y)
                x -= iconContainer.anchors.leftMargin
                y -= iconContainer.anchors.topMargin
                for (var i = 0; i < desktopIconView.count; ++i) {
                    var item = desktopIconView.itemAt(i)
                    if (x >= item.x && x <= item.x + desktopIconView.gridWidth &&
                            y >= item.y && y <= item.y + desktopIconView.gridHeight) {
                        console.log("获取索引", i, item.uri)
                        return i // 返回匹配的索引
                    }
                }
                return -1 // 未找到匹配项
            }

            function onUpdateDropIndicator(pos) {
                console.log("current mouse pos:",pos.x, pos.y)
                var hoverIndex = desktopIconView.indexAt(pos.x, pos.y)
                console.log("hover index:",hoverIndex)
                for (var i = 0; i < desktopIconView.count; i++) {
                   var item = desktopIconView.itemAt(i);
                   if (item) {
                       item.dragHovered = false;
                   }
                }
                if (hoverIndex >= 0) {
                    var hoverItem = desktopIconView.itemAt(hoverIndex)
                    hoverItem.dragHovered = true
                }
                if (desktopIconView.autoArrange === false) {
                    return
                }
                if (hoverIndex >= 0) {
                    var hoverItem = desktopIconView.itemAt(hoverIndex)
                    hoverItem.dragHovered = true
                    var localPos = hoverItem.mapFromItem(desktopIconView, pos.x, pos.y)

                    if (localPos.y < hoverItem.height * 0.3) {
                        dropIndicator.y = hoverItem.y
                        dropIndicator.visible = true
                    }
                    else if (localPos.y > hoverItem.height * 0.7) {
                        dropIndicator.y = hoverItem.y + hoverItem.height
                        dropIndicator.visible = true
                    }
                    else {
                        dropIndicator.visible = false
                        return
                    }
                    dropIndicator.x = hoverItem.x
                } else {
                    dropIndicator.visible = false
                }
            }

            delegate: Rectangle {
                id: dele
                // 初始隐藏，等 PAG 首帧就绪后由 initAnimation 统一带入
                opacity: 0
                // 使用 Binding 元素确保位置绑定
                Binding {
                    target: dele
                    property: "x"
                    value: window.LayoutMirroring.enabled? (iconContainer.width - (pos.x + 1)*desktopIconView.gridWidth): pos.x*desktopIconView.gridWidth
                    when: true
                }

                Binding {
                    target: dele
                    property: "y"
                    value: pos.y * desktopIconView.gridHeight
                    when: true
                }

                z: selected? parent.z + 1 : parent.z
                width: desktopIconView.gridWidth - 5
                height: viewIconRect.height + delegateText.height > desktopIconView.gridHeight - 5 ?
                            viewIconRect.height + delegateText.height + 10 : desktopIconView.gridHeight - 5
                radius: themeStyle.radius
                color: selected && !delegateText.editing ? themeStyle.highlight() : hovered && !delegateText.editing ? themeStyle.midlight() : "transparent"
                focus: true
                activeFocusOnTab: true
                layer.samples: 4     // 抗锯齿
                property string uri: model.uri
                property string display: model.display
                property point pos: model.position
                property bool selected: false
                property bool hovered: gridViewDelegateMouseArea.containsMouse && gridViewDelegateMouseArea.reallyContainsMouse || dragHovered
                property bool dragHovered: false
                property alias delegateText: delegateText  // 将子项暴露为属性
                property alias pressPos1: gridViewDelegateMouseArea.pressPos1
                property point delegateGlobalPos
                property bool isFromDesktopView: true
                property int itemIndex: index
                property int dragx: x
                property int dragtox: x
                property int dragy: y
                property int dragtoy: y
                property alias animItem: anim
                property alias animyItem: animy
                property alias xBehaviorObject: xTransBehavior
                property alias yBehaviorObject: yTransBehavior
                property var decoration: model ? model.decoration : ""
                property var emblems:  model ? model.emblems : ""
                function startSortanima() {
                    //animaresort.start()
                    //animaresorty.start()
                }
                function startinitanima() {
                    //initanim.start()
                }

                ToolTip {
                    id: delegateToolTip
                    // 此处绑定方法会导致ukui主题tooltip概率性偏移，需要用其它方式实现
                    //text: visible? model.tooltip: ""
                    visible: hovered && !dragHovered
                    delay: 200
                    closePolicy: Popup.CloseOnPressOutside
                    //property int textFormat: Qt.RichText
                    property int maxWidth: 400
                    property var wrapMode: Text.WrapAnywhere
                    // 动态更新tooltip内容
                    onAboutToShow: {
                        text = model.tooltip
                    }
                }

                ParallelAnimation {
                    id: initAnimation
                    XAnimator {
                        target: dele
                        from: x - desktopIconView.gridWidth
                        to: x
                        duration: 300
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [0.25, 0.1, 0.38, 1]
                    }
                    OpacityAnimator {
                        target: dele
                        //property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 300
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [0.25, 0.1, 0.38, 1]
                    }
                }

                Component.onCompleted: {
                    if (initDesktop)
                        opacity = 1
                }

                Connections {
                    target: window
                    function onInitDesktopChanged() {
                        // initDesktop false→true 时才处理（false 时什么也不做，避免
                        // 解锁期间 initDesktop 临时归零触发不必要的操作）
                        if (!initDesktop) return
                        //快速启动时防抖 桌面进程启动时先不走动画流程
                        if(windowManager.getQuickStartAvailable() && windowManager.getLockState()){
                              dele.opacity = 0
                        }else{
                            initAnimation.start()
                            hasInitAnima = true
                        }
                    }
                }

                Connections{
                   target: windowManager
                   function onStartInitAnimaRequest() {
                       // 解锁：立即隐藏图标，等待 PAG 首帧就绪（onInitDesktopChanged）后重新入场。
                       // 不在此处直接播放动效，动效由 initDesktop false→true 触发，
                       // 确保图标与 PAG 首帧同步出现。
                       dele.opacity = 0
                   }
                }

                //            ParallelAnimation {
                //                id: initanim
                //                NumberAnimation {
                //                    target: dele
                //                    property: "x"
                //                    from: dele.x - desktopIconView.gridWidth
                //                    to: dele.x
                //                    duration: 300
                //                    easing.type: Easing.Bezier
                //                    easing.bezierCurve: [0.25, 0.1, 0.38, 1]
                //                }
                //                NumberAnimation {
                //                    target: dele
                //                    property: "opacity"
                //                    from: 0.0
                //                    to: 1.0
                //                    duration: 300
                //                    easing.type: Easing.Bezier
                //                    easing.bezierCurve: [0.25, 0.1, 0.38, 1]

                //                    onStopped: {
                //                        desktopIconView.initDataCount = desktopIconView.initDataCount + 1
                //                        desktopIconView.initDesktop = desktopIconView.initDataCount === desktopIconView.model.count() ? true : false
                //                    }
                //                }
                //            }

                //            Component.onCompleted: {
                //                 if (false === desktopIconView.initDesktop) {
                //                    //initanim.start()
                //                 }
                //                 desktopIconView.addItemPosition(model.uri, model.position.x, model.position.y)
                //            }

                Behavior on x {
                    enabled: initDesktop && !isDraging
                    id: xTransBehavior
                    NumberAnimation {
                        easing.type: Easing.Bezier;
                        easing.bezierCurve: [0.25, 0.1, 0.38, 1];
                        duration: 300
                        onFinished: {
                            window.sorting = false
                        }
                    }
                }
                Behavior on y {
                    enabled: initDesktop && !isDraging
                    id: yTransBehavior
                    NumberAnimation {
                        easing.type: Easing.Bezier;
                        easing.bezierCurve: [0.25, 0.1, 0.38, 1];
                        duration: 300
                    }
                }

                NumberAnimation {
                    id: anim
                    target: dele
                    property: "x"
                    from: dragx
                    to: dragtox
                    duration: 180
                    easing.type: Easing.Bezier
                    easing.bezierCurve:  [0.25, 0.1, 0.38, 1] // 标准缓入缓出贝塞尔曲线
                    // 也可以使用预设的缓入缓出类型
                    // easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    id: animy
                    target: dele
                    property: "y"
                    from: dragy
                    to: dragtoy
                    duration: 180
                    easing.type: Easing.Bezier
                    easing.bezierCurve:  [0.25, 0.1, 0.38, 1] // 标准缓入缓出贝塞尔曲线
                    // 也可以使用预设的缓入缓出类型
                    // easing.type: Easing.InOutQuad
                }
                //            NumberAnimation {
                //                id: animaresort
                //                target: dele
                //                property: "x"
                //                from:desktopIconView.transtogridposx(desktopIconView.preitemsPos[dele.uri].x*dele.width,dele.width)
                //                to: dele.x
                //                duration: 300
                //                easing.type: Easing.Bezier
                //                easing.bezierCurve: [0.25, 0.1, 0.38, 1] // 标准缓入缓出贝塞尔曲线
                //                // 也可以使用预设的缓入缓出类型
                //                // easing.type: Easing.InOutQuad
                //                onFinished: {
                //                    addItemPosition(dele.uri, dele.x, dele.y)
                //                }
                //            }
                //            NumberAnimation {
                //                id: animaresorty
                //                target: dele
                //                property: "y"
                //                from: desktopIconView.transtogridposy(desktopIconView.preitemsPos[dele.uri].y*dele.height,dele.height)
                //                to: dele.y
                //                duration: 300
                //                easing.type: Easing.Bezier
                //                easing.bezierCurve: [0.25, 0.1, 0.38, 1] // 标准缓入缓出贝塞尔曲线
                //                // 也可以使用预设的缓入缓出类型
                //                // easing.type: Easing.InOutQuad
                //            }

                DelegateIcon {
                    id: viewIconRect
                    width:  controller.iconSize.width
                    height:  controller.iconSize.height
                    icon : model.decoration
                    emblems: model.emblems
                    emblemSize: controller.emblemSize
                    uri: model.uri
                }

                DelegateText {
                    id: delegateText
                    anchors.horizontalCenter: viewIconRect.horizontalCenter
                    anchors.top: viewIconRect.bottom
                    display: dele.display  // 显示的文本内容
                    editing: desktopIconView.currentEditIndex === index && desktopIconView.currentEditIndex !== -1 // 控制是否进入编辑状态
                    showFullText: !ismultifilemode && !isBoxSelection && !(desktopIconView.menuVisible && desktopIconView.hasMultipleFilesInMenu)
                    selected: parent.selected
                    isDir: model.isDir
                    isSoftLink: model.softLink
                    maxVisibleHeight: {
                        var globalY = dele.y + viewIconRect.height;
                        var maxHeight = contentArea.height - globalY - 20
                        return Math.max(0, maxHeight)
                    }
                }
                // Canvas {
                //       id: gridLabelColors
                //       property var colors: model.labelColors
                //       anchors.bottom: viewIconRect.bottom
                //       anchors.left: viewIconRect.right
                //       anchors.right: parent.right
                //       height: 8
                //       visible: colors.length > 0
                //       onPaint: {
                //           var offset = 0
                //           var ctx = getContext("2d")
                //           ctx.reset() // 清除之前的绘制内容
                //           for (var i = 0; i < colors.length; i++) {
                //               ctx.beginPath()
                //               ctx.arc(offset + height / 2, height / 2, height / 2, 0,
                //                       Math.PI * 2)
                //               ctx.closePath()

                //               ctx.fillStyle = colors[i]
                //               ctx.fill()

                //               offset += height / 2
                //           }
                //       }
                //       onColorsChanged: {
                //             if (colors && colors.length > 0) {
                //                 requestPaint()
                //             }
                //         }
                //   }
                Connections {
                    target: delegateText
                    function onEditFinish(selections, newText, isRename) {
                        if (isRename) {
                            selectionModel.clearSelection();
                            controller.rename(selections, newText)
                            desktopIconView.currentEditIndex = -1
                        } else {
                            selectionModel.setSelection(selections);
                        }
                    }
                }

                MouseArea {
                    id: gridViewDelegateMouseArea
                    z: 8
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    propagateComposedEvents: true
                    hoverEnabled: true

                    property bool reallyContainsMouse: false
                    property point pressPos1: Qt.point(0, 0)
                    property point releasepos: Qt.point(0, 0)
                    property bool doubleClickHandled: false
                    property bool allowDrag: true
                    property Timer editTriggerTimer: Timer {
                        id: editTriggerTimer
                        interval: 300
                        onTriggered: {
                            //console.info("doubleClickHandled: " + gridViewDelegateMouseArea.doubleClickHandled + " selected: " + dele.selected)
                            if (!gridViewDelegateMouseArea.doubleClickHandled && dele.selected) {
                                controller.editUris(selectionModel.getSelections());
                            }
                        }
                    }
                    onEntered: {
                        reallyContainsMouse = false
                    }
                    onExited: {
                        reallyContainsMouse = false
                    }
                    onPositionChanged: {
                        reallyContainsMouse = true
                    }
                    onPressed: (mouse) => {
                                   delegateToolTip.hide()
                                   doubleClickHandled = false
                                   console.log("mouse onPressed on delegate, boxselection:",isBoxSelection,dele.selected);
                                   controller.setClickedScreenId(desktopIconView.currentScreentId)
                                   if (mouseX < 0 || mouseX > width || mouseY < 0 || mouseY > height) {
                                       mouse.accepted = false
                                       return;
                                   }
                                   
                                   // Check mouse click permission
                                   if (mouse.button === Qt.LeftButton && !ManagerControl.checkMouseClickPermission(dele.uri, false)) {
                                       console.log("Left click disabled for item:", dele.uri);
                                       mouse.accepted = false;
                                       return;
                                   }

                                   // 只有左键才允许拖拽
                                   allowDrag = mouse.button !== Qt.RightButton

                                   var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier;
                                   var controlKeyPressed = mouse.modifiers & Qt.ControlModifier;
                                   if ( shiftKeyPressed || controlKeyPressed || (isBoxSelection && dele.selected)) {
                                       console.log("Ctrl or shift key is pressed!");
                                       ismultifilemode = true
                                       if (shiftKeyPressed) {
                                           var startIndex = selectionModel.firstIndex();
                                           selectionModel.selectBetween(startIndex, index)
                                       }
                                   }
                                   console.log("mouse onPressed ismultifilemode",ismultifilemode);
                                   if (mouse.button === Qt.RightButton) {
                                       // Check right-click menu permission
                                       if (!ManagerControl.checkRightMenuPermission(dele.uri)) {
                                           console.log("Right menu disabled for item:", dele.uri);
                                           mouse.accepted = false;
                                           return;
                                       }
                                       
                                       if (!ismultifilemode) {
                                           selectionModel.clearSelection()
                                       }
                                       desktopIconView.menuVisible = true;
                                       var globalPos = mapToGlobal(mouse.x, mouse.y);
                                       selectionModel.updateSelection(index, true)
                                       var selections = selectionModel.getSelections();
                                       desktopIconView.hasMultipleFilesInMenu = selections.length > 1 ? true : false
                                       console.log("open menu", globalPos, selections, mouse.x, mouse.y)
                                       Qt.callLater(() => controller.showMenu(selections, globalPos))
                                       mouse.accepted = true  // 阻止事件继续传递
                                       return
                                   }
                                   controller.closeMenu()
                                   console.log("closeMenu........")
                                   if (!ismultifilemode) {
                                       if (delegateText.editing) {
                                           console.log("editing........")
                                           mouse.accepted = false
                                           return
                                       }
                                       desktopIconView.closeEdit()

                                       if (dele.selected) {
                                           editTriggerTimer.start()
                                       } else {
                                           selectionModel.clearSelection()
                                       }
                                   }

                                   pressPos1 = Qt.point(mouseX, mouseY)
                                   var realBounds = dragIconView.calculateContentBounds()
                                   dragIconView.offsetToBoundsTopLeftX = dele.x
                                   dragIconView.offsetToBoundsTopLeftY = dele.y
                                   if (controlKeyPressed && dele.selected) {
                                       dele.selected = false;
                                   } else {
                                       selectionModel.updateSelection(index, true)
                                   }
                                   desktopIconView.menuVisible = false;
                               }
                    onReleased: {
                        console.log("mouse released");
                        isBoxSelection = false;
                        var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier;
                        var controlKeyPressed = mouse.modifiers & Qt.ControlModifier;
                        if (!(shiftKeyPressed || controlKeyPressed) && desktopIconView.currentEditIndex === -1 && !doubleClickHandled) {
                            ismultifilemode = false
                            if (!desktopIconView.menuVisible) {
                                selectionModel.selectCurrent(index)
                            }
                        }
                    }
                    onClicked: {
                        console.log("mouse onClicked");
                        mouse.accepted = true
                        DesktopOrganizationManager.cancelEdit()
                    }
                    onDoubleClicked: {
                        // Check double-click permission
                        if (!ManagerControl.checkMouseClickPermission(dele.uri, true)) {
                            console.log("Double click disabled for item:", dele.uri);
                            mouse.accepted = false;
                            return;
                        }
                        
                        doubleClickHandled = true
                        editTriggerTimer.stop()
                        var item = desktopIconView.itemAt(index)
                        controller.openFileByUri(item.uri);
                        console.log("on double clicked", item.uri)
                        mouse.accepted = true
                        selectionModel.clearSelection()
                    }

                    drag.target: dragIconView
                    drag.onActiveChanged: {
                        console.log("mouse area drag active", drag.active, allowDrag)
                        if (drag.active && allowDrag) {
                            editTriggerTimer.stop()
                            desktopIconView.closeEdit()
                            // hotspot的计算要在bounds更新之后
                            dragIconView.Drag.source =  dele
                            dele.delegateGlobalPos = mapToItem(desktopIconView, 0, 0)
                            dragIconView.Drag.mimeData = {
                                "text/uri-list": selectionModel.getSelections()
                                .map(uri => uri.trim())  // 去除每个URI两端的空格
                                .filter(uri => uri !== "")  // 过滤空URI
                                .join("\r\n"),
                                "peony-qt/encoded-uris": selectionModel.getSelections()
                                .filter(uri => uri.trim() !== "")
                                .join(" "),
                                "peony-qt/pos-list": selectionModel.getSelectionsPositions()
                                .map(point => `${point.x},${point.y}`) // 将QPoint转为"x,y"格式
                                .join(" "), // 用空格分隔多个坐标
                                "text/ukui_desktop": true,
                                "text/plain":selectionModel.getSelections()
                                .map(uri => uri.trim())  // 去除每个URI两端的空格
                                .filter(uri => uri !== "").join(" "),
                                "application/x-qabstractitemmodeldatalist": selectionModel.getSelections()
                                    .map(uri => uri.trim())
                                    .filter(uri => uri !== "")
                                    .map(uri => ({
                                        "0": {
                                            "text/uri-list": uri
                                        }
                                    }))
                                    .join(","),
                                 "peony-qt/iconContainerWidth": iconContainer.width
                            };

                            // grabToImage不会立即返回所以在回调中处理
                            var items = selectionModel.getSelectedItems()
                            dragitemModel.clear()
                            for(var i = 0;i < items.length; i++){
                                dragitemModel.append(items[i])
                            }
                            // 设置hotSpot为鼠标在按下相对于选中（bounds）左上角区域的位置
                            dragIconView.Drag.hotSpot.x = gridViewDelegateMouseArea.pressPos1.x + dragIconView.offsetToBoundsTopLeftX - dragIconView.bounds.x
                            dragIconView.Drag.hotSpot.y = gridViewDelegateMouseArea.pressPos1.y + dragIconView.offsetToBoundsTopLeftY - dragIconView.bounds.y
                            dragIconView.grabToImage(function(result) {
                                dragIconView.Drag.imageSource = result.url
                                dragIconView.Drag.active = true
                            })
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
                            desktopIconView.model.invalidateModel()
                        }
                    }
                }
            }

            Component.onCompleted: {

                // 监听变化
                globalSettings.onValueChanged.connect(function(key, value) {
                    console.log("Setting changed:", key, "=", value)
                    if (key === "desktopUseAutoLayout") {
                        autoArrange = globalSettings.useAutoLayout();
                    }
                })
            }
        }

        // GridView {
        //     id: testWidgetGrid
        //     z: -999
        //     Rectangle {
        //         anchors.fill: parent
        //         color: "blue"
        //         z: -999
        //     }

        //     anchors.fill: parent
        //     model: 500
        //     cellWidth: 64
        //     cellHeight: 64
        //     rightMargin: -4
        //     LayoutMirroring.enabled: true
        //     flow: GridView.FlowTopToBottom
        //     delegate: Rectangle {
        //         width: 60
        //         height: 60
        //         color: "red"
        //     }
        // }

        Rectangle {
            id: dragIndicatorRect
            // z: -1
            border.width: 4
            border.color: UKUIQuickPlatform.GlobalTheme.kFontWhitePlaceholderText.pureColor
            radius: UKUIQuickPlatform.GlobalTheme.kRadiusWindow
            color: "transparent"
            visible: false
            // width: 200
            // height: 200

            transform: Translate {
                x: widgetsLayout.width - dragIndicatorRect.width - dragIndicatorRect.x * 2 //镜像放置x1 + width + x2 = parent width, 偏移量为x2 - x1
                y: 0
            }
        }

        WidgetsLayout {
            id: widgetsLayout
            dragIndicator: dragIndicatorRect
           Component.onCompleted: {
               initModel()
               widgetsLayout.model = widgetsProxyModel
           }

           function initModel() {
               // 延迟设置 sourceModel
               Qt.callLater(function() {
                   widgetsProxyModel.sourceModel = window.widgetItemModel
                   console.log("ProxyModel sourceModel set after layout ready")
               })
           }
            model: WidgetsProxyModel {
                id: widgetsProxyModel
                //sourceModel: window.widgetItemModel
            }

        }

        } // iconContainer end

        //框选矩形
        Rectangle {
            id: selectionRect
            visible: false
            color: "lightblue"
            opacity: 0.3
            border.color: "blue"
            border.width: 1
            z: 999
        }
        // 框选功能的 MouseArea
        MouseArea {
            anchors.fill: parent
            z: -1
            // 初始化选择框
            acceptedButtons: Qt.LeftButton | Qt.RightButton  // 明确接受右键
            onPressed: (mouse) => {
                           controller.dragDesktopWidgets = false
                           console.log("mouse onPressed on desktop area");
                           controller.setClickedScreenId(desktopIconView.currentScreentId)
                           // 检查是否点击在图标上
                           var clickedOnIcon = false;
                           var index = desktopIconView.indexAt(mouseX, mouseY)
                           var item = desktopIconView.itemAt(index)
                           if (item) {
                               // 检查点击位置是否真的在图标范围内
                               if (mouseX >= item.x + iconContainer.anchors.leftMargin &&
                                   mouseX <= item.x + iconContainer.anchors.leftMargin + item.width &&
                                   mouseY >= item.y + iconContainer.anchors.topMargin &&
                                   mouseY <= item.y + iconContainer.anchors.topMargin + item.height) {
                                   clickedOnIcon = true;
                               }
                           }

                           // 如果点击在图标上且图标正在编辑，则不触发框选
                           if (clickedOnIcon && item.delegateText && item.delegateText.editing) {
                               // 点击在正在编辑的图标上，应该触发拖拽而不是框选
                               pressPos = Qt.point(-1, -1) // 设置特殊值表示不进行框选
                               selectionRect.visible = false
                               selectionRect.width = 0
                               selectionRect.height = 0
                               // 不清除选择，保持当前选择状态
                               desktopIconView.closeEdit() // 关闭编辑状态
                               controller.closeMenu()
                               mouse.accepted = false // 允许事件传递到图标组件
                               return
                           }
                           // 点在图标上：交给 delegate MouseArea 处理（选中、多选、右键菜单、拖拽等）
                           if (clickedOnIcon) {
                               pressPos = Qt.point(-1, -1)
                               selectionRect.visible = false
                               mouse.accepted = false
                               return
                           }
                           var shiftKeyPressed = mouse.modifiers & Qt.ShiftModifier;
                           var controlKeyPressed = mouse.modifiers & Qt.ControlModifier;
                           if (shiftKeyPressed || controlKeyPressed) {
                               console.log("Ctrl or Shift key is pressed!");
                               ismultifilemode = true
                           } else {
                               var index = desktopIconView.indexAt(mouseX, mouseY)
                               var item = desktopIconView.itemAt(index)
                               if (item) {
                                   if (!(mouseX >= item.x && mouseX <= item.x + item.width &&
                                         mouseY >= item.y && mouseY <= item.y + item.height)) {
                                       index = -1
                                   }
                               }
                               if (index === -1) {
                                   selectionModel.clearSelection();
                               }
                           }
                           console.log("drop to current index:", index)
                           if (mouse.button === Qt.LeftButton) {
                               pressPos = Qt.point(mouseX, mouseY)
                               selectionRect.x = mouseX
                               selectionRect.y = mouseY
                               selectionRect.width = 0
                               selectionRect.height = 0
                               selectionRect.visible = true
                               desktopIconView.closeEdit()
                               controller.closeMenu()
                           } else if (mouse.button === Qt.RightButton) {
                               desktopIconView.closeEdit()
                               if (selectionRect.width == 0 || selectionRect.height == 0) {
                                   selectionModel.clearSelection()
                               }
                               var i = desktopIconView.indexAt(mouse.x, mouse.y)
                               if (i !== -1) {
                                   selectionModel.updateSelection(i, true)
                               }

                               // Check right-click menu permission
                               var selections = selectionModel.getSelections();
                               if (selections.length === 0) {
                                   // Desktop background right-click
                                   if (!ManagerControl.checkBackgroundMenuPermission()) {
                                       console.log("Background menu disabled by management control");
                                       mouse.accepted = false;
                                       return;
                                   }
                               } else {
                                   // Desktop file right-click - check each selected item
                                   for (var j = 0; j < selections.length; j++) {
                                       if (!ManagerControl.checkRightMenuPermission(selections[j])) {
                                           console.log("Right menu disabled for item:", selections[j]);
                                           mouse.accepted = false;
                                           return;
                                       }
                                   }
                               }

                               pressPos = Qt.point(-1, -1)
                               selectionRect.visible = false
                               selectionRect.width = 0
                               selectionRect.height = 0
                               desktopIconView.menuVisible = true;
                               var pos = mapToGlobal(mouse.x, mouse.y)
                               var selections = selectionModel.getSelections();
                               desktopIconView.hasMultipleFilesInMenu = selections.length > 1 ? true : false
                               Qt.callLater(() => controller.showMenu(selections, pos))
                               isBoxSelection = false;
                               mouse.accepted = true
                               return
                           }
                       }
            onClicked: {
                console.log("mouse onClicked on desktop area");
                 DesktopOrganizationManager.cancelEdit()
            }
            // 实时更新选择框尺寸
            onPositionChanged: {
                desktopIconView.menuVisible = false;
                if (mouse.button === Qt.RightButton) {
                    desktopIconView.closeEdit()
                    isBoxSelection = false;
                    pressPos = Qt.point(-1, -1)
                    selectionRect.visible = false
                    selectionRect.width = 0
                    selectionRect.height = 0
                    mouse.accepted = true
                    return
                }
                if (pressPos.x === -1 && pressPos.y === -1) {
                    mouse.accepted = true
                    return
                }

                const newX = Math.min(mouseX, pressPos.x)
                const newY = Math.min(mouseY, pressPos.y)
                const newWidth = Math.abs(mouseX - pressPos.x)
                const newHeight = Math.abs(mouseY - pressPos.y)

                // 框选矩形未变则跳过后续 O(n) 选中更新（触摸会高频触发 positionChanged）
                if (newX === selectionRect.x && newY === selectionRect.y
                        && newWidth === selectionRect.width && newHeight === selectionRect.height) {
                    return
                }

                selectionRect.x = newX
                selectionRect.y = newY
                selectionRect.width = newWidth
                selectionRect.height = newHeight

                var tmpSelectionRect = Qt.rect(newX, newY, newWidth, newHeight)
                for (var i = 0; i < desktopIconView.count; i++) {
                    const item = desktopIconView.itemAt(i)
                    if (item && item.visible) {
                        const itemRect = Qt.rect(item.x + iconContainer.anchors.leftMargin,
                                                 item.y + iconContainer.anchors.topMargin,
                                                 item.width, item.height)
                        const isSelected = rectanglesIntersect(itemRect, tmpSelectionRect)
                        if (item.selected !== isSelected) {
                            item.selected = isSelected
                            selectionModel.updateSelection(i, isSelected)
                        }
                        if (isSelected) {
                            isBoxSelection = true
                        }
                    }
                }
            }

            function rectanglesIntersect(rectA, rectB) {
                return (rectA.x < rectB.x + rectB.width &&
                        rectA.x + rectA.width > rectB.x &&
                        rectA.y < rectB.y + rectB.height &&
                        rectA.y + rectA.height > rectB.y)
            }
            // 鼠标释放时校验选中项
            onReleased: {
                console.log("mouse onReleased on desktop area");
                desktopIconView.menuVisible = false;
                if (mouse.button === Qt.RightButton) {
                    desktopIconView.closeEdit()
                    selectionRect.visible = false
                    selectionRect.width = 0
                    selectionRect.height = 0
                    pressPos = Qt.point(-1, -1)
                    mouse.accepted = true
                    return
                }

                if (selectionRect.width == 0 || selectionRect.height == 0) {
                    selectionRect.visible = false
                    isBoxSelection = false;
                    pressPos = Qt.point(-1, -1)
                    return
                }
                for (var i = 0; i < desktopIconView.count; i++) {
                    const item = desktopIconView.itemAt(i)
                    if (item && item.visible) {
                        const itemRect = Qt.rect(item.x + iconContainer.anchors.leftMargin,
                                                 item.y + iconContainer.anchors.topMargin,
                                                 item.width,item.height)
                        // 检测相交
                        var tmpSelectionRect = Qt.rect(selectionRect.x, selectionRect.y, selectionRect.width, selectionRect.height);
                        const isSelected = rectanglesIntersect(itemRect, tmpSelectionRect)
                        if (isSelected) isBoxSelection = true
                        item.selected = isSelected || (ismultifilemode && item.selected)
                        selectionModel.updateSelection(i, item.selected)
                    }
                }
                selectionRect.visible = false
                selectionRect.width = 0
                selectionRect.height = 0
                pressPos = Qt.point(-1, -1)
            }
            // grab 被 PinchHandler 或系统手势抢走时，Qt 发 canceled 而非 released
            onCanceled: {
                selectionRect.visible = false
                selectionRect.width = 0
                selectionRect.height = 0
                pressPos = Qt.point(-1, -1)
                isBoxSelection = false
            }
        }

        // 双指及以上取消框选：框选期间全程启用，onReleased 后 selectionRect.visible=false 自动关闭，不干扰后续拖拽
        PinchHandler {
            id: desktopPinchCancel
            enabled: selectionRect.visible
            target: null
            minimumPointCount: 2
            maximumPointCount: 10
            acceptedDevices: PointerDevice.TouchScreen
            onActiveChanged: {
                if (active)
                    window.cancelActiveDesktopBoxSelect()
            }
        }

        DropArea {
            id: desktopWidgetDropArea
            anchors.fill: parent
            visible: true
            z:-1
            onEntered: {
                var fmt = drag.formats;
                var hasUriList = window.dropMimeListHas(fmt, "text/uri-list");
                var hasUrl = window.dropMimeListHas(fmt, "url");
                var hasWidget = window.dropMimeListHas(fmt, "application/x-peony-widget")
                    || window.dropMimeListHas(fmt, "application/x-peony-widget-drag")
                    || window.dropMimeListHas(fmt, "application/json");
                window.pendingPeonyWidgetDrop = hasWidget;
                var hasUriList = drag.formats.indexOf("text/uri-list") >= 0;
                var hasUrl = drag.formats.indexOf("url") >= 0;
                var hasDesktopWidgets = drag.formats.indexOf("DesktopWidgets") >= 0;
                if (!acceptDrop || ((!hasUriList && !hasUrl && !hasWidget) && !hasDesktopWidgets)) {
                    drag.accepted = false;
                    return;
                }
                if (hasWidget) {
                    drag.accepted = true;
                    drag.acceptedAction = Qt.CopyAction;
                }
            }

            onExited: {
                window.pendingPeonyWidgetDrop = false;
                dropIndicator.visible = false;
                for (var i = 0; i < desktopIconView.count; i++) {
                   var item = desktopIconView.itemAt(i);
                   if (item) {
                       item.dragHovered = false;
                   }
                }
            }

            onPositionChanged: (mouse) => {
                                   var fmt = drag.formats;
                                   var hasWidgetDrag = window.dropMimeListHas(fmt, "application/x-peony-widget")
                                       || window.dropMimeListHas(fmt, "application/x-peony-widget-drag")
                                       || window.dropMimeListHas(fmt, "application/json")
                                       || window.pendingPeonyWidgetDrop;
                                   if (hasWidgetDrag) {
                                       drag.acceptedAction = Qt.CopyAction;
                                   } else {
                                       drag.acceptedAction = (drag.modifiers & Qt.ControlModifier)
                                       ? Qt.CopyAction
                                       : Qt.MoveAction;
                                   }
                                   var currentPos = Qt.point(mouse.x, mouse.y)
                                   desktopIconView.onUpdateDropIndicator(currentPos)
                               }

            onDropped: {
                window.pendingPeonyWidgetDrop = false;
                var widgetPayload = drop.getDataAsString("application/x-peony-widget");
                if (!widgetPayload || widgetPayload.length === 0) {
                    widgetPayload = drop.getDataAsString("application/json");
                }
                if (!widgetPayload || widgetPayload.length === 0) {
                    widgetPayload = drop.getDataAsString("text/plain");
                }
                if (window.isLikelyPeonyWidgetPayload(widgetPayload)) {
                    widgetPayload = String(widgetPayload || "").replace(/^\uFEFF/, "").trim();
                    var posInLayout = widgetsLayout.mapFromItem(desktopWidgetDropArea, drop.x, drop.y);
                    if (platformWindow.addWidgetFromDropJson(widgetPayload, true,
                                                   Math.round(posInLayout.x), Math.round(posInLayout.y))) {
                        return;
                    }
                }
                // 1. 解析拖拽文件路径
                var srcUris = []

                // 优先尝试读取peony自定义格式
                var peonyUriData = drop.getDataAsString("peony-qt/encoded-uris")
                var orgstr = drop.getDataAsString("text/oganization_tool")
                var isDrapByOrgtool = false
                if(orgstr && orgstr == "oganization_tool"){
                    isDrapByOrgtool = true
                }

                // 检查是否是桌面插件拖拽到新屏幕
                var hasDesktopWidgets = drop.getDataAsString("DesktopWidgets") == "true"
                if (hasDesktopWidgets) {
                    var instanceIdStr = drop.getDataAsString("DesktopWidgets_instanceId")
                    var sourceScreenIdStr = drop.getDataAsString("DesktopWidgets_sourceScreenId")
                    if (instanceIdStr && sourceScreenIdStr) {
                        var instanceId = Number(instanceIdStr)
                        var sourceScreenId = Number(sourceScreenIdStr)
                        var targetScreenId = desktopIconView.currentScreentId

                        // 读取拖拽时在窗口内的按下偏移，修正 dropPos 为 widget 左上角位置
                        var pressPosStr = drop.getDataAsString("DesktopWidgets_pressPosition")
                        var pressX = 0, pressY = 0
                        if (pressPosStr) {
                            var parts = pressPosStr.split(",")
                            pressX = Number(parts[0])
                            pressY = Number(parts[1])
                        }
                        var dropPos = Qt.point(drop.x - pressX, drop.y - pressY)
                        console.log("[DesktopView] DesktopWidgets dropped: instanceId=" + instanceId + ", sourceScreenId=" + sourceScreenId + ", targetScreenId=" + targetScreenId)
                        island.handleDesktopWidgetDrop(instanceId, sourceScreenId, targetScreenId, dropPos)
                        return
                    }
                }
                if (peonyUriData && peonyUriData.length > 0) {
                    var uris = peonyUriData.split(" ")
                    for (var i = 0; i < uris.length; i++) {
                        if (!uris[i].startsWith("recent://")) {
                            srcUris.push(uris[i])
                        }
                    }
                } else {
                    // 回退到标准uri-list格式
                    var uriString = drop.getDataAsString("text/uri-list");
                    var uris = uriString.split(/\r?\n/)
                                        .map(uri => uri.trim())
                                        .filter(uri => uri !== "");

                    // 尝试从url格式获取单个文件路径
                    if (uris.length === 0) {
                        var appUrl = drop.getDataAsString("url")
                        if (appUrl && appUrl.length > 0) {
                            uris = [appUrl]
                        }
                    }
                }

                var y = drop.y
                if (dropIndicator.visible && y < dropIndicator.y) {
                    y = y + desktopIconView.gridHeight
                }
                var index = desktopIconView.indexAt(drop.x, y)
                console.log("drop to current index:", index)

                if (!(drag.source && drag.source.isFromDesktopView)) {
                    var dropIndicatorvisible = dropIndicator.visible
                    var onItem =  globalSettings.useAutoLayout() && -1 !== index? !dropIndicator.visible : -1 !== index
                    if(isDrapByOrgtool){
                        dropIndicator.visible = false
                        // drop事件从c++更新图标网格时，需要屏蔽x和y默认的behavior，使用拖拽动效
                        window.isDraging = true
                        var posresult =  desktopIconView.model.updateIconPositionsByOrgtool(uris, drop.proposedAction, index,onItem);
                        var size = Object.keys(posresult).length
                        if(size == 0){
                            for (var i = 0; i < desktopIconView.count; i++) {
                               var item = desktopIconView.itemAt(i);
                               if (item) {
                                   item.dragHovered = false;
                               }
                            }
                            return;
                        }
                        var orgstr = drop.getDataAsString("text/org_instanceid")
                        var dragFrominstanceid = Number(orgstr)
                        var dragpresspoint = drop.getDataAsString("text/drag-global-start-pos")
                        var startdragx = Number(dragpresspoint.split(",")[0])
                        var startdragy = Number(dragpresspoint.split(",")[1])
                        // offx = startdragx - viewlocalePos.x
                        // offy = startdragy - viewlocalePos.y
                        DesktopOrganizationManager.updateOrganizationView(dragFrominstanceid)
                        var dropGlobalPos = Qt.point(drop.x, y)  // drop.x, drop.y 是相对于接收拖拽的 Item 的坐标
                        //var gridpos = platformWindow.getGridPosFromMousePos(dropGlobalPos)
                        var count = 0
                        var gridpos = Qt.point(0, 0)
                        for (var i = 0; i < desktopIconView.count; i++) {
                            const item = desktopIconView.itemAt(i)
                            if (!(item && item.visible)) {
                                continue;
                            }
                            item.dragHovered = false;
                            if (uris.includes(item.uri)) {
                                var j = uris.indexOf(item.uri)
                                if (j === -1) {
                                    continue
                                }
                                //var dropGlobalPos = Qt.point(drop.x, drop.y)  // drop.x, drop.y 是相对于接收拖拽的 Item 的坐标
                                //var dropGlobalPos = mapToGlobal(dropLocalPos)  // 转换为全局坐标
                                item.visible = false
                                var tmpDragX = dropGlobalPos.x
                                //var dropGlobalPos = mapToGlobal(dropLocalPos)  // 转换为全局坐标                               
                                if(globalSettings.useAutoLayout()){
                                    if(dropIndicatorvisible){
                                        if(count>=1){
                                            gridpos = platformWindow.findNextEmptyGridPos(gridpos)
                                        }else{
                                            gridpos = platformWindow.getGridPosFromMousePos(dropGlobalPos)
                                        }
                                    }else{
                                        gridpos = platformWindow.findNextEmptyGridPos(Qt.point(0, 0))
                                    }
                                }else{
                                    gridpos = posresult[item.uri]
                                }
                                item.visible = false
                                var tmpDragX = dropGlobalPos.x
                                item.dragx = /*LayoutMirroring.enabled? (iconContainerWidth - tmpDragX - desktopIconView.gridWidth + offx*2): */tmpDragX
                                item.dragy=  dropGlobalPos.y//item.pos.y*desktopIconView.gridHeight + offy
                                var gridposx = desktopIconView.transtogridposx(gridpos.x*desktopIconView.gridWidth,desktopIconView.gridWidth)
                                var gridposy = desktopIconView.transtogridposy(gridpos.y*desktopIconView.gridHeight,desktopIconView.gridHeight)
                                item.dragtox =  gridposx
                                item.dragtoy = gridposy
                                console.log("anim end pos", item.dragtox, item.dragtoy)
                                item.animItem.start()
                                item.animyItem.start()
                                item.visible = true
                                count = count + 1
                            }
                        }
                        window.isDraging = false
                        return
                    }
                    console.log("其他应用拖拽", uris, desktopIconView.count)
                    dropIndicator.visible = false
                    var peonyIsSearch = drop.getDataAsString("peony-qt/is-search")
                    if (dropIndicatorvisible) {
                        windowManager.setQAppProperty("isDragFromOtherView",true)
                        windowManager.setQAppProperty("dragInsertIndex",index)
                        // desktopIconView.model.updateIconPositionsByOtherView(uris, drop.proposedAction, index,onItem);
                    }
                    desktopIconView.model.addFile(uris, drop.proposedAction, index, peonyIsSearch);
                    return;
                }

                // 2. 计算热区点的全局坐标（delegate初始位置 + 热区偏移）
                var hotspotGlobalX = drop.source.delegateGlobalPos.x + drop.source.pressPos1.x
                var hotspotGlobalY = drop.source.delegateGlobalPos.y + drop.source.pressPos1.y


                // 3. 计算释放点相对于热区的偏移（网格单位）
                var viewlocalePos = Qt.point(drop.x - iconContainer.anchors.leftMargin, drop.y - iconContainer.anchors.topMargin);
                var localeXInLayout = LayoutMirroring.enabled? iconContainer.width - viewlocalePos.x : viewlocalePos.x
                var sourceXInLayout  = hotspotGlobalX
                var iconContainerWidth = iconContainer.width;
                if (LayoutMirroring.enabled) {
                    iconContainerWidth = drop.getDataAsString("peony-qt/iconContainerWidth");
                    if (iconContainerWidth) {
                        iconContainerWidth = parseInt(iconContainerWidth);
                        sourceXInLayout  = iconContainerWidth - hotspotGlobalX
                    }
                }
                var offGridX = Math.floor(localeXInLayout / desktopIconView.gridWidth) - Math.floor(sourceXInLayout / desktopIconView.gridWidth);
                var offGridY = Math.floor(viewlocalePos.y / desktopIconView.gridHeight) - Math.floor(hotspotGlobalY / desktopIconView.gridHeight);
                var offx = viewlocalePos.x - hotspotGlobalX;
                var offy = viewlocalePos.y - hotspotGlobalY;

                console.log("drop parameter", drop.x, drop.y, localeXInLayout,sourceXInLayout, offGridX, offGridY,drop.source.delegateGlobalPos,drop.source.pressPos1)
                console.log("drop parameter1", dragIconView.Drag.hotSpot.x,dragIconView.Drag.hotSpot.y, viewlocalePos,hotspotGlobalX, hotspotGlobalY)

                var dragSource = drag.source
                if (drag.source && drag.source.isFromDesktopView) {
                    var onItem =  globalSettings.useAutoLayout() && -1 !== index? !dropIndicator.visible : -1 !== index
                    dropIndicator.visible = false
                    // drop事件从c++更新图标网格时，需要屏蔽x和y默认的behavior，使用拖拽动效
                    window.isDraging = true
                    desktopIconView.model.updateIconPositions(uris, drop.proposedAction, index, Qt.point(offGridX, offGridY), onItem);
                    window.isDraging = false
                }

                var posString = drop.getDataAsString("peony-qt/pos-list");

                var points = posString.split(" ")
                .map(pointStr => {
                         var xy = pointStr.split(",");
                         return xy.length === 2 ?
                             Qt.point(parseInt(xy[0]), parseInt(xy[1])) :
                             null;
                     })
                .filter(point => point !== null);

                points.forEach((pt, index) => {
                                   console.log(`Item ${index}: (${pt.x}, ${pt.y})`);
                               });

                for (var i = 0; i < desktopIconView.count; i++) {
                    const item = desktopIconView.itemAt(i)
                    if (!(item && item.visible)) {
                        continue;
                    }
                    item.dragHovered = false;
                    if (uris.includes(item.uri)) {
                        var j = uris.indexOf(item.uri)
                        if (j === -1) {
                            continue
                        }
                        item.visible = false
                        var tmpDragX = points[j].x*desktopIconView.gridWidth + offx
                        item.dragx = LayoutMirroring.enabled? (iconContainerWidth - tmpDragX - desktopIconView.gridWidth + offx*2): tmpDragX
                        item.dragy= points[j].y*desktopIconView.gridHeight + offy
                        console.log("anim start pos", j, item.dragx, item.dragy, offx, offy, item.x, item.y)
                        var gridposx = desktopIconView.transtogridposx(item.pos.x*desktopIconView.gridWidth,desktopIconView.gridWidth)
                        var gridposy = desktopIconView.transtogridposy(item.pos.y*desktopIconView.gridHeight,desktopIconView.gridHeight)
                        item.dragtox = LayoutMirroring.enabled? (iconContainer.width - gridposx - desktopIconView.gridWidth): gridposx
                        item.dragtoy = gridposy
                        console.log("anim end pos", item.dragtox, item.dragtoy)
                        item.animItem.start()
                        item.animyItem.start()
                        item.visible = true
                    }
                }
                drop.source.pressPos1.x = 0
                drop.source.pressPos1.y = 0
                selectionModel.clearSelection();
            }
        }

        Connections {
            target: myProxyModel
            function onViewUpdated() {
                modelViewReady = true
                // PAG 模式下等首帧就绪后再触发；非 PAG 模式直接触发
                if (!initDesktop && (!backgroundItem.isPagWallpaper || backgroundItem.pagFirstFrameReady))
                    initDesktop = true
                platformWindow.updateDesktopWidgetsUsedRegion()
            }
        }

        // PAG 首帧就绪时，若图标数据已加载完成则立即触发入场动效
        Connections {
            target: backgroundItem
            function onPagFirstFrameReadyChanged() {
                if (backgroundItem.pagFirstFrameReady && modelViewReady && !initDesktop)
                    initDesktop = true
            }
        }

        Connections{
            target: myProxyModel
            function onLayoutUpdate() {
                console.log("model layout updated")
                layoutchange()
            }
        }

        Connections{
            target: controller
            function onClickedScreenIdChanged(screenId) {
                if (screenId !== desktopIconView.currentScreentId) {
                    selectionModel.clearSelection();
                }
                console.log("Click on screen", screenId);
            }
            function onEdit (uri) {
                for (var i = 0; i < desktopIconView.count; i++) {
                    var item = desktopIconView.itemAt(i);
                    if (item && item.uri === uri && item.delegateText) {
                        console.log("Editing item at index:", i);
                        desktopIconView.currentEditIndex = i;
                        item.delegateText.selectedUris = selectionModel.getSelections();
                        if (!item.delegateText.selectedUris.includes(uri)) {
                            item.delegateText.selectedUris.push(uri)
                        }
                        item.delegateText.forceActiveFocus();
                    }
                }
            }
            function onChangeOpacity(opacity) {
                for (var i = 0; i < desktopIconView.count; i++) {
                    var item = desktopIconView.itemAt(i);
                    if (item) {
                        if (item.selected) {
                            item.opacity = opacity
                        } else {
                            item.opacity = 1
                        }
                    }
                }
            }

            function onSelectedUrisChanged (uriList) {
                for (var i = 0; i < desktopIconView.count; i++) {
                    var item = desktopIconView.itemAt(i);
                    if (item) {
                        if (uriList.includes(item.uri)) {
                            selectionModel.updateSelection(i, true)
                        } else {
                            selectionModel.updateSelection(i, false)
                        }
                    }
                }
            }
        }
        function selectFirstItemStartingWith(prefix) {
            for (var i = 0; i < desktopIconView.count; i++) {
                var item = desktopIconView.itemAt(i);
                if (item && item.display && item.display.toLowerCase().startsWith(prefix)) {
                    selectionModel.selectCurrent(i); // 选中该项
                    return;
                }
            }
        }
        Keys.onPressed: (event) => {
                            var itemIndex
                            switch(event.key) {
                                case Qt.Key_Home:
                                selectionModel.selectHome();
                                event.accepted = true;
                                break;
                                case Qt.Key_End:
                                selectionModel.selectEnd();
                                event.accepted = true;
                                break;
                                case Qt.Key_Up:
                                selectionModel.selectPrevious();
                                event.accepted = true;
                                break;
                                case Qt.Key_Down:
                                selectionModel.selectNext();
                                event.accepted = true;
                                break;
                                case Qt.Key_Left:
                                selectionModel.selectLeft();
                                event.accepted = true;
                                break;
                                case Qt.Key_Right:
                                selectionModel.selectRight();
                                event.accepted = true;
                                break;
                                case Qt.Key_Enter:
                                case Qt.Key_Return:
                                var uris = selectionModel.getSelections()
                                controller.openFileByUri(uris);
                                event.accepted = true;
                                break;
                                default:
                                if (event.text.length > 0) {
                                        var letter = event.text; // 获取输入的字母
                                        searchBuffer += letter.toLowerCase();
                                        searchResetTimer.start(); // 启动计时器，超时后清空 buffer
                                        selectFirstItemStartingWith(searchBuffer);
                                        event.accepted = true;
                                }
                            }
                        }
    }
}
