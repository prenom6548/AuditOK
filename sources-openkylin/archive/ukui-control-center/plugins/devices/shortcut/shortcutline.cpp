/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <QDebug>
#include <QKeySequence>
#include <unistd.h>
#include <QApplication>
#include <QTimer>
#include <QEvent>
#include <QGSettings>
#include "shortcutline.h"
#include "clickfixlabel.h"

void updateShortcutFont(QWidget *widget, const QString &key, bool refresh)
{
    if (key == "systemFont" || key == "systemFontSize") {
        widget->setFont(qApp->font());
        if (refresh) {
            widget->update();
        }
    }
}

#define SNULL  "NULL"
#define SCTRL  "Ctrl"
#define SALT   "Alt"
#define SSHIFT "Shift"

int allowKey[] = { // 0-9  -/+ A-Z ,/.
                Qt::Key_0,     Qt::Key_1,     Qt::Key_2,     Qt::Key_3,
                Qt::Key_4,     Qt::Key_5,     Qt::Key_6,     Qt::Key_7,
                Qt::Key_8,     Qt::Key_9,     Qt::Key_Minus, Qt::Key_Equal,
                Qt::Key_A,     Qt::Key_B,     Qt::Key_C,     Qt::Key_D,
                Qt::Key_E,     Qt::Key_F,     Qt::Key_G,     Qt::Key_H,
                Qt::Key_I,     Qt::Key_J,     Qt::Key_K,     Qt::Key_L,
                Qt::Key_M,     Qt::Key_N,     Qt::Key_O,     Qt::Key_P,
                Qt::Key_Q,     Qt::Key_R,     Qt::Key_S,     Qt::Key_T,
                Qt::Key_U,     Qt::Key_V,     Qt::Key_W,     Qt::Key_X,
                Qt::Key_Y,     Qt::Key_Z,     Qt::Key_Comma, Qt::Key_Period,
                Qt::Key_Print, Qt::Key_Escape, Qt::Key_Delete, Qt::Key_Left,
                Qt::Key_Up, Qt::Key_Right, Qt::Key_Down, Qt::Key_Home, Qt::Key_Tab,
                Qt::Key_F1, Qt::Key_F2, Qt::Key_F3, Qt::Key_F4, Qt::Key_F5, Qt::Key_F6,
                Qt::Key_F7, Qt::Key_F8, Qt::Key_F9, Qt::Key_F10, Qt::Key_F11, Qt::Key_F12

};

int numKey[] = {
    Qt::Key_0,     Qt::Key_1,     Qt::Key_2,     Qt::Key_3,
    Qt::Key_4,     Qt::Key_5,     Qt::Key_6,     Qt::Key_7,
    Qt::Key_8,     Qt::Key_9,     Qt::Key_Minus, Qt::Key_Equal,
    Qt::Key_Period
};


ShortcutLine::ShortcutLine(QList<KeyEntry> generalEntries,
                           QList<KeyEntry> windowEntries,
                           QList<KeyEntry> customEntries, QWidget *parent) :
                           QLineEdit(parent),
                           systemEntry(generalEntries),
                           m_windowEntry(windowEntries),
                           customEntry(customEntries) {
    initInterface();
    initInputKeyAndText(true);
    setFont(qApp->font());
    QGSettings *fontSetting = new QGSettings(QByteArray("org.ukui.style"), QByteArray(), this);
    connect(fontSetting, &QGSettings::changed, this, [=](const QString &key) {
        updateShortcutFont(this, key);
    });
}


ShortcutLine::~ShortcutLine()
{

}

void ShortcutLine::initInputKeyAndText(const bool &clearText)
{
    firstKey  = SNULL;
    secondKey = SNULL;
    thirdKey  = SNULL;
    forthKey  = SNULL;
    if (true == clearText) {
        // 用基类 setText，避免 DoubleClickShortCut 把 validShortcut 一并清掉
        QLineEdit::setText(QString());
        shortCutObtainedFlag = false;
    }
}

