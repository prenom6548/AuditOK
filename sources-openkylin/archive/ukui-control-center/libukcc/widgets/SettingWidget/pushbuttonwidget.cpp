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
#include "pushbuttonwidget.h"

#include <QDebug>

PushButtonWidget::PushButtonWidget(const QString title, QWidget *parent, UkccFrame::BorderRadiusStyle style)
   : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_pushButton = new KPushButton(this);
    m_iconLabel = new QLabel(this);
    // setBuddy 建立 LABELLED_BY，读屏聚焦「去设置」等按钮时能读到左侧行标题
    m_settingName->setAccessibleName(title);
    m_settingName->setBuddy(m_pushButton);
    m_pushButton->setAccessibleName(title);
    init();
}

PushButtonWidget::~PushButtonWidget()
{

}

void PushButtonWidget::init()
{
    m_iconLabel->setVisible(false);
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_pushButton->setMinimumWidth(100);
    m_pushButton->setMaximumWidth(200);
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_settingName);
    mainLayout->addStretch();
    mainLayout->addWidget(m_pushButton);
    this->setLayout(mainLayout);

    connect(m_pushButton, &QPushButton::clicked, this, &PushButtonWidget::clicked);
}

void PushButtonWidget::setEnabled(const bool checked)
{
    if (m_pushButton) {
        m_pushButton->setEnabled(checked);
    }
}

bool PushButtonWidget::isEnabled() const
{
    return m_pushButton->isEnabled();
}

void PushButtonWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
        m_settingName->setAccessibleName(title);
        m_settingName->setBuddy(m_pushButton);
    }
    this->setAccessibleName(title);
    m_pushButton->setAccessibleName(title);
}

QString PushButtonWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void PushButtonWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

void PushButtonWidget::setButtonText(const QString &text)
{
    m_pushButton->setText(text);
    m_pushButton->setAccessibleName(text);
}

void PushButtonWidget::setButtonFixedWidth(int w)
{
     m_pushButton->setFixedWidth(w);
}

KPushButton *PushButtonWidget::pushButton()
{
    return m_pushButton;
}

void PushButtonWidget::setIconLabel(QIcon icon)
{
    m_iconLabel->setPixmap(icon.pixmap(16, 16));
    m_iconLabel->setVisible(true);
}
