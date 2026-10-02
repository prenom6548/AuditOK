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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "widget.h"
#include "ktextedit.h"
#include "ktitlewindow.h"
#include <QBoxLayout>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KTitleWindow *window = new KTitleWindow();
    QPushButton *btn1 = new QPushButton();
    btn1->setText("测试button");
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(btn1);
    KTextEdit *edit = new KTextEdit();
    edit->setLabelVisible(false);
    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(layout);
    vlayout->addWidget(edit);
    window->baseWidget()->setLayout(vlayout);
    window->show();
}

Widget::~Widget()
{
}
