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

#include "shortcutcontroller.h"
#include <QDBusPendingReply>
#include <QDebug>
#include "fcitxdbusprovider.h"
#include "kglobalacceldbusproxy.h"

ShortcutController::ShortcutController(FcitxDbusProvider *provider,
                                       QObject *parent)
    : QObject(parent), m_fcitxDbus(provider) {
    registerShortcutDBusTypes();
    m_kglobalAccelDbus = std::make_unique<KGlobalAccelDbusProxy>(this);
}

ShortcutController::~ShortcutController() = default;

void ShortcutController::init() {
    getSystemShortcuts();
    getWindowShortcuts();
    getFcitxShortcuts();
}

QVariantMap
ShortcutController::checkShortcutConflict(const QString &shortcut,
                                          const QString &uniqueName) const {
    qDebug() << __func__ << shortcut << "uniqueName:" << uniqueName;

    QVariantMap result;
    result["hasConflict"] = false;
    result["shortcut"] = shortcut;

    if (shortcut.isEmpty()) {
        return result;
    }

    auto sequencedShortcut = parseFcitxShortcut(shortcut);
    auto deduplicatedKey = deduplicateFcitxKeys(shortcut);
    qDebug() << __func__ << "sequencedShortcut:" << sequencedShortcut
             << "isEmpty():" << sequencedShortcut.isEmpty()
             << "toString():" << sequencedShortcut.toString()
             << "deduplicatedKey:" << deduplicatedKey;

    // 检查系统快捷键和窗口快捷键冲突
    bool hasSystemOrWindowConflict = checkSystemShortcutsConflict(sequencedShortcut, uniqueName, result) ||
                                    checkWindowShortcutsConflict(sequencedShortcut, uniqueName, result);

    // 如果没有系统或窗口快捷键冲突，检查fcitx快捷键冲突
    if (!hasSystemOrWindowConflict) {
        // 先检查临时映射中的冲突，再检查真实映射
        if (!checkFcitxShortcutsConflictWithTempMap(deduplicatedKey, uniqueName, result)) {
            checkFcitxShortcutsConflict(deduplicatedKey, uniqueName, result);
        }
    }

    result["normalizedShortcut"] = deduplicatedKey;
    qDebug() << __func__ << "check result:" << result["hasConflict"];

    return result;
}

void ShortcutController::blockSystemShortcut() {
    if (m_kglobalAccelDbus == nullptr) {
        qWarning() << __func__ << "m_kglobalAccelDbus is null";
        return;
    }
    m_kglobalAccelDbus->blockGlobalShortcuts();
}

void ShortcutController::unblockSystemShortcut() {
    if (m_kglobalAccelDbus == nullptr) {
        qWarning() << "m_kglobalAccelDbus is null";
        return;
    }
    m_kglobalAccelDbus->unblockGlobalShortcuts();
}

void ShortcutController::clearAllShortcuts() {
    m_systemShortcuts.clear();
    m_windowShortcuts.clear();
    m_fcitxShortcuts.clear();
}

QString
ShortcutController::keySequenceToDisplayText(const QKeySequence &keySequence) const {
    return keySequence.toString(QKeySequence::NativeText);
}

QKeySequence ShortcutController::parseFcitxShortcut(const QString &shortcut) const {
    if (shortcut.isEmpty()) {
        return QKeySequence();
    }

    QString normalized = deduplicateFcitxKeys(shortcut);
    QKeySequence keySequence(normalized.replace(' ', '+'));
    if (keySequence.isEmpty() || keySequence.toString().isEmpty()) {
        qWarning() << __func__ << "can not parse fcitx shortcut:" << shortcut
                   << "normalized:" << normalized;
        return QKeySequence();
    }
    return keySequence;
}

void ShortcutController::getSystemShortcuts() {
    if (m_kglobalAccelDbus == nullptr) {
        qWarning() << __func__ << "m_kglobalAccelDbus is null";
        return;
    }

    auto call = m_kglobalAccelDbus->getSystemShortcuts();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ShortcutController::fetchSystemShortcutsFinished);
}