void ShortcutLine::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat()) {  //一直按着导致触发的事件，不再处理
        return;
    }
    if (isShortCutObtained == true) {
        initInputKeyAndText(true);
        isShortCutObtained = false;
    }
    int keyValue = e->key();
    int keyCode = e->nativeVirtualKey();

    // 尚未开始录入：Esc 退出编辑；Tab 离开编辑框（勿当作快捷键主键）
    if (firstKey == SNULL) {
        auto focusFixOrNext = [this](bool next, bool onlyFix) {
            // 常驻编辑框（与添加对话框一致）：直接在焦点链上移动，不切灰框
            if (property("ukccAlwaysShowEdit").toBool()) {
                clearFocus();
                if (!onlyFix) {
                    QTimer::singleShot(0, this, [this, next]() {
                        focusNextPrevChild(next);
                    });
                }
                return;
            }
            ClickFixLabel *fix = qobject_cast<ClickFixLabel *>(
                        property("ukccFixLabel").value<QWidget *>());
            clearFocus();
            if (!fix) {
                return;
            }
            if (onlyFix) {
                fix->setFocus(Qt::OtherFocusReason);
            } else {
                QTimer::singleShot(0, fix, [fix, next]() {
                    fix->moveFocus(next);
                });
            }
        };
        if (keyValue == Qt::Key_Escape) {
            focusFixOrNext(true, true);
            return;
        }
        if (keyValue == Qt::Key_Tab || keyValue == Qt::Key_Backtab) {
            focusFixOrNext(keyValue != Qt::Key_Backtab, false);
            return;
        }
    }

    qDebug() << "0x" << QString().sprintf("%04X", keyValue);
    qDebug() << "keyCode = = = = =  =" << keyCode << "    " << keyValue;

    if (keyValue == Qt::Key_Meta && e->modifiers() != Qt::MetaModifier) { // bug#194489
        if (keyCode >= 65511 && keyCode <= 65514) {
            keyValue = Qt::Key_Alt;
        }
    }
    if (firstKey == SNULL) {
        firstKey = keyToString(keyValue);
        if (keyValue == Qt::Key_Control || keyValue == Qt::Key_Alt \
                || keyValue == Qt::Key_Shift || keyValue == Qt::Key_Meta || keyValue == Qt::Key_Print
                || keyValue == Qt::Key_Super_L) {
            firstKey.replace("Start", "Super");
            if (keyValue == Qt::Key_Print) {
                this->setText(firstKey);
                shortCutObtained(true, 1);
            } else {
                this->setText(firstKey + QString("   "));
            }
        } else {                       //第一个键不是三个辅助键中的其中一个
            this->setText(firstKey);  //显示一下，增强用户交互性
            shortCutObtained(false);  //快捷键获取失败
            return;
        }
    } else if(secondKey == SNULL) {
        /*第二个键是辅助键中的另外一个*/
        if ((keyValue == Qt::Key_Control || keyValue == Qt::Key_Alt || \
             keyValue == Qt::Key_Shift || keyValue == Qt::Key_Meta || keyValue == Qt::Key_Print
             || keyValue == Qt::Key_Super_L) &&
                keyToString(keyValue) != firstKey) {
            secondKey = keyToString(keyValue);
            this->setText(firstKey + QString("   ") + secondKey + QString("   "));
        } else {  //第二个键是主键(最后一个键)
            if (lastKeyIsAvailable(keyValue, keyCode)) {   // 合法
                secondKey = keyToString(keyValue);
                shortCutObtained(true, 2);
            } else {                             //非法
                secondKey = keyToString(keyValue);
                shortCutObtained(false);
                return;
            }
        }
    } else if (thirdKey == SNULL) {
        /*第三个键是辅助键中的另外一个*/
        if ((keyValue == Qt::Key_Control || keyValue == Qt::Key_Alt || \
             keyValue == Qt::Key_Shift || keyValue == Qt::Key_Meta || keyValue == Qt::Key_Print
             || keyValue == Qt::Key_Super_L) &&
                keyToString(keyValue) != firstKey) {
            thirdKey = keyToString(keyValue);
            this->setText(firstKey + QString("   ") + secondKey + QString("   ") + thirdKey + QString("   "));
        } else {  //第三个键是主键(最后一个键)
            if (lastKeyIsAvailable(keyValue, keyCode)) {   // 合法
                thirdKey = keyToString(keyValue);
                shortCutObtained(true, 3);
            } else {                             //非法
                shortCutObtained(false);
                return;
            }
        }

    } else if (forthKey == SNULL) { //第四个键只能是主键
        if (lastKeyIsAvailable(keyValue, keyCode)) {   // 合法
            forthKey = keyToString(keyValue);
            shortCutObtained(true, 4);
        } else {                             //非法
            shortCutObtained(false);
        }
    }

}

