/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KLabel.
 * 样式和颜色对齐 kysdk-qtwidgets/src/klabel.cpp。
 */

import QtQuick 2.12
import QtGraphicalEffects 1.0
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: root

    enum LabelType { NormalType = 0, DataType = 1, CircleType = 2 }
    // 与 QPalette::ColorRole 的数值保持一致，供 setFontColorRole() 使用。
    enum FontColorRole {
        WindowText = 0, Button = 1, Light = 2, Midlight = 3, Dark = 4,
        Mid = 5, Text = 6, BrightText = 7, ButtonText = 8, Base = 9,
        Window = 10, Shadow = 11, Highlight = 12, HighlightedText = 13,
        Link = 14, LinkVisited = 15, AlternateBase = 16, NoRole = 17,
        ToolTipBase = 18, ToolTipText = 19, PlaceholderText = 20
    }

    property int labelType: KLabel.LabelType.NormalType
    property string text: ""
    property string iconName: ""
    property url iconSource: ""
    property int iconWidth: 16
    property int iconHeight: 16
    property int iconSpacing: 4
    property bool wordWrap: false
    property int horizontalAlignment: Text.AlignLeft
    property int verticalAlignment: Text.AlignVCenter
    property int leftPadding: 0
    property int topPadding: 0
    property int rightPadding: 0
    property int bottomPadding: 0
    property font font: app.font

    property bool dataHighlightColor: false
    property bool backgroundColorEnabled: false
    property color backgroundColor: "white"
    property bool highlightBackground: false
    property color fontColor: "transparent"
    property bool customFontColor: false
    property int fontColorRole: KLabel.FontColorRole.Text
    property bool fontColorRoleEnabled: false
    property bool tristate: false
    property bool checked: false
    property int topLeftRadius: 6
    property int topRightRadius: 6
    property int bottomLeftRadius: 6
    property int bottomRightRadius: 6

    signal enterWidget()
    signal leaveWidget()
    signal clicked(bool checked)
    signal doubleClicked()

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    property color _textActive: "transparent"
    property color _highlight: "transparent"
    property color _button: "transparent"
    property color _windowText: "transparent"
    property color _placeholderText: "transparent"
    property color _highlightedText: "transparent"
    property color _brightText: "transparent"
    property color _buttonText: "transparent"
    property color _toolTipText: "transparent"
    property color _light: "transparent"
    property color _midlight: "transparent"
    property color _dark: "transparent"
    property color _mid: "transparent"
    property color _base: "transparent"
    property color _window: "transparent"
    property color _shadow: "transparent"
    property color _link: "transparent"
    property color _linkVisited: "transparent"
    property color _alternateBase: "transparent"
    property color _toolTipBase: "transparent"

    readonly property bool _hasIcon: iconName !== "" || iconSource !== ""
    readonly property int _naturalContentWidth: (_hasIcon ? iconWidth + (text !== "" ? iconSpacing : 0) : 0)
                                               + textItem.implicitWidth
    readonly property int _naturalContentHeight: Math.max(_hasIcon ? iconHeight : 0, textItem.implicitHeight)
    // DataType 使用主题高亮色的 15% 透明度作为底色。
    readonly property color _dataBackground: Qt.rgba(_highlight.r, _highlight.g, _highlight.b, 0.15)
    readonly property color _background: {
        if (labelType === KLabel.LabelType.DataType)
            return backgroundColorEnabled ? backgroundColor : _dataBackground
        if (backgroundColorEnabled)
            return backgroundColor
        if (highlightBackground)
            return _highlight
        if (labelType === KLabel.LabelType.CircleType)
            return _button
        return "transparent"
    }
    readonly property color _roleColor: {
        switch (fontColorRole) {
        case KLabel.FontColorRole.WindowText: return _windowText
        case KLabel.FontColorRole.Button: return _button
        case KLabel.FontColorRole.Light: return _light
        case KLabel.FontColorRole.Midlight: return _midlight
        case KLabel.FontColorRole.Dark: return _dark
        case KLabel.FontColorRole.Mid: return _mid
        case KLabel.FontColorRole.BrightText: return _brightText
        case KLabel.FontColorRole.ButtonText: return _buttonText
        case KLabel.FontColorRole.Base: return _base
        case KLabel.FontColorRole.Window: return _window
        case KLabel.FontColorRole.Shadow: return _shadow
        case KLabel.FontColorRole.Highlight: return _highlight
        case KLabel.FontColorRole.HighlightedText: return _highlightedText
        case KLabel.FontColorRole.Link: return _link
        case KLabel.FontColorRole.LinkVisited: return _linkVisited
        case KLabel.FontColorRole.AlternateBase: return _alternateBase
        case KLabel.FontColorRole.ToolTipBase: return _toolTipBase
        case KLabel.FontColorRole.ToolTipText: return _toolTipText
        case KLabel.FontColorRole.PlaceholderText: return _placeholderText
        default: return _textActive
        }
    }
    // 三态标签悬浮/按下时，按原生逻辑混合高亮与亮文字色。
    readonly property color _tristateHoverColor: _mixColor(_highlight, _brightText,
                                                             interaction.pressed ? 0.2 : 0.05)
    readonly property color _textColor: {
        if (tristate)
            return interaction.containsMouse ? _tristateHoverColor : _placeholderText
        if (customFontColor)
            return fontColor
        if (fontColorRoleEnabled)
            return _roleColor
        if (dataHighlightColor)
            return _highlight
        return _textActive
    }

    implicitWidth: leftPadding + _naturalContentWidth + rightPadding
    implicitHeight: topPadding + _naturalContentHeight + bottomPadding
    clip: true

    function _mixColor(first, second, bias) {
        return Qt.rgba(first.r * (1 - bias) + second.r * bias,
                       first.g * (1 - bias) + second.g * bias,
                       first.b * (1 - bias) + second.b * bias,
                       first.a * (1 - bias) + second.a * bias)
    }

    function _refreshColors() {
        // ParseColorInterface 读取当前主题 token；主题切换后重新取色。
        _textActive = parseColor.getDtColor("text-active")
        _highlight = parseColor.getDtColor("highlight-active")
        _button = parseColor.getDtColor("button-active")
        _windowText = parseColor.getDtColor("windowtext-active")
        _placeholderText = parseColor.getDtColor("placeholdertext-active")
        _highlightedText = parseColor.getDtColor("highlightedtext-active")
        _brightText = parseColor.getDtColor("brighttext-active")
        _buttonText = parseColor.getDtColor("buttontext-active")
        _toolTipText = parseColor.getDtColor("tooltiptext-active")
        _light = parseColor.getDtColor("light-active")
        _midlight = parseColor.getDtColor("midlight-active")
        _dark = parseColor.getDtColor("dark-active")
        _mid = parseColor.getDtColor("mid-active")
        _base = parseColor.getDtColor("base-active")
        _window = parseColor.getDtColor("window-active")
        _shadow = parseColor.getDtColor("shadow-active")
        _link = parseColor.getDtColor("link-active")
        _linkVisited = parseColor.getDtColor("linkvisited-active")
        _alternateBase = parseColor.getDtColor("alternatebase-active")
        _toolTipBase = parseColor.getDtColor("tooltipbase-active")
        backgroundCanvas.requestPaint()
    }

    function setLabelType(type) { labelType = type }
    function setDataHightColor(flag) { dataHighlightColor = flag }
    function setBackgroundColor(flag, color) {
        highlightBackground = false
        backgroundColorEnabled = flag
        if (color !== undefined)
            backgroundColor = color
    }
    function setHighlightBackground(flag) {
        backgroundColorEnabled = false
        highlightBackground = flag
    }
    function setBorderRadius(bottomLeft, topLeft, topRight, bottomRight) {
        // 兼容原生的一参（统一圆角）和四参（分别设置圆角）重载。
        if (arguments.length === 1) {
            topLeftRadius = bottomLeft
            topRightRadius = bottomLeft
            bottomLeftRadius = bottomLeft
            bottomRightRadius = bottomLeft
            return
        }
        bottomLeftRadius = bottomLeft
        topLeftRadius = topLeft
        topRightRadius = topRight
        bottomRightRadius = bottomRight
    }
    function setWordWrap(flag) { wordWrap = flag }
    function setFontColor(color) {
        fontColor = color
        customFontColor = true
        fontColorRoleEnabled = false
    }
    function setFontColorRole(role) {
        fontColorRole = role
        fontColorRoleEnabled = true
        customFontColor = false
    }
    function setTristate(flag) {
        tristate = flag
        if (flag)
            setFontColorRole(KLabel.FontColorRole.PlaceholderText)
    }
    function isTristate() { return tristate }
    function setPixmap(icon, width, height) {
        if (typeof icon === "string")
            iconName = icon
        if (width !== undefined)
            iconWidth = width
        if (height !== undefined)
            iconHeight = height
    }

    Canvas {
        id: backgroundCanvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            if (root._background === "transparent" || width <= 0 || height <= 0)
                return

            ctx.fillStyle = root._background
            ctx.beginPath()
            if (root.labelType === KLabel.LabelType.CircleType) {
                // CircleType 按控件完整矩形绘制椭圆。
                ctx.save()
                ctx.translate(width / 2, height / 2)
                ctx.scale(width / 2, height / 2)
                ctx.arc(0, 0, 1, 0, Math.PI * 2, false)
                ctx.restore()
            } else {
                var tl = Math.max(0, Math.min(root.topLeftRadius, width / 2, height / 2))
                var tr = Math.max(0, Math.min(root.topRightRadius, width / 2, height / 2))
                var br = Math.max(0, Math.min(root.bottomRightRadius, width / 2, height / 2))
                var bl = Math.max(0, Math.min(root.bottomLeftRadius, width / 2, height / 2))
                ctx.moveTo(tl, 0)
                ctx.lineTo(width - tr, 0)
                ctx.quadraticCurveTo(width, 0, width, tr)
                ctx.lineTo(width, height - br)
                ctx.quadraticCurveTo(width, height, width - br, height)
                ctx.lineTo(bl, height)
                ctx.quadraticCurveTo(0, height, 0, height - bl)
                ctx.lineTo(0, tl)
                ctx.quadraticCurveTo(0, 0, tl, 0)
            }
            ctx.closePath()
            ctx.fill()
        }
    }

    Item {
        id: content
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: root.leftPadding
        anchors.right: parent.right
        anchors.rightMargin: root.rightPadding

        Item {
            id: contentRow
            width: root.wordWrap ? parent.width : Math.min(parent.width, root._naturalContentWidth)
            height: parent.height
            x: root.horizontalAlignment === Text.AlignRight ? parent.width - width
              : root.horizontalAlignment === Text.AlignHCenter ? (parent.width - width) / 2 : 0

            Private.KyIcon {
                id: themedIcon
                width: root.iconWidth
                height: root.iconHeight
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                visible: root.iconName !== "" && !root.dataHighlightColor
                iconName: root.iconName
                icontype: "ordinary"
            }

            Image {
                id: highlightIcon
                width: root.iconWidth
                height: root.iconHeight
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                visible: false
                source: root.iconName !== "" ? "image://imageProvider/" + root.iconName + "/normal"
                                             : root.iconSource
                fillMode: Image.PreserveAspectFit
                smooth: true
                cache: false
            }

            // 数据高亮图标固定使用 highlight-active，与原生 KLabel 一致。
            ColorOverlay {
                anchors.fill: highlightIcon
                visible: root.dataHighlightColor && root._hasIcon
                source: highlightIcon
                color: root._highlight
            }

            Image {
                id: sourceIcon
                width: root.iconWidth
                height: root.iconHeight
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                visible: root.iconName === "" && root.iconSource !== "" && !root.dataHighlightColor
                source: root.iconSource
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Text {
                id: textItem
                anchors.left: parent.left
                anchors.leftMargin: root._hasIcon ? root.iconWidth + (root.text !== "" ? root.iconSpacing : 0) : 0
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: root.text
                font: root.font
                color: root._textColor
                wrapMode: root.wordWrap ? Text.WordWrap : Text.NoWrap
                elide: root.wordWrap ? Text.ElideNone : Text.ElideRight
                horizontalAlignment: root._hasIcon ? Text.AlignLeft : root.horizontalAlignment
                verticalAlignment: root.verticalAlignment
            }
        }
    }

    MouseArea {
        id: interaction
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onEntered: root.enterWidget()
        onExited: root.leaveWidget()
        onReleased: {
            // 原生 KLabel 每次左键释放切换一次 clicked 状态。
            if (mouse.button === Qt.LeftButton) {
                root.checked = !root.checked
                root.clicked(root.checked)
            }
        }
        onDoubleClicked: root.doubleClicked()
    }

    Component.onCompleted: _refreshColors()
    onLabelTypeChanged: backgroundCanvas.requestPaint()
    onBackgroundColorEnabledChanged: backgroundCanvas.requestPaint()
    onBackgroundColorChanged: backgroundCanvas.requestPaint()
    onHighlightBackgroundChanged: backgroundCanvas.requestPaint()
    onTopLeftRadiusChanged: backgroundCanvas.requestPaint()
    onTopRightRadiusChanged: backgroundCanvas.requestPaint()
    onBottomLeftRadiusChanged: backgroundCanvas.requestPaint()
    onBottomRightRadiusChanged: backgroundCanvas.requestPaint()
    onWidthChanged: backgroundCanvas.requestPaint()
    onHeightChanged: backgroundCanvas.requestPaint()

    Connections {
        target: app
        function onParametryChanged() { root._refreshColors() }
    }
}
