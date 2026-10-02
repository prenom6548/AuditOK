/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "ktooltip.h"
#include "klabel.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>

Tooltip::Tooltip(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    tipLabel = new KLabel(this);
    QHBoxLayout *mLayout = new QHBoxLayout(this);
    mLayout->addWidget(tipLabel);
    mLayout->setContentsMargins(0, 0, 0, 0);
}

Tooltip::~Tooltip()
{
}

void Tooltip::setText(const QString &text, const int fontSize)
{
    tipLabel->setFontSize(fontSize);
    tipLabel->setText(text);
    tipLabel->setContentsMargins(2, 0, 2, 0);
}

void Tooltip::popup(Tooltip *kToolTip, const QString &text, const int fontSize)
{
    kToolTip->setText(text, fontSize);
    kToolTip->adjustSize();
    kToolTip->show();
}

void Tooltip::tipClose(Tooltip *kToolTip)
{
    kToolTip->hide();
    this->close();
}

void Tooltip::paintEvent(QPaintEvent * event){
    Q_UNUSED(event)

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

