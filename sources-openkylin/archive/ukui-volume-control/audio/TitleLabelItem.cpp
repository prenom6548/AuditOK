/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "TitleLabelItem.h"

#include <QHBoxLayout>

TitleLabelItem::TitleLabelItem(QLabel* label, QWidget* parent)
    : m_pLabel(label), QWidget(parent)
{
    initUi();
}

void TitleLabelItem::initUi()
{
    m_pLabel->setFixedHeight(28);
    auto hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pLabel);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
}

QWidget* TitleLabelItem::getWidget()
{
    return this;
}
