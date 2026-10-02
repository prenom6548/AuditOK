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
#include <QPainter>
#include <gsettingmonitor.h>
#include <QPainterPath>
#include "blankpage.h"


BlankPage::BlankPage(QWidget *parent)
    : QWidget(parent)
{
    setWidgetUi();
    initLayout();
    initGsetting();
}

BlankPage::~BlankPage()
{
}

void BlankPage::setWidgetUi()
{
    m_blankLabel = new QLabel(this);

    m_blankTextLabel = new QLabel(this);

    m_blankTextLabel->setText(tr("No Font"));
    QString labGrayStyle = "color:#8C8C8C;";
    m_blankTextLabel->setStyleSheet(labGrayStyle);
    m_blankTextLabel->setMinimumWidth(180);
    m_blankTextLabel->setMinimumHeight(24);
    m_blankTextLabel->setWordWrap(true);
    m_blankTextLabel->setAlignment(Qt::AlignCenter);
    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();
}

void BlankPage::updateFontSize() {
    QFont font14;
    double g_fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    double g_font14pxToPt = (g_fontSize * (14.00 / 11)) - 3;
    font14.setPointSizeF(g_font14pxToPt);
    m_blankTextLabel->setFont(font14);
}

void BlankPage::initLayout()
{
    this->setMinimumHeight(196);
    this->setBackgroundRole(QPalette::Base);

    m_vLayout->addStretch();
    m_vLayout->addWidget(m_blankLabel, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_blankTextLabel, 0, Qt::AlignHCenter);
    m_vLayout->addStretch();
    m_vLayout->setSpacing(0);
    m_vLayout->setContentsMargins(0, 12, 0, 12);

    m_hLayout->addStretch();
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addStretch();
    m_hLayout->setSpacing(0);
    m_hLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_hLayout);
}

void BlankPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    updateFontSize();
    QPainter p(this);
    QPainterPath rectPath;
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    return;
}

void BlankPage::initGsetting()
{
}

void BlankPage::changeSearchText()
{
    m_blankTextLabel->setText(tr("No Search Results"));
}

void BlankPage::changeBlankText()
{
    m_blankTextLabel->setText(tr("No Font"));
}
