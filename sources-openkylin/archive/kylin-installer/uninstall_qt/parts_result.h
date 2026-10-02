/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef PARTS_RESULT_H
#define PARTS_RESULT_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class parts_result : public QWidget
{
   Q_OBJECT

public:
   explicit parts_result(int , QWidget *parent = nullptr);
   void init();
   void create_interface(int);
   QLabel *p_result_txt;

private:
   QLabel *p_icon;
   QHBoxLayout *p_hlayout_1;

signals:

public slots:
};

#endif // PARTS_RESULT_H
