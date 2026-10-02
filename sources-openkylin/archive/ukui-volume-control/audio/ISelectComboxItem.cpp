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

#include "ISelectComboxItem.h"
//#include <QHBoxLayout>
#include <QDebug>

ISelectComboxItem::ISelectComboxItem(std::shared_ptr<ISelectBox> box, QLabel* label)
    : m_pSelectBox(box), m_pDisplayLabel(label)
{

}

void ISelectComboxItem::setCurrentIndex(int idx)
{
    if (m_pSelectBox->isEnabled())
        m_pSelectBox->setCurrentIndex(idx);
}

int ISelectComboxItem::findData(const QVariant& value, int role)
{
    return m_pSelectBox->findData(value, role);
}

int ISelectComboxItem::findText(const QString& text)
{
    return m_pSelectBox->findText(text);
}

void ISelectComboxItem::insertItem(const QString& text, const QVariant& userData)
{
    m_pSelectBox->insertItem(text, userData);
}

void ISelectComboxItem::removeItem(int index)
{
    m_pSelectBox->removeItem(index);
}

QVariant ISelectComboxItem::currentData(int role) const
{
    return m_pSelectBox->currentData();
}

int ISelectComboxItem::count() const
{
    return m_pSelectBox->count();
}

void ISelectComboxItem::clear()
{
    m_pSelectBox->clear();
}

std::shared_ptr<ISelectBox> ISelectComboxItem::getSelectBox() const
{
    return m_pSelectBox;
}

std::list<QVariant> ISelectComboxItem::getDataList() const
{
    return m_pSelectBox->getDataList();
}