void ShortcutController::getWindowShortcuts() {
    if (m_kglobalAccelDbus == nullptr) {
        return;
    }

    auto call = m_kglobalAccelDbus->getWindowShortcuts();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ShortcutController::fetchWindowShortcutsFinished);
}

void ShortcutController::getFcitxShortcuts() {
    if (m_fcitxDbus == nullptr) {
        qWarning() << __func__ << "m_fcitxDbus is null";
        return;
    }
    auto call = m_fcitxDbus->dbusProxy()->GetAllHotkeys();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ShortcutController::fetchFcitxShortcutsFinished);
}

void ShortcutController::fetchSystemShortcutsFinished(
    QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<KGlobalAccelShortcutInfoList> reply = *watcher;

    if (reply.isError()) {
        qWarning() << __func__ << "reply error:" << reply.error().message();
        return;
    }

    auto shortcuts = reply.value();
    qDebug() << __func__ << "shortcuts count:" << shortcuts.size();
    m_systemShortcuts.clear();
    for (auto &info : shortcuts) {
        for (const auto &keyCode : info.defaultKeys()) {
            QKeySequence keySequence(keyCode);
            if (!keySequence.isEmpty()) {
                UnifiedShortcutInfo detailInfo;
                detailInfo.name = info.componentUniqueName();
                detailInfo.description = info.description();
                detailInfo.source = m_systemSourceName;
                detailInfo.keySequence = keySequence;
                detailInfo.displayText = keySequenceToDisplayText(keySequence);
                m_systemShortcuts.insert({keySequence, detailInfo});
                //                qDebug() << __func__ << "info:" <<
                //                info.description()
                //                         << "keyCode:" << keyCode << "->" <<
                //                         keySequence.toString();
            }
        }
    }
}

void ShortcutController::fetchWindowShortcutsFinished(
    QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<KGlobalAccelShortcutInfoList> reply = *watcher;

    if (reply.isError()) {
        qWarning() << __func__ << "reply error:" << reply.error().message();
        return;
    }

    auto shortcuts = reply.value();
    qDebug() << __func__ << "shortcuts count:" << shortcuts.size();
    m_windowShortcuts.clear();
    for (auto &info : shortcuts) {
        for (const auto &keyCode : info.defaultKeys()) {
            QKeySequence keySequence(keyCode);
            if (!keySequence.isEmpty() || !keySequence.toString().isEmpty()) {
                UnifiedShortcutInfo detailInfo;
                detailInfo.name = info.componentUniqueName();
                detailInfo.description = info.shortcutKey();
                detailInfo.source = m_windowSourceName;
                detailInfo.keySequence = keySequence;
                detailInfo.displayText = keySequenceToDisplayText(keySequence);
                m_windowShortcuts.insert({keySequence, detailInfo});

                //                qDebug() << __func__ << "info:" <<
                //                info.shortcutKey()
                //                         << "keyCode:" << keyCode << "->" <<
                //                         keySequence.toString();
            }
        }
    }
}

void ShortcutController::fetchFcitxShortcutsFinished(
    QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<FcitxQtHotkeyInfoList> reply = *watcher;

    if (reply.isError()) {
        qWarning() << __func__
                   << "fcitx reply error:" << reply.error().message();
        return;
    }

    auto hotkeys = reply.value();
    qDebug() << __func__ << "fcitx shortcuts count:" << hotkeys.size();
    m_fcitxShortcuts.clear();
    for (auto &hotkey : hotkeys) {
        FcitxShortcutInfo fcitxInfo;
        fcitxInfo.uniqueName = hotkey.category() == "InputMethod" ?  hotkey.name() + "_" + hotkey.uniqueName() : hotkey.uniqueName();
        fcitxInfo.description = hotkey.description();
        fcitxInfo.category = hotkey.category();
        qDebug() << __func__ << "init map:" << fcitxInfo.uniqueName << hotkey.description();
        for (const auto &keyCode : hotkey.keys()) {
            auto deduplicatedKey = deduplicateFcitxKeys(keyCode);
            QKeySequence keySequence = parseFcitxShortcut(keyCode);
            QString displayText = keySequence.toString().isEmpty()
                                      ? keyCode
                                      : keySequenceToDisplayText(keySequence);
            fcitxInfo.shortcuts.append(displayText);
            fcitxInfo.deduplicatedKeys.append(deduplicatedKey);
            fcitxInfo.displayTexts.append(displayText);
            qDebug() << __func__ << ":" << fcitxInfo.uniqueName << hotkey.description()
                     << "keyCode:" << keyCode << "->" << keySequence.toString()
                     << "deduplicatedKey:" << deduplicatedKey;
        }

        m_fcitxShortcuts[fcitxInfo.uniqueName] = fcitxInfo;
    }
    emit fcitxShortcutsUpdated();
}

