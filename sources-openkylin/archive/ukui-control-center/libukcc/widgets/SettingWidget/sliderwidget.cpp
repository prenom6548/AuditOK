/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "sliderwidget.h"

#include <QDebug>
#include <QIcon>

SliderWidget::SliderWidget(QString title, bool hasNode, QWidget *parent, UkccFrame::BorderRadiusStyle style, Qt::Orientation orientation):
    UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_slider = new KSlider(orientation, this);
    m_slider->setNodeVisible(hasNode);
    init();
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::init()
{
    mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_leftLabel = new KLabel(this);
    m_leftLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_rightLabel = new KLabel(this);
    m_rightLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_settingName->setFixedWidth(214);
    m_CopywritingVLayout = new QVBoxLayout();
    m_CopywritingVLayout->setSpacing(0);
    m_CopywritingVLayout->setContentsMargins(0, 0, 0, 0);
    m_CopywritingVLayout->addWidget(m_settingName);
    mainLayout->addLayout(m_CopywritingVLayout);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_leftLabel);
    mainLayout->addWidget(m_slider);
    mainLayout->addWidget(m_rightLabel);
    setFocusPolicy(Qt::NoFocus);
    m_settingName->setFocusPolicy(Qt::NoFocus);
    m_leftLabel->setFocusPolicy(Qt::NoFocus);
    m_rightLabel->setFocusPolicy(Qt::NoFocus);
    m_slider->setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(m_slider);
    this->setLayout(mainLayout);
    connect(m_slider, &KSlider::valueChanged, this, &SliderWidget::valueChanged);
}

void SliderWidget::addHintText(QString text)
{
    LightLabel *tipsLabel = new LightLabel(text);
    tipsLabel->setFixedWidth(214);
    m_CopywritingVLayout->addWidget(tipsLabel);
    mainLayout->setContentsMargins(16, 16, 16, 16);
}


void SliderWidget::setEnabled(const bool checked)
{
    UkccFrame::setEnabled(checked);
    if (m_slider) {
        m_slider->setEnabled(checked);
    }
}

bool SliderWidget::isEnabled() const
{
    return m_slider->isEnabled();
}

void SliderWidget::setValue(const int value)
{
    m_slider->setValue(value);
}

void SliderWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_slider->setAccessibleName(title);
}

void SliderWidget::setLeftText(const QString text)
{
    m_leftLabel->setText(text);
}

void SliderWidget::setRightText(const QString text)
{
    m_rightLabel->setText(text);
}

QString SliderWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void SliderWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

int SliderWidget::value() const
{
    return m_slider->value();
}

KSlider *SliderWidget::slider() const
{
    return m_slider;
}

KLabel *SliderWidget::leftLabel() const
{
    return m_leftLabel;
}

KLabel *SliderWidget::rightLabel() const
{
    return m_rightLabel;
}

void SliderWidget::insertWidget(int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
    mainLayout->insertWidget(index, widget, stretch, alignment);
}

void SliderWidget::setLeftPixmap(const QPixmap &pixmap)
{
    m_leftLabel->setPixmap(QIcon(pixmap), 24, 24);
}

void SliderWidget::setRightPixmap(const QPixmap &pixmap)
{
    m_rightLabel->setPixmap(QIcon(pixmap), 24, 24);
}


