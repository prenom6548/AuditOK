import QtQuick 2.15
import QtQuick.Controls 2.15
import org.ukui.peony.desktop 1.0
import org.ukui.quick.items 1.0 as UKUIQuickItems
import org.ukui.quick.platform 1.0 as UKUIQuickPlatform
import org.ukui.quick.widgets 1.0
import org.ukui.quick.container 1.0
import QtGraphicalEffects 1.15

Repeater {
    id: widgetsLayout

    property Item lastUnderLayerItem: null
    property Item lastIconLayerItem: null
    property Item lastAboveLayerItem: null
    property int gridWidth: 80
    property Rectangle dragIndicator: null
    anchors.fill: parent
    anchors.bottomMargin: 10 //需要抵消icon container的margin
    property var currentActiveFocusinstanceId: -1

    // 判断指定 Item 中的坐标是否落在任意桌面插件上。
    // 使用 mapFromItem 而不是直接比较 x/y，以兼容布局的镜像 Transform。
    function containsWidgetAt(sourceItem, sourceX, sourceY) {
        for (var i = count - 1; i >= 0; --i) {
            var item = itemAt(i)
            if (!item || !item.visible || item.width <= 0 || item.height <= 0)
                continue

            var localPos = item.mapFromItem(sourceItem, sourceX, sourceY)
            if (localPos.x >= 0 && localPos.x < item.width
                    && localPos.y >= 0 && localPos.y < item.height) {
                return true
            }
        }
        return false
    }

    /*!
     * 跨屏落点后：将新插件从松手位置收敛动画到 indicator 吸附位
     */
    function snapDroppedWidgetToIndicator(instanceId, fromX, fromY, toX, toY) {
        function trySnap() {
            for (var i = 0; i < count; i++) {
                var item = itemAt(i)
                if (item && item.instanceid === instanceId) {
                    item.snapFromTo(fromX, fromY, toX, toY)
                    return
                }
            }
            Qt.callLater(trySnap)
        }
        trySnap()
    }

    delegate: FocusScope {
        id: desktopWidget
        focus: true
        readonly property var widgetsLayoutRef: widgetsLayout
        property Item widgetItem: model.widgetItem
        property bool itemReady: false
        property point widgetPos:  model.position
        property bool layoutWithGrid: model.sizePolicy
        property size widgetSize: model.size
        property bool dragable: model.dragable
        property bool hasAnchorsConfig: model.hasAnchors
        property var anchrosList: hasAnchorsConfig? model.anchors: undefined
        property int index: model.index
        property int instanceid: model.instanceId
        // 插件 QML 无 engine 根上下文里的 platformWindow；将 Island/PlatformWindow 传给子级 WidgetItem（如时钟右键删除）
        readonly property var peonyShell: typeof island !== "undefined" ? island : null
        transform: Translate {
            x: widgetsLayout.width > 0? (widgetsLayout.width - desktopWidget.width - desktopWidget.x*2): 0  //镜像放置x1 + width + x2 = parent width, 偏移量为x2 - x1
            y: 0
        }
        onActiveFocusChanged: {
            if (activeFocus) {
                widgetsLayout.currentActiveFocusinstanceId = instanceid
            }
        }
        onWidgetItemChanged: {
            if (widgetItem) {
                widgetItem.parent = desktopWidget
            }
        }

        property bool crossScreenDragStarted: false
        property bool crossScreenDragPending: false
        property bool crossScreenSystemDragActive: false

        // 离屏样式副本：含装饰背景与插件内容，不含阴影（阴影会在 RGBA 纹理四角留下直角不透明像素）
        Item {
            id: dragStyleReplica
            visible: false
            width: desktopWidget.width
            height: desktopWidget.height

            UKUIQuickItems.DtThemeBackground {
                z: 0
                anchors.fill: parent
                visible: model.doDecoration && parent.width > 50
                radius: UKUIQuickPlatform.GlobalTheme.kRadiusWindow
                useStyleTransparency: false
                alpha: 0.36
            }

            ShaderEffectSource {
                id: pluginLayerCapture
                z: 1
                sourceItem: desktopWidget.widgetItem
                width: desktopWidget.widgetItem ? desktopWidget.widgetItem.width : 0
                height: desktopWidget.widgetItem ? desktopWidget.widgetItem.height : 0
                x: desktopWidget.widgetItem ? (parent.width - width) / 2 : 0
                y: desktopWidget.widgetItem ? (parent.height - height) / 2 : 0
                live: false
                hideSource: false
                visible: false
                format: ShaderEffectSource.RGBA
            }
        }

        // 捕获样式副本，依赖源纹理 alpha 与 OpenGL clear 实现透明背景
        ShaderEffectSource {
            id: widgetItemTexture
            sourceItem: dragStyleReplica
            live: false
            hideSource: false
            visible: false
            sourceRect: Qt.rect(0, 0, desktopWidget.width, desktopWidget.height)
            anchors.fill: parent
            format: ShaderEffectSource.RGBA
        }

        Timer {
            id: dragHideTimer
            interval: 32   // 可按需调整，原逻辑约 100ms 可改为 interval: 100
            repeat: false
            property var pendingGlobalPos: null
            onTriggered: {
                desktopWidget.visible = false
                widgetItemTexture.visible = false
                dragStyleReplica.visible = false
                pluginLayerCapture.visible = false
            }
        }
        function triggerCrossScreenDrag(globalPos) {
            if (crossScreenDragPending || !desktopWidget.widgetItem) {
                return
            }
            crossScreenDragPending = true
            crossScreenDragStarted = true
            island.setCurrentDragMoveItem(desktopWidget.widgetItem)

            // 计算源屏幕 indicator 贴边位置，用于跨屏拖拽取消时复位
            var indicatorX = desktopWidget.x
            var indicatorY = desktopWidget.y
            if (desktopWidget.x < 0) {
                indicatorX = 0
            } else if (desktopWidget.x + desktopWidget.width > widgetsLayout.width) {
                indicatorX = widgetsLayout.width - desktopWidget.width
            }
            if (desktopWidget.y < 0) {
                indicatorY = 0
            } else if (desktopWidget.y + desktopWidget.height > widgetsLayout.height) {
                indicatorY = widgetsLayout.height - desktopWidget.height
            }
            dragIndicator.x = indicatorX
            dragIndicator.y = indicatorY
            dragIndicator.width = desktopWidget.width
            dragIndicator.height = desktopWidget.height
            dragIndicator.visible = true

            pluginLayerCapture.scheduleUpdate()
            Qt.callLater(function() {
                dragStyleReplica.visible = true
                pluginLayerCapture.visible = true
                widgetItemTexture.visible = true
                widgetItemTexture.scheduleUpdate()
                Qt.callLater(function() {
                    var dragWindow = windowManager.createDragPreviewWithTexture(
                        widgetItemTexture, globalPos,
                        desktopWidget.width, desktopWidget.height)
                    dragHideTimer.start()
                    crossScreenSystemDragActive = true
                    var movedToOtherScreen = island.startSystemDrag(dragWindow, globalPos,
                        desktopWidget.instanceid, desktopIconView.currentScreentId,
                        desktopWidget.width, desktopWidget.height,
                        indicatorX, indicatorY,
                        desktopWidget.layoutWithGrid,
                        island.widgetItemGridUnitSize(desktopWidget.widgetItem))
                    crossScreenSystemDragActive = false
                    crossScreenDragPending = false
                    crossScreenDragStarted = false
                    // 未成功跨屏移动时，在 QML 侧恢复显示并复位到 indicator 位置
                    if (!movedToOtherScreen) {
                        desktopWidget.visible = true
                        desktopWidget.x = indicatorX
                        desktopWidget.y = indicatorY
                        dragIndicator.visible = false
                        island.setCurrentDragMoveItem(null)
                        island.clearWidgetExceptedPos(desktopWidget.widgetItem)
                        widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
                    }
                })
            })
        }

        Connections {
            target: controller
            function onGridSizeChanged() {
                console.log("grid size changed, update item size")
                widgetsProxyModel.initWithConfig(widgetItem)
            }
        }

        Connections {
            target: DesktopOrganizationManager
            function onExpandStatusChangedRequest(id,expand) {
                if(instanceid == id && expand){
                    var isintersected = island.checkItemIntersected(desktopWidget);
                    var height = desktopWidget.height
                    //island.updateWidgetSizetoForIntersected(desktopWidget,desktopWidget.widgetItem)
                    if(isintersected || height == 0){
                        island.updateWidgetSizetoForIntersected(desktopWidget,desktopWidget.widgetItem)
                    }
                    //DesktopOrganizationManager.startInitAnima(id)
                    console.log("grid size changed, update item size")
                }
            }
        }

        // x: hasAnchorsConfig? 999: (layoutWithGrid? widgetPos.x * widgetsLayout.gridWidth: widgetPos.x)
        // y: hasAnchorsConfig? 999: (layoutWithGrid? widgetPos.y * widgetsLayout.gridHeight: widgetPos.y)
        z: model.layer === 0? 0: (model.layer > 0? 999: -999)
        // width: layoutWithGrid? widgetSize.width * desktopIconView.gridWidth: widgetSize.width
        // height: layoutWithGrid? widgetSize.height * desktopIconView.gridHeight: widgetSize.height
        // width: itemReady? widgetItem.childrenRect.width: 0
        // height: itemReady? widgetItem.childrenRect.height: 0
        width: itemReady? widgetItem.width: 0
        height: itemReady? widgetItem.height: 0

        anchors.left: hasAnchorsConfig? getDesktopAnchor(anchrosList[0]): undefined
        anchors.horizontalCenter: hasAnchorsConfig? getDesktopAnchor(anchrosList[1]): undefined
        anchors.right: hasAnchorsConfig? getDesktopAnchor(anchrosList[2]): undefined
        anchors.top: hasAnchorsConfig? getDesktopAnchor(anchrosList[3]): undefined
        anchors.verticalCenter: hasAnchorsConfig? getDesktopAnchor(anchrosList[4]): undefined
        anchors.bottom: hasAnchorsConfig? getDesktopAnchor(anchrosList[5]): undefined
        // anchors.top: hasAnchorsConfig? widgetsLayout.top: undefined
        // anchors.right: hasAnchorsConfig? widgetsLayout.right: undefined

        NumberAnimation {
            id: xAnimator
            target: desktopWidget
            property: "x"
            duration: 150
            onFinished: widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
        }

        NumberAnimation {
            id: yAnimator
            target: desktopWidget
            property: "y"
            duration: 150
            onFinished: widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
        }

        NumberAnimation {
            id: widthAnimator
            target: desktopWidget
            property: "width"
            duration: 150
            onFinished: widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
        }

        NumberAnimation {
            id: heightAnimator
            target: desktopWidget
            property: "height"
            duration: 150
            onFinished: widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
        }

        function snapFromTo(fromX, fromY, toX, toY) {
            desktopWidget.x = toX
            desktopWidget.y = toY
            xAnimator.from = fromX
            xAnimator.to = toX
            yAnimator.from = fromY
            yAnimator.to = toY
            xAnimator.start()
            yAnimator.start()
        }

        function getDesktopAnchor(desktopAnchor) {
            console.log(desktopWidget.anchors)
            console.log("getDesktopAnchor", anchrosList);
            switch (desktopAnchor) {
            case 1: {
                return widgetsLayout.left;
            }
            case 2: {
                return widgetsLayout.horizontalCenter;
            }
            case 3: {
                return widgetsLayout.right;
            }
            case 4: {
                return widgetsLayout.top;
            }
            case 5: {
                return widgetsLayout.verticalCenter;
            }
            case 6: {
                return widgetsLayout.bottom;
            }
            default: {
                return undefined;
            }
            }
        }

        // fixme: 动效影响拖拽
        // Behavior on x {
        //     enabled: layoutWithGrid && !resizeHandler.resizing && !resizeHandler.pressed && !widgetDragArea.containsPress
        //     id: widgetXTransBehavior
        //     NumberAnimation {
        //         easing.type: Easing.Bezier;
        //         easing.bezierCurve: [0.25, 0.1, 0.38, 1];
        //         duration: 180
        //     }
        // }
        // Behavior on y {
        //     enabled: layoutWithGrid && !resizeHandler.resizing && !resizeHandler.pressed && !widgetDragArea.containsPress
        //     id: widgetYTransBehavior
        //     NumberAnimation {
        //         easing.type: Easing.Bezier;
        //         easing.bezierCurve: [0.25, 0.1, 0.38, 1];
        //         duration: 180
        //     }
        // }

        RotationAnimator on rotation {
            id: widgetRotaionAnimator
            target: desktopWidget.widgetItem
            from: controller.dragDesktopWidgets? Math.atan(-5/desktopWidget.width) * 180/3.14: 0
            to: controller.dragDesktopWidgets? Math.atan(5/desktopWidget.width) * 180/3.14: 0
            duration: 180
            alwaysRunToEnd: true
            loops: Animation.Infinite
            running: controller.dragDesktopWidgets

            onStopped: {
                desktopWidget.widgetItem.rotation = 0
            }
        }

        Item {
            id: animationContainer
            anchors.fill: parent

            RotationAnimator on rotation {
                id: rotaionAnimator
                target: animationContainer
                from: controller.dragDesktopWidgets? Math.atan(-5/desktopWidget.width) * 180/3.14: 0
                to: controller.dragDesktopWidgets? Math.atan(5/desktopWidget.width) * 180/3.14: 0
                duration: 180
                alwaysRunToEnd: true
                loops: Animation.Infinite
                running: controller.dragDesktopWidgets

                onStopped: {
                    animationContainer.rotation = 0
                }
            }

            Item {
                id: blurBackgroundItem
                visible: model.doBlur
                anchors.fill: parent
                ShaderEffectSource {
                    id: blurSource
                    sourceItem: backgroundItem// fixme 对于layer为1的widget模糊效果需要包含图标？
                    anchors.fill: parent
                    visible: false
                    sourceRect: Qt.rect(widgetsLayout.width - desktopWidget.x - desktopWidget.width + 8, desktopWidget.y, desktopWidget.width, desktopWidget.height)
                }

                MaskedBlur {
                    id: maskedblur
                    visible: false
                    anchors.fill: parent
                    maskSource: dtBackground.visible? dtBackground: widgetItem
                    source: blurSource
                    radius: 60
                }

                ThresholdMask {
                    id: clipedSource
                    anchors.fill: parent
                    source: maskedblur
                    maskSource: dtBackground.visible? dtBackground: widgetItem
                }
            }

            UKUIQuickItems.DtDropShadow {
                anchors.fill: parent
                visible: model.doDecoration && parent.width > 50 && parent.height>40
                shadowRadius: UKUIQuickPlatform.GlobalTheme.kRadiusWindow
                // shadowData: UKUIQuickPlatform.GlobalTheme.kShadowComponent
                shadowData: UKUIQuickPlatform.GlobalTheme.kShadowPrimaryActive
            }

            UKUIQuickItems.DtThemeBackground {
                id: dtBackground
                visible: model.doDecoration && parent.width > 50
                anchors.fill: parent
                radius: UKUIQuickPlatform.GlobalTheme.kRadiusWindow
                useStyleTransparency: false
                alpha:0.36
            }
        }

        ResizeHandler {
            id: resizeHandler
            property int lastX
            property int lastY
            property int lastWidth
            property int lastHeight
            targetItem: desktopWidget.widgetItem
            visible: model.resizable
            maxHeight: widgetsLayout.height
            maxWidth: widgetsLayout.width
            onPressedChanged: {
                if (pressed) {
                    island.updateDesktopWidgetsUsedRegionInstantly(targetItem, true)
                    island.updateDragResizeUsedRegion(targetItem)
                    // 记录当前尺寸和位置作为last rect
                    resizeHandler.lastX = desktopWidget.x
                    resizeHandler.lastY = desktopWidget.y
                    resizeHandler.lastWidth = desktopWidget.width
                    resizeHandler.lastHeight = desktopWidget.height
                    island.updateLastRect(desktopWidget, desktopWidget.widgetItem)
                }
            }
            onRequestUpdateConfig: {
                widgetsProxyModel.updateGeometryConfig(model.index, desktopWidget.widgetItem, desktopWidget)
            }
            onResizingChanged: {
                // fixme: platformwindow属性在这个作用域找不到，这里使用ukui quick的island属性，等价于platfomwindow
                console.log("resizing changed", resizing)
                if (resizing) {
                    if (island.checkItemIntersected(desktopWidget)) {
                        desktopWidget.x = resizeHandler.lastX
                        desktopWidget.y = resizeHandler.lastY
                        resizeHandler.targetItem.width = resizeHandler.lastWidth
                        resizeHandler.targetItem.height = resizeHandler.lastHeight
                    }
                } else {
                    island.updateDesktopWidgetsUsedRegion()
                    if (!desktopWidget.layoutWithGrid) {
                        if (island.checkItemIntersected(desktopWidget)) {
                            desktopWidget.x = resizeHandler.lastX
                            desktopWidget.y = resizeHandler.lastY
                            resizeHandler.targetItem.width = resizeHandler.lastWidth
                            resizeHandler.targetItem.height = resizeHandler.lastHeight
                        }
                    } else {
                        var currentX = desktopWidget.x
                        var currentY = desktopWidget.y
                        var currentWidth = desktopWidget.width
                        var currentHeight = desktopWidget.height
                        // 实施网格吸附
                        island.adjustGridLayoutWidget(desktopWidget, desktopWidget.widgetItem, true)

                        xAnimator.from = currentX
                        xAnimator.to = desktopWidget.x
                        yAnimator.from = currentY
                        yAnimator.to = desktopWidget.y
                        widthAnimator.from = currentWidth
                        widthAnimator.to = desktopWidget.width
                        heightAnimator.from = currentHeight
                        heightAnimator.to = desktopWidget.height
                        xAnimator.start()
                        yAnimator.start()
                        widthAnimator.start()
                        heightAnimator.start()
                    }
                }
                console.log("last width:", desktopWidget.width)
            }
        }

        // platformwindow属性在这个作用域找不到，这里使用ukui quick的island属性，等价于platfomwindow
        function handleSizeChanged() {
            console.log("size changed")
            if (island.checkItemIntersected(desktopWidget)) {
                // 恢复之前的位置和尺寸
                console.log("intersected")
                console.log("last rect", resizeHandler.lastX, resizeHandler.lastY, resizeHandler.lastWidth, resizeHandler.lastHeight)
                resizeHandler.intersected = true
            } else {
                // 记录当前尺寸和位置作为last rect
                resizeHandler.lastX = desktopWidget.x
                resizeHandler.lastY = desktopWidget.y
                resizeHandler.lastWidth = desktopWidget.width
                resizeHandler.lastHeight = desktopWidget.height
                resizeHandler.intersected = false
            }
        }

        Connections {
            enabled: resizeHandler.resizing
            onWidthChanged: handleSizeChanged()
            onHeightChanged: handleSizeChanged()
        }

        MouseArea {
            id: widgetDragArea
            visible: dragable
            anchors.fill: parent
            property int oldX: -1
            property int oldY: -1
            property real originalX: desktopWidget.x
            property int hotSpotX: -1
            property int hotSpotY: -1
            // property bool intersected: false

            z: controller.dragDesktopWidgets? 19999: 0

            drag.target: parent
            // 配置拖拽参数，fixme: 如何处理拖拽和其它事件冲突？长按触发拖拽？
            drag.axis: Drag.XAndYAxis  // 可在X和Y轴拖拽
            // 限定拖拽范围，不得相交和超出屏幕
            drag.minimumX: 0 /*intersected? Math.max(0, oldX - 4): 0*/
            drag.minimumY: 0 /*intersected? Math.max(0, oldY - 4): 0*/
            drag.maximumX: Math.max(0, widgetsLayout.width - desktopWidget.width) /*intersected? Math.min(oldX + 4, widgetsLayout.width - desktopWidget.width): Math.max(0, widgetsLayout.width - desktopWidget.width)*/
            drag.maximumY: Math.max(0, widgetsLayout.height - desktopWidget.height)/*intersected? Math.min(oldY + 4, widgetsLayout.height - desktopWidget.height): Math.max(0, widgetsLayout.height - desktopWidget.height)*/

            onPositionChanged: {
                if (drag.active) {
                    // console.log("mouse x,y:", mouse.x, mouse.y)
                    desktopWidget.x = desktopWidget.x - mouse.x + hotSpotX

                    // 计算可放置区域
                    if (island.checkItemIntersected(desktopWidget)) {
                        // console.log("item intersected", desktopWidget, oldX, oldY)
                        dragIndicator.visible = true
                        // dragIndicator.x = desktopWidget.x
                        // dragIndicator.y = desktopWidget.y
                        dragIndicator.width = desktopWidget.width
                        dragIndicator.height = desktopWidget.height

                        // todo: 网格需要吸附

                        // 处理相交，自适应到合适位置
                        if (!island.adjustIntersectedItem(desktopWidget, dragIndicator, widgetsLayout.width, desktopWidget.layoutWithGrid)) {
                            // 未找到合适位置，还原到之前位置
                            // console.log("set widget pos", oldX, oldY)
                            dragIndicator.x = oldX
                            dragIndicator.y = oldY
                        } else {
                            // console.log("find suit rect", dragIndicator.x, dragIndicator.y)
                            // 找到合适位置，记录处理成功的位置
                            oldX = dragIndicator.x
                            oldY = dragIndicator.y
                        }
                    } else {
                        // 先判断是否超出屏幕
                        if (desktopWidget.x < 0 || desktopWidget.y < 0 || desktopWidget.x + desktopWidget.width > widgetsLayout.width || desktopWidget.y + desktopWidget.height > widgetsLayout.height) {
                            dragIndicator.visible = true

                            // 仅当拖拽方向朝向扩展屏邻接区域时才启动跨屏预览
                            if (windowManager.isInExtendMode()) {
                                var widgetCenter = desktopWidget.mapToGlobal(Qt.point(desktopWidget.width / 2, desktopWidget.height / 2))
                                var globalPos = Qt.point(widgetCenter.x - desktopWidget.width / 2, widgetCenter.y - desktopWidget.height / 2)
                                if (island.isDraggingToOtherScreen(globalPos, desktopWidget.width, desktopWidget.height)) {
                                    triggerCrossScreenDrag(globalPos)
                                }
                            }
                        } else {
                            dragIndicator.visible = false
                            oldX = desktopWidget.x
                            oldY = desktopWidget.y
                        }

                        // intersected = false

                        if (desktopWidget.layoutWithGrid) {
                            // 处理网格吸附，如果满足吸附条件，将indicator置于期望的位置
                            if (!island.adjustGridLayoutWidget(desktopWidget, desktopWidget.widgetItem, false, dragIndicator, widgetsProxyModel, widgetsLayout.width)) {
                                // 干扰项把吸附候选区域推出屏幕或推到其他插件上时，
                                // 保留本次拖拽已记录的最后合法释放位置。
                                dragIndicator.x = oldX
                                dragIndicator.y = oldY
                                dragIndicator.width = desktopWidget.width
                                dragIndicator.height = desktopWidget.height
                                dragIndicator.visible = oldX >= 0 && oldY >= 0
                            } else if (dragIndicator.visible) {
                                oldX = dragIndicator.x
                                oldY = dragIndicator.y
                            }
                        } else if (dragIndicator.visible) {
                            // 处理自由布局插件超出屏幕边缘场景
                            dragIndicator.x = desktopWidget.x
                            dragIndicator.y = desktopWidget.y

                            if (desktopWidget.x < 0) {
                                dragIndicator.x = 0
                            } else if (desktopWidget.x + desktopWidget.width > widgetsLayout.width) {
                                dragIndicator.x = widgetsLayout.width - desktopWidget.width
                            }

                            if (desktopWidget.y < 0) {
                                dragIndicator.y = 0
                            } else if (desktopWidget.y + desktopWidget.height > widgetsLayout.height) {
                                dragIndicator.y = widgetsLayout.height - desktopWidget.height
                            }
                        }
                    }
                } else {
                    dragIndicator.visible = false
                }
            }

            drag.onActiveChanged: {
                // intersected = false
                if (!drag.active) {
                    var currentX = desktopWidget.x
                    var currentY = desktopWidget.y

                    // 纹理尚未就绪、系统拖拽未启动时松手，恢复显示
                    if (crossScreenDragPending && !crossScreenSystemDragActive) {
                        crossScreenDragPending = false
                        crossScreenDragStarted = false
                        desktopWidget.visible = true
                        windowManager.closeDragPreviewWithTexture()
                    }

                    island.setCurrentDragMoveItem(null)
                    if (dragIndicator && dragIndicator.visible) {
                        oldX = dragIndicator.x
                        oldY = dragIndicator.y
                        dragIndicator.visible = false
                        desktopWidget.x = oldX
                        desktopWidget.y = oldY
                    }

                    island.clearWidgetExceptedPos(desktopWidget.widgetItem)

                    xAnimator.from = currentX
                    xAnimator.to = desktopWidget.x
                    yAnimator.from = currentY
                    yAnimator.to = desktopWidget.y
                    xAnimator.start()
                    yAnimator.start()

                    // 动效结束后触发保存配置
                    //widgetsProxyModel.updateGeometryConfig(desktopWidget.index, desktopWidget.widgetItem, desktopWidget)
                } else {
                    island.setCurrentDragMoveItem(desktopWidget.widgetItem)
                }
            }

            onPressed: {
                desktopWidget.forceActiveFocus()
                island.updateDragResizeUsedRegion(desktopWidget.widgetItem)
                oldX = desktopWidget.x
                oldY = desktopWidget.y
                hotSpotX = mouse.x
                hotSpotY = mouse.y

                console.log("pressed x,y:", oldX, oldY, "\nhotspot x,y:", hotSpotX, hotSpotY);
            }
            onDoubleClicked: {
                mouse.accepted = false
                var instanceId = model.instanceId
                var x = mouseX
                DesktopOrganizationManager.renameFocusRequest(instanceId,x)
                console.log("pressed");
            }
        }

        MouseArea {
            id: widgetStacker
            anchors.fill: parent
            z: 9999
            propagateComposedEvents: true
            onPressed: {
                desktopWidget.forceActiveFocus()
                console.log("点击到item", desktopWidget);
                if (desktopWidget.z === 0) {
                    if (widgetsLayout.lastIconLayerItem
                            && widgetsLayout.lastIconLayerItem !== desktopWidget) {
                        // 将新item至于last item的顶部
                        widgetsProxyModel.doStackAfter(desktopWidget, widgetsLayout.lastIconLayerItem)
                    }
                    // 将此置顶的窗口设置为last items
                    widgetsLayout.lastIconLayerItem = desktopWidget
                } else if (desktopWidget.z > 0) {
                    if (widgetsLayout.lastAboveLayerItem
                            && widgetsLayout.lastAboveLayerItem !== desktopWidget) {
                        // 将新item至于last item的顶部
                        widgetsProxyModel.doStackAfter(desktopWidget, widgetsLayout.lastAboveLayerItem)
                    }
                    // 将此置顶的窗口设置为last items
                    widgetsLayout.lastAboveLayerItem = desktopWidget
                } else {
                    if (widgetsLayout.lastUnderLayerItem
                            && widgetsLayout.lastUnderLayerItem !== desktopWidget) {
                        // 将新item至于last item的顶部
                        widgetsProxyModel.doStackAfter(desktopWidget, widgetsLayout.lastUnderLayerItem)
                    }
                    // 将此置顶的窗口设置为last items
                    widgetsLayout.lastUnderLayerItem = desktopWidget
                }

                mouse.accepted = false
            }
        }

        onXChanged: {
            island.updateDesktopWidgetsUsedRegion();
        }
        onYChanged: {
            island.updateDesktopWidgetsUsedRegion();
        }
        onWidthChanged: {
            island.updateDesktopWidgetsUsedRegion();
        }
        onHeightChanged: {
            island.updateDesktopWidgetsUsedRegion();
        }
    }

    onItemAdded: {
        console.log("item added:", index, item, "rect:", item.x, item.y, item.width, item.height, item.z);
        if (item.z === 0) {
            if (lastIconLayerItem && lastIconLayerItem !== item) {
                widgetsProxyModel.doStackAfter(item, lastIconLayerItem)
            }
            lastIconLayerItem = item
            console.log("lastIconLayerItem", lastIconLayerItem)
        } else if (item.z > 0) {
            if (lastAboveLayerItem && lastAboveLayerItem !== item) {
                widgetsProxyModel.doStackAfter(item, lastAboveLayerItem)
            }
            lastAboveLayerItem = item
            console.log("lastAboveLayerItem", lastAboveLayerItem)
        } else {
            if (lastUnderLayerItem && lastUnderLayerItem !== item) {
                widgetsProxyModel.doStackAfter(item, lastUnderLayerItem)
            }
            lastUnderLayerItem = item
            console.log("lastUnderLayerItem", lastUnderLayerItem)
        }

        // 检查item位置是否被初始化
        if (!widgetsProxyModel.hasPositionConfig(item.widgetItem)) {
            console.log("新加入的插件，进行缺省布局");
            island.layoutAddedWidgetItem(item, item.widgetItem, widgetsProxyModel)
        } else {
            // 从配置中读取大小
            widgetsProxyModel.initWithConfig(item.widgetItem, item)
            // 检测是否和现有插件相交，如果相交则重新排样
            if (!island.isPlatformWindowHasRootItem()) {
                 // 初始化流程，platformwindow的rootitem还未就绪，此时需要在qml端传入widgets proxy model实例用于更新widgets占用区域
                   // 另外由于初始化时widgetsLayout的尺寸还没有就绪，此时结合translate效果map到widgetslayout的desktopwidget会有问题，
                    // 这里的处理是在widgetslayout的width为0时，translate不使用镜像效果，这里后续可能需要简化相关逻辑，避免长链条影响
                   island.updateDragResizeUsedRegion(item.widgetItem, widgetsProxyModel)
                    island.updateDesktopWidgetsUsedRegionInstantly(item.widgetItem, true, widgetsProxyModel)
                } else {
                    island.updateDragResizeUsedRegion(item.widgetItem)
                    island.updateDesktopWidgetsUsedRegionInstantly(item.widgetItem, true)
            }
            if (island.checkItemIntersected(item, item.widgetItem)) {
                // 重排之前记录期望的位置
                // fixme: 区分屏幕合并和恢复，可能需要借助oldScreenId配置来判断
                console.log("item intersected:", item.widgetItem.Widget.instanceId, item.x, item.y, item.widgetItem.width, item.widgetItem.height)
                island.recoredCurrentPositionAndSize(item, item.widgetItem)
                island.layoutAddedWidgetItem(item, item.widgetItem, widgetsProxyModel, true)
            }
        }

        island.updateDesktopWidgetsUsedRegion();

        item.itemReady = true
    }

    onItemRemoved: {
        island.updateDesktopWidgetsUsedRegion();
    }
}