QString ShortcutController::normalizeQtKeySequence(const QString &shortcut) const {
    if (shortcut.isEmpty()) {
        return shortcut;
    }

    QString normalized = shortcut;
    normalized.replace("+", " ", Qt::CaseInsensitive);

    // 标准化修饰键名称
    normalized.replace("Control_L", "Ctrl", Qt::CaseInsensitive);
    normalized.replace("Control_R", "Ctrl", Qt::CaseInsensitive);
    normalized.replace("Control", "Ctrl", Qt::CaseInsensitive);
    normalized.replace("Alt_L", "Alt", Qt::CaseInsensitive);
    normalized.replace("Alt_R", "Alt", Qt::CaseInsensitive);
    normalized.replace("Shift_L", "Shift", Qt::CaseInsensitive);
    normalized.replace("Shift_R", "Shift", Qt::CaseInsensitive);
    normalized.replace("Meta_L", "Meta", Qt::CaseInsensitive);
    normalized.replace("Meta_R", "Meta", Qt::CaseInsensitive);
    normalized.replace("Meta", "Meta", Qt::CaseInsensitive);
    normalized.replace("Super", "Meta", Qt::CaseInsensitive);
    normalized.replace("Super_L", "Meta", Qt::CaseInsensitive);
    normalized.replace("Super_R", "Meta", Qt::CaseInsensitive);

    // 标准化数字键盘键
    normalized.replace("KP_0", "0", Qt::CaseInsensitive);
    normalized.replace("KP_1", "1", Qt::CaseInsensitive);
    normalized.replace("KP_2", "2", Qt::CaseInsensitive);
    normalized.replace("KP_3", "3", Qt::CaseInsensitive);
    normalized.replace("KP_4", "4", Qt::CaseInsensitive);
    normalized.replace("KP_5", "5", Qt::CaseInsensitive);
    normalized.replace("KP_6", "6", Qt::CaseInsensitive);
    normalized.replace("KP_7", "7", Qt::CaseInsensitive);
    normalized.replace("KP_8", "8", Qt::CaseInsensitive);
    normalized.replace("KP_9", "9", Qt::CaseInsensitive);
    normalized.replace("KP_Add", "+", Qt::CaseInsensitive);
    normalized.replace("KP_Subtract", "-", Qt::CaseInsensitive);
    normalized.replace("KP_Multiply", "*", Qt::CaseInsensitive);
    normalized.replace("KP_Divide", "/", Qt::CaseInsensitive);
    normalized.replace("KP_Decimal", ".", Qt::CaseInsensitive);
    normalized.replace("KP_Enter", "Enter", Qt::CaseInsensitive);
    normalized.replace("KP_Equal", "=", Qt::CaseInsensitive);
    normalized.replace("KP_Separator", ",", Qt::CaseInsensitive);
    normalized.replace("KP_Left", "Left", Qt::CaseInsensitive);
    normalized.replace("KP_Right", "Right", Qt::CaseInsensitive);
    normalized.replace("KP_Up", "Up", Qt::CaseInsensitive);
    normalized.replace("KP_Down", "Down", Qt::CaseInsensitive);
    normalized.replace("KP_Page_Up", "PageUp", Qt::CaseInsensitive);
    normalized.replace("Next", "PageDown", Qt::CaseInsensitive);
    normalized.replace("KP_Home", "Home", Qt::CaseInsensitive);
    normalized.replace("KP_End", "End", Qt::CaseInsensitive);
    normalized.replace("KP_Insert", "Insert", Qt::CaseInsensitive);
    normalized.replace("KP_Delete", "Delete", Qt::CaseInsensitive);

    // 标准化符号键
    normalized.replace("asciitilde", "~", Qt::CaseInsensitive);
    normalized.replace("exclam", "!", Qt::CaseInsensitive);
    normalized.replace("at", "@", Qt::CaseInsensitive);
    normalized.replace("numbersign", "#", Qt::CaseInsensitive);
    normalized.replace("dollar", "$", Qt::CaseInsensitive);
    normalized.replace("percent", "%", Qt::CaseInsensitive);
    normalized.replace("asciicircum", "^", Qt::CaseInsensitive);
    normalized.replace("ampersand", "&", Qt::CaseInsensitive);
    normalized.replace("asterisk", "*", Qt::CaseInsensitive);
    normalized.replace("parenleft", "(", Qt::CaseInsensitive);
    normalized.replace("parenright", ")", Qt::CaseInsensitive);
    normalized.replace("minus", "-", Qt::CaseInsensitive);
    normalized.replace("underscore", "_", Qt::CaseInsensitive);
    normalized.replace("equal", "=", Qt::CaseInsensitive);
    normalized.replace("plus", "+", Qt::CaseInsensitive);
    normalized.replace("bracketleft", "[", Qt::CaseInsensitive);
    normalized.replace("bracketright", "]", Qt::CaseInsensitive);
    normalized.replace("braceleft", "{", Qt::CaseInsensitive);
    normalized.replace("braceright", "}", Qt::CaseInsensitive);
    normalized.replace("backslash", "\\", Qt::CaseInsensitive);
    normalized.replace("bar", "|", Qt::CaseInsensitive);
    normalized.replace("semicolon", ";", Qt::CaseInsensitive);
    normalized.replace("colon", ":", Qt::CaseInsensitive);
    normalized.replace("quotedbl", "\"", Qt::CaseInsensitive);
    normalized.replace("apostrophe", "'", Qt::CaseInsensitive);
    normalized.replace("comma", ",", Qt::CaseInsensitive);
    normalized.replace("period", ".", Qt::CaseInsensitive);
    normalized.replace("slash", "/", Qt::CaseInsensitive);
    normalized.replace("question", "?", Qt::CaseInsensitive);
    normalized.replace("less", "<", Qt::CaseInsensitive);
    normalized.replace("greater", ">", Qt::CaseInsensitive);
    normalized.replace("grave", "`", Qt::CaseInsensitive);

    // 标准化功能键
    normalized.replace("F1", "F1", Qt::CaseInsensitive);
    normalized.replace("F2", "F2", Qt::CaseInsensitive);
    normalized.replace("F3", "F3", Qt::CaseInsensitive);
    normalized.replace("F4", "F4", Qt::CaseInsensitive);
    normalized.replace("F5", "F5", Qt::CaseInsensitive);
    normalized.replace("F6", "F6", Qt::CaseInsensitive);
    normalized.replace("F7", "F7", Qt::CaseInsensitive);
    normalized.replace("F8", "F8", Qt::CaseInsensitive);
    normalized.replace("F9", "F9", Qt::CaseInsensitive);
    normalized.replace("F10", "F10", Qt::CaseInsensitive);
    normalized.replace("F11", "F11", Qt::CaseInsensitive);
    normalized.replace("F12", "F12", Qt::CaseInsensitive);

    // 标准化特殊键
    normalized.replace("Return", "Enter", Qt::CaseInsensitive);
    normalized.replace("Tab", "Tab", Qt::CaseInsensitive);
    normalized.replace("Escape", "Esc", Qt::CaseInsensitive);
    normalized.replace("BackSpace", "Backspace", Qt::CaseInsensitive);
    normalized.replace("Space", "Space", Qt::CaseInsensitive);
    normalized.replace("Scroll_Lock", "ScrollLock", Qt::CaseInsensitive);
    normalized.replace("Num_Lock", "NumLock", Qt::CaseInsensitive);
    return normalized;
}

