/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ukuistylehelper/ukuistylehelper.h"
#include "widget.h"
#include "windowmanager/windowmanager.h"
#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget window1(nullptr, Qt::Window);
    window1.setWindowTitle("Window 1");
    window1.setLayout(new QVBoxLayout);
    QPushButton p("Raise the Window 2");
    window1.layout()->addWidget(&p);
    window1.show();

    QWidget window2(nullptr, Qt::Window);
    window2.setWindowTitle("Window 2");
    window2.show();
    QObject::connect(&p, &QPushButton::clicked, &a, [&] {
        kdk::WindowManager::activateWindow(window1.windowHandle(), window2.windowHandle());
    });
    return a.exec();
}
