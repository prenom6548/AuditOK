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

#include "widget.h"
#include <QDebug>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    IdleNotifier *notifier = new IdleNotifier(this);
    notifier->setIdleInterval(10);
    connect(notifier, &IdleNotifier::idled, [=]() {
        qDebug() << "idled";
    });
    connect(notifier, &IdleNotifier::resumed, [=]() {
        qDebug() << "resumed";
    });
}

Widget::~Widget()
{
}