QString ShortcutController::beautifyQtKeyString(const QString &shortcut) const {
    if (shortcut.isEmpty()) {
        return shortcut;
    }

    QString beautified = shortcut;
    beautified.replace("Meta_L", "Super", Qt::CaseInsensitive);
    beautified.replace("Meta_R", "Super", Qt::CaseInsensitive);
    beautified.replace("Meta", "Super", Qt::CaseInsensitive);
    return beautified;
}

QString ShortcutController::deduplicateFcitxKeys(const QString &shortcut) const {
    if (shortcut.isEmpty()) {
        return shortcut;
    }

    QString normalized = normalizeQtKeySequence(shortcut);
    QStringList parts = normalized.split(" ", Qt::KeepEmptyParts);
    QSet<QString> uniqueKeys;
    QStringList deduplicatedParts;

    for (const QString &part : parts) {
        QString trimmedPart = part.trimmed();
        if (!trimmedPart.isEmpty() && !uniqueKeys.contains(trimmedPart)) {
            uniqueKeys.insert(trimmedPart);
            deduplicatedParts.append(trimmedPart);
        }
    }
    return deduplicatedParts.join(" ");
}

bool ShortcutController::checkSystemShortcutsConflict(
    const QKeySequence &keySeq, const QString &uniqueName,
    QVariantMap &result) const {
    Q_UNUSED(uniqueName);
    auto range = m_systemShortcuts.equal_range(keySeq);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &info = it->second;
        ConflictInfo conflictInfo(info.name, info.description, info.source,
                                 info.displayText, beautifyQtKeyString(info.keySequence.toString()).replace("+", " "));
        setConflictInfo(result, conflictInfo);
        qDebug() << __func__ << "found [" << info.source
                 << "] conflict:" << info.name << "(" << info.source << ")"
                 << "->" << info.displayText;
        return true;
    }
    return false;
}

