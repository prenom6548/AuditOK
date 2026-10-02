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

#include "mymenu.h"
#include <QDebug>

MyMenu::MyMenu(QWidget *parent)
{

}

void MyMenu::keyPressEvent(QKeyEvent *event)
{
    // 快捷键Shift+F10 隐藏上下文菜单
    if ((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_F10))
    {
        if (isActiveWindow()) {
//            qDebug() << "MyMenu::keyPressEvent(QKeyEvent *event) "  << "if isActiveWindow() = " << isActiveWindow();
            this->hide();
        } else {
//            qDebug() << "MyMenu::keyPressEvent(QKeyEvent *event) "  << "else m_oMenu->isActiveWindow() = " << isActiveWindow();
        }

    }

}