void ShortcutLine::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat()) {  //一直按着导致触发的事件
        return;
    }
    // Tab 切入编辑后松开 Tab 也会进这里；尚未开始录入时不能清空已有快捷键
    if (firstKey == SNULL && !shortCutObtainedFlag) {
        return;
    }
    if (shortCutObtainedFlag) { //快捷键输入完毕
        initInputKeyAndText(false);
    } else {                           //快捷键输入放弃
        initInputKeyAndText(true);
    }
}

void ShortcutLine::setConflictInfo(QString key, QString value) {
    m_conflictKey = key;
    m_conflictValue = value;
}

void ShortcutLine::setShortCutObtainedFlag(bool flag) {
    shortCutObtainedFlag = flag;
}

void ShortcutLine::focusInEvent(QFocusEvent *e)
{
    mKglobalIfc->call("blockGlobalShortcuts", true);
    QLineEdit::focusInEvent(e);
    initInputKeyAndText(false);
    // Tab/鼠标进入：保留原快捷键，进入选中态便于重录与读屏
    if (!text().isEmpty()
            && (e->reason() == Qt::TabFocusReason
                || e->reason() == Qt::BacktabFocusReason
                || e->reason() == Qt::MouseFocusReason
                || e->reason() == Qt::ShortcutFocusReason)) {
        selectAll();
    }
}

void ShortcutLine::focusOutEvent(QFocusEvent *e)
{
    mKglobalIfc->call("blockGlobalShortcuts", false);
    QLineEdit::focusOutEvent(e);
}

void ShortcutLine::setAddDialogEdit(bool addDialog) {
    m_addDialogEdit = addDialog;
}