bool ShortcutController::checkWindowShortcutsConflict(
    const QKeySequence &keySeq, const QString &uniqueName,
    QVariantMap &result) const {
    Q_UNUSED(uniqueName);
    auto range = m_windowShortcuts.equal_range(keySeq);
    for (auto it = range.first; it != range.second; ++it) {
        const auto &info = it->second;
        ConflictInfo conflictInfo(info.name, info.description, info.source,
                                 info.displayText, beautifyQtKeyString(info.keySequence.toString()).replace("+", " "));
        setConflictInfo(result, conflictInfo);
        qDebug() << __func__ << "found [" << info.source
                 << "] conflict:" << info.name << "(" << info.source << ")"
                 << "->" << info.displayText;
        return true;
    }
    return false;
}

bool ShortcutController::checkFcitxShortcutsConflict(const QString &key,
                                                     const QString &uniqueName,
                                                     QVariantMap &result) const {
    for (const auto &pair : m_fcitxShortcuts) {
        const auto &fcitxInfo = pair.second;

        // 跳过自己
        if (!uniqueName.isEmpty() && fcitxInfo.uniqueName == uniqueName) {
            continue;
        }

        for (const auto &deduplicatedKey : fcitxInfo.deduplicatedKeys) {
            // 跳过空字符串，空字符串不参与冲突检测
            if (!deduplicatedKey.isEmpty() && deduplicatedKey == key) {
                ConflictInfo conflictInfo(fcitxInfo.uniqueName, fcitxInfo.description,
                                         fcitxInfo.category, key, key);
                setConflictInfo(result, conflictInfo);
                qDebug() << __func__
                         << "found [fcitx] conflict:" << "key:" << key << fcitxInfo.uniqueName
                         << fcitxInfo.description << "->"
                         << fcitxInfo.displayTexts;
                return true;
            }
        }
    }
    return false;
}

