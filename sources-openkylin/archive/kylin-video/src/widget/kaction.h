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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef KACTION_H
#define KACTION_H

#include <QAction>

class KAction : public QAction
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    //! Creates a new KAction with name \a name. If \a autoadd is true
    //! the action will be added to the parent
    KAction ( QObject * parent, const char * name, bool autoadd = true );

    //! Creates a new KAction. If \a autoadd is true
    //! the action will be added to the parent
    KAction ( QObject * parent, bool autoadd = true );

    KAction ( const QString & text, QKeySequence accel,
               QObject * parent, const char * name = "",
               bool autoadd = true );

    KAction ( QKeySequence accel, QObject * parent,
               const char * name = "", bool autoadd = true );

    ~KAction();

    void addShortcut(QKeySequence key);

    //! Change the icon and text of the action.
    void change(const QIcon & icon, const QString & text );

    //! Change the text of the action.
    void change(const QString & text);

protected:
    //! Checks if the parent is a QWidget and adds the action to it.
    void addActionToParent();
};

#endif // KACTION_H
