/*
 * Qt5-UKUI's Library
 *
 * QML counterpart of kdk::KInputDialog.
 * Layout, sizes and control states follow kysdk-qtwidgets/src/kinputdialog.cpp.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import org.ukui.platformtheme.controls 1.0 as Private
import org.ukui.qqc2style.private 1.0 as StylePrivate

Private.KDialog {
    id: root

    enum InputMode {
        TextInput = 0,
        IntInput = 1,
        DoubleInput = 2
    }

    enum InputDialogOption {
        NoButtons = 1,
        UseListViewForComboBoxItems = 2,
        UsePlainTextEditForTextInput = 4
    }

    property int inputMode: KInputDialog.InputMode.TextInput
    property int options: 0
    property string labelText: qsTr("Enter a value:")
    property string tipText: ""
    property string placeholderText: ""
    property string textValue: ""
    property int textEchoMode: TextInput.Normal
    property int intValue: 0
    property int intMinimum: 0
    property int intMaximum: 99
    property int intStep: 1
    property real doubleValue: 0.0
    property real doubleMinimum: 0.0
    property real doubleMaximum: 99.99
    property int doubleDecimals: 2
    property real doubleStep: 1.0
    property string okButtonText: qsTr("ok")
    property string cancelButtonText: qsTr("cancel")

    readonly property bool usePlainTextEdit: inputMode === KInputDialog.InputMode.TextInput
                                                  && (options & KInputDialog.InputDialogOption.UsePlainTextEditForTextInput) !== 0
    readonly property bool noButtons: (options & KInputDialog.InputDialogOption.NoButtons) !== 0
    readonly property bool _singleLine: inputMode === KInputDialog.InputMode.TextInput && !usePlainTextEdit
    readonly property bool _intInput: inputMode === KInputDialog.InputMode.IntInput
    readonly property bool _doubleInput: inputMode === KInputDialog.InputMode.DoubleInput
    readonly property int _inputHeight: usePlainTextEdit ? 192 : 36
    // 标签下方统一保留 8px 间距。
    readonly property int _inputY: 28
    readonly property int _tipY: _inputY + _inputHeight + (_singleLine ? 0 : 8)
    readonly property int _buttonY: _tipY + 19 + 32
    readonly property int _bodyHeight: _buttonY + (noButtons ? 0 : 36) + 24
    readonly property int _dialogWidth: usePlainTextEdit ? 304 : 336
    readonly property int _doubleFactor: Math.pow(10, Math.max(0, doubleDecimals))
    readonly property bool _numericInputAcceptable: {
        var spin = _intInput ? intSpinBox : doubleSpinBox
        return !_intInput && !_doubleInput
                || !spin.contentItem
                || spin.contentItem.acceptableInput
    }

    signal textValueSelected(string value)
    signal intValueSelected(int value)
    signal doubleValueSelected(real value)
    signal confirmed(var value)
    signal cancelled()

    title: ""
    width: _dialogWidth
    height: 40 + _bodyHeight
    minimumWidth: _dialogWidth
    maximumWidth: _dialogWidth
    minimumHeight: 40 + _bodyHeight
    maximumHeight: 40 + _bodyHeight
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    blurEnabled: false

    StylePrivate.APPParameter { id: app }
    palette: app.palette

    readonly property bool _hasToken: typeof ukui_platformtheme_token !== "undefined"
                                      && ukui_platformtheme_token
    readonly property color _primaryText: _hasToken
                                          ? ukui_platformtheme_token.kFontPrimary
                                          : palette.windowText
    property bool _suppressClosingCancel: false

    Component.onCompleted: _applyChrome()
    onCloseBtnClicked: cancelDialog()
    onClosing: {
        if (!_suppressClosingCancel)
            cancelled()
    }
    onVisibleChanged: {
        if (visible)
            Qt.callLater(_focusInput)
    }

    function _applyChrome() {
        if (minButton)
            minButton.visible = false
        if (maxButton)
            maxButton.visible = false
        if (menuButton)
            menuButton.visible = false
        if (closeButton)
            closeButton.visible = true
    }

    function _focusInput() {
        var input = _singleLine ? lineEdit
                                : usePlainTextEdit ? plainTextEdit
                                                   : _intInput ? intSpinBox : doubleSpinBox
        input.forceActiveFocus()
        if (input.selectAll)
            input.selectAll()
        else if (input.contentItem && input.contentItem.selectAll)
            input.contentItem.selectAll()
    }

    function setOption(option, on) {
        if (on === undefined)
            on = true
        options = on ? (options | option) : (options & ~option)
    }

    function testOption(option) {
        return (options & option) !== 0
    }

    function setIntRange(minimum, maximum) {
        intMinimum = minimum
        intMaximum = maximum
    }

    function setDoubleRange(minimum, maximum) {
        doubleMinimum = minimum
        doubleMaximum = maximum
    }

    function showDialog() {
        show()
        raise()
        requestActivate()
        Qt.callLater(_focusInput)
    }

    function acceptDialog() {
        if (!_numericInputAcceptable)
            return

        _suppressClosingCancel = true
        var value
        if (_intInput) {
            value = intValue
            intValueSelected(value)
        } else if (_doubleInput) {
            value = doubleValue
            doubleValueSelected(value)
        } else {
            value = textValue
            textValueSelected(value)
        }
        confirmed(value)
        close()
        Qt.callLater(function() { _suppressClosingCancel = false })
    }

    function cancelDialog() {
        if (!visible)
            return
        _suppressClosingCancel = true
        cancelled()
        close()
        Qt.callLater(function() { _suppressClosingCancel = false })
    }

    Item {
        anchors.fill: parent

        Label {
            x: 24
            y: 0
            width: parent.width - 48
            height: 20
            text: root.labelText
            color: root._primaryText
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        TextField {
            id: lineEdit
            x: 24
            y: root._inputY
            width: 288
            height: 36
            visible: root._singleLine
            property bool transparent: false
            text: root.textValue
            placeholderText: root.placeholderText
            echoMode: root.textEchoMode
            selectByMouse: true
            onTextChanged: root.textValue = text
            onAccepted: root.acceptDialog()
        }

        TextArea {
            id: plainTextEdit
            x: 24
            y: root._inputY
            width: 256
            height: 192
            visible: root.usePlainTextEdit
            property bool transparent: false
            text: root.textValue
            placeholderText: root.placeholderText
            selectByMouse: true
            wrapMode: TextEdit.NoWrap
            clip: true
            onTextChanged: root.textValue = text
        }

        SpinBox {
            id: intSpinBox
            x: 24
            y: root._inputY
            width: 288
            height: 36
            visible: root._intInput
            property bool transparent: false
            editable: true
            from: root.intMinimum
            to: root.intMaximum
            stepSize: root.intStep
            value: root.intValue
            onValueChanged: root.intValue = value
            Keys.onReturnPressed: root.acceptDialog()
            Keys.onEnterPressed: root.acceptDialog()
        }

        SpinBox {
            id: doubleSpinBox
            x: 24
            y: root._inputY
            width: 288
            height: 36
            visible: root._doubleInput
            property bool transparent: false
            editable: true
            from: Math.round(root.doubleMinimum * root._doubleFactor)
            to: Math.round(root.doubleMaximum * root._doubleFactor)
            stepSize: Math.max(1, Math.round(root.doubleStep * root._doubleFactor))
            value: Math.round(root.doubleValue * root._doubleFactor)
            validator: DoubleValidator {
                bottom: Math.min(root.doubleMinimum, root.doubleMaximum)
                top: Math.max(root.doubleMinimum, root.doubleMaximum)
                decimals: root.doubleDecimals
                notation: DoubleValidator.StandardNotation
            }
            textFromValue: function(value, locale) {
                return Number(value / root._doubleFactor).toLocaleString(locale, "f", root.doubleDecimals)
            }
            valueFromText: function(text, locale) {
                return Math.round(Number.fromLocaleString(locale, text) * root._doubleFactor)
            }
            onValueChanged: root.doubleValue = value / root._doubleFactor
            Keys.onReturnPressed: root.acceptDialog()
            Keys.onEnterPressed: root.acceptDialog()
        }

        Label {
            x: 24
            y: root._tipY
            width: parent.width - 48
            height: 19
            text: root.tipText
            color: "red"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            maximumLineCount: 1
        }

        Row {
            x: parent.width - 24 - width
            y: root._buttonY
            visible: !root.noButtons
            spacing: 10

            Button {
                width: 96
                height: 36
                property bool transparent: false
                text: root.cancelButtonText
                onClicked: root.cancelDialog()
            }

            Button {
                width: 96
                height: 36
                property bool transparent: false
                text: root.okButtonText
                highlighted: true
                enabled: root._numericInputAcceptable
                onClicked: root.acceptDialog()
            }
        }
    }
}
