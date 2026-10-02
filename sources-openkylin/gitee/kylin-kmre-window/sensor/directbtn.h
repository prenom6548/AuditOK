/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
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

#ifndef DIRECTBTN_H
#define DIRECTBTN_H
#include <QPushButton>
#include <QLabel>

class DirectBtn: public QPushButton
{
    Q_OBJECT
public:
    DirectBtn(QWidget *parent  =nullptr);
    QLabel *num =nullptr;
private:
    void connectServer();
};

#endif // DIRECTBTN_H
