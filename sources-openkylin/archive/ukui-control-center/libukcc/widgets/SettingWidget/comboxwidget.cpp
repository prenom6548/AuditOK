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
#include "comboxwidget.h"

#include <QDebug>

ComboxWidget::ComboxWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_combox = new QComboBox(this);
    // 行标题由 FixLabel + buddy 提供；勿对 combo setAccessibleName(标题)，
    // 否则 AT Name 盖住 currentText，短日期等当前值读屏读不到
    m_settingName->setBuddy(m_combox);
    init();
}

ComboxWidget::~ComboxWidget()
{
}

void ComboxWidget::init()
{
    mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName, Qt::AlignLeft);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(m_combox);
    this->setLayout(mainLayout);
    connect(m_combox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComboxWidget::currentIndexChanged);
    connect(m_combox, &QComboBox::currentTextChanged, this, &ComboxWidget::currentTextChanged);
}

void ComboxWidget::setEnabled(const bool checked)
{
    if (m_combox) {
        m_combox->setEnabled(checked);
    }
}

bool ComboxWidget::isEnabled() const
{
    return m_combox->isEnabled();
}

void ComboxWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
        m_settingName->setBuddy(m_combox);
    }
    this->setAccessibleName(title);
}

QString ComboxWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void ComboxWidget::setCurrentText(const QString &text)
{
    m_combox->setCurrentText(text);
}

void ComboxWidget::setCurrentIndex(int index)
{
    m_combox->setCurrentIndex(index);
}

void ComboxWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

void ComboxWidget::setTitleMaximumWidth(int w)
{
    m_settingName->setMaximumWidth(w);
}

FixLabel *ComboxWidget::label() const
{
    return m_settingName;
}

QComboBox *ComboxWidget::comboBox() const
{
    return m_combox;
}


