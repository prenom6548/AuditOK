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

#include "Ukui2VolumeSliderItem.h"

#include <QHBoxLayout>
#include <QDebug>

Ukui2VolumeSlider::Ukui2VolumeSlider(QSlider *s)
    : m_pSlider(s)
{
    m_pSlider->setOrientation(Qt::Horizontal);
}

void Ukui2VolumeSlider::setRange(int min, int max)
{
    m_pSlider->setRange(min, max);
}

void Ukui2VolumeSlider::setValue(int value)
{
    if (m_pSlider)
        m_pSlider->setValue(value);
}

void Ukui2VolumeSlider::setEnabled(bool enable)
{
    // TODO
}

bool Ukui2VolumeSlider::isEnabled() const
{
    return m_pSlider->isEnabled();
}

uint32_t Ukui2VolumeSlider::getValue() const
{
    return m_pSlider->value();
}

QWidget* Ukui2VolumeSlider::getWidget()
{
    return m_pSlider;
}

Ukui2VolumeSliderItem::Ukui2VolumeSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel* disLabel, QLabel* percLabel, QPushButton* muteBtn, QWidget* parent)
    : IVolumeSliderItem(slider, disLabel), m_pPercentLabel(percLabel), m_pMuteBtn(muteBtn), QFrame(parent)
{
    m_pMuteBtn->setFocusPolicy(Qt::NoFocus);
}

void Ukui2VolumeSliderItem::initUi()
{
    QHBoxLayout* hlayout = new QHBoxLayout(this);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pDisplayLabel);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hlayout->addWidget(m_pMuteBtn);
    hlayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hlayout->addWidget(m_pVolumeSlider->getWidget());
    hlayout->addItem(new QSpacerItem(13, 20, QSizePolicy::Maximum));
    hlayout->addWidget(m_pPercentLabel);
    hlayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Maximum));
    hlayout->setSpacing(0);

    setLayout(hlayout);
    setFrameShape(QFrame::Shape::Box);
}

void Ukui2VolumeSliderItem::initSlots()
{

}

QWidget* Ukui2VolumeSliderItem::getWidget()
{
    return this;
}

void Ukui2VolumeSliderItem::setValue(int value)
{
    m_pVolumeSlider->setValue(value);
    setPercent(value);
    setIcon(QIcon::fromTheme("firefox"));
}

void Ukui2VolumeSliderItem::setMute(bool)
{
    // TODO
}

void Ukui2VolumeSliderItem::setEnabled(bool enable)
{
    // TODO
}

QPushButton* Ukui2VolumeSliderItem::getMuteBtn() const
{
    return m_pMuteBtn;
}

void Ukui2VolumeSliderItem::setIcon(const QIcon& icon)
{
    m_pMuteBtn->setIcon(icon);
}

void Ukui2VolumeSliderItem::setPercent(int percent)
{
    m_pPercentLabel->setText(QString("").arg(percent) + "%");
}