bool ShortcutController::checkFcitxShortcutsConflictWithTempMap(
    const QString &key, const QString &uniqueName, QVariantMap &result) const {
    for (const auto &pair : m_tempFcitxShortcuts) {
        const auto &tempFcitxInfo = pair.second;

        // 跳过自己
        if (!uniqueName.isEmpty() && tempFcitxInfo.uniqueName == uniqueName) {
            continue;
        }

        for (const auto &deduplicatedKey : tempFcitxInfo.deduplicatedKeys) {
            // 跳过空字符串，空字符串不参与冲突检测
            if (!deduplicatedKey.isEmpty() && deduplicatedKey == key) {
                ConflictInfo conflictInfo(tempFcitxInfo.uniqueName, tempFcitxInfo.description,
                                         tempFcitxInfo.category, key, key);
                setConflictInfo(result, conflictInfo);
                qDebug() << __func__ << "found [temp fcitx] conflict:" << "key:" << key
                         << tempFcitxInfo.description << "->" << key;
                return true;
            }
        }
    }
    return false;
}

void ShortcutController::updateFcitxShortcutMap(const QString &name,
                                                const QString &oldShortcut,
                                                const QString &newShortcut) {
    auto it = m_fcitxShortcuts.find(name);
    if (it == m_fcitxShortcuts.end()) {
        qDebug() << __func__ << "can not find fcitx name:" << name;
        return;
    }

    FcitxShortcutInfo &fcitxInfo = it->second;
    qDebug() << __func__ << name << "uniqueName:" << fcitxInfo.uniqueName << "from:" << oldShortcut
             << "to:" << newShortcut  << "shortcuts" << fcitxInfo.shortcuts;

    // 如果新快捷键为空，将第一个元素设置为空字符串
    if (newShortcut.isEmpty()) {
        if (!fcitxInfo.shortcuts.isEmpty()) {
            fcitxInfo.shortcuts[0] = "";
            fcitxInfo.deduplicatedKeys[0] = "";
            fcitxInfo.displayTexts[0] = "";
            qDebug() << __func__ << "clear finished new info:" << name << "shortcuts:"
                     << fcitxInfo.shortcuts << "deduplicatedKeys:" << fcitxInfo.deduplicatedKeys;
        }
        return;
    }

    QString newDeduplicatedKey = deduplicateFcitxKeys(newShortcut);
    QKeySequence keySequence = parseFcitxShortcut(newShortcut);
    QString displayText = keySequence.toString().isEmpty()
                              ? newShortcut
                              : keySequenceToDisplayText(keySequence);

    // 如果列表为空，添加新元素；否则更新第一个元素
    if (fcitxInfo.shortcuts.isEmpty()) {
        fcitxInfo.shortcuts.append(newShortcut);
        fcitxInfo.deduplicatedKeys.append(newDeduplicatedKey);
        fcitxInfo.displayTexts.append(displayText);
        qDebug() << __func__ << "add fcitx shortcut first element:" << name
                 << "->" << newDeduplicatedKey;
    } else {
        fcitxInfo.shortcuts[0] = newShortcut;
        fcitxInfo.deduplicatedKeys[0] = newDeduplicatedKey;
        fcitxInfo.displayTexts[0] = displayText;
        qDebug() << __func__ << "update fcitx shortcut first element:" << name
                 << "->" << newDeduplicatedKey;
    }

    qDebug() << __func__ << "fcitx shortcut map update complete size:"
             << m_fcitxShortcuts.size() << "shortcuts:" << fcitxInfo.shortcuts;
}

