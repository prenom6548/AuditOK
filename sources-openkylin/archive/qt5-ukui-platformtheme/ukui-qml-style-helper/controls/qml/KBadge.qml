/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KBadge.
 * 消息提醒气泡。value<1 或不可见 → 10x10 圆点；1~999 → 数字胶囊；>=1000 → "999"宽胶囊 + 三点。
 * 背景：默认 highlight-active；useErrorColor=true → kerror-normal；useCustomColor → 自定义色。
 * 颜色随主题变化（getDtColor 不跟主题，靠 _refreshColors 在主题切换时重算）。
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.qqc2style.private 1.0 as StylePrivate

Item {
    id: control

    property int value: -1
    property bool valueVisiable: true
    property color color: "transparent" // 自定义色，仅 useCustomColor 时生效
    property bool useCustomColor: false
    property bool useErrorColor: false // 背景 kerror-normal（错误态）
    property int fontSize: 14

    implicitWidth: 40
    implicitHeight: 30

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property bool _showDots: value >= 1000 && valueVisiable
    readonly property bool _showNumber: value >= 1 && value < 1000 && valueVisiable
    readonly property bool _showDot: value < 1 || !valueVisiable

    // 字体度量对齐 QFontMetrics：height = ascent + descent + 1（QML 的 fm.height 少了 +1）
    FontMetrics {
        id: fm
        font.pixelSize: control.fontSize
    }
    readonly property int _fmHeight: fm.ascent + fm.descent + 1

    function _badgeWidth() {
        if (_showDot)
            return 10
        var text = _showDots ? "999" : (value >= 0 ? String(value) : "")
        return Math.max(fm.advanceWidth(text) + 10, _fmHeight) // 对齐 cpp: max(width, height)
    }
    function _badgeHeight() {
        if (_showDot)
            return 10
        return _fmHeight
    }

    // 配色（随主题刷新）：优先自定义色，其次 kerror-normal，默认 highlight-active；文字 light-active
    property color _bgColor: "transparent"
    property color _textColor: "transparent"
    function _refreshColors() {
        _textColor = parseColor.getDtColor("light-active")
        if (useCustomColor)
            _bgColor = color
        else if (useErrorColor)
            _bgColor = parseColor.getDtColor("kerror-normal")
        else
            _bgColor = parseColor.getDtColor("highlight-active")
    }
    Component.onCompleted: _refreshColors()
    onUseCustomColorChanged: _refreshColors()
    onUseErrorColorChanged: _refreshColors()
    onColorChanged: _refreshColors()
    Connections {
        target: app
        function onParametryChanged() { control._refreshColors() } // 主题切换刷新配色
    }

    // 胶囊背景：所有情况都画（cpp:147 drawRoundedRect 无条件执行；>=1000 也有底色胶囊）
    Rectangle {
        id: pill
        anchors.centerIn: parent
        width: control._badgeWidth()
        height: control._badgeHeight()
        radius: height / 2
        color: control._bgColor
        antialiasing: true

        // 对齐 cpp drawText(tmpRect, Qt::AlignCenter)：铺满 pill 再对齐，避免 anchors.centerIn 把 descent 空白算进包围盒导致个位数偏上
        Text {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            visible: control._showNumber
            text: control.value >= 0 ? String(control.value) : ""
            font.pixelSize: control.fontSize
            color: control._textColor
        }
    }

    // >=1000：三点叠在胶囊上（cpp:159-164 drawEllipse 半径1.5，圆心 center.x ±10 / 0）
    // Row 三点 3px、spacing 7 → 总宽 23，居中后圆心正好在 -10/0/+10
    Row {
        anchors.centerIn: parent
        visible: control._showDots
        spacing: 7
        Repeater {
            model: 3
            Rectangle {
                width: 3
                height: 3
                radius: 1.5
                color: control._textColor
                antialiasing: true
            }
        }
    }

    function setValue(v) { value = v }
    function setValueVisiable(flag) { valueVisiable = flag }
    function setColor(c) { useCustomColor = true; color = c }
    function setUseErrorColor(flag) { useErrorColor = flag }
    function setFontSize(s) { if (s >= 1 && s <= 100) fontSize = s }
}
