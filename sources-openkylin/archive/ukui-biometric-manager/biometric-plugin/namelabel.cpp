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
#include "namelabel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QIcon>
#include <QMouseEvent>

NameLabel::NameLabel(QWidget *parent) : QWidget(parent)
{
    setMaximumWidth(500);
    QHBoxLayout *nameLayout = new QHBoxLayout(this);

    nameLabel = new KALabel(this);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setAlignment(Qt::AlignLeft);

    changeNameLabel = new QLabel(this);
    changeNameLabel->installEventFilter(this);
    changeNameLabel->setFixedSize(QSize(16, 16));
    changeNameLabel->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(changeNameLabel->size()));
    changeNameLabel->setProperty("useIconHighlightEffect", 0x2);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(changeNameLabel);
}

void NameLabel::mouseReleaseEvent(QMouseEvent * ev){
   emit clicked();
}

void NameLabel::setText(QString text)
{
    nameLabel->setText(text);
}
