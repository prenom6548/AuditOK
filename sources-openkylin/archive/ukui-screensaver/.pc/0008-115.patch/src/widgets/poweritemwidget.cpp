/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>
#include "poweritemwidget.h"
#include "statusbutton.h"

PowerItemWidget::PowerItemWidget(QWidget *parent) : QFrame(parent)
{
    itemFace = new StatusButton(this, POWERBUTTON);
    itemFace->setFixedSize(130, 130);
    textLabel = new KLabel(this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("color: white");
    textLabel->setMaximumWidth(130);
    QVBoxLayout *itemlayout = new QVBoxLayout(this);
    itemlayout->addWidget(itemFace);
    itemlayout->addWidget(textLabel);
    itemlayout->setAlignment(Qt::AlignHCenter);
}

PowerItemWidget::~PowerItemWidget() {}

bool PowerItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {

    } else if (event->type() == QEvent::Leave) {
    }
    return QWidget::eventFilter(obj, event);
}

void PowerItemWidget::setText(const QString text)
{
    textLabel->setText(text);
}

void PowerItemWidget::setToolTip(const QString tip)
{
    itemFace->setToolTip(tip);
}

void PowerItemWidget::setIcon(const QIcon &icon)
{
    itemFace->setIcon(icon);
}

void PowerItemWidget::setSelected(int status)
{
    itemFace->setClickedStatus(status);
}

void PowerItemWidget::setFontFamily(QString fontFamily)
{
    textLabel->setFontFamily(fontFamily);
}

void PowerItemWidget::setFontSize(double fontSize)
{
    textLabel->setFontSize(fontSize);
}