void ShortcutLine::shortCutObtained(const bool &flag, const int &keyNum)
{
    isShortCutObtained = true;
    qDebug() << Q_FUNC_INFO << flag << keyNum;
    bool isM900Type = UkccCommon::getCpuInfo().contains("PANGU M900", Qt::CaseInsensitive);
    if (true == flag && (1 == keyNum || 2 == keyNum || 3 == keyNum || 4 == keyNum)) {
        shortCutObtainedFlag = true;
        if (1 == keyNum) {
            this->setText(firstKey);
            if (firstKey == "PrtSc") {
                firstKey = "Print";
            }
            seq = QKeySequence(firstKey);
            qDebug() << Q_FUNC_INFO << firstKey << seq.toString();
        } else if (2 == keyNum) {
            this->setText(firstKey + QString("   ") + secondKey);
            if (secondKey == "PrtSc") {
                secondKey = "Print";
            }

            if (firstKey == "Super" || firstKey == "Start") {
                seq = QKeySequence("Meta" + QString("+") + secondKey);
            } else {
                seq = QKeySequence(firstKey + QString("+") + secondKey);
            }
            qDebug() << Q_FUNC_INFO << firstKey << secondKey << seq.toString() << m_addDialogEdit;
        } else if (3 == keyNum) {
            this->setText(firstKey + QString("   ") + secondKey + QString("   ") + thirdKey);
            if (thirdKey == "PrtSc") {
                thirdKey = "Print";
            }
            if (firstKey == "Super" || firstKey == "Start") {
                seq = QKeySequence("Meta" + QString("+") + secondKey + QString("+") + thirdKey);
            } else if (secondKey == "Super" || firstKey == "Start") {
                seq = QKeySequence(firstKey + QString("+") + "Meta" + QString("+") + thirdKey);
            } else {
                seq = QKeySequence(firstKey + QString("+") + secondKey + QString("+") + thirdKey);
            }
            qDebug() << Q_FUNC_INFO << firstKey << secondKey << thirdKey << seq.toString();
        } else if (4 == keyNum){
            this->setText(firstKey + QString("   ") + secondKey + QString("   ") + thirdKey + QString("   ") + forthKey);
            if (forthKey == "PrtSc") {
                forthKey = "Print";
            }
            if (firstKey == "Super" || firstKey == "Start") {
                seq = QKeySequence("Meta" + QString("+") + secondKey + QString("+") + thirdKey + QString("+") + forthKey);
            } else if (secondKey == "Super" || firstKey == "Start") {
                seq = QKeySequence(firstKey + QString("+") + "Meta" + QString("+") + thirdKey + QString("+") + forthKey);
            } else if (thirdKey == "Super" || firstKey == "Start") {
                seq = QKeySequence(firstKey + QString("+") + secondKey + QString("+") + "Meta" + QString("+") + forthKey);
            } else {
                seq = QKeySequence(firstKey + QString("+") + secondKey + QString("+") + thirdKey + QString("+") + forthKey);
            }
            qDebug() << Q_FUNC_INFO << firstKey << secondKey << thirdKey << forthKey << seq.toString();
        }
        if (!m_addDialogEdit) {
            emit conflictDetect(seq);
            qDebug() << Q_FUNC_INFO << "not add dialogEdit" << seq.toString();
            return;
        }
        if (conflictWithSystemShortcuts(seq) ||
                conflictWithWindowShortcuts(seq) ||
                conflictWithCustomShortcuts(seq)) { //快捷键冲突
            if (isM900Type) {
                QTimer::singleShot(100, [&]() {  // bug#215905
                    Q_EMIT shortCutAvailable(-2);
                });
            } else {
                Q_EMIT shortCutAvailable(-2);
            }
        } else if (conflictWithGlobalShortcuts(seq) || conflictWithStandardShortcuts(seq)) {
            shortCutObtainedFlag = false;
            initInputKeyAndText(true);
            if (isM900Type) {
                QTimer::singleShot(100, [&]() {  // bug#215905
                    Q_EMIT shortCutAvailable(-1);
                });
            } else {
                Q_EMIT shortCutAvailable(-1);
            }
        } else {
            Q_EMIT shortCutAvailable(0);
        }
    } else {  //快捷键无效
        if (firstKey == "Enter(main)" || firstKey == "Enter(num)") {
            Q_EMIT shortCutAvailable(4);
        } else {
            qDebug() << Q_FUNC_INFO << "illegal firstKey" << firstKey << "secondKey" << secondKey << "keyNum" << keyNum;
            if (!secondKey.isEmpty() && secondKey != "NULL") {
                m_conflictValue = m_conflictKey = firstKey + "+" + secondKey;
            } else {
                m_conflictValue = m_conflictKey = firstKey;
            }
            m_conflictValue = m_conflictValue.isEmpty() ? "Fn" : m_conflictValue;
            if (m_conflictValue == "Fn") {
                m_conflictKey = m_conflictValue;
            }
            shortCutObtainedFlag = false;
            initInputKeyAndText(true);
            Q_EMIT shortCutAvailable(-1);
        }
    }
}

