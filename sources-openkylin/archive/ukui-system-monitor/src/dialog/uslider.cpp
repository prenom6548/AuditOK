/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "uslider.h"
#include <QDebug>

USlider::USlider(Qt::Orientation orientation, QWidget *parent):QSlider(orientation,parent)
{

}

USlider::~USlider()
{

}

void USlider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QMouseEvent a(e->type(),e->localPos(),Qt::MiddleButton,
                      QFlags<Qt::MouseButton>(Qt::MidButton|Qt::MiddleButton),e->modifiers());
        QSlider::mousePressEvent(&a); //左键事件转换为中键事件，实现滑动条点击移动
        return;
    } else {
        QSlider::mousePressEvent(e);
        return;
    }
}
