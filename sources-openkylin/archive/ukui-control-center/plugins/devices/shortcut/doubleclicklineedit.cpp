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
#include "doubleclicklineedit.h"
#include "clickfixlabel.h"
#include <QDebug>
#include <QApplication>
#include <QGSettings>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>

DoubleClickLineEdit::DoubleClickLineEdit(QList<KeyEntry> customEntries,QWidget *parent):
    QLineEdit(parent),
    customEntryList(customEntries){
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");
    this->setContextMenuPolicy(Qt::NoContextMenu);
    defaultQss = this->styleSheet();


    connect(this, &DoubleClickLineEdit::textChanged, this, [=](){
        changeName();
        if (false == strIsAvailable) {
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        } else {
           this->setStyleSheet(defaultQss);
        }
    });

    setFont(qApp->font());
    QGSettings *fontSetting = new QGSettings(QByteArray("org.ukui.style"), QByteArray(), this);
    connect(fontSetting, &QGSettings::changed, this, [=](const QString &key) {
        updateShortcutFont(this, key);
    });
}

DoubleClickLineEdit::~DoubleClickLineEdit() {

}

void DoubleClickLineEdit::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    return;
}

void DoubleClickLineEdit::keyPressEvent(QKeyEvent *e)
{
    auto leaveEdit = [this](bool next, bool onlyFix) {
        // 常驻编辑框：在焦点链上移动，不切回灰框
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
    if (e->key() == Qt::Key_Escape) {
        blockSignals(true);
        setText(validStr);
        blockSignals(false);
        leaveEdit(true, true);
        return;
    }
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        leaveEdit(true, true);
        return;
    }
    if (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
        leaveEdit(e->key() != Qt::Key_Backtab, false);
        return;
    }
    QLineEdit::keyPressEvent(e);
}

void DoubleClickLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    // 获焦后再写 accessibleName，避免 Qt 用「用户手册」等当前文本盖掉「自定义快捷键」
    const QString labelName = property("ukccShortcutName").toString().trimmed();
    if (!labelName.isEmpty()) {
        setAccessibleName(labelName);
    }
}

void DoubleClickLineEdit::focusOutEvent(QFocusEvent *e) {
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");

    QString str;
    if (strIsAvailable) {
        str = this->text();
    } else {
        str = validStr;
    }
    this->blockSignals(true);
    this->setText(str);
    this->blockSignals(false);
    if (strIsAvailable) {
        Q_EMIT strChanged();
    }
    this->setStyleSheet(defaultQss);
    // 必须调用基类：停止光标闪烁；常驻编辑框失焦后仍可见，否则会出现多行同时闪烁光标
    QLineEdit::focusOutEvent(e);
    Q_EMIT focusOut();
}

void DoubleClickLineEdit::setText(const QString &str) {
    validStr = str;
    QLineEdit::setText(str);
}

void DoubleClickLineEdit::changeName() {
    QStringList customName;
    QString text = this->text();
    if (text.isEmpty()) {
         strIsAvailable = false;
         return;
    }
    if (customEntryList.isEmpty()) {
        strIsAvailable = true;
    } else {
        for (KeyEntry ckeyEntry : customEntryList) {
            customName << ckeyEntry.nameStr;
            if (customName.contains(text) && text != validStr) {
                strIsAvailable = false;
            } else {
                strIsAvailable = true;
            }
        }
    }
}

/*-----------------------------------------DoubleClickShortCut-----------------------------------------------*/

DoubleClickShortCut::DoubleClickShortCut(QList<KeyEntry> generalEntries,
                                         QList<KeyEntry> windowEntries,
                                         QList<KeyEntry> customEntries,
                                         QWidget *parent) :
    ShortcutLine(generalEntries, windowEntries, customEntries, parent){
    this->customEntryList = customEntries;
    this->setContextMenuPolicy(Qt::NoContextMenu);
    defaultQss = this->styleSheet();

    connect(this, &DoubleClickShortCut::textChanged, this, [=](){
        if (this->text().isEmpty()) {
            shortcutIsAvailable = false;
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        }
    });

    connect(this, &DoubleClickShortCut::shortCutAvailable, this, [=](int flag) {

        shortcutSymbol = flag;
        if (0 == flag) {
            shortcutIsAvailable = true;
            this->setStyleSheet(defaultQss);
        } else if (-1 == flag) {
            Q_EMIT shortcutOccupy();
            shortcutIsAvailable = false;
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        } else if (-2 == flag) {
            Q_EMIT shortcutConflict();
            shortcutIsAvailable = false;
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        }
    });
}

DoubleClickShortCut::~DoubleClickShortCut() {

}

void DoubleClickShortCut::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    return;
}

void DoubleClickShortCut::focusInEvent(QFocusEvent *e) {
    setShortcutIsAvailable(false);
    ShortcutLine::focusInEvent(e);
    // 获焦后再写一次名称，避免 Qt 用当前文本覆盖 accessibleName
    const QString shortcutName = property("ukccShortcutName").toString().trimmed();
    if (!shortcutName.isEmpty()) {
        setAccessibleName(shortcutName);
    }
    if (QLabel *nameLab = qobject_cast<QLabel *>(
                property("ukccNameLabel").value<QWidget *>())) {
        nameLab->setBuddy(this);
    }
}
void DoubleClickShortCut::focusOutEvent(QFocusEvent *e) {
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");

    QString str;
    if (shortcutIsAvailable) {
        str = this->text();
    } else {
        str = validShortcut;
    }
    this->blockSignals(true);
    this->setText(str);
    this->blockSignals(false);
    if (shortcutIsAvailable) {
        Q_EMIT shortcutChanged();
    }
    this->setStyleSheet(defaultQss);
    ShortcutLine::focusOutEvent(e);
    Q_EMIT focusOut();
}

void DoubleClickShortCut::setText(const QString &str) {
    validShortcut = str;
    QLineEdit::setText(str);
}

void DoubleClickShortCut::setShortcutIsAvailable(bool value)
{
    this->shortcutIsAvailable = value;
}
