/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "nodevicewidget.h"
#include <QPainter>

NoDeviceWidget::NoDeviceWidget(QWidget *parent)
    : QWidget(parent),
      m_PageIcon(new QLabel()),
      m_PageText(new QLabel()),
      m_pageVLayout(new QVBoxLayout())
{
    init();
}

void NoDeviceWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    pen.setColor(QColor("#8C8C8C"));
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void NoDeviceWidget::init()
{
    m_PageIcon->setPixmap(QPixmap(":/default-connect-page/detect_fail_light.svg"));
    m_PageText->setText(tr("Scanner not detected"));
    m_PageText->setEnabled(false);


    m_pageVLayout->setSpacing(0);
    m_pageVLayout->addStretch();
    m_pageVLayout->addWidget(m_PageIcon, 0,  Qt::AlignCenter);
    m_pageVLayout->addSpacing(8);
    m_pageVLayout->addWidget(m_PageText, 0,  Qt::AlignCenter);
    m_pageVLayout->addStretch();
    m_pageVLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_pageVLayout);
}
