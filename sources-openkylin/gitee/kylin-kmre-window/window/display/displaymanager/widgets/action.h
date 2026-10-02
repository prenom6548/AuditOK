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

#ifndef ACTION_H
#define ACTION_H

#include <QAction>
#include <QString>

class Action : public QAction
{
    Q_OBJECT
private:
    bool mIsChecked;

public:
    Action (const QString &text, QObject *parent = nullptr);
    Action (const QString &text, bool checked, QObject *parent = nullptr);

    // Action ( const QString & text, QKeySequence accel,
    //            QObject * parent, const char * name = "",
    //            bool autoadd = true );

    // Action ( QKeySequence accel, QObject * parent,
    //            const char * name = "", bool autoadd = true );

    ~Action();

    bool isChecked() {return mIsChecked;}
    void update(bool checked);
    void addShortcut(QKeySequence key);
};

#endif // ACTION_H
