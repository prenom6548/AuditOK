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
#include "lineeditwidget.h"

#include <QDebug>

LineEditWidget::LineEditWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_lineEdit = new QLineEdit(this);
    init();
    m_settingName->setAccessibleName(title);
    this->setAccessibleName(title);
}

LineEditWidget::~LineEditWidget()
{
}

void LineEditWidget::init()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(m_lineEdit);
    this->setLayout(mainLayout);
}

void LineEditWidget::setEnabled(const bool checked)
{
    if (m_lineEdit) {
        m_lineEdit->setEnabled(checked);
    }
}

bool LineEditWidget::isEnabled() const
{
    return m_lineEdit->isEnabled();
}

void LineEditWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
    m_lineEdit->setAccessibleName(title);
}

QString LineEditWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void LineEditWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

QLineEdit *LineEditWidget::lineEdit()
{
    return m_lineEdit;
}
