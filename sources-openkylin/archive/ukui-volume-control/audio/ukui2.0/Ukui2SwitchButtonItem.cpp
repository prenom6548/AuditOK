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

#include "Ukui2SwitchButtonItem.h"
#include <QHBoxLayout>

//void Ukui2SwitchButton::setChecked(bool checked)
//{
//    m_pSwitchBtn->setChecked(checked);
//}

//QWidget* Ukui2SwitchButton::getWidget()
//{
//    return m_pSwitchBtn;
//}

//Ukui2SwitchButtonItem::Ukui2SwitchButtonItem(std::shared_ptr<ISwitchButton> btn, QLabel* label, QWidget* parent)
//    : ISwitchButtonItem(btn, label), QFrame(parent)
//{

//}

//void Ukui2SwitchButtonItem::initUi()
//{
//    QHBoxLayout* hlayout = new QHBoxLayout;
//    hlayout->addWidget(m_pDisplayLabel);
//    hlayout->addWidget(m_pSwitchBtn->getWidget());
//    setLayout(hlayout);
//}

//QWidget* Ukui2SwitchButtonItem::getWidget()
//{
//    return this;
//}

Ukui2SwitchButton::Ukui2SwitchButton(SwitchButton *btn)
    : m_pSwitchBtn(btn)
{

}

bool Ukui2SwitchButton::isChecked() const
{
    return m_pSwitchBtn->isChecked();
}

void Ukui2SwitchButton::setChecked(bool checked)
{
    m_pSwitchBtn->setChecked(checked);
}

QWidget* Ukui2SwitchButton::getWidget()
{
    return m_pSwitchBtn;
}

Ukui2SwitchButtonItem::Ukui2SwitchButtonItem(std::shared_ptr<ISwitchButton> btn, QLabel* label, SwitchButtonType type, QWidget* parent)
    : ISwitchButtonItem(btn, label, type), QFrame(parent)
{

}

void Ukui2SwitchButtonItem::initUi()
{
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hlayout->addWidget(m_pSwitchBtn->getWidget());
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    setLayout(hlayout);
    setFrameShape(QFrame::Shape::Box);
}

void Ukui2SwitchButtonItem::initSlots()
{

}

QWidget* Ukui2SwitchButtonItem::getWidget()
{
    return this;
}

Ukui2SwitchButtonWithDescItem::Ukui2SwitchButtonWithDescItem(std::shared_ptr<ISwitchButton> btn, QLabel* disLabel, QLabel* descLabel, SwitchButtonType type, QWidget* parent)
    : ISwitchButtonItem(btn, disLabel, type), m_pDescLabel(descLabel), QFrame(parent)
{

}

void Ukui2SwitchButtonWithDescItem::initUi()
{
    QPalette palette = m_pDescLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText, color);
    m_pDescLabel->setPalette(palette);

    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(m_pDisplayLabel);
    vLayout->addWidget(m_pDescLabel);
    vLayout->addStretch();
    vLayout->setSpacing(0);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addLayout(vLayout);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hLayout->addWidget(m_pSwitchBtn->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    setLayout(hLayout);
    setFrameShape(QFrame::Shape::Box);
}

void Ukui2SwitchButtonWithDescItem::initSlots()
{

}

QWidget* Ukui2SwitchButtonWithDescItem::getWidget()
{
    return this;
}

