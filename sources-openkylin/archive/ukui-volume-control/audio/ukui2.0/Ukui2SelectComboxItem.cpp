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

#include "Ukui2SelectComboxItem.h"

#include <QHBoxLayout>

Ukui2SelectCombox::Ukui2SelectCombox(QComboBox* box)
    : m_pCombox(box)
{

}

void Ukui2SelectCombox::setCurrentIndex(int idx)
{
    m_pCombox->setCurrentIndex(idx);
}

void Ukui2SelectCombox::setEnabled(bool enable)
{
    m_pCombox->setEnabled(enable);
}

bool Ukui2SelectCombox::isEnabled() const
{
    return m_pCombox->isEnabled();
}

int Ukui2SelectCombox::findData(const QVariant& value, int role)
{
    return m_pCombox->findData(value, role);
}

int Ukui2SelectCombox::findText(const QString& text)
{
    return m_pCombox->findText(text);
}

void Ukui2SelectCombox::insertItem(const QString& text, const QVariant& userData)
{
    m_pCombox->blockSignals(true);
    m_pCombox->insertItem(m_pCombox->count(), text, userData);
    m_pCombox->blockSignals(false);
}

void Ukui2SelectCombox::removeItem(int index)
{
    m_pCombox->blockSignals(true);
    m_pCombox->removeItem(index);
    m_pCombox->blockSignals(false);
}

QVariant Ukui2SelectCombox::currentData(int role) const
{
    return m_pCombox->currentData();
}

std::list<QVariant> Ukui2SelectCombox::getDataList() const
{
    std::list<QVariant> list;

    for (int i = 0; i < m_pCombox->count(); ++i) {
        auto data = m_pCombox->itemData(i).toStringList();
        if (data.size() < 2) {
            continue;
        }

        QList<QVariant> v;
        v.push_back(i);
        v.push_back(data.at(0));
        v.push_back(data.at(1));
        list.push_back(QVariant(v));
    }

    return list;
}

int Ukui2SelectCombox::count() const
{
    return m_pCombox->count();
}

void Ukui2SelectCombox::clear()
{
    m_pCombox->clear();
}

QWidget* Ukui2SelectCombox::getWidget()
{
    return m_pCombox;
}

void Ukui2SelectCombox::setParent(QWidget* parent)
{
    m_pCombox->setParent(parent);
}

Ukui2SelectComboxItem::Ukui2SelectComboxItem(std::shared_ptr<ISelectBox> box, QLabel* label, QWidget* parent)
    : ISelectComboxItem(box, label), QFrame(parent)
{

}

void Ukui2SelectComboxItem::initUi()
{
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hLayout->addWidget(m_pSelectBox->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->setSpacing(0);
    setLayout(hLayout);
    setFrameShape(QFrame::Shape::Box);
}

void Ukui2SelectComboxItem::initSlots()
{
    connect(m_pSelectBox->getWidget(), SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChangedSlots(int)));
}

void Ukui2SelectComboxItem::setEnabled(bool enable)
{
    m_pSelectBox->setEnabled(enable);
}

QWidget* Ukui2SelectComboxItem::getWidget()
{
    return this;
}
