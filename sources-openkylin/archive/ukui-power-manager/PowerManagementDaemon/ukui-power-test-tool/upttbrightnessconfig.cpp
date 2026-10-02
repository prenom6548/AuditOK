/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */
#include "upttbrightnessconfig.h"

UpttBrightnessConfig::UpttBrightnessConfig(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    m_sliderLabel = new QLabel("屏幕亮度", this);
    m_sliderLabel->setFixedWidth(100);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(0);
    m_slider->setMaximum(100);
    m_slider->setFixedWidth(300);
    m_slider->setValue(80);

    m_valueLable = new QLabel(this);
    m_valueLable->setText(QString::number(m_slider->value()));
    m_valueLable->setContentsMargins(30, 0, 0, 0);

    mainLayout->addWidget(m_sliderLabel);
    mainLayout->addWidget(m_slider);
    mainLayout->addWidget(m_valueLable);

    connect(m_slider, &QSlider::valueChanged, [&](int value) {
        m_valueLable->setText(QString::number(value));
    });
}

UpttBrightnessConfig::~UpttBrightnessConfig() {}

int UpttBrightnessConfig::getBrightnessPercentage()
{
    return m_slider->value();
}
