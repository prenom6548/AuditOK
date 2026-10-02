/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "action.h"

#define GET_ACTION_ICON(checked) ((checked) ? QIcon(":/res/checkbox_checked.png") : QIcon(":/res/checkbox_unchecked.png"))


Action::Action(const QString &text, QObject *parent)
    : QAction(text, parent)
    , mIsChecked(false)
{
    
}

Action::Action(const QString &text, bool checked, QObject *parent)
    : QAction(GET_ACTION_ICON(checked), text, parent)
    , mIsChecked(checked)
{

}

// Action::Action(const QString & text, QKeySequence accel, QObject * parent, const char * name, bool autoadd )
//     : QAction(parent)
// {
//     setObjectName(name);
//     setText(text);
//     setShortcut(accel);
//     if (autoadd) addActionToParent();
// }

// Action::Action(QKeySequence accel, QObject * parent, const char * name,
//                    bool autoadd )
//     : QAction(parent)
// {
//     setObjectName(name);
//     setShortcut(accel);
//     if (autoadd) addActionToParent();
// }

Action::~Action()
{
}

void Action::update(bool checked)
{
    mIsChecked = checked;
    setIcon(GET_ACTION_ICON(checked));
}

void Action::addShortcut(QKeySequence key)
{
    setShortcuts( shortcuts() << key);
}

