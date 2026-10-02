/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KSearchLineEdit.
 * 搜索图标、自定义 placeholder 动画、清除键、智能语义按钮、补全历史、圆角描边。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate
import org.ukui.platformtheme.controls 1.0 as Private

T.TextField {
    id: control

    readonly property bool _rtl: {
        var loc = Qt.locale().name
        return loc === "ug_CN" || loc === "kk_KZ" || loc === "ky_KG"
    }
    readonly property int _iconSize: 16
    readonly property int _iconMargin: 6
    readonly property int _iconTextSpacing: 8
    readonly property int _toolSpacing: 4
    // 智能语义按钮内置图标（UKUI 主题）；C++ init 为 QIcon() 空图标，由应用通过 customButton()->setIcon 设置
    readonly property string _builtinCustomButtonIcon: "ukui-ai-search-symbolic"

    // 对齐 ksearchlineedit.h
    property bool clearButtonEnabled: false
    property int placeHolderAlignment: Qt.AlignCenter
    property bool translucent: false
    property bool gradationEnabled: false
    property bool customButtonVisible: false
    property alias customButton: customBtn

    property string placeHolderText: qsTr("Search")
    property string _toolTipPlaceHolder: ""

    font: app.font
    palette: app.palette
    hoverEnabled: true
    selectByMouse: true
    focusReason: Qt.MouseFocusReason
    placeholderText: ""
    color: getStartColor(!enabled ? textfield.disableTextColor : textfield.normalTextColor)
    selectionColor: getStartColor(textfield.focusBorderColor)
    selectedTextColor: palette.highlightedText
    verticalAlignment: TextInput.AlignVCenter
    horizontalAlignment: _rtl ? Text.AlignRight : Text.AlignLeft

    // 宽度由布局/显式 width 决定，不随输入文本变长（对齐 C++ QLineEdit::sizeHint）
    implicitWidth: textfield.normalWidth
    implicitHeight: Math.max(textfield.normalHeight,
                             contentHeight + topPadding + bottomPadding, 36)

    // 搜索图标与文本/输入区间距 8px（_iconTextSpacing）
    leftPadding: _rtl ? _rightChromeWidth : (_iconMargin + _iconSize + _iconTextSpacing)
    rightPadding: _rtl ? (_iconMargin + _iconSize + _iconTextSpacing) : _rightChromeWidth
    topPadding: textfield.upDownMargin
    bottomPadding: textfield.upDownMargin

    readonly property bool _atSide: length > 0 || activeFocus

    // placeholder 可用宽度：扣除左侧 icon 区与右侧 chrome（custom/clear）
    readonly property int _placeHolderLeadingInset: _rtl ? _rightChromeWidth : _iconMargin
    readonly property int _placeHolderTrailingInset: _rtl ? _iconMargin : _rightChromeWidth
    readonly property int _placeHolderAreaWidth: Math.max(_iconSize,
        width - _placeHolderLeadingInset - _placeHolderTrailingInset)
    readonly property int _placeHolderLabelMaxWidth: Math.max(0,
        _placeHolderAreaWidth - _iconTextSpacing - _iconSize)

    readonly property bool _placeHolderNeedsElide: Math.ceil(placeHolderMetrics.width)
        > _placeHolderLabelMaxWidth

    readonly property int _placeHolderTextWidth: _placeHolderNeedsElide
        ? Math.ceil(placeHolderLabel.paintedWidth)
        : Math.ceil(placeHolderMetrics.width)

    readonly property int _placeHolderRowNaturalWidth: length > 0
        ? _iconSize
        : (_iconSize + _iconTextSpacing + Math.max(1, _placeHolderTextWidth))

    readonly property int _placeHolderContentWidth: Math.min(
        _placeHolderRowNaturalWidth, _placeHolderAreaWidth)

    // C++：widget 高度 = max(icon, textLabel->height())
    readonly property int _placeHolderRowHeight: length > 0
        ? _iconSize
        : Math.max(_iconSize, Math.ceil(placeHolderMetrics.height))

    function _clampHolderX(x, rowW) {
        var minX = _placeHolderLeadingInset
        var maxX = Math.max(minX, width - rowW - _placeHolderTrailingInset)
        return Math.max(minX, Math.min(x, maxX))
    }

    // C++ m_sideRect：LTR moveLeft(iconMargin)；RTL moveLeft(width - w - iconMargin)
    function _holderSideX() {
        var rowW = _placeHolderContentWidth
        var x = _rtl ? (width - rowW - _iconMargin) : _iconMargin
        return _clampHolderX(x, rowW)
    }

    // C++ m_holderRect：居中/左右对齐均在扣除右侧 chrome 后的区域内计算
    function _holderIdleX() {
        var rowW = _placeHolderContentWidth
        var minX = _placeHolderLeadingInset
        var x
        if (placeHolderAlignment & Qt.AlignRight)
            x = _rtl ? minX : (width - rowW - _placeHolderTrailingInset)
        else if (placeHolderAlignment & Qt.AlignLeft)
            x = _rtl ? (width - rowW - _iconMargin) : minX
        else
            x = minX + (_placeHolderAreaWidth - rowW) / 2
        return _clampHolderX(x, rowW)
    }

    function snapPlaceHolder() {
        placeHolderRow.x = (length > 0 || activeFocus) ? _holderSideX() : _holderIdleX()
    }

    function animatePlaceHolder() {
        if (length > 0) {
            placeHolderRow.x = _holderSideX()
            return
        }
        // C++ FocusIn: holderRect -> sideRect；FocusOut: sideRect -> holderRect
        var target = activeFocus ? _holderSideX() : _holderIdleX()
        if (Math.abs(placeHolderRow.x - target) < 0.5)
            return
        placeHolderAnim.stop()
        placeHolderAnim.from = placeHolderRow.x
        placeHolderAnim.to = target
        placeHolderAnim.start()
    }

    onActiveFocusChanged: {
        animatePlaceHolder()
        updateChrome()
        if (!activeFocus)
            historyLayer.close()
        else
            refreshHistoryPopup()
    }
    onTextChanged: {
        updateChrome()
        updateIconSources()
        refreshHistoryPopup()
        if (length > 0)
            snapPlaceHolder()
        else
            animatePlaceHolder()
    }
    onEnabledChanged: {
        updateIconSources()
        updateChrome()
    }
    onCustomButtonVisibleChanged: {
        updateChrome()
        if (!placeHolderAnim.running)
            snapPlaceHolder()
        else
            animatePlaceHolder()
    }
    onHeightChanged: {
        repaintAiGradientBorder()
        if (!placeHolderAnim.running)
            snapPlaceHolder()
    }

    Connections {
        target: rightChromeRow
        function onWidthChanged() {
            if (!placeHolderAnim.running)
                control.snapPlaceHolder()
        }
    }
    onAccepted: {
        appendSearchHistory(text)
        focus = false
    }
    onEditingFinished: appendSearchHistory(text)

    function setTranslucent(flag) {
        translucent = flag
    }
    function isTranslucent() {
        return translucent
    }
    function setGradation(flag) {
        gradationEnabled = flag
    }
    function setCustomButtonVisible(flag) {
        customButtonVisible = flag
        updateChrome()
    }
    function isCustomButtonVisible() {
        return customButtonVisible
    }

    function clearField() {
        if (!length)
            return
        control.text = ""
        animatePlaceHolder()
        updateChrome()
    }

    function appendSearchHistory(entry) {
        var t = entry.trim()
        if (t === "")
            return
        for (var i = 0; i < historyModel.count; ++i) {
            if (historyModel.get(i).text.toLowerCase() === t.toLowerCase())
                return
        }
        historyModel.append({ text: t })
    }

    function updateChrome() {
        // 对齐 C++：显示 customButton 时右侧区域变宽，清除键与 custom 不同时出现
        clearBtn.visible = clearButtonEnabled && enabled && activeFocus && length > 0
                && !customButtonVisible
        chromeLine.visible = customButtonVisible
        customBtn.visible = customButtonVisible
        repaintAiGradientBorder()
        if (!placeHolderAnim.running)
            snapPlaceHolder()
    }

    function repaintAiGradientBorder() {
        if (aiGradientBorder)
            aiGradientBorder.requestPaint()
    }

    function updateIconSources() {
        var model = "normalWidthThemeColor"
        if (!enabled)
            model = "disableWidthThemeColor"
        searchIcon.source = "image://imageProvider/search-symbolic/" + model
        _chromeLineColor = parseColor.getDtColor("kline-normal")
    }

    // 对齐 C++ KVLineFrame / m_pLineFrame：kline-normal DT
    property color _chromeLineColor: parseColor.getDtColor("kline-normal")

    // 取右侧 Row 实测宽度，避免隐藏 clearBtn 仍占位导致 placeholder/文本错位
    readonly property int _rightChromeWidth: _iconMargin + rightChromeRow.width

    StylePrivate.ParseColorInterface {
        id: parseColor
    }

    function getStartColor(dtcolor) {
        return parseColor.startColor(dtcolor)
    }

    function getEndColor(dtcolor) {
        return parseColor.endColor(dtcolor)
    }

    property bool click: false

    // 与 TextField.qml 一致：translucent=true 用 *TransparentBC，false 用实色 BC
    property var backColor: !control.enabled ? (translucent ? textfield.disableTransparentBC : textfield.disableBC) :
                             (control.activeFocus) ? (translucent ? textfield.inputNormalTransparentBC : textfield.inputNormalBC) :
                             click ? (translucent ? textfield.clickedTransparentBC : textfield.clickedBC) :
                             control.hovered ? (translucent ? textfield.hoveredTransparentBC : textfield.hoveredBC) :
                                               (translucent ? textfield.normalTransparentBC : textfield.normalBC)

    property var backBorderColor: !control.enabled ? textfield.disableBorderColor : (control.activeFocus) ? textfield.focusBorderColor :
                                                          control.hovered ? textfield.hoverBorderColor : textfield.normalBorderColor

    onPressed: click = true
    onReleased: click = false

    StylePrivate.UKUITextFiled {
        id: textfield
    }

    StylePrivate.UKUIComboBox {
        id: combobox
    }

    StylePrivate.APPParameter {
        id: app
        onParametryChanged: updateIconSources()
        onIconThemeChanged: updateIconSources()
    }

    ListModel {
        id: historyModel
    }

    ListModel {
        id: filteredHistory
    }

    readonly property int _historyRowHeight: 36

    function historyOverlayParent() {
        var item = control
        while (item) {
            if (item.overlay)
                return item.overlay
            item = item.parent
        }
        if (Window.window) {
            if (Window.window.overlay)
                return Window.window.overlay
            return Window.window.contentItem
        }
        return typeof Overlay !== "undefined" ? Overlay.overlay : null
    }

    function attachHistoryLayer() {
        var p = historyOverlayParent()
        if (p && historyLayer.parent !== p)
            historyLayer.parent = p
    }

    function refreshHistoryPopup() {
        if (!activeFocus || text.length === 0 || historyModel.count === 0) {
            if (historyLayer.opened)
                historyLayer.close()
            return
        }
        var needle = text.toLowerCase()
        var newItems = []
        for (var i = 0; i < historyModel.count; ++i) {
            var entry = historyModel.get(i).text
            if (entry.toLowerCase().indexOf(needle) === 0)
                newItems.push({ text: entry })
        }
        if (newItems.length === 0) {
            if (historyLayer.opened)
                historyLayer.close()
            return
        }
        filteredHistory.clear()
        for (var j = 0; j < newItems.length; ++j)
            filteredHistory.append(newItems[j])
        historyLayer.reposition()
        historyLayer.open()
        control.forceActiveFocus()
    }

    function applyHistoryEntry(entry) {
        control.text = entry
        historyLayer.close()
        control.forceActiveFocus()
    }

    // KPopup：边距对齐 ComboBox 下拉（top/bottomMargin 6，内边距 upDownMargin）
    Private.KPopup {
        id: historyLayer
        modal: false
        focus: false
        acceptFocus: false
        focusTarget: control
        animated: false
        topMargin: 6
        bottomMargin: 6
        padding: combobox.upDownMargin
        transparent: control.translucent
        closePolicy: T.Popup.CloseOnPressOutside | T.Popup.CloseOnEscape
        width: control.width
        height: filteredHistory.count > 0
               ? Math.min(filteredHistory.count * _historyRowHeight, 208) + 2 * combobox.upDownMargin
               : 0

        onClosed: filteredHistory.clear()

        Component.onCompleted: control.attachHistoryLayer()

        Timer {
            interval: 50
            running: historyLayer.parent === control
            repeat: false
            onTriggered: control.attachHistoryLayer()
        }

        onAboutToShow: reposition()
        onOpened: Qt.callLater(function() { control.forceActiveFocus() })

        function reposition() {
            var overlay = historyOverlayParent()
            if (!overlay)
                return
            if (historyLayer.parent !== overlay)
                historyLayer.parent = overlay
            var pos = control.mapToItem(overlay, 0, control.height)
            historyLayer.x = pos.x
            historyLayer.y = pos.y + historyLayer.topMargin
            historyLayer.width = control.width
            if (historyLayer.visible) {
                historyLayer.syncWindowSize()
                historyLayer.syncWindowPosition()
            }
        }

        Connections {
            target: control
            function onWidthChanged() { historyLayer.reposition() }
            function onXChanged() { if (historyLayer.opened) historyLayer.reposition() }
            function onYChanged() { if (historyLayer.opened) historyLayer.reposition() }
        }

        ListView {
            id: historyList
            width: parent.width
            clip: true
            focus: false
            activeFocusOnTab: false
            implicitHeight: contentHeight
            model: filteredHistory
            boundsBehavior: Flickable.StopAtBounds
            delegate: ItemDelegate {
                width: historyList.width
                height: _historyRowHeight
                focus: false
                text: model.text
                onClicked: control.applyHistoryEntry(model.text)
            }
        }
    }

    TextMetrics {
        id: placeHolderMetrics
        font: control.font
        text: placeHolderText
    }

    onWidthChanged: {
        repaintAiGradientBorder()
        if (!placeHolderAnim.running)
            snapPlaceHolder()
    }
    onPlaceHolderTextChanged: {
        placeHolderHost.refreshToolTip()
        if (!placeHolderAnim.running)
            snapPlaceHolder()
    }

    Component.onCompleted: {
        snapPlaceHolder()
        updateIconSources()
        updateChrome()
        attachHistoryLayer()
        if (contentItem)
            contentItem.z = 0
    }

    // 对齐 C++ paintEvent：有焦点或语义模式(m_isGradient)时画边框
    readonly property bool _aiGradientActive: gradationEnabled && customBtn.checked && enabled
    readonly property bool _showBorder: enabled && (activeFocus || _aiGradientActive)

    // 非渐变：highlight-active；禁用：button-disable（与 C++ QPen 一致）
    readonly property color _borderStrokeColor: !enabled
            ? parseColor.getDtColor("button-disable")
            : parseColor.getDtColor("highlight-active")

    background: Item {
        implicitWidth: textfield.normalWidth
        implicitHeight: textfield.normalHeight

        Rectangle {
            id: backrec
            anchors.fill: parent
            radius: textfield.radius
            color: gradientRec.visible ? "transparent" : getStartColor(backColor)

            Rectangle {
                id: gradientRec
                anchors.fill: parent
                radius: parent.radius
                gradient: Gradient {
                    GradientStop { position: 0.0; color: getStartColor(backColor) }
                    GradientStop { position: 1.0; color: getEndColor(backColor) }
                }
                visible: !parseColor.isSolidPattern(backColor)
            }

            // C++ m_isGradient：四色横向渐变描边
            Canvas {
                id: aiGradientBorder
                anchors.fill: parent
                visible: control._showBorder && control._aiGradientActive
                antialiasing: true

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    if (width <= 0 || height <= 0)
                        return

                    // 与上方 Rectangle border 一致：贴控件外缘描边，不用 C++ adjusted(1,3) 内缩
                    var lineW = textfield.focusBorderWidth > 0 ? textfield.focusBorderWidth : 2
                    var half = lineW / 2
                    var x = half
                    var y = half
                    var rw = width - lineW
                    var rh = height - lineW
                    if (rw <= 0 || rh <= 0)
                        return

                    var radius = (textfield.radius > 0 ? textfield.radius : 6) - half
                    radius = Math.max(0, Math.min(radius, rw / 2, rh / 2))

                    var grd = ctx.createLinearGradient(0, height / 2, width, height / 2)
                    grd.addColorStop(0, "#E888F8")
                    grd.addColorStop(0.4, "#8A41FF")
                    grd.addColorStop(0.7, "#4E75FF")
                    grd.addColorStop(1, "#0036F5")

                    ctx.strokeStyle = grd
                    ctx.lineWidth = lineW
                    ctx.beginPath()
                    ctx.moveTo(x + radius, y)
                    ctx.lineTo(x + rw - radius, y)
                    ctx.arcTo(x + rw, y, x + rw, y + radius, radius)
                    ctx.lineTo(x + rw, y + rh - radius)
                    ctx.arcTo(x + rw, y + rh, x + rw - radius, y + rh, radius)
                    ctx.lineTo(x + radius, y + rh)
                    ctx.arcTo(x, y + rh, x, y + rh - radius, radius)
                    ctx.lineTo(x, y + radius)
                    ctx.arcTo(x, y, x + radius, y, radius)
                    ctx.closePath()
                    ctx.stroke()
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: "transparent"
                border.width: control._showBorder && !control._aiGradientActive
                        ? textfield.focusBorderWidth : 0
                border.color: control._borderStrokeColor
                antialiasing: true
            }
        }
    }

    // 右侧清除/语义按钮必须在 contentItem 之上
    Item {
        id: rightChromeHost
        z: 10
        anchors.fill: parent

        Row {
            id: rightChromeRow
            spacing: _toolSpacing
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: _rtl ? undefined : parent.right
            anchors.left: _rtl ? parent.left : undefined
            anchors.rightMargin: 3//_iconMargin
            anchors.leftMargin: _iconMargin

            RoundButton {
                id: clearBtn
                width: visible ? height : 0
                height: visible ? (control.height >= 36 ? 30 : (control.height - 6)) : 0
                visible: false
                flat: true
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                display: AbstractButton.IconOnly
                padding: 0
                leftPadding: 0
                rightPadding: 0
                topPadding: 0
                bottomPadding: 0
                icon.name: "application-exit-symbolic"
                icon.width: _iconSize
                icon.height: _iconSize                

                onClicked: control.clearField()
            }

            Rectangle {
                id: chromeLine
                width: visible ? 1 : 0
                height: Math.max(16, parent.height - 12)
                visible: customBtn.visible
                color: control._chromeLineColor
                anchors.verticalCenter: parent.verticalCenter
            }

            // 对齐 C++ KToolButton；icon.name 由 org.ukui.style ToolButton 经 ParseInterface 加载主题图
            Button {
                id: customBtn
                width: visible ? height : 0
                height: visible ? (control.height >= 36 ? 30 : (control.height - 6)) : 0
                visible: false
                //flat: true
                checkable: true
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                display: AbstractButton.IconOnly
                padding: 0
                leftPadding: 0
                rightPadding: 0
                topPadding: 0
                bottomPadding: 0
                //highlighted: gradationEnabled && checked
                icon.name: _builtinCustomButtonIcon
                icon.width: _iconSize
                icon.height: _iconSize

                flat: true
                onCheckedChanged: control.updateChrome()
            }
        }
    }

    // 对齐 C++ m_pWidget：在整框内定位，超出部分由 clip 裁切（与 QWidget 子控件一致）
    Item {
        id: placeHolderHost
        z: 10
        anchors.fill: parent
        clip: true
        enabled: false

        Item {
            id: placeHolderRow
            width: _placeHolderContentWidth
            height: _placeHolderRowHeight
            clip: true
            anchors.verticalCenter: parent.verticalCenter

            Image {
                id: searchIcon
                width: _iconSize
                height: _iconSize
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                smooth: true
                cache: false
            }

            Text {
                id: placeHolderLabel
                visible: !control.length
                x: _iconSize + _iconTextSpacing
                anchors.verticalCenter: parent.verticalCenter
                width: _placeHolderNeedsElide
                    ? _placeHolderLabelMaxWidth
                    : Math.max(1, Math.ceil(placeHolderMetrics.width))
                text: placeHolderText
                font: control.font
                color: getStartColor(!control.enabled
                    ? textfield.placeHolderDisableTextColor
                    : textfield.placeHolderNormalTextColor)
                elide: _placeHolderNeedsElide ? Text.ElideRight : Text.ElideNone

                onPaintedWidthChanged: {
                    if (!placeHolderAnim.running)
                        control.snapPlaceHolder()
                }
            }
        }

        Component.onCompleted: {
            control.snapPlaceHolder()
            refreshToolTip()
        }

        PropertyAnimation {
            id: placeHolderAnim
            target: placeHolderRow
            property: "x"
            duration: 100
            easing.type: Easing.InOutQuad
        }

        ToolTip {
            delay: Qt.styleHints.mousePressAndHoldInterval
            visible: control.hovered && _toolTipPlaceHolder !== ""
            text: _toolTipPlaceHolder
        }

        function refreshToolTip() {
            if (_placeHolderNeedsElide)
                _toolTipPlaceHolder = placeHolderText
            else
                _toolTipPlaceHolder = ""
        }

        onWidthChanged: refreshToolTip()
    }
}