bool ShortcutLine::lastKeyIsAvailable(const int &keyValue, const int &keyCode)
{

    for (u_int i = 0; i < sizeof(numKey) / sizeof(int); ++i) {
        if (keyValue == numKey[i] && keyValue != keyCode) {  //数字键盘上的
            return false;
        }
    }

    for (u_int i = 0; i < sizeof(allowKey) / sizeof(int); ++i) {
        if (keyValue == allowKey[i]) {
            return true;
        }
    }
    return false;
}

QKeySequence ShortcutLine::keySequence()
{
    return this->seq;
}

bool ShortcutLine::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
    QHash<QKeySequence, QList<KGlobalShortcutInfo> > clashing;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence keys(keySequence[i]);
        qDebug() << "全局快捷键冲突" << keySequence.count() << keySequence << keys;
        if (!KGlobalAccel::isGlobalShortcutAvailable(keySequence)) {
            clashing.insert(keySequence, KGlobalAccel::getGlobalShortcutsByKey(keys));
        }
    }

    if (clashing.isEmpty() || (clashing[keySequence].empty() && keySequence.toString() != "Alt+M")) {
        qDebug() << Q_FUNC_INFO << keySequence << "not conflict with Global Shortcuts key" << keySequence.toString();
        return false;
    } else {
        qDebug() << Q_FUNC_INFO << "conflict With Global Shortcuts key" << keySequence;
        if (clashing[keySequence].empty()) {
            m_conflictValue = "";
        } else {
            m_conflictValue = clashing[keySequence][0].friendlyName();
            qDebug() << Q_FUNC_INFO << "conflict With Global Shortcuts" << m_conflictValue;
        }
        m_conflictKey = keySequence.toString();
    }

    return true;
}

bool ShortcutLine::conflictWithStandardShortcuts(const QKeySequence &seq)
{
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(seq);
    if (ssc != KStandardShortcut::AccelNone) {
        qDebug() << "conflict With Standard Shortcuts" << seq;
        m_conflictValue = KStandardShortcut::label(ssc);
        m_conflictKey = seq.toString();
        return true;
    }
    return false;
}

bool ShortcutLine::conflictWithSystemShortcuts(const QKeySequence &seq) {
    QString systemKeyStr = keyToLib(seq.toString());
    if (systemKeyStr.contains("Meta")) {
        systemKeyStr.replace("Meta", "Super");
    }
    if (systemKeyStr.contains("Start")) {
        systemKeyStr.replace("Start", "Super");
    }

    if (systemKeyStr.contains("PrtSc", Qt::CaseInsensitive)) {
        systemKeyStr.replace("PrtSc", "Print", Qt::CaseInsensitive);
    }
    for (KeyEntry& ckeyEntry : systemEntry) {
        QString ckeyString = ckeyEntry.valueStr;
        if (ckeyString.contains("Control", Qt::CaseInsensitive)) {
            ckeyString.replace("Control", "Ctrl", Qt::CaseInsensitive);
        }

        if (ckeyString.contains("Start")) {
            ckeyString.replace("Start", "Super");
        }

        if (ckeyString.contains("Win")) {
            ckeyString.replace("Win", "Super");
        }

        if (ckeyString.contains("PrtSc", Qt::CaseInsensitive)) {
            ckeyString.replace("PrtSc", "Print", Qt::CaseInsensitive);
        }
        if (!systemKeyStr.compare(ckeyString, Qt::CaseInsensitive)) {
            qDebug() << "conflictWithSystemShortcuts" << seq;
            m_conflictValue.clear();
            m_conflictKey = ckeyEntry.keyStr;
            return true;
        }
    }

    return false;
}

