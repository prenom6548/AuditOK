/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "kaction.h"

KAction::KAction ( QObject * parent, const char * name, bool autoadd )
    : QAction(parent)
{
    //qDebug("KAction::KAction: name: '%s'", name);
    setObjectName(name);
    if (autoadd) addActionToParent();
}


KAction::KAction( QObject * parent, bool autoadd )
    : QAction(parent)
{
    //qDebug("KAction::KAction: QObject, bool");
    if (autoadd) addActionToParent();
}

KAction::KAction(const QString & text, QKeySequence accel,
                   QObject * parent, const char * name, bool autoadd )
    : QAction(parent)
{
    setObjectName(name);
    setText(text);
    setShortcut(accel);
    if (autoadd) addActionToParent();
}

KAction::KAction(QKeySequence accel, QObject * parent, const char * name,
                   bool autoadd )
    : QAction(parent)
{
    setObjectName(name);
    setShortcut(accel);
    if (autoadd) addActionToParent();
}

KAction::~KAction() {
}

void KAction::addShortcut(QKeySequence key) {
    setShortcuts( shortcuts() << key);
}

void KAction::addActionToParent() {
    if (parent()) {
        if (parent()->inherits("QWidget")) {
            QWidget *w = static_cast<QWidget*> (parent());
            w->addAction(this);
        }
    }
}

void KAction::change(const QIcon & icon, const QString & text) {
    setIcon( icon );
    change(text);
}

void KAction::change(const QString & text ) {
    setText( text );

    QString accel_text = shortcut().toString();

    QString s = text;
    s.replace("&","");
    if (!accel_text.isEmpty()) {
        setToolTip(s + " ("+ accel_text +")");
        setIconText(s);
    }
}

