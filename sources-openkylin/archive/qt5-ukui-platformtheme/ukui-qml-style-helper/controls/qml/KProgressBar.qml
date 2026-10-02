/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KProgressBar。
 * 内部复用 org.ukui.style ProgressBar（背景/填充/居中文字/扫光/indeterminate）；
 * 本控件补充：三态、label 位置、外侧图标，以及 bodyWidth / format / 自定义色等接口。
 *
 * 规则：
 *  - Center：仅居中百分比文字，不会有图标
 *  - End：文本/图标都在条外（水平右侧，竖直下方）；成功/失败显示图标
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.12 as T
import org.ukui.qqc2style.private 1.0 as StylePrivate

T.Control {
    id: control

    enum State { NormalProgress = 0, FailedProgress = 1, SuccessProgress = 2 }
    /// Center：百分比居中叠在条上（无图标）；End：文本/图标在条外
    enum LabelPosition { Center = 0, End = 1 }

    property int state: KProgressBar.State.NormalProgress
    property int orientation: Qt.Horizontal
    property int labelPosition: KProgressBar.LabelPosition.Center
    property bool textVisible: true
    property bool animationEnabled: false
    property bool indeterminate: false
    /// 与 QProgressBar::format 一致，支持 %p / %v / %m
    property string format: "%p%"
    /// 槽厚度；0 表示用样式默认
    property int bodyWidth: 0
    /// 反向填充（从右/上往左/下）
    property bool invertedAppearance: false
    /// 自定义双色渐变（Normal 态）
    property bool gradation: false
    property var gradationColor1: null
    property var gradationColor2: null
    /// 自定义背景 / 填充（null 表示跟主题）
    property var backgroundColor: null
    property var fillColor: null

    property real from: 0
    property real to: 100
    property real value: 0

    font: app.font
    palette: app.palette

    readonly property bool _horizontal: orientation === Qt.Horizontal
    readonly property int _barThickness: bodyWidth > 0 ? bodyWidth : (_horizontal ? 30 : 0)
    implicitWidth: _horizontal ? 240 : Math.max(48, _barThickness > 0 ? _barThickness + 16 : 48)
    implicitHeight: _horizontal ? Math.max(30, _barThickness) : 220

    StylePrivate.APPParameter { id: app }
    StylePrivate.ParseColorInterface { id: parseColor }

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token

    readonly property bool _isNormal: state === KProgressBar.State.NormalProgress
    readonly property bool _isFailed: state === KProgressBar.State.FailedProgress
    readonly property bool _isSuccess: state === KProgressBar.State.SuccessProgress
    readonly property bool _labelAtEnd: labelPosition === KProgressBar.LabelPosition.End

    readonly property bool _barTextVisible: textVisible && _isNormal && !indeterminate
                                            && !_labelAtEnd
    readonly property bool _sideTextVisible: textVisible && _isNormal && !indeterminate
                                            && _labelAtEnd
    readonly property bool _sideIconVisible: textVisible && (_isFailed || _isSuccess)
                                             && !indeterminate && _labelAtEnd

    readonly property color _sideTextColor: {
        if (!enabled) {
            if (_hasToken)
                return ukui_platformtheme_token.kFontPrimaryDisable
            return parseColor.getDtColor("kfont-primary-disable")
        }
        return palette.windowText
    }

    readonly property string _statusIcon: {
        if (_isFailed)
            return "image://imageProvider/dialog-error/normal"
        if (_isSuccess)
            return "image://imageProvider/ukui-dialog-success/normal"
        return ""
    }

    /// 对应 C++ text()：按 format 替换 %p/%v/%m
    readonly property string displayText: {
        if (!_isNormal || indeterminate)
            return ""
        var total = to - from
        var p = total <= 0 ? 100 : Math.round((value - from) * 100.0 / total)
        var v = Math.round(value)
        var m = Math.round(total)
        var result = format
        // 与 QProgressBar 替换顺序一致：%m → %v → %p
        result = result.split("%m").join("" + m)
        result = result.split("%v").join("" + v)
        result = result.split("%p").join("" + p)
        return result
    }

    readonly property bool _useCustomFill: {
        if (_isFailed || _isSuccess)
            return false
        if (gradation && gradationColor1)
            return true
        if (fillColor)
            return true
        return false
    }

    /// 传给内部 ProgressBar 的填充（含三态主题色）
    property var progressChildrenColor: {
        if (_isFailed) {
            if (_hasToken)
                return ukui_platformtheme_token.kErrorNormal
            return parseColor.getDtColor("kerror-normal")
        }
        if (_isSuccess) {
            if (_hasToken)
                return ukui_platformtheme_token.kSuccessNormal
            return parseColor.getDtColor("ksuccess-normal")
        }
        if (gradation && gradationColor1)
            return gradationColor1
        if (fillColor)
            return fillColor
        return null
    }

    property var progressChildrenColorEnd: {
        if (_isFailed || _isSuccess)
            return null
        if (gradation && gradationColor2)
            return gradationColor2
        return null
    }

    onValueChanged: {
        if (!indeterminate && value >= to && to > from)
            state = KProgressBar.State.SuccessProgress
    }

    contentItem: Loader {
        sourceComponent: control._horizontal ? hLayout : vLayout
    }

    Component {
        id: hLayout
        RowLayout {
            spacing: 8

            ProgressBar {
                Layout.fillWidth: true
                Layout.preferredHeight: control._barThickness > 0 ? control._barThickness : 30
                Layout.minimumWidth: 40
                from: control.from
                to: control.to
                value: control.value
                indeterminate: control.indeterminate
                property bool textVisible: control._barTextVisible
                property string progressText: control.displayText
                property bool showNormalProgressBarAnim: control.animationEnabled
                                                        && control.enabled
                                                        && control._isNormal
                                                        && !control.indeterminate
                property var progressChildrenColor: control.progressChildrenColor
                property var progressChildrenColorEnd: control.progressChildrenColorEnd
                property bool progressFillPlain: control._useCustomFill
                property var progressBackgroundColor: control.backgroundColor
                property bool invertedAppearance: control.invertedAppearance
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                visible: control._sideTextVisible
                text: control.displayText
                color: control._sideTextColor
            }

            Image {
                Layout.alignment: Qt.AlignVCenter
                visible: control._sideIconVisible
                opacity: control.enabled ? 1.0 : 0.45
                source: visible ? control._statusIcon : ""
                width: 16; height: 16
                sourceSize.width: 16; sourceSize.height: 16
                fillMode: Image.PreserveAspectFit
                smooth: true; cache: false
            }
        }
    }

    Component {
        id: vLayout
        ColumnLayout {
            spacing: 8

            Item {
                Layout.preferredWidth: control.bodyWidth > 0 ? control.bodyWidth : vBar.implicitHeight
                Layout.fillHeight: true
                Layout.minimumHeight: 40
                Layout.alignment: Qt.AlignHCenter

                ProgressBar {
                    id: vBar
                    width: parent.height
                    height: control.bodyWidth > 0 ? control.bodyWidth : implicitHeight
                    anchors.centerIn: parent
                    rotation: -90
                    from: control.from
                    to: control.to
                    value: control.value
                    indeterminate: control.indeterminate
                    property bool textVisible: control._barTextVisible
                    property string progressText: control.displayText
                    property bool showNormalProgressBarAnim: control.animationEnabled
                                                            && control.enabled
                                                            && control._isNormal
                                                            && !control.indeterminate
                    property var progressChildrenColor: control.progressChildrenColor
                    property var progressChildrenColorEnd: control.progressChildrenColorEnd
                    property bool progressFillPlain: control._useCustomFill
                    property var progressBackgroundColor: control.backgroundColor
                    property bool invertedAppearance: control.invertedAppearance
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                visible: control._sideTextVisible
                text: control.displayText
                color: control._sideTextColor
            }

            Image {
                Layout.alignment: Qt.AlignHCenter
                visible: control._sideIconVisible
                opacity: control.enabled ? 1.0 : 0.45
                source: visible ? control._statusIcon : ""
                width: 16; height: 16
                sourceSize.width: 16; sourceSize.height: 16
                fillMode: Image.PreserveAspectFit
                smooth: true; cache: false
            }
        }
    }

    function text() { return displayText }
    function setState(s) { state = s }
    function setOrientation(o) { orientation = o }
    function setLabelPosition(p) { labelPosition = p }
    function setTextVisible(v) { textVisible = v }
    function setAnimation(flag) { animationEnabled = flag }
    function animation() { return animationEnabled }
    function setRange(min, max) { from = min; to = max }
    function setValue(v) { value = v }
    function setFormat(f) { format = f }
    function setBodyWidth(w) { bodyWidth = w }
    function setBackgroundColor(c) { backgroundColor = c }
    function setFillColor(c) { fillColor = c }
    function setGradation(flag) { gradation = flag }
    function setGradationColor(c1, c2) {
        gradationColor1 = c1
        gradationColor2 = c2
        gradation = true
    }
}
