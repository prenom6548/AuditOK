/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KProgressDialog。
 * 基于 KDialog；对齐 C++ 四种 BarType 布局与常用接口。
 *
 * BarType:
 *  - NormalBar：标题栏 + 主文案 + 进度条 + 详情行（副文案/百分比/value+suffix）+ 取消
 *  - NothingBar：无标题栏按钮；主文案 + 进度条与百分比同行；无取消
 *  - ScrollBar：不确定进度；仅主文案 + 条；无取消/详情
 *  - SpeedBar：主文案 + 条与百分比；副文案 + 速度文案；自定义按钮（最多 3 个），无取消
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

Private.KDialog {
    id: root

    enum BarType { ScrollBar = 0, NothingBar = 1, NormalBar = 2, SpeedBar = 3 }

    property int barType: KProgressDialog.BarType.NormalBar
    property int minimum: 0
    property int maximum: 100
    property int value: 0
    property string labelText: ""
    property string subContent: ""
    property string cancelButtonText: qsTr("cancel")
    /// NormalBar 详情后缀：显示为 "value+suffix/maximum+suffix"
    property string suffix: ""
    /// SpeedBar：速度与单位（对应 C++ setSuffix(numerator, denominator)）
    property string numerator: ""
    property string denominator: ""
    property real speed: 0
    property bool showDetail: true
    property bool autoReset: true
    property bool autoClose: true
    /// SpeedBar 额外按钮文案，最多 3 个
    property var extraButtons: []

    readonly property bool _isScroll: barType === KProgressDialog.BarType.ScrollBar
    readonly property bool _isNothing: barType === KProgressDialog.BarType.NothingBar
    readonly property bool _isNormal: barType === KProgressDialog.BarType.NormalBar
    readonly property bool _isSpeed: barType === KProgressDialog.BarType.SpeedBar
    /// 无底部操作按钮时：不显示标题文字与关闭按钮（NothingBar / ScrollBar）
    readonly property bool _hasActionButtons: _isNormal
                                              || (_isSpeed && extraButtons && extraButtons.length > 0)
    readonly property bool _showTitleBar: _hasActionButtons

    readonly property int _percent: {
        if (maximum <= minimum)
            return 0
        return Math.round((value - minimum) * 100.0 / (maximum - minimum))
    }
    readonly property string _percentText: _percent + "%"
    readonly property string _progressDetailText: {
        if (_isSpeed)
            return speed + " " + numerator + "/" + denominator
        return value + suffix + "/" + maximum + suffix
    }

    signal canceled()
    signal extraButtonClicked(int index)

    // 有按钮时可显示标题/图标，窗口按钮仅关闭；无按钮时隐藏整栏标题栏
    title: ""
    blurEnabled: true
    modality: Qt.ApplicationModal
    readonly property int _titleBarH: _showTitleBar && header ? header.height : 0
    readonly property int _bodyH: contentCol.implicitHeight
    width: 376
    height: _titleBarH + _bodyH
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height

    StylePrivate.APPParameter { id: app }
    palette: app.palette

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token
    readonly property color _primaryText: _hasToken
                                          ? ukui_platformtheme_token.kFontPrimary
                                          : palette.windowText
    readonly property color _secondaryText: _hasToken
                                            ? ukui_platformtheme_token.kFontSecondary
                                            : palette.placeholderText

    Component.onCompleted: _applyChrome()
    onBarTypeChanged: _applyChrome()
    on_ShowTitleBarChanged: _applyChrome()
    onExtraButtonsChanged: _applyChrome()

    function _applyChrome() {
        if (header)
            header.visible = root._showTitleBar
        if (minButton)
            minButton.visible = false
        if (maxButton)
            maxButton.visible = false
        if (menuButton)
            menuButton.visible = false
        if (closeButton)
            closeButton.visible = root._showTitleBar
    }

    onValueChanged: {
        if (_isScroll)
            return
        if (value < maximum)
            return
        // 与 QProgressDialog 语义接近：满值可自动重置/关闭
        if (autoClose)
            close()
        else if (autoReset)
            _resetValueOnly()
    }

    function _resetValueOnly() {
        value = minimum
    }

    function reset() {
        _resetValueOnly()
        if (autoClose)
            close()
    }

    function cancel() {
        canceled()
        value = minimum
        close()
    }

    Item {
        anchors.fill: parent

        ColumnLayout {
            id: contentCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: 0

            Item { Layout.preferredHeight: root._isNormal ? 30 : (root._isSpeed ? 4 : 24); Layout.fillWidth: true }

        Label {
            text: root.labelText
            color: root._primaryText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            horizontalAlignment: Text.AlignLeft
        }

        Item { Layout.preferredHeight: root._isNormal ? 10 : 8; Layout.fillWidth: true }

        // —— ScrollBar：不确定进度 ——
        ProgressBar {
            visible: root._isScroll
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            indeterminate: true
        }

        // —— NothingBar / SpeedBar：条 + 百分比同行 ——
        RowLayout {
            visible: root._isNothing || root._isSpeed
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: 8

            Private.KProgressBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 8
                from: root.minimum
                to: root.maximum
                value: root.value
                textVisible: false
                bodyWidth: 8
            }

            Label {
                text: root._percentText
                color: root._primaryText
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // —— NormalBar：单独进度条 ——
        Private.KProgressBar {
            visible: root._isNormal
            Layout.fillWidth: true
            Layout.preferredHeight: 8
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            from: root.minimum
            to: root.maximum
            value: root.value
            textVisible: false
            bodyWidth: 8
        }

        // —— NormalBar 详情行：副文案 | 百分比 | value/max ——
        RowLayout {
            visible: root._isNormal && root.showDetail
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: 0

            Label {
                text: root.subContent
                color: root._secondaryText
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: root._percentText
                color: root._secondaryText
            }

            Item { Layout.preferredWidth: 15 }

            Label {
                text: root._progressDetailText
                color: root._secondaryText
            }
        }

        // —— SpeedBar 详情行：副文案 | 速度 ——
        RowLayout {
            visible: root._isSpeed && root.showDetail
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: 0
            Layout.topMargin: 4

            Label {
                text: root.subContent
                color: root._secondaryText
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: root._progressDetailText
                color: root._secondaryText
            }

            Item { Layout.preferredWidth: 36 }
        }

        Item {
            visible: root._isNormal || root._isSpeed
            Layout.preferredHeight: root._isNormal ? 30 : 32
            Layout.fillWidth: true
        }

        // —— NormalBar 取消按钮 ——
        RowLayout {
            visible: root._isNormal
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Item { Layout.fillWidth: true }
            Button {
                text: root.cancelButtonText
                onClicked: root.cancel()
            }
        }

        // —— SpeedBar 自定义按钮 ——
        RowLayout {
            visible: root._isSpeed && root.extraButtons && root.extraButtons.length > 0
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            spacing: 8

            Item {
                visible: root.extraButtons.length < 3
                Layout.fillWidth: true
            }

            Repeater {
                model: {
                    var src = root.extraButtons || []
                    var n = Math.min(3, src.length)
                    if (n <= 0)
                        return []
                    if (n === 1)
                        return [src[0]]
                    if (n === 2)
                        return [src[1], src[0]]
                    return [src[2], src[1], src[0]]
                }

                Button {
                    text: typeof modelData === "string" ? modelData : (modelData.text || "")
                    Layout.fillWidth: root.extraButtons.length >= 3 && index === 0
                    onClicked: {
                        var src = root.extraButtons || []
                        var n = Math.min(3, src.length)
                        var realIndex = index
                        if (n === 2)
                            realIndex = 1 - index
                        else if (n === 3)
                            realIndex = 2 - index
                        root.extraButtonClicked(realIndex)
                    }
                }
            }
        }

        // 内容距底 24（有按钮时即按钮距底）
        Item { Layout.preferredHeight: 24; Layout.fillWidth: true }
        }
    }

    function setRange(min, max) { minimum = min; maximum = max }
    function setMaximum(v) { maximum = v }
    function setMinimum(v) { minimum = v }
    function setValue(v) { value = v }
    function setLabelText(t) { labelText = t }
    function setCancelButtonText(t) { cancelButtonText = t }
    function setSubContent(t) {
        subContent = t
        showDetail = true
    }
    function setSuffix(a, b) {
        if (b === undefined || b === null) {
            suffix = a
        } else {
            numerator = a
            denominator = b
        }
    }
    function setShowDetail(flag) { showDetail = flag }
    function setAutoReset(flag) { autoReset = flag }
    function setAutoClose(flag) { autoClose = flag }
    function setBarType(t) { barType = t }
    function setSpeed(s) { speed = s }
    function myType() { return barType }

    function addButton(text) {
        var list = (extraButtons || []).slice()
        if (list.length >= 3)
            return
        list.push(text)
        extraButtons = list
    }

    function removeButton(textOrIndex) {
        var list = (extraButtons || []).slice()
        if (typeof textOrIndex === "number") {
            if (textOrIndex < 0 || textOrIndex >= list.length)
                return
            list.splice(textOrIndex, 1)
        } else {
            var i = list.indexOf(textOrIndex)
            if (i < 0)
                return
            list.splice(i, 1)
        }
        extraButtons = list
    }
}
