/*
 * Qt5-UKUI's Library
 *
 * 面包屑/目录栏单项外壳，供 KBreadCrumb、KDirTabBar 共用。
 * 布局：高 36；左 8 + 16x16 图标 + 间距 8 + 文字 + 间距 8 + 30x30 箭头 + 右 8
 *  - backgroundVisible=false：悬浮/点击 → 文字高亮，无背景
 *  - backgroundVisible=true：悬浮/点击 → Button 背景，文字不变色
 *  - separatorInteractive=true：箭头单独发 separatorClicked；图标随 expanded 在 end/down 间切换
 *  - separatorInteractive=false：箭头点击与正文同属 clicked
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: root

    property string text: ""
    property string iconName: ""
    property bool showSeparator: true
    property bool current: false
    property bool flat: true
    property bool backgroundVisible: false
    property bool translucent: true
    // true：箭头独立交互（KDirTabBar）；false：整项点击（KBreadCrumb）
    property bool separatorInteractive: false
    // 菜单展开时箭头为 ukui-down-symbolic
    property bool expanded: false
    property int iconRev: 0
    // KDirTabBar 对齐 C++（去掉 TabFocus）；KBreadCrumb 保持可 Tab
    property bool tabFocusEnabled: true

    signal clicked()
    signal separatorClicked()

    Layout.alignment: Qt.AlignVCenter
    Layout.preferredHeight: 36
    Layout.minimumHeight: 36
    readonly property int _contentWidth: {
        var w = itemLabel.implicitWidth
        if (_hasIcon)
            w += 16 + 8
        if (showSeparator)
            w += 30 + 8
        return w + 16
    }
    Layout.preferredWidth: _contentWidth
    implicitHeight: 36
    implicitWidth: _contentWidth
    activeFocusOnTab: tabFocusEnabled
    focus: false

    StylePrivate.APPParameter { id: app }
    StylePrivate.UKUIButton { id: btn }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property bool isDark: app.isDark()
    readonly property bool _hasIcon: iconName !== ""
    readonly property bool _hovered: hoverArea.containsMouse
    // 鼠标按下或键盘空格/回车按住时，背景用点击态
    readonly property bool _pressed: clickArea.pressed || sepBtn.down || _keyPressed
    property bool _keyPressed: false
    property bool _mouseFocusing: false
    property bool _focusEnable: false
    readonly property bool _showFocusBorder: app.focusEnable && _focusEnable

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token
    readonly property int _radius: {
        if (_hasToken && ukui_platformtheme_token.kradiusNormal > 0)
            return ukui_platformtheme_token.kradiusNormal
        return btn.radius > 0 ? btn.radius : 6
    }
    readonly property color _highLightColor: _hasToken
                                             ? ukui_platformtheme_token.highLightActive
                                             : "#3790FA"
    readonly property color _focusTextColor: _hasToken
                                             ? ukui_platformtheme_token.kFontPrimary
                                             : (isDark ? "#D9D9D9" : "#262626")
    readonly property color _baseColor: {
        if (flat)
            return _focusTextColor
        if (_hasToken)
            return isDark ? ukui_platformtheme_token.kGray6
                          : ukui_platformtheme_token.kGray10
        return isDark ? "#47474A" : "#B3B3B3"
    }

    function _resolveIconUrl(name) {
        if (!name || name === "")
            return ""
        var s = String(name)
        var prefix = "image://imageProvider/"
        if (s.indexOf(prefix) === 0) {
            var path = s.substring(prefix.length)
            var slash = path.lastIndexOf("/")
            if (slash >= 0) {
                var model = path.substring(slash + 1)
                if (model === "normal" || model === "disenable" || model === "clicked"
                        || model === "hover" || model === "highlight" || model === "highDisbale"
                        || model === "hoveredWidthThemeColor" || model === "clickedWidthThemeColor"
                        || model === "normalWidthThemeColor" || model === "disableWidthThemeColor")
                    path = path.substring(0, slash)
            }
            s = path
        }
        return prefix + s + "/normalWidthThemeColor"
    }

    function _bgBrush(hovered, pressed) {
        if (translucent) {
            if (pressed)
                return btn.clickedTransparentBC
            if (hovered)
                return btn.hoveredTransparentBC
            return btn.normalTransparentBC
        }
        if (pressed)
            return btn.clickedBC
        if (hovered)
            return btn.hoveredBC
        return btn.normalBC
    }

    function _borderBrush(hovered, pressed) {
        if (translucent) {
            if (pressed)
                return btn.clickedBorderTransparentBC
            if (hovered)
                return btn.hoveredBorderTransparentBC
            return btn.normalBorderTransparentBC
        }
        if (pressed)
            return btn.clickBorderColor
        if (hovered)
            return btn.hoverBorderColor
        return btn.normalBorderColor
    }

    function _bgStart(hovered, pressed) {
        if (!backgroundVisible || (!hovered && !pressed))
            return "transparent"
        return parseColor.startColor(_bgBrush(hovered, pressed))
    }

    function _bgEnd(hovered, pressed) {
        if (!backgroundVisible || (!hovered && !pressed))
            return "transparent"
        return parseColor.endColor(_bgBrush(hovered, pressed))
    }

    function _showChrome(hovered, pressed) {
        return backgroundVisible && (hovered || pressed)
    }

    function refreshIcons() {
        itemIcon.source = ""
        itemIcon.source = _hasIcon ? _resolveIconUrl(iconName) : ""
        var sepName = expanded ? "ukui-down-symbolic" : "ukui-end-symbolic"
        sepBtn.icon.name = ""
        sepBtn.icon.name = sepName
    }

    onIconNameChanged: refreshIcons()
    onExpandedChanged: refreshIcons()
    onIconRevChanged: refreshIcons()
    Component.onCompleted: refreshIcons()

    onActiveFocusChanged: {
        if (!activeFocus) {
            _focusEnable = false
            _keyPressed = false
            return
        }
        // Tab 走焦：_mouseFocusing 为 false → 显示焦点框
        // 鼠标点击：_mouseFocusing 为 true → 不显示焦点框
        _focusEnable = !_mouseFocusing
        _mouseFocusing = false
    }

    function _activateByMouse() {
        _mouseFocusing = true
        forceActiveFocus()
        _focusEnable = false
    }

    // 菜单收起：清掉焦点框并释放焦点，避免 Popup 归还焦点被误判为键盘获焦；
    // 之后可用 Tab 重新走焦并正常显示焦点样式
    function dismissFocusChrome() {
        _focusEnable = false
        _mouseFocusing = false
        focus = false
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: root._radius
        antialiasing: true
        color: root._bgStart(root._hovered, root._pressed)
        border.width: root._showFocusBorder ? btn.focusBorderWidth
                      : (root._showChrome(root._hovered, root._pressed) ? btn.borderWidth : 0)
        border.color: root._showFocusBorder
                      ? parseColor.startColor(btn.focusBorderColor)
                      : parseColor.startColor(root._borderBrush(root._hovered, root._pressed))

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            border.width: parent.border.width
            border.color: parent.border.color
            visible: root._showChrome(root._hovered, root._pressed)
                     && !parseColor.isSolidPattern(root._bgBrush(root._hovered, root._pressed))
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: root._bgStart(root._hovered, root._pressed)
                }
                GradientStop {
                    position: 1.0
                    color: root._bgEnd(root._hovered, root._pressed)
                }
            }
        }
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Image {
            id: itemIcon
            visible: root._hasIcon && status === Image.Ready
            sourceSize.width: 16
            sourceSize.height: 16
            Layout.preferredWidth: root._hasIcon ? 16 : 0
            Layout.preferredHeight: 16
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
            smooth: true
            cache: false
        }

        Text {
            id: itemLabel
            text: root.text
            font: app.font
            color: (!root.backgroundVisible && root._hovered)
                   ? root._highLightColor
                   : (root.current ? root._focusTextColor : root._baseColor)
            Layout.alignment: Qt.AlignVCenter
            elide: Text.ElideRight
        }

        Button {
            id: sepBtn
            visible: root.showSeparator
            flat: true
            hoverEnabled: false
            focusPolicy: Qt.NoFocus
            activeFocusOnTab: false
            implicitWidth: 30
            implicitHeight: 30
            Layout.preferredWidth: root.showSeparator ? 30 : 0
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignVCenter
            icon.name: "ukui-end-symbolic"
            icon.width: 16
            icon.height: 16
            background: Rectangle {
                implicitWidth: 30
                implicitHeight: 30
                radius: root._radius
                color: "transparent"
            }
            onPressed: {
                root._mouseFocusing = true
                root._focusEnable = false
            }
            onClicked: {
                root._activateByMouse()
                if (root.separatorInteractive)
                    root.separatorClicked()
                else
                    root.clicked()
            }
        }
    }

    MouseArea {
        id: clickArea
        anchors.fill: parent
        z: -1
        hoverEnabled: false
        cursorShape: Qt.PointingHandCursor
        onPressed: {
            root._mouseFocusing = true
            root._focusEnable = false
        }
        onClicked: {
            root._activateByMouse()
            root.clicked()
        }
        onCanceled: root._mouseFocusing = false
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        z: 100
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        cursorShape: Qt.PointingHandCursor
    }

    Keys.onShortcutOverride: {
        // 防止外层 ScrollView 等抢走空格/回车
        if (event.key === Qt.Key_Space || event.key === Qt.Key_Return
                || event.key === Qt.Key_Enter) {
            event.accepted = true
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                || event.key === Qt.Key_Space) {
            // 按住期间显示与鼠标点击相同的按下背景
            _keyPressed = true
            event.accepted = true
        } else if (event.key === Qt.Key_Down
                   && root.separatorInteractive && root.showSeparator) {
            // DirTabBar：↓ 打开子目录菜单
            root.separatorClicked()
            event.accepted = true
        }
    }

    Keys.onReleased: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                || event.key === Qt.Key_Space) {
            if (_keyPressed) {
                _keyPressed = false
                root.clicked()
            }
            event.accepted = true
        }
    }
}