bool ShortcutLine::conflictWithWindowShortcuts(const QKeySequence &seq) {
    QString windowKeyStr = seq.toString();
    if (windowKeyStr.contains("Meta")) {
        windowKeyStr.replace("Meta", "Super");
    }
    if (windowKeyStr.contains("Start")) {
        windowKeyStr.replace("Start", "Super");
    }

    if (windowKeyStr.contains("PrtSc", Qt::CaseInsensitive)) {
        windowKeyStr.replace("PrtSc", "Print", Qt::CaseInsensitive);
    }
    if (windowKeyStr.contains("↑")) {
        windowKeyStr.replace("↑", "Up");
    }
    if (windowKeyStr.contains("→")) {
        windowKeyStr.replace("→", "Right");
    }
    if (windowKeyStr.contains("←")) {
        windowKeyStr.replace("←", "Left");
    }
    if (windowKeyStr.contains("↓")) {
        windowKeyStr.replace("↓", "Down");
    }
    for (KeyEntry& ckeyEntry : m_windowEntry) {
        QString ckeyString = ckeyEntry.valueStr;
        if (ckeyString.contains("Control", Qt::CaseInsensitive)) {
            ckeyString.replace("Control", "Ctrl", Qt::CaseInsensitive);
        }
        if (ckeyString.contains("Meta")) {
            ckeyString.replace("Meta", "Super");
        }
        if (ckeyString.contains("Start")) {
            ckeyString.replace("Start", "Super");
        }

        if (ckeyString.contains("PrtSc", Qt::CaseInsensitive)) {
            ckeyString.replace("PrtSc", "Print", Qt::CaseInsensitive);
        }
        qDebug() << Q_FUNC_INFO << "window conflict detect" << ckeyString << windowKeyStr;
        if (!windowKeyStr.compare(ckeyString, Qt::CaseInsensitive)) {
            m_conflictValue.clear();
            m_conflictKey = ckeyEntry.keyStr;
            qDebug() << "conflictWithWindowShortcuts" << seq << m_conflictKey << m_conflictValue << ckeyEntry.keyStr;
            return true;
        }
    }

    return false;
}

bool ShortcutLine::conflictWithCustomShortcuts(const QKeySequence &seq)
{
    QString customKeyStr = keyToLib(seq.toString());
    if (customKeyStr.contains("Meta")) {
        customKeyStr.replace("Meta", "Super");
    }
    for (const KeyEntry& ckeyEntry : customEntry) {
        QString ckeyString = ckeyEntry.bindingStr;
        if (ckeyString.contains("Control")) {
            ckeyString.replace("Control", "Ctrl");
        }
        if (ckeyString.contains("Meta")) {
            ckeyString.replace("Meta", "Super");
        }
        if (customKeyStr == ckeyString) {
            qDebug() << "conflictWithCustomShortcuts" << seq << ckeyEntry.keyStr;
            m_conflictValue.clear();
            m_conflictKey = ckeyEntry.actionStr;
            return true;
        }
    }
    return false;
}

QString ShortcutLine::keyToLib(QString key)
{
    if (key.contains("+")) {
        QStringList keys = key.split("+");
        if (keys.count() == 2) {
            QString lower = keys.at(1);
            QString keyToLib = "<" + keys.at(0) + ">" + lower.toLower();

            return keyToLib;
        } else if (keys.count() == 3) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + lower.toLower();

            return keyToLib;
        } else if (keys.count() == 4) {
            QString lower = keys.at(3);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + "<" + keys.at(2) + ">" + lower.toLower();

            return keyToLib;
        }
    }
    return key;
}


void ShortcutLine::setKeySequence(QKeySequence setSeq){
    this->seq = setSeq;
}

void ShortcutLine::initInterface()
{
    mKglobalIfc = new QDBusInterface("org.kde.kglobalaccel",
                                     "/kglobalaccel",
                                     "org.kde.KGlobalAccel",
                                     QDBusConnection::sessionBus(),
                                     this);
}

