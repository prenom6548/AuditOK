/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef INPUT_H
#define INPUT_H

#include <QWidget>

namespace Ui {
class Input;
}

class Input : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void content(QString);
    void result(QString);
public:
    explicit Input(QWidget *parent = nullptr);
    ~Input();
public:
    QString result();
private:
    Ui::Input *ui;
};

#endif // INPUT_H
