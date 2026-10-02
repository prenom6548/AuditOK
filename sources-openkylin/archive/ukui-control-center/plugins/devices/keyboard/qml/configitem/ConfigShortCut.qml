/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

import "../"
import "../childwindow/"
import "../utils.js" as UtilsScripts
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "keyutils.js" as KeyHelper
import org.ukui.quick.items 1.0 as UkuiItems
import org.ukui.quick.platform 1.0

Item {
    id: shortcut

    property string value: ""
    property string name: ""
    property string shortcutString: ""
    property var currentKeys: []
    property bool allowModifierLess: false
    property bool allowModifierOnly: false
    property alias labelText: label.text
    property int layoutMargins: 0
    property int itemIndex: -1
    property bool enableBackground: true
    property string currentText: ""
    property bool isSystemShortcutBlocked: false
    property string source: ""
    property var modifierMap: ({
        [Qt.Key_Control_R]: Qt.Key_Control,
        [Qt.Key_Shift_R]: Qt.Key_Shift,
        [Qt.Key_AltGr]: Qt.Key_Alt
    })

    signal shortcutChanged(string value, int itemIndex)
    signal shortcutUpdateRequested(string name, string oldValue, string newValue) // 新增信号

    function normalizeShortcut(shortcut) {
        var genericModifiers = ["Control", "Shift", "Alt", "Meta"];
        var keys = shortcut.split('+');
        var excludeGenerics = new Set();
        for (var i = 0; i < keys.length; i++) {
            var keyOriginal = keys[i];
            var match = keyOriginal.match(/^(Control|Shift|Alt|Meta)_[LR]$/);
            if (match)
                excludeGenerics.add(match[1]);

        }
        var processedKeys = [];
        for (var j = 0; j < keys.length; j++) {
            var key = keys[j];
            if (genericModifiers.includes(key)) {
                if (!excludeGenerics.has(key))
                    processedKeys.push(key);

            } else {
                processedKeys.push(key);
            }
        }
        return processedKeys.join('+');
    }

    function cppToQml(cppStr) {
        const mapped = normalizeShortcut(cppStr).split('+').map((key) => {
            var keyQml = KeyHelper.UnifiedKeyMap.cppToQml(key.trim());
            return keyQml || key.trim();
        });
        const filtered = mapped.filter((item, index) => {
            return mapped.indexOf(item) === index;
        });
        console.debug("filtered:", filtered);
        const beautified = filtered.map((qmlKey) => {
            return KeyHelper.beautifyModifierKey(qmlKey);
        });
        return beautified.join(' ');
    }

    function qmlToCpp(qmlStr) {
        return qmlStr.split(' ').map((key) => {
            return KeyHelper.UnifiedKeyMap.qmlToCpp(key.trim()) || key.trim();
        }).join('+');
    }

    function blockSystemShortcut() {
        console.debug('--------blockSystemShortcut---------');
        shortcutController.blockSystemShortcut();
        isSystemShortcutBlocked = true;
    }

    function unblockSystemShortcut() {
        if (!isSystemShortcutBlocked)
            return ;

        console.debug('--------unblockSystemShortcut---------');
        shortcutController.unblockSystemShortcut();
        isSystemShortcutBlocked = false;
    }

    function isOkWhenModifierless(event) {
        var isSpecialKey = event.key === Qt.Key_Return || event.key === Qt.Key_Space || event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab || event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete;
        if (isSpecialKey) {
            if ((event.modifiers & Qt.ShiftModifier) != 0)
                return true;

            return false;
        } else if (event.text.length === 1) {
            return false;
        }
        return true;
    }

    function normalizeModifier(key) {
        const numKey = Number(key);
        return modifierMap[numKey] ?? numKey;
    }

    function isLetter(keyCode) {
        return (keyCode >= Qt.Key_A && keyCode <= Qt.Key_Z);
    }

    function isShortcutAllowed(key) {
        if (!key || key.trim() === '')
            return false;

        const fields = key.split(' ').filter((field) => {
            return field.trim() !== '';
        });
        if (fields.length === 0)
            return false;

        const modifierKeys = ['Ctrl', 'Shift', 'Alt', 'Meta', 'Super', 'Hyper'];
        const hasOnlyModifiers = fields.every((field) => {
            return modifierKeys.includes(field);
        });
        if (!allowModifierOnly && hasOnlyModifiers)
            return false;

        // 检查是否只有修饰键（没有其他非修饰键）
        const hasNonModifiers = fields.some((field) => {
            return !modifierKeys.includes(field);
        });
        // 如果不允许只有修饰键且当前只有修饰键（没有其他非修饰键），返回false
        if (!allowModifierLess && !hasNonModifiers)
            return false;

        return true;
    }

    function checkConflict(result) {
        shortcutString = KeyHelper.sortAndDeduplicateKeys(result);
        if (!isShortcutAllowed(shortcutString))
            return ;

        var fixedUniqueName = source === "" ? name : source + "_" + name;
        var fcitxShortcut = qmlToCpp(result);
        var resultInfo = shortcutController.checkShortcutConflict(fcitxShortcut, fixedUniqueName);
        if (resultInfo.hasConflict)
            showConflictDialog(resultInfo, fcitxShortcut);
        else
            handleShortcutChanged(fcitxShortcut);
    }

    function handleFocusLost() {
        const result = KeyHelper.processCombinationKeys(currentKeys);
        console.debug("processCombinationKeys result:", result);
        if (result === "")
            resetToDefault();
        else
            checkConflict(result);
        currentKeys = [];
        shortcutString = "";
    }

    function showConflictDialog(resultInfo, fcitxShortcut) {
        var conflict = resultInfo.conflict;
        var beforeModifyValue = value;
        var conflictDialog = conflictDialogComponent.createObject(shortcut, {
            "conflictName": conflict.name,
            "conflictDescription": conflict.source === "Global" ? UtilsScripts.getTranslation(conflict.name) || "" : conflict.description,
            "conflictSource": conflict.source,
            "conflictShortcut": conflict.shortcut
        });
        conflictDialog.continueWithAccept.connect(function() {
            handleShortcutChanged(fcitxShortcut);
        });
        conflictDialog.restoreOriginal.connect(function() {
            resetToDefault();
        });
        conflictDialog.show();
    }

    function isKeypadKey(event) {
        return KeyHelper.scanCodeKeyPad.includes(event.nativeScanCode);
    }

    function beautifyShortcut(originalvalue) {
        if (originalvalue === "") {
            if (control.activeFocus)
                return "...";
            else
                return "";
        }
        return originalvalue.replace(/\s/g, " + ");
    }

    function resetToDefault() {
        console.info("reset");
        shortcutString = cppToQml(value);
    }

    function handleShortcutChanged(newValue) {
        // 发送更新请求信号
        shortcut.shortcutUpdateRequested(name, value, newValue);
        shortcut.shortcutChanged(newValue, itemIndex);
    }

    Component.onDestruction: {
        unblockSystemShortcut();
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.forceActiveFocus();
        }
    }

    RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.leftMargin: layoutMargins
        anchors.rightMargin: layoutMargins

        UkuiItems.DtThemeText {
            id: label

            elide: Qt.ElideRight
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
        }

        TextField {
            id: control

            function updateDisplay() {
                var keys = currentKeys.map(function(data) {
                    return KeyHelper.getKeyName(data.key);
                });
                shortcutString = keys.join("+");
            }

            Accessible.name: label.text
            readOnly: true
            Layout.alignment: Qt.AlignRight
            implicitWidth: enableBackground ? 180 : 240
            text: {
                if (control.activeFocus)
                    return beautifyShortcut(shortcutString);

                return beautifyShortcut(cppToQml(value));
            }
            Keys.onPressed: {
                if (!activeFocus)
                    return ;

                event.accepted = true;
                const rawKey = event.key;
                const normalizedKey = normalizeModifier(rawKey);
                var isIncluded = currentKeys.some(function(data) {
                    return data.key === normalizedKey;
                });
                if (isIncluded)
                    return ;

                console.debug("event.nativeScanCode:", event.nativeScanCode, ",event.text:", event.text, ",normalizedKey:", normalizedKey);
                var needModify = false;
                var isKeyPadKey = false;
                if (currentKeys.length === 1 && [Qt.Key_Shift, Qt.Key_Shift_R, Qt.Key_Shift_L].includes(currentKeys[0].key)) {
                    if (isLetter(normalizedKey)) {
                        currentKeys = [];
                    } else if (KeyHelper.mainKbDigitMofified.includes(event.text)) {
                        currentKeys = [];
                        needModify = true;
                    } else if (KeyHelper.KeyMaps.scanCode.hasOwnProperty(event.nativeScanCode)) {
                        needModify = true;
                    }
                }
                var done = true;
                if (control.text === "")
                    done = false;

                //修饰键状态提取.得到一个新的位掩码 modifiers2，它只包含 Shift、Ctrl、Alt、Meta 这几个修饰键的状态信息，其他无关的修饰键状态会被过滤掉
                var modifiers2 = event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier | Qt.KeypadModifier);
                //检查是否只有 Shift 键被按下
                if (modifiers2 !== 0 && (modifiers2 & ~Qt.ShiftModifier) == 0) {
                    if (!isOkWhenModifierless(event) && !allowModifierLess)
                        done = false;

                }
                if (modifiers2 !== 0 && (modifiers2 & ~Qt.MetaModifier) == 0)
                    needModify = true;

                //检查是否是小键盘按键
                if (modifiers2 !== 0 && (modifiers2 & ~Qt.KeypadModifier) == 0)
                    isKeyPadKey = true;
                else if (isKeypadKey(event))
                    isKeyPadKey = true;
                if ((event.key === Qt.Key_Shift || event.key === Qt.Key_Control || event.key === Qt.Key_Meta || event.key === Qt.Key_Super_L || event.key === Qt.Key_Super_R || event.key === Qt.Key_Hyper_L || event.key === Qt.Key_Hyper_R || event.key === Qt.Key_Alt))
                    done = false;

                var data = {
                    "key": normalizedKey,
                    "code": event.nativeScanCode,
                    "text": event.text,
                    "isModifier": !done,
                    "needModify": needModify,
                    "isKeyPadKey": isKeyPadKey
                };
                console.debug("normalizedKey:", normalizedKey, ",event.nativeScanCode:", event.nativeScanCode, ",event.text:", event.text, ",done:", done, ",needModify:", needModify, ",isKeyPadKey:", isKeyPadKey);
                currentKeys.push(data);
                updateDisplay();
                if (done)
                    control.focus = false;

            }
            Keys.onReleased: {
                event.accepted = true;
                if (!activeFocus)
                    return ;

                var done = false;
                if ((allowModifierOnly || allowModifierLess) && (event.key === Qt.Key_Shift || event.key === Qt.Key_Control || event.key === Qt.Key_Meta || event.key === Qt.Key_Super_L || event.key === Qt.Key_Super_R || event.key === Qt.Key_Hyper_L || event.key === Qt.Key_Hyper_R || event.key === Qt.Key_Alt))
                    done = true;

                const rawKey = event.key;
                const normalizedKey = normalizeModifier(rawKey);
                console.debug("event.nativeScanCode:", event.nativeScanCode, ",event.text:", event.text, ",normalizedKey:", normalizedKey);
                if (normalizedKey === "")
                    done = false;

                if (done) {
                    var data = {
                        "key": normalizedKey,
                        "code": event.nativeScanCode,
                        "text": event.text,
                        "isModifier": done,
                        "needModify": false,
                        "isKeyPadKey": false
                    };
                    currentKeys.push(data);
                }
                control.focus = false;
            }
            onFocusChanged: {
                if (!focus) {
                    unblockSystemShortcut();
                    handleFocusLost();
                } else {
                    blockSystemShortcut();
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    control.forceActiveFocus();
                    currentKeys = [];
                    shortcutString = "";
                }
            }

        }

    }

    // 冲突弹窗组件
    Component {
        id: conflictDialogComponent

        ShortcutConflictWarning {
            id: conflictDialog
        }

    }

}
