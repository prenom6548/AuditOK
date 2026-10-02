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
#include "klineedit.h"
#include <QBoxLayout>
#include <QDebug>
#include <QPushButton>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);

    KLineEdit *edit = new KLineEdit(this);

    QPushButton *btn = edit->pushbutton();
    btn->setIcon(QIcon::fromTheme("system-computer-symbolic"));
    connect(btn, &QPushButton::clicked, this, [=] {
        qDebug() << edit->text();
    });
    vlayout->addWidget(edit);

    QPushButton *btn1 = new QPushButton(this);
    vlayout->addWidget(btn1);
}

Widget::~Widget()
{
}