QString ShortcutLine::keyToString(int keyValue)
{
    QString keyValue_QT_KEY;//表示意义

    //键盘上大部分键值对应的都是其表示的ASCII码值
    keyValue_QT_KEY = QString(keyValue);
    if (keyValue == 0) keyValue_QT_KEY = "Fn";

    //对于特殊意义的键值[无法用ASCII码展示]
    switch (keyValue)
    {
    case Qt::Key_Escape:
        keyValue_QT_KEY = QString("Esc");
        break;

    case Qt::Key_Tab:
        keyValue_QT_KEY = QString("Tab");
        break;

    case Qt::Key_CapsLock:
        keyValue_QT_KEY = QString("CapsLock");
        break;
    case Qt::Key_Shift:
        keyValue_QT_KEY = QString(SSHIFT);
        break;
    case Qt::Key_Control:
        keyValue_QT_KEY = QString(SCTRL);
        break;
    case Qt::Key_Alt:
        keyValue_QT_KEY = QString(SALT);
        break;
    case Qt::Key_Backspace:
        keyValue_QT_KEY = QString("Backspace");
        break;
    case Qt::Key_Meta:
        keyValue_QT_KEY = QString("Super");
        break;
    case Qt::Key_Return:
        keyValue_QT_KEY = QString("Enter(main)");
        break;
    case Qt::Key_Enter:
        keyValue_QT_KEY = QString("Enter(num)");
        break;
    case Qt::Key_Home:
        keyValue_QT_KEY = QString("Home");
        break;
    case Qt::Key_End:
        keyValue_QT_KEY = QString("End");
        break;
    case Qt::Key_PageUp:
        keyValue_QT_KEY = QString("PageUp");
        break;
    case Qt::Key_PageDown:
        keyValue_QT_KEY = QString("PageDown");
        break;
    case Qt::Key_Insert:
        keyValue_QT_KEY = QString("Insert");
        break;
    case Qt::Key_Up:
        keyValue_QT_KEY = QString::fromLocal8Bit("↑");
        break;
    case Qt::Key_Right:
        keyValue_QT_KEY = QString::fromLocal8Bit("→");
        break;
    case Qt::Key_Left:
        keyValue_QT_KEY = QString::fromLocal8Bit("←");
        break;
    case Qt::Key_Down:
        keyValue_QT_KEY = QString::fromLocal8Bit("↓");
        break;
    case Qt::Key_Delete:
        keyValue_QT_KEY = QString("Del");
        break;
    case Qt::Key_Space:
        keyValue_QT_KEY = QString("Space");
        break;
    case Qt::Key_F1:
        keyValue_QT_KEY = QString("F1");
        break;
    case Qt::Key_F2:
        keyValue_QT_KEY = QString("F2");
        break;
    case Qt::Key_F3:
        keyValue_QT_KEY = QString("F3");
        break;
    case Qt::Key_F4:
        keyValue_QT_KEY = QString("F4");
        break;
    case Qt::Key_F5:
        keyValue_QT_KEY = QString("F5");
        break;
    case Qt::Key_F6:
        keyValue_QT_KEY = QString("F6");
        break;
    case Qt::Key_F7:
        keyValue_QT_KEY = QString("F7");
        break;
    case Qt::Key_F8:
        keyValue_QT_KEY = QString("F8");
        break;
    case Qt::Key_F9:
        keyValue_QT_KEY = QString("F9");
        break;
    case Qt::Key_F10:
        keyValue_QT_KEY = QString("F10");
        break;
    case Qt::Key_F11:
        keyValue_QT_KEY = QString("F11");
        break;
    case Qt::Key_F12:
        keyValue_QT_KEY = QString("F12");
        break;
    case Qt::Key_NumLock:
        keyValue_QT_KEY = QString("NumLock");
        break;
    case Qt::Key_ScrollLock:
        keyValue_QT_KEY = QString("ScrollLock");
        break;
    case Qt::Key_Pause:
        keyValue_QT_KEY = QString("Pause");
        break;
    case Qt::Key_Print:
        keyValue_QT_KEY = QString("PrtSc");
        break;
    case Qt::Key_Super_L:
        keyValue_QT_KEY = QString("Super");
        break;
    case Qt::Key::Key_WakeUp:
        keyValue_QT_KEY = QString("Fn");
        break;
    default:
        keyValue_QT_KEY = QKeySequence(keyValue).toString();
        break;
    }

    return keyValue_QT_KEY;
}
