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
#ifndef PARTS_TITLE_H
#define PARTS_TITLE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPalette>
#include <QIcon>
#include "picturetowhite.h"

class parts_title : public QWidget
{
    Q_OBJECT

public:
   explicit parts_title(QWidget *parent = nullptr);
   void init();
   void create_interface();
   void create_interrupt();

private:
   QLabel *p_icon;
   QLabel *p_explain;
   QPushButton *p_minimize;
   QPushButton *p_close;
   QHBoxLayout *p_hlayout;
   PictureToWhite *p_picturetowhite;
   QGSettings  *p_gsettings;

   QIcon icon_minimize_reverse;
   QIcon icon_close_reverse;

signals:
   void sig_close_interface();
   void sig_minimize_interface();

public slots:
   void slot_close_interface();
   void slot_minimize_interface();
};

#endif // PARTS_TITLE_H
