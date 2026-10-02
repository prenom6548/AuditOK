/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "empty.h"
#include "QDebug"
Empty::Empty(QWidget *parent) : QWidget(parent)
{
    m_emptyLayout = new QHBoxLayout(this);

    m_emptyWid = new QWidget(this);
    m_emptyWid->setAccessibleName("kylin-weather_QWidget_emptyWid");

    addItems();
}

void Empty::resizeLayout(double detio)
{
    if (detio <= 1.0) {
        m_emptyWid->setFixedSize(360, 25);
    } else {
        m_emptyWid->setFixedSize(360, 25 * 1.2 * detio);
    }
}

void Empty::addItems()
{
    m_emptyLayout->addWidget(m_emptyWid);
    m_emptyWid->setFixedSize(360, 25);
}
