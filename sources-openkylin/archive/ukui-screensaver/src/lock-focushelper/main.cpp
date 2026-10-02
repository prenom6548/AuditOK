/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QString>
#include <QCursor>
#include <QPalette>
#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>
#include "blackwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BlackWindow w;
    w.setGeometry(QApplication::desktop()->geometry());
    //w.activateWindow();
    w.show();
    a.exec();
    return 0;
}
