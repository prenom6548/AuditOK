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
#include "leftsidebarwidget.h"
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>
#include <QWindow>
#include <QDebug>

#define PERSONALSIE_SCHEMA     "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

LeftsidebarWidget::LeftsidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        personalQgsettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
        connect(personalQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听透明度变化
            if (changedKey == CONTAIN_PERSONALSIE_TRAN_KEY) {
               transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
               this->update();
            }
        });
    } else {
        personalQgsettings = nullptr;
        qDebug()<<PERSONALSIE_SCHEMA<<" not installed";
    }
}

LeftsidebarWidget::~LeftsidebarWidget()
{

}

void LeftsidebarWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QColor color = palette().color(QPalette::Base);
    // QColor color = palette().color(QPalette::Window);
    color.setAlpha(transparency);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window,QColor(color));
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool LeftsidebarWidget::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        if (personalQgsettings != nullptr && personalQgsettings->keys().contains(PERSONALSIE_TRAN_KEY)) {
            transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
        }
    } else if (event->type() == QEvent::WindowDeactivate) {
        if (QApplication::focusWindow()) {
            if (QApplication::focusWindow()->type() != Qt::Popup) {
                transparency = 255;
            }
        } else {
            transparency = 255;
        }
    }
    return QWidget::event(event);
}