void ShortcutController::updateTempFcitxShortcutMap(const QString &name,
                                                   const QString &oldShortcut,
                                                   const QString &newShortcut) {
    // 从真实映射中复制原始数据
    auto realIt = m_fcitxShortcuts.find(name);
    if (realIt == m_fcitxShortcuts.end()) {
        qWarning() << __func__ << "can not find fcitx name in real map:" << name;
        return;
    }

    // 如果临时映射中不存在，则复制真实映射的数据
    auto tempIt = m_tempFcitxShortcuts.find(name);
    if (tempIt == m_tempFcitxShortcuts.end()) {
        m_tempFcitxShortcuts[name] = realIt->second;
        tempIt = m_tempFcitxShortcuts.find(name);
    }

    FcitxShortcutInfo &tempFcitxInfo = tempIt->second;

    qDebug() << __func__ << "temp update:" << name << "from:" << oldShortcut
             << "to:" << newShortcut << "shortcuts" << tempFcitxInfo.shortcuts;
    // 如果新快捷键为空，将第一个元素设置为空字符
    if (newShortcut.isEmpty()) {
        if (!tempFcitxInfo.shortcuts.isEmpty()) {
            tempFcitxInfo.shortcuts[0] = "";
            tempFcitxInfo.deduplicatedKeys[0] = "";
            tempFcitxInfo.displayTexts[0] = "";
            qDebug() << __func__ << "clear finished new info:" << name << "shortcuts:"
                     << tempFcitxInfo.shortcuts << "deduplicatedKeys:" << tempFcitxInfo.deduplicatedKeys;
        }
        return;
    }

    QString newDeduplicatedKey = deduplicateFcitxKeys(newShortcut);
    QKeySequence keySequence = parseFcitxShortcut(newShortcut);
    QString displayText = keySequence.toString().isEmpty()
                              ? newShortcut
                              : keySequenceToDisplayText(keySequence);

    // 如果列表为空，添加新元素；否则更新第一个元素
    if (tempFcitxInfo.shortcuts.isEmpty()) {
        tempFcitxInfo.shortcuts.append(newShortcut);
        tempFcitxInfo.deduplicatedKeys.append(newDeduplicatedKey);
        tempFcitxInfo.displayTexts.append(displayText);
        qDebug() << __func__ << "add temp fcitx shortcut first element:" << name
                 << "->" << newDeduplicatedKey;
    } else {
        tempFcitxInfo.shortcuts[0] = newShortcut;
        tempFcitxInfo.deduplicatedKeys[0] = newDeduplicatedKey;
        tempFcitxInfo.displayTexts[0] = displayText;
        qDebug() << __func__ << "update temp fcitx shortcut first element:" << name
                 << "->" << newDeduplicatedKey;
    }

    qDebug() << __func__ << "temp fcitx shortcut map update complete size:"
             << m_tempFcitxShortcuts.size() << "shortcuts:" << tempFcitxInfo.shortcuts;
}

void ShortcutController::clearTempFcitxShortcutMap() {
    qDebug() << __func__ << "clearing temp fcitx shortcut map, size:" << m_tempFcitxShortcuts.size();
    m_tempFcitxShortcuts.clear();
}

void ShortcutController::mergeTempFcitxShortcutMap() {
    qDebug() << __func__ << "merging temp fcitx shortcut map, size:" << m_tempFcitxShortcuts.size();

    for (const auto &pair : m_tempFcitxShortcuts) {
        const QString &name = pair.first;
        const FcitxShortcutInfo &tempInfo = pair.second;
        auto realIt = m_fcitxShortcuts.find(name);
        if (realIt != m_fcitxShortcuts.end()) {
            // 更新真实映射
            realIt->second = tempInfo;
            qDebug() << __func__ << "merged:" << name;
        }
    }
    // 清空临时映射
    m_tempFcitxShortcuts.clear();
}

void ShortcutController::setConflictInfo(QVariantMap &result, const ConflictInfo &conflictInfo) const {
    result["conflict"] = QVariantMap{
        {"name", conflictInfo.name},
        {"description", conflictInfo.description},
        {"source", conflictInfo.source},
        {"displayText", conflictInfo.displayText},
        {"shortcut", conflictInfo.shortcut}
    };
    result["hasConflict"] = true;
}
