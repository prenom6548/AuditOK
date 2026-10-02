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
#ifndef DOUBLECLICKLINEEDIT_H
#define DOUBLECLICKLINEEDIT_H
#include <QLineEdit>
#include "shortcutline.h"
#include <QStyleOption>
#include <QPainter>

class DoubleClickLineEdit : public QLineEdit {
    Q_OBJECT
public:
    DoubleClickLineEdit(QList<KeyEntry> customEntries, QWidget *parent = nullptr);
    ~DoubleClickLineEdit();
    void setText(const QString &str);
    void changeName();
protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;

private:
    QString validStr;
    bool strIsAvailable;
    QList<KeyEntry> customEntryList;
    QString defaultQss;

Q_SIGNALS:
    void focusOut();
    void strChanged();
};

class DoubleClickShortCut : public ShortcutLine {
    Q_OBJECT
public:
    DoubleClickShortCut(QList<KeyEntry> generalEntries,
                        QList<KeyEntry> windowEntries,
                        QList<KeyEntry> customEntries,
                        QWidget *parent = nullptr);
    ~DoubleClickShortCut();
    void setText(const QString &str);
    void setShortcutIsAvailable(bool value);
protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void focusInEvent(QFocusEvent *e);

private:
    QString validShortcut;
    bool shortcutIsAvailable = false;
    int shortcutSymbol;
    QList<KeyEntry> customEntryList;
    QString defaultQss;

Q_SIGNALS:
    void focusOut();
    void shortcutChanged();
    void shortcutConflict();
    void shortcutOccupy();
};


#endif // DOUBLECLICKLINEEDIT_H
