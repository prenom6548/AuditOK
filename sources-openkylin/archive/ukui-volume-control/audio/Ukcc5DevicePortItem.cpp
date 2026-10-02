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

#include "Ukcc5DevicePortItem.h"

#include <QHBoxLayout>
#include <QDebug>

Ukcc5DevicePortItem::Ukcc5DevicePortItem(KLabel* label, QLabel* iconLabel, const QString& devName, const QString& portName, QWidget* parent) :
    m_pDisplayLabel(label), m_pIconLabel(iconLabel), IDevicePortItem(devName, portName), QWidget(parent)
{
    initUi();
}

void Ukcc5DevicePortItem::initUi()
{
    setFixedHeight(36);
    this->setContentsMargins(0, 0, 0, 0);

    m_pDisplayLabel->setFixedHeight(28);
    auto hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(16, 10, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_pIconLabel);
    hLayout->addItem(new QSpacerItem(16, 90, QSizePolicy::Fixed));
    hLayout->setContentsMargins(0, 4, 0, 4);
    this->setLayout(hLayout);
}

bool Ukcc5DevicePortItem::isChecked()
{
    return m_bIsChecked;
}

void Ukcc5DevicePortItem::setChecked(bool checked)
{
    m_bIsChecked = checked;

    if (checked) {
        setIcon(QIcon::fromTheme("ukui-selected"));
    }
    else {
        setIcon(QIcon::fromTheme(""));
    }
}

void Ukcc5DevicePortItem::setIcon(const QIcon& icon)
{
    m_pIconLabel->setPixmap(icon.pixmap(icon.actualSize(QSize(16, 16))));
}

QWidget* Ukcc5DevicePortItem::getWidget()
{
    return this;
}
