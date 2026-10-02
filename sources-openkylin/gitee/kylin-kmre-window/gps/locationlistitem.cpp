/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "locationlistitem.h"
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
namespace kmre_gps
{
LocationListItem::LocationListItem(QPushButton *parent) : QPushButton(parent)
{
    initWidget();
}

LocationListItem::~LocationListItem() {}

void LocationListItem::initWidget()
{
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    QVBoxLayout *labelVLayout = new QVBoxLayout();
    m_itemNameLabel = new QLabel(this);
    QFont font;
    font.setBold(false);
    font.setPointSize(8);
    m_itemNameLabel->setFont(font);
    labelVLayout->addStretch();
    labelVLayout->addWidget(m_itemNameLabel);
    labelVLayout->addStretch();
    mainHLayout->addSpacing(3);
    mainHLayout->addSpacing(2);
    mainHLayout->addLayout(labelVLayout);
    mainHLayout->addStretch();
    mainHLayout->setSpacing(0);
    mainHLayout->setMargin(0);
    setLayout(mainHLayout);
}

//设置项名称
void LocationListItem::setItemName(QString name)
{
    m_itemNameLabel->setText(name);
}

} // namespace kmre_gps
