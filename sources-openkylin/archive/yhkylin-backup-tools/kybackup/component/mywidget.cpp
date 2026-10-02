/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "mywidget.h"
#include <QStyleOption>
#include <QPainter>

MyWidget::MyWidget(QWidget *parent) :
    QWidget(parent)
{}

MyWidget::~MyWidget()
{}

void MyWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QColor color = palette().color(QPalette::Base);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Window, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
