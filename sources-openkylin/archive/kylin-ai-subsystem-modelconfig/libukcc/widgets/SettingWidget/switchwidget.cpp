/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "switchwidget.h"

#include <QDebug>

SwitchWidget::SwitchWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style, const QString hintText) :
    UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_hintText = new LightLabel(hintText, this);
    m_switchButton = new KSwitchButton(this);
    init();
}

SwitchWidget::~SwitchWidget()
{
}

void SwitchWidget::init()
{
    mainLayout = new QHBoxLayout();
    m_vLayout  = new QVBoxLayout();
    mainLayout->setContentsMargins(16, 0, 16, 0);

    m_vLayout->addWidget(m_settingName);
    if (!m_hintText->text().isEmpty()) {
        m_vLayout->addWidget(m_hintText);
    }
    mainLayout->addLayout(m_vLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_switchButton);
    setLayout(mainLayout);
    connect(m_switchButton, &KSwitchButton::stateChanged, this, &SwitchWidget::stateChanged);
}

void SwitchWidget::setHintTextVisible(bool visible)
{
    m_hintText->setVisible(visible);
}

void SwitchWidget::setEnabled(const bool checked)
{
    if (m_switchButton) {
        m_switchButton->setEnabled(checked);
    }
}

void SwitchWidget::setChecked(const bool checked)
{
    m_switchButton->setChecked(checked);
}

bool SwitchWidget::isChecked() const
{
    return m_switchButton->isChecked();
}

bool SwitchWidget::isEnabled() const
{
    return m_switchButton->isEnabled();
}

void SwitchWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_switchButton->setAccessibleName(title);
}

QString SwitchWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

bool SwitchWidget::blockSignals(bool b)
{
    return m_switchButton->blockSignals(b);
}

void SwitchWidget::insertWidget(int index, QWidget *widget, int stretch, Qt::Alignment alignment)
{
    mainLayout->insertWidget(index, widget, stretch, alignment);
}

KSwitchButton *SwitchWidget::switchButton() const
{
    return m_switchButton;
}